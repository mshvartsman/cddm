#include "axcpt.h"
#include <armadillo>
#include <iostream>
#include <limits>

double DBL_TOL = 10*std::numeric_limits<double>::min();

AxcptTask::~AxcptTask(){
    delete _belief; 
}

/**
 * @brief Constructor for the AX-CPT task. 
 * @details Grabs the configuration, does some error checking, and initializes things. 
 * 
 * @param c A Config, containing \ref timePerStep, \ref maxTrials, \ref maxSamps, 
 * \ref contextNoise, \ref targetNoise, \ref decisionThresh, \ref pPrematureResp, 
 * \ref retentionIntervalDur. Also containing either \ref totalNoise and \ref proportionContextNoise, 
 * or \ref contextNoise and \ref targetNoise. Also optionally contains \ref retentionNoise 
 * (which is otherwise set to \ref contextNoise or \ref totalNoise * \ref proportionContextNoise, 
 * depending on what is available). 
 * @param r a Recorder. 
 */
AxcptTask::AxcptTask(const Config * c, Recorder * r):Task(c,r),_arch(Architecture(c)), _trialTime(-1), _nPrecomputeSamps(-1), _decayTo(Informative) {
    _belief = new DecayBelief(c); 
    _traceDatumNames = {"post"}; 
    _summaryDatumNames = {"RT", "Resp", "Acc","CorrectRT","IncorrectRT"};
    _eventDatumNames = {"eblEvent", "motorPlanEvent", "motorExecEvent", "samplingBothEvent", "samplingContextEvent"}; 
    _timePerStep = _config->get<double>("timePerStep"); 
    _retentionIntervalDur = _config->get<double>("retentionIntervalDur");
    _maxTrials = _config->get<int>("maxTrials"); 
    _maxSamps = _config->get<int>("maxSamps"); 
    _pPrematureResponse = _config->get<double>("pPrematureResp");
    // can set noises either directly or via total + proportion
    if (_config->keyExists("totalNoise") && _config->keyExists("proportionContextNoise")){
        #ifndef DISABLE_ERROR_CHECKS
            if(_config->keyExists("contextNoise") || c->keyExists("targetNoise")) throw fatal_error() << "ERROR: have totalNoise and proportionContextNoise but also direct assignments of context or target noise!";
        #endif 
        double totalNoiseVar  = _config->get<double>("totalNoise");
        totalNoiseVar = totalNoiseVar * totalNoiseVar; 
        double proportionContextNoise = _config->get<double>("proportionContextNoise"); 
        _contextNoise = sqrt(totalNoiseVar * proportionContextNoise); 
        _targetNoise = sqrt(totalNoiseVar * (1-proportionContextNoise)); 
    } else if(_config->keyExists("contextNoise") && c->keyExists("targetNoise")){
        #ifndef DISABLE_ERROR_CHECKS
            if (_config->keyExists("totalNoise") || _config->keyExists("proportionContextNoise")) throw fatal_error() << "ERROR: have context or target noise but also assignments of totalNoise and proportionContextNoise!";
        #endif 

        _contextNoise = _config->get<double>("contextNoise"); 
        _targetNoise = _config->get<double>("targetNoise"); 
    } 
    #ifndef DISABLE_ERROR_CHECKS
        else throw fatal_error() << "ERROR: Unknown noise configuration! Need to pass in either contextNoise and targetNoise, or totalNoise and proportionContextNoise!";
    #endif
    if (_config->keyExists("retentionNoise")){
        _retentionNoise = _config->get<double>("retentionNoise"); 
    } else {
        _retentionNoise = _contextNoise; 
    }
    _decisionThresh = _config->get<double>("decisionThresh"); 
    #ifndef DISABLE_ERROR_CHECKS
    if (_nPrecomputeSamps != int(_nPrecomputeSamps)) throw fatal_error() << "ERROR: Retention interval duration does not divide evenly into timePerStep! This is undefined behavior -- what's happening during that extra partial sample?";
    if (_contextNoise<0) throw fatal_error() << "ERROR: contextNoise < 0, did you set it?";
    if (_targetNoise<0) throw fatal_error() << "ERROR: targetNoise < 0, did you set it?";
    if (_decisionThresh < 0) throw fatal_error() << "ERROR: decisionThresh < 0, did you set it (MinimalArchAxcptTask is implemented in prob space, not log space)?";
    if (_decisionThresh >= 1) throw fatal_error() << "ERROR: decisionThresh >=1 (MinimalArchAxcptTask is implemented in prob space, not log space) ";
    #endif
    _belief = new DecayBelief(c);  // create a new one, will be cleared by the minimal arch destructor
    _retentionIntervalDur = _config->get<double>("retentionIntervalDur");
    if (!c->keyExists("decayTo")){
        _decayTo = Informative; 
    } else {
        _decayTo = _config->get<int>("decayTo") == 0 ? Informative : Uniform; // this is going to bite us? 
    }
}

/**
 * @brief Record the current posterior into Recorder. 
 */
void AxcptTask::_recordBelief(){
    mat post = _belief->getBelief(); 
    _recorder->updateDatum(_trialLabel + "post", Timepoint(_trialTime,vectorise(post)));
}

/**
 * @brief Run one trial of the event loop for AX-CPT. 
 * @details At t0, there is some small probability of instantly responding. 
 * If this happens, motor planning commences as soon as the target comes on. 
 * This implementation does not model the perceptual sampling from context, 
 * so the event loop starts when the context disappears and retention interval starts. 
 * Then, context is sampled from memory until the target appears. Then both context
 * and target are sampled, the former from memory. If the decision threshold has been
 * crossed when the target appears, motor planning starts immediately. Otherwise, 
 * both are sampled from until a decision threshold over the response is reached, 
 * at which point motor planning commences. 
 */
void AxcptTask::run(){
    drawTrialType();
    _belief->setTrueStim(_context, _target);
    _belief->reset(); 
    _trialTime = 0; 
    int cresp = _context == _target ? 1 : 0; 
    double eblDur = _arch.drawEBL(); 
    _nPrecomputeSamps = (_retentionIntervalDur) / _timePerStep; 
    _recorder->updateDatum(_trialLabel+"eblEvent", Event(_retentionIntervalDur, _retentionIntervalDur+eblDur)); 
    _precomputeSamples(); 
    mat post; 
    double dv=0, oldDv=0; 
    int samp = 0; 
    double sampStart = _trialTime; 
    // to mimic Yu et al 2009, introduce parameter gamma that governs an early random response at t0
    if (_pPrematureResponse>0){
        int prematureResponse = RNG::rbernoulli(_pPrematureResponse);

        if (prematureResponse==1){
            // immediately start motor planning
            double motorPlanning = _arch.drawMotorPlanning(); 
            double motorTimeDur = _arch.drawMotorExec(); 
            // coin flip for the response
            double resp = RNG::rbernoulli(0.5);
            int acc = resp == cresp; 
            _recorder->updateDatum(_trialLabel+"motorPlanEvent", Event(0, motorPlanning)); 
            _recorder->updateDatum(_trialLabel + "Resp", double(resp));
            _recorder->updateDatum(_trialLabel + "Acc", double(acc)); 
            _recorder->updateDatum(_trialLabel+"motorExecEvent", Event(0, motorTimeDur)); 
            _recorder->updateDatum(_trialLabel + "RT", motorTimeDur);
            return; // finish the trial
        }
        
    }
    
    for (samp; samp < _maxSamps; ++samp){
        static_cast<DecayBelief*>(_belief)->updateFromContext(_contextNoise, _trialTime, _decayTo); 
        _belief->updateFromTarget(_targetNoise); 
        post = _belief->getBelief();
        _trialTime += _timePerStep; 
        _recordBelief(); 
        oldDv = dv; 
        dv = trace(post);
        // if we crossed threshold OR DV hasn't changed based on the last sample (usually means we latched)
        if (dv > _decisionThresh || dv < (1-_decisionThresh) || (fabs(oldDv-dv) <= DBL_TOL)){
            // std::cout << dv << " " << (1-dv) << " " << _decisionThresh << std::endl; 
            double motorPlanning = _arch.drawMotorPlanning(); 
            _recorder->updateDatum(_trialLabel+"motorPlanEvent", Event(_trialTime, _trialTime + motorPlanning)); 
            // response is locked in now. but we sample for d'oh effects and plotting
            // 1 is left, 0 is right
            int resp = dv > 0.5 ? 1 : 0; 
            int acc = resp == cresp ? 1 : 0; 
            // use ints for resp and cresp to not run into float comparison issues...
            // but then convert to doubles for mean and variance
            _recorder->updateDatum(_trialLabel + "Resp", double(resp));
            _recorder->updateDatum(_trialLabel + "Acc", double(acc)); 
            // keep sampling during planning just for plotting
            int sampsDuringMotorPlan = motorPlanning / _timePerStep; 
            int i=0; 
            for (i; i < sampsDuringMotorPlan; ++i){
                _belief->updateFromContext(_contextNoise); 
                _belief->updateFromTarget(_targetNoise); 
                post = _belief->getBelief();
                _trialTime += _timePerStep; 
                _recordBelief(); 
            }
            double motorTimeDur = _arch.drawMotorExec(); 
            _recorder->updateDatum(_trialLabel+"motorExecEvent", Event(_trialTime, _trialTime + motorTimeDur)); 
            double rt = _trialTime + motorTimeDur - _retentionIntervalDur + eblDur;
            _recorder->updateDatum(_trialLabel + "RT", rt);
            if (acc == 1){
                _recorder->updateDatum(_trialLabel + "CorrectRT", rt);
            } else {
                _recorder->updateDatum(_trialLabel + "IncorrectRT", rt);
            }
            break; 
        }            
    }

    _recorder->updateDatum(_trialLabel+"samplingBothEvent", Event(sampStart, _trialTime)); 
    #ifndef DISABLE_ERROR_CHECKS
    if (samp == _maxSamps) throw fatal_error() << "ERROR: hit maxSamps (" << _maxSamps << ")! If you're sure you know what you're doing, you can increase maxSamps to prevent this, but verifying that you really need this many should be a first step!"; 
    #endif

}

/**
 * @brief Update from memory of the context during the retention interval. 
 */
void AxcptTask::_precomputeSamples(){
    for (unsigned samp=0; samp < _nPrecomputeSamps; ++samp){
        _recordBelief(); 
        _trialTime += _timePerStep; 
        static_cast<DecayBelief*>(_belief)->updateFromContext(_retentionNoise, _trialTime, _decayTo); 
    }
    _recorder->updateDatum(_trialLabel+"samplingContextEvent", Event(0, _nPrecomputeSamps*_timePerStep)); 
}
