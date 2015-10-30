// include guard
#ifndef RNG_H
#define RNG_H


/**
 * @brief Wrapper class for random number generation. 
 * @details This class is designed so that client code doesn't have to worry
 * about the RNG engine we use (c++11, MKL, TRNG, GSL etc) or keeping streams
 * properly separated in parallelized code. 
 * \todo Add interfaces to other RNG engines
 */
class RNG{
	private: 
		static double _rgamma_gsl_arma (const double k, const double theta);
		static int _rbernoulli_gsl_arma(const double p); 

	public:
		static double rgamma(const double m, const double s); 
		static double rnorm(const double m, const double s); 
		static double runif(const double max); 
		static int runif_int(const int max); 
		static int rbernoulli(const double p); 
		static double dnorm(const double x, const double m, const double s); 
};

#endif
