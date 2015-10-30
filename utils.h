// include guard
#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <type_traits> // for enable_if and is_floating_point

using std::vector; 

/**
 * @brief Miscellaneous useful things. 
 */
namespace utils{
	double roundToIncrement(double val, double prec);
	
	namespace detail{
		enum class enabled {}; 
	}

	/**
	 * @brief SFINAE trick for templated corrected sum for floating point only
	 * \sa http://loungecpp.wikidot.com/tips-and-tricks:enable-if-for-c-11
	 */
    template <typename Condition>
	using EnableIf = typename std::enable_if<Condition::value, detail::enabled>::type; 

	// real kahan_sum only makes sense for floats, so restrict to floats:
	// http://www.drdobbs.com/floating-point-summation/184403224?pgno=8, modified for templating
	/**
	 * @brief Compute float sum with correction. 
	 * @details Keeps track of accumulated errors and adds them up at the end
	 * 
	 * @param container iterable container that can hold floating point numbers
	 * @tparam SFINAE trick. 
	 * @return Same type as inside the container, summed. 
	 * \sa http://www.drdobbs.com/floating-point-summation/184403224?pgno=8
	 */
	template<typename Container, EnableIf<std::is_floating_point<typename Container::value_type>>...>
	typename Container::value_type kahanSum(const Container& container) {
    
		typename Container::const_iterator b = container.begin();
		typename Container::const_iterator e = container.end(); 
		long double sum, correction;
		sum = 0;
		correction = 0.0L;
		for (typename Container::const_iterator it=b; it!=e; ++it)
		{
			long double corrected_next_term = *it - correction;
			long double new_sum = sum + corrected_next_term;
			correction = (new_sum - sum) - corrected_next_term;
			sum = new_sum;
		}
		return sum;
	}

	/**
	 * @brief Mean using corrected sums. 
	 * @details Should be more accurate than using regular sum. 
	 * 
	 * @param container iterable container of floating point numbers
	 * @tparam SFINAE trick. 
	 * @return mean, same type as in container
	 */
	template<typename Container, EnableIf<std::is_floating_point<typename Container::value_type>>...>
	typename Container::value_type mean(const Container& container) {
		typename Container::value_type sum = kahanSum(container); 
		return sum / container.size(); 
	}

	/**
	 * @brief Variance using corrected sums. 
	 * @details Should be more accurate than using regular sum. 
	 * 
	 * @param container iterable container of floating point numbers
	 * @tparam SFINAE trick. 
	 * @return mean, same type as in container
	 */
	template<typename Container, EnableIf<std::is_floating_point<typename Container::value_type>>...>
	typename Container::value_type variance(const Container& container) {
		// http://roth.cs.kuleuven.be/w-ess/index.php/Accurate_variance_and_mean_calculations_in_C%2B%2B11
	// (has faster 1-pass algos in there but RawVectorsDatum is not meant for speed and we run getMean and getVar once)
		typename Container::value_type m = utils::mean(container); 
		typename Container::value_type s2 = 0; 
		for(auto x : container) {
			s2 += (x - m) * (x - m);
		}
		return s2 / (container.size()-1);
	}
	// // ...but if we get an int in, just transparently return int sum
	// template<typename Container, EnableIf<std::is_integral<typename Container::value_type>>...>
	// typename Container::value_type kahanSum(const Container& container) {
	// 	// because we're in a header, don't #include <algorithm> just for std::accumulate because it will bloat compilation times
	// 	typename Container::const_iterator b = container.begin();
	// 	typename Container::const_iterator e = container.end(); 
	// 	typename Container::value_type sum = 0;   
	// 	for (typename Container::const_iterator it=b; it!=e; ++it){
	// 		sum += *it; 
	// 	}
	// 	return sum;
	// }



}

#endif