#include "catch_main.h"
#include "../belief.h"
#include "../config.h"
#include <armadillo>

using arma::mat; 
using arma::vec;

TEST_CASE("Error throws from belief"){
	// set up our config
	Config conf = Config(); 

	conf.set("urPrior", "0.4 0.2 0.1; 0.1 0.1 0.1"); 

	Belief b = Belief(&conf); 

	REQUIRE_THROWS(b.setTrueStim(2,0));
	REQUIRE_THROWS(b.setTrueStim(0,3)); 
	REQUIRE_THROWS(b.setTrueStim(-1,0));
	REQUIRE_THROWS(b.setTrueStim(0,-5)); 
	
	REQUIRE_NOTHROW(b.setTrueStim(0,0)); 
	REQUIRE_NOTHROW(b.setTrueStim(0,1)); 
	REQUIRE_NOTHROW(b.setTrueStim(1,0)); 
	REQUIRE_NOTHROW(b.setTrueStim(1,1)); 
	REQUIRE_NOTHROW(b.setTrueStim(0,2)); 
	REQUIRE_NOTHROW(b.setTrueStim(1,2)); 	
}

TEST_CASE("Asymptotic convergence of updates"){
	
	Config conf = Config(); 
	mat upIn; 
	upIn << 0.5 << 0.2 << arma::endr << 0.2 << 0.1; 

	conf.set("urPrior", "0.5 0.2; 0.2 0.1"); 

	Belief b = Belief(&conf); 
	mat out; 
	
	out.set_size(2,2); 
	
	b.setTrueStim(0,0);

	SECTION("Asymptotic convergence of updates from context"){
		for (unsigned i=0; i<1000; i++){
			b.updateFromContext(1); 
		}

		out = b.getBelief();

		REQUIRE(out(0,0)==Approx(0.5/0.7));
		REQUIRE(out(0,1)==Approx(0.2/0.7));
		REQUIRE(out(1,0)==Approx(0));
		REQUIRE(out(1,1)==Approx(0));
	}

	SECTION("Asymptotic convergence of updates from target"){
		for (unsigned i=0; i<1000; i++){
			b.updateFromTarget(1.3); 
		}

		out = b.getBelief();

		REQUIRE(out(0,0)==Approx(0.5/0.7));
		REQUIRE(out(1,0)==Approx(0.2/0.7));
		REQUIRE(out(0,1)==Approx(0));
		REQUIRE(out(1,1)==Approx(0));
		REQUIRE(trace(out)==Approx(0.5/0.7));
	}

}

TEST_CASE("DecayBelief tests"){
	Config conf = Config(); 
	mat upIn; 
	upIn << 0.5 << 0.2 << arma::endr << 0.2 << 0.1; 

	conf.set("urPrior", "0.5 0.2; 0.2 0.1"); 

	SECTION("decayRate=0 means we just update correctly"){
		vec out(100); 
		conf.set("decayRate", 0);
		DecayBelief b(&conf); 

		b.setTrueStim(0,0);
		for (unsigned i=0; i<100; ++i){
			out[i] = b.updateFromContext(5, i*10); 
		}
		REQUIRE(all(out==-1)); 
	}

	SECTION("Correct proportion of incorrect draws with decayRate!=0"){
		vec trueTrialDrawCount(100);
		vec falseTrialDrawCount(100);
		conf.set("decayRate", 0.01); 
		DecayBelief b(&conf); 

		b.setTrueStim(0,0);

		trueTrialDrawCount.fill(0);
		falseTrialDrawCount.fill(0);
		for (unsigned repeats=0; repeats<=10000; ++repeats){
			for (unsigned i=0; i<100; ++i){
				b.reset(); // otherwise we're going to eventually undeflow somewhere
				int drawType = b.updateFromContext(5, i*10); 
				if (drawType==-1){
					++trueTrialDrawCount[i];
				} else{
					++falseTrialDrawCount[i];
				}
			}
		}
		vec actual(100);
		actual = trueTrialDrawCount / (trueTrialDrawCount + falseTrialDrawCount);
		
		vec correct(100);
		for (unsigned i=0; i<100; ++i){
			correct[i] = exp(-0.01*i*10);
		}
		
		arma::vec correctbool(100); 
		for (unsigned i=0; i<correctbool.size(); ++i){
			correctbool[i] = correct[i] == Approx(actual[i]).epsilon(0.01);
		} 
		REQUIRE(arma::all(correctbool));
	}

	SECTION("Correct distribution of incorrect draws with decayRate!=0 and decay to prior"){
		int context0DrawCount=0, context1DrawCount=0; 
		conf.set("decayRate", 1); 
		DecayBelief b(&conf); 
		b.setTrueStim(0,0);
		for (unsigned i=0; i<10000; ++i){
				b.reset(); // otherwise we're going to eventually undeflow somewhere
				int drawType = b.updateFromContext(5, 1000); 
				switch (drawType){
					case -1: 
					break; 
					case 0: 
					++context0DrawCount;
					break; 
					case 1: 
					++context1DrawCount; 
					break; 
					default: 
					throw fatal_error() << "unknown draw type (" << drawType << ")!";
				}
			}
			REQUIRE(context0DrawCount==Approx(7000).epsilon(10)); 
			REQUIRE(context0DrawCount==Approx(3000).epsilon(10)); 
		}
		SECTION("Correct distribution of incorrect draws with decayRate!=0 and decay to uniform"){
			int context0DrawCount=0, context1DrawCount=0; 
			conf.set("decayRate", 1); 
			DecayBelief b(&conf); 
			b.setTrueStim(0,0);
			for (unsigned i=0; i<10000; ++i){
				b.reset(); // otherwise we're going to eventually undeflow somewhere
				int drawType = b.updateFromContext(5, 1000, Uniform); 
				switch (drawType){
					case -1: 
					break; 
					case 0: 
					++context0DrawCount;
					break; 
					case 1: 
					++context1DrawCount; 
					break; 
					default: 
					throw fatal_error() << "unknown draw type (" << drawType << ")!";
				}
			}
			REQUIRE(context0DrawCount==Approx(5000).epsilon(10)); 
			REQUIRE(context0DrawCount==Approx(5000).epsilon(10)); 
		}

		SECTION("Correct likelihood computation for DecayBelief"){
			// urPrior is 0.5 0.2 0.2 0.1 so our marginals are 0.7 0.3
			// the likelihood should be dnorm of the correct samp times p(correct update) + 
			// (1-p(correct update)) * (marginal1 * lik1 + marginal2 * lik2)
			std::vector<double> correctA = {0.2250525,0.2480430,0.2707248,0.2926155,0.3132173,0.3320360,0.3486013,0.3624861,0.3733256,0.3808335,0.3848148,0.3851745,0.3819211,0.3751658,0.3651160,0.3520653,0.3363787,0.3184760,0.2988132,0.2778633,0.2560982};
			std::vector<double> correctB = {0.09346672,0.10771440,0.12320585,0.13987210,0.15760463,0.17625262,0.19562143,0.21547263,0.23552595,0.25546325,0.27493475,0.29356738,0.31097529,0.32677205,0.34058417,0.35206533,0.36091058,0.36686970,0.36975895,0.36947041,0.36597825};
			std::vector<double> samps = {-1.0,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,-0.2,-0.1,0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0};
			DecayBelief b(&conf); 
			for (unsigned i=0; i<samps.size(); ++i){
				b._computeLikelihoods(samps[i],1, Context, 0.7); 
				arma::mat lik = b.getLik(); 
				REQUIRE(lik(0,0)==Approx(correctA[i]));
				REQUIRE(lik(0,1)==Approx(correctA[i]));
				REQUIRE(lik(1,0)==Approx(correctB[i]));
				REQUIRE(lik(1,1)==Approx(correctB[i]));
			}
	}
}