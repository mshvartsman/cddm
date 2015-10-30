#ifndef AXCPT_H
#define AXCPT_H

#include "cddm_main.h"

/**
 * @brief A task implementation for the AX-Continuous Performance Test. 
 * @details Inference is described in the NIPS submission. 
 * Defines trace datum for "post" (posterior), summary datums for RT, correct RT, incorrect 
 * RT, Resp, and Acc, and event datums for ebl, motor planning, motor execution, 
 * sampling context, sampling target, and sampling both.
 * \todo Make this task description more detailed and self-sufficient. 
 */
class AxcptTask: public Task{
public:
    AxcptTask(const Config * c, Recorder * r);
    ~AxcptTask(); 
    virtual void run(); 

protected: 
    virtual void _precomputeSamples();
    void _recordBelief();

    Belief * _belief; ///< pointer to the belief object. 
    Architecture _arch; ///< the cognitive architecture object. 
    double _trialTime; ///< Current trial time. 
    double _timePerStep; ///< Time (in ms) that each timestep takes. 
    double _retentionIntervalDur; ///< Duration of the retention interval after the context comes off but before target comes on. 
    int _nPrecomputeSamps; ///< Number of samples to draw during retention interval, i.e. _retentionIntervalDur/_timePerStep
    double _retentionNoise; ///< Standard deviation of the context evidence distribution when only context is on screen. 
    double _contextNoise; ///< Standard deviation of the context evidence distribution when both target is on screen. 
    double _targetNoise; ///< Standard deviation of the target evidence distribution. 
    double _decisionThresh; ///< The threshold (over the decision variable) at which sampling stops. 
    double _pPrematureResponse; ///< Probability of responding instantly at random without sampling, following \cite Yu2009. 
    int _maxTrials; ///< Number of trials to run. 
    int _maxSamps; ///< Maximum number of samples to allow per trial (this is a sentry for infinite loops, hitting this throws an error). 
    PriorType _decayTo; ///< Determines how to draw a "bad" context sample (from the prior or at uniform).
};


void populateDefaults(Config * c);

#endif