#include "flanker.h"
#include <armadillo>
#include <iostream>

FlankerTask::~FlankerTask(){
    delete _belief; 
}


/**
 * @brief Constructor for the flanker task. 
 * @details Grabs the configuration, does some error checking, and initializes things. 
 * 
 * @param c A Config, containing \ref timePerStep, \ref maxTrials, \ref maxSamps, 
 * \ref contextNoise, \ref targetNoise, \ref decisionThresh, and \ref pPrematureResp
 * @param r a Recorder. 
 */
FlankerTask::FlankerTask(const Config * c, Recorder * r): Task(c, r), _arch(Architecture(c)), _trialTime(-1){
    _belief = new Belief(c); 
    _traceDatumNames = {"post"}; 
    _summaryDatumNames = {"RT", "Resp", "Acc"};
    _eventDatumNames = {"eblEvent", "motorPlanEvent", "motorExecEvent", "samplingEvent"}; 
    _timePerStep = _config->get<double>("timePerStep"); 
    _maxTrials = _config->get<int>("maxTrials"); 
    _maxSamps = _config->get<int>("maxSamps"); 
    _contextNoise = _config->get<double>("contextNoise"); 
    _targetNoise = _config->get<double>("targetNoise"); 
    _decisionThresh = _config->get<double>("decisionThresh"); 
    _pPrematureResponse = _config->get<double>("pPrematureResp");
    #ifndef DISABLE_ERROR_CHECKS
    if (_contextNoise<0) throw fatal_error() << "ERROR: contextNoise < 0, did you set it?";
    if (_targetNoise<0) throw fatal_error() << "ERROR: targetNoise < 0, did you set it?";
    if (_decisionThresh < 0) throw fatal_error() << "ERROR: decisionThresh < 0, did you set it (FlankerTask is implemented in prob space, not log space)?";
    if (_decisionThresh >= 1) throw fatal_error() << "ERROR: decisionThresh >=1 (FlankerTask is implemented in prob space, not log space) ";
    #endif
}

/**
 * @brief Record the current posterior into Recorder. 
 */
void FlankerTask::_recordBelief(){
    mat post = _belief->getBelief(); 
    _recorder->updateDatum(_trialLabel + "post", Timepoint(_trialTime,vectorise(post)));
}

/**
 * @brief Run one trial of the event loop for Flanker. 
 * @details At t0, there is some small probability of instantly responding. 
 * If this happens, motor planning commences immediatley. Otherwise,
 * context and target are both sampled from until a decision threshold over 
 * the target identity is reached, at which point motor planning commences. 
 */
void FlankerTask::run(){
    mat post; 
    double dv; 
    int samp = 0; 

    _trialTime = 0; 

    double eblDur = _arch.drawEBL(); 

    double sampStart = _trialTime; 
    drawTrialType(); // provided by Task superclass, populates _context and _target, and _trialLabel

    int cresp = _target == 0 ? 0 : 1; 

    _belief->setTrueStim(_context, _target);
    _belief->reset(); 
    _recorder->updateDatum(_trialLabel+"eblEvent", Event(0, eblDur)); 
    
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
        // update from context twice! we have two flankers
        _belief->updateFromContext(_contextNoise); 
        _belief->updateFromContext(_contextNoise); 
        _belief->updateFromTarget(_targetNoise); 
        post = _belief->getBelief();
        _trialTime += _timePerStep; 
        _recordBelief(); 
        dv = post(0,0) + post(1, 0); 
        if (dv > _decisionThresh || dv < (1-_decisionThresh)){
            double motorPlanning = _arch.drawMotorPlanning(); 
            _recorder->updateDatum(_trialLabel+"motorPlanEvent", Event(_trialTime, _trialTime + motorPlanning)); 
            // response is locked in now. but we sample for d'oh effects and plotting
            // 1 is left, 0 is right
            int resp = dv > 0.5 ? 0 : 1; 
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
            double rt = _trialTime + motorTimeDur + eblDur;
            _recorder->updateDatum(_trialLabel + "RT", rt);
            break; 
        }            
    }
    _recorder->updateDatum(_trialLabel+"samplingEvent", Event(sampStart, _trialTime)); 
    #ifndef DISABLE_ERROR_CHECKS
    if (samp == _maxSamps) throw fatal_error() << "ERROR: hit maxSamps (" << _maxSamps << ")! If you're sure you know what you're doing, you can increase maxSamps to prevent this, but verifying that you really need this many should be a first step!"; 
    #endif
}
