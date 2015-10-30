#ifndef FLANKER_H
#define FLANKER_H

#include "cddm_main.h"

/**
 * @brief A task implementation for the Eriksen flanker task. 
 * @details Inference is theoretically identical to \cite Yu2009, though
 * there may be some slight differences in parameterization or implementation. 
 * Defines trace datum for "post" (posterior), summary datums for RT, Resp, and Acc,
 * and event datums for ebl, motor planning, motor execution, and sampling. 
 * \todo Make this task description more detailed and self-sufficient. 
 */
class FlankerTask : public Task {
public:
    FlankerTask(const Config * c, Recorder * r);
    ~FlankerTask(); 
    virtual void run(); 

protected: 
    void _recordBelief();
    Belief * _belief;  ///< pointer to the belief object. 
    Architecture _arch; ///< the cognitive architecture object. 
    double _trialTime; ///< Current trial time. 
    double _timePerStep; ///< Time (in ms) that each timestep takes. 
    double _contextNoise; ///< Standard deviation of the context evidence distributions. 
    double _targetNoise; ///< Standard deviation of the target evidence distributions. 
    double _decisionThresh; ///< The threshold (over the decision variable) at which sampling stops. 
    double _pPrematureResponse; ///< Probability of responding instantlly at random without sampling, following \cite Yu2009. 
    int _maxTrials; ///< Number of trials to run. 
    int _maxSamps; ///< Maximum number of samples to allow per trial (this is a sentry for infinite loops, hitting this throws an error). 
};

void populateDefaults(Config * c);

#endif