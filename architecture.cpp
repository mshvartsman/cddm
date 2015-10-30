#include <math.h> // for modf for rounding
#include "rng.h" 
#include "utils.h" // provides roundToIncrement()
#include "architecture.h"

 
/**
 * @brief Draw a random eye-brain lag. 
 * @details Draws a random gamma variate for eye brain lag (perceptual 
 * nondecision time) and round it to the simulation granularity. 
 * @return a single draw of the EBL, rounded to the discretization level. 
 */
double Architecture::drawEBL(){
	double unrounded= RNG::rgamma(_eblMean, _eblSd);
	return utils::roundToIncrement(unrounded, _timePerStep); 
}

/**
 * @brief Draw a random motor execution duration. 
 * @details Draws a random gamma variate for motor execution duration and
 * round it to the simulation granularity. 
 * @return a single draw of motor execution, rounded to the discretization level. 
 */
double Architecture::drawMotorExec(){
	double unrounded= RNG::rgamma(_motorPlanMean, _motorSd);
	return utils::roundToIncrement(unrounded, _motorSd); 
}

/**
 * @brief Draw a random motor planning duration. 
 * @details Draw a random gamma variate for motor planning time (motor nondecision 
 * before motor execution) and round it to the simulation granularity.
 * Theoreticaly one could replace this with something with motor cancellation,
 * replanning and additional sophistication. 
 * @return a single draw of motor planning, rounded to the discretization level. 
 */
double Architecture::drawMotorPlanning(){

	double unrounded= RNG::rgamma(_motorExecuteMean, _motorSd);
	return utils::roundToIncrement(unrounded, _motorSd); 
}

/**
* @brief Constructor for the Architecture class.
* @details Constructor for the Architecture class. Expects a configuration object 
* with registered values for \ref timePerStep, \ref eblMean, \ref eblSd, 
* \ref motorPlanMean, \ref motorExecMean, and \ref motorSd (the latter is shared
* for both motor components).
*/
Architecture::Architecture(const Config * c){
	_timePerStep = c->get<double>("timePerStep"); 
	_eblMean = c->get<double>("eblMean");
	_eblSd = c->get<double>("eblSd");
	_motorPlanMean = c->get<double>("motorPlanMean");
	_motorExecuteMean = c->get<double>("motorExecMean");
	_motorSd = c->get<double>("motorSd");
	
}
