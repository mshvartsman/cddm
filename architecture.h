// include guard
#ifndef ARCH_H
#define ARCH_H

#include "config.h"


/**
 * @brief Class implementing the cognitive architecture of the agent. 
 * @details Currently we have three components to the architecture: 
 * an eye-brain lag, motor planning, and motor execution.
 * All three are gamma-distributed. The methods provided here
 * draw random variates from the distributions of these durations,
 * truncated to the simulation granularity (timePerStep). 
 * \todo make Architecture a pure abstract class. 
 */
class Architecture {


 public:
    double drawEBL();
    double drawMotorExec();
    double drawMotorPlanning();
    Architecture(const Config * c);

 private:
    double _eblMean; ///< Mean of the eye-brain lag (perceptual nondecision time)
    double _motorPlanMean; ///< Mean of the motor planning time. 
    double _motorExecuteMean; ///< Mean of motor execution time. 
    double _eblSd; ///< SD of the eye-brain lag (perceptual nondecision time)
    double _motorSd; ///< SD of the motor planning and execution times. 
    double _timePerStep; ///< Discretization rate: each "step" of the simulation takes this many ms.
};


#endif
