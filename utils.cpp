#include <cmath>

#include "utils.h"


namespace utils{
	/**
	 * @brief Rounds val to a multiple of prec. 
	 * @details Used for aliasing everything to the simulation granularity. 
	 */
	double roundToIncrement(double val, double prec){
		return floor(val / prec + 0.5) * prec;
	}
}
