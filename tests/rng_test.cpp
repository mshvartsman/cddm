#include "catch_main.h"
#include "../rng.h"
#include <fstream>


TEST_CASE("Test for probability density function correctness"){

	double res = RNG::dnorm(0,0,1); 
	REQUIRE(res == Approx(0.3989423));
}


TEST_CASE("Dump some gaussians and their dnorm"){
	std::ofstream f; 
	f.open("gaussdump0"); 
	for (unsigned i=0; i<100000; ++i){
		double tmp = RNG::rnorm(0,10); 
		f << tmp << "," << RNG::dnorm(tmp, 0, 10) << std::endl; 
	}
	f.close(); 

	f.open("gaussdump1"); 
	for (unsigned i=0; i<100000; ++i){
		double tmp = RNG::rnorm(1,10); 
		f << tmp << "," << RNG::dnorm(tmp, 1, 10) << std::endl; 
	}
	f.close(); 
}

TEST_CASE("Dump some bernoullis"){
	std::ofstream f; 
	f.open("bernDump01"); 
	for (unsigned i=0; i<100000; ++i){
		double tmp = RNG::rbernoulli(0.1); 
		f << tmp << std::endl; 
	}
	f.close(); 

	f.open("bernDump09"); 
	for (unsigned i=0; i<100000; ++i){
		double tmp = RNG::rbernoulli(0.9); 
		f << tmp << std::endl; 
	}
	f.close(); 

}

