#include "catch_main.h"
#include "../utils.h"
#include <armadillo>

TEST_CASE("Mean and variance computations are correct"){
	vector<double> inputVec{0.1, 2, 3578, 4.1, 5.7, 6.4}; 
	arma::vec inputArma{1, 2, 3, 4.4, 5.5, 6.6}; 
	
	SECTION("Mean"){
		REQUIRE(utils::mean(inputVec) == Approx(599.3833));
		REQUIRE(utils::mean(inputArma) == Approx(3.75));
	}

	SECTION("Variance"){
		REQUIRE(utils::variance(inputVec) == Approx(2129323));
		REQUIRE(utils::variance(inputArma) == Approx(4.559));
	}	
}


TEST_CASE("Round to increment works with integer increments"){
	REQUIRE(utils::roundToIncrement(3.3,1) == Approx(3)); 
	REQUIRE(utils::roundToIncrement(73.59,10) == Approx(70)); 
	REQUIRE(utils::roundToIncrement(75.59,10) == Approx(80)); 

}

TEST_CASE("Round to increment works with double increments"){
	REQUIRE(utils::roundToIncrement(3.3,1.5) == Approx(3)); 
	REQUIRE(utils::roundToIncrement(4.3,1.5) == Approx(4.5)); 
}

TEST_CASE("kahan summation works for different types!"){
	using namespace arma;

	SECTION("kahan sum works for STL vector of doubles"){
		std::vector<double> v = {1.1, 2, 3.5, 4.7};
		REQUIRE(std::accumulate(v.begin(), v.end(), 0.0L)==Approx(utils::kahanSum(v)));
	}

	SECTION("kahan sum works for STL vector of floats"){
		std::vector<float> v = {1.1, 2, 3.5, 4.7};
		REQUIRE(std::accumulate(v.begin(), v.end(), 0.0L)==Approx(utils::kahanSum(v)));
	}


	// SECTION("kahan sum works for STL vector of ints by delegating to the non-kahan version"){
	// 	std::vector<int> v = {1, 2, 3, 4, 5};
	// 	REQUIRE(utils::kahanSum(v)==15);
	// }


	SECTION("kahan sum works for arma::vec"){
		vec v = randu<vec>(10);
		REQUIRE(accu(v)==Approx(utils::kahanSum(v)));
	}

	SECTION("kahan sum works for arma::mat"){
		mat m = randu<mat>(10,10);
		REQUIRE(accu(m)==Approx(utils::kahanSum(m)));
	}
}