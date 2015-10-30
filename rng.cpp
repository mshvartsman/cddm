#include <armadillo>
#include "rng.h"

// pdf with mean 0 (z=mu+x, so with a different mu we do z-x, I think)
/**
 * @brief Returns a random bernoulli draw. 
 * \sa https://fossies.org/dox/gsl-1.16/bernoulli_8c_source.html
 */
int RNG::_rbernoulli_gsl_arma(const double p){
	double u = arma::randu(1)[0];
	if (u < p){
		return 1 ;
	}
	else {
		return 0 ;
	}
}

/**
 * @brief Returns a random gamma variate. 
 * @details This is the algorithm GSL uses, modified to use the arma
 * RNG instead of the GSL one (both should be marsenne twister but the
 * arma interface is easier to deal with than the C one in GSL). It is also 
 * reparameterized slightly: the var names in GSL source are a and b but as
 * per https://www.gnu.org/software/gsl/manual/html_node/The-Gamma-Distribution.html 
 * the distr function is \f$p(x) dx = {1 \over \Gamma(a) b^a} x^{a-1} e^{-x/b} dx \f$
 * which matches against \f$a=k\f$ and \f$b=\theta\f$ rather than \f$a=\alpha\f$ and 
 * \f$b=\beta\f$ as per http://en.wikipedia.org/wiki/Gamma_distribution. 
 * 
 * @param k shape
 * @param theta scale]
 */
double RNG::_rgamma_gsl_arma (const double k, const double theta){
if (k < 1){
  double u = arma::randu(1)[0];
  return _rgamma_gsl_arma (1.0 + k, theta) * pow (u, 1.0 / k);
}

{
  double x, v, u;
  double d = k - 1.0 / 3.0;
  double c = (1.0 / 3.0) / sqrt (d);

  while (1)
  {
    do
    {
      x = arma::randn(1)[0];
      v = 1.0 + c * x;
    }
    while (v <= 0);

    v = v * v * v;
    u = arma::randu(1)[0];

    if (u < 1 - 0.0331 * x * x * x * x)
      break;

    if (log (u) < 0.5 * x * x + d * (1 - v + log (v)))
      break;
  }

  return theta * d * v;
}
}

/**
 * @brief Compute gaussian probability density function.  
 * @param x point to evaluate the PDF
 * @param m mean of the gaussian
 * @param s standard deviation of the gaussian
 * @return value of the PDF, in probability (not log) space
 */
double RNG::dnorm(double x, double m, double s){
    // http://stackoverflow.com/questions/10847007/using-the-gaussian-probability-density-function-in-c
  static const double inv_sqrt_2pi = 0.3989422804014327;
  double a = (x - m) / s;
  return inv_sqrt_2pi / s * std::exp(-0.5f * a * a);
}

/**
 * @brief Generate random gamma variates parameterized by expected value and variance.
 * @details Gamma scale and shape are hard to interpret, so it's easier to use
 * mean and variance for parameterizing it. We maintain the advantage of it being 
 * zero-bounded, though the skewness is pretty small. 
 * 
 * @param m expected value of the gamma distribution to draw from. 
 * @param s standard deviation of the gamma distribution to draw from. 
 */
double RNG::rgamma(double m, double s)
{
  if (m==0) {
    return(0);
  }
  double k = (m*m) / (s*s);      // shape k = m^2/s^2
  double theta = (s*s) / m;     // scale theta = s^2 / m
  return RNG::_rgamma_gsl_arma(k, theta); 
}

/**
 * @brief Generate univariate gaussian variates. 
 * @param m mean of the gaussian
 * @param s standard deviation of the gaussian
 */
double RNG::rnorm(const double m, const double s){
	return m + (s * arma::randn(1)[0]); 
}

/**
 * @brief Generate uniform random variates (min 0). 
 * @param max upper bound of uniform distribution to draw from. 
 */
double RNG::runif(const double max){
	return arma::randu(1)[0]*max; 
}

/**
 * @brief Generate uniform random variates (min 0). 
 * @param max upper bound of uniform distribution to draw from. 
 * \todo rewrite this with templates so we don't need a separate one for int. 
 */
int RNG::runif_int(const int max){
  return arma::randi(1, arma::distr_param(0,max))[0];
}

/**
 * @brief Generate bernoulli variates. 
 * @details Currently aliased to the internal implementation but might some day
 * point elsewhere. 
 */
int RNG::rbernoulli(const double p){
  return RNG::_rbernoulli_gsl_arma(p); 
}
