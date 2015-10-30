#include "catch_main.h"
#include "../config.h"

#include <string>
#include <armadillo>


TEST_CASE("Config not from file tests"){
	Config conf; 

	SECTION("key exists test"){
		conf.set<double>("eyeBrainLag", 10.3); 
		REQUIRE(conf.keyExists("eyeBrainLag")==true);
	}

	SECTION("Set and get doubles"){
		conf.set<double>("eyeBrainLag", 10.3); 
		REQUIRE(conf.get<double>("eyeBrainLag")==Approx(10.3)); 

		conf.set<double>("motorSd", 1.3); 
		REQUIRE(conf.get<double>("motorSd")==Approx(1.3)); 

		conf.set<double>("motorMean", 0.3); 
		REQUIRE(conf.get<double>("motorMean")==Approx(0.3)); 
	}

	SECTION("Set and get ints"){
		conf.set<int>("eyeBrainLag", 10); 
		REQUIRE(conf.get<int>("eyeBrainLag")==10); 

		conf.set<int>("motorSd", 1); 
		REQUIRE(conf.get<int>("motorSd")==1); 

		conf.set<int>("motorMean", 0); 
		REQUIRE(conf.get<int>("motorMean")==0); 
	}
	
	SECTION("Set and get strings"){
		conf.set<std::string>("testString", std::string("hello"));
		REQUIRE(conf.get<std::string>("testString").compare("hello")==0);
	}

	SECTION("Set arma mat as string, get as matrix"){
		conf.set<std::string>("trialDist", "0.4 0.3; 0.2 0.1"); 
		arma::mat out = conf.get<arma::mat>("trialDist"); 
		arma::mat correct; 
		correct << 0.4 << 0.3 << arma::endr << 0.2 << 0.1; 

		REQUIRE(all(vectorise(correct)==vectorise(out)));
	}

	SECTION("Set and get arma mat as mat"){
		mat in, out, correct; 
		in << 0.4 << 0.2 << 0.1 << arma::endr << 0.1 << 0.1 << 0.1; 
		correct << 0.4 << 0.2 << 0.1 << arma::endr << 0.1 << 0.1 << 0.1; 
		conf.set<arma::mat>("trialDist", in);
		out = conf.get<arma::mat>("trialDist");

		REQUIRE(all(vectorise(correct)==vectorise(out)));
	}

	SECTION("Throw on unknown key"){
		REQUIRE_THROWS(conf.get<int>("invalid"));
	}

	SECTION("Throw on deleting unknown key"){
		REQUIRE_THROWS(conf.unset("invalid"));	
	}

	SECTION("remove keys"){
		REQUIRE_THROWS(conf.get<double>("key")); // doesn't exist
		conf.set<double>("key", 0.3);  
		REQUIRE_NOTHROW(conf.get<double>("key")); // exists
		conf.unset("key");
		REQUIRE_THROWS(conf.get<double>("key")); // doesn't exist
	}

}

TEST_CASE("Read config from file"){
	Config conf;
	conf.loadFromFile("test_config.cfg"); 

	SECTION("Read scalars"){
		REQUIRE(conf.get<double>("coefVar")==Approx(0.3));
		REQUIRE(conf.get<int>("eyeBrainLag")==30);
	}

	SECTION("Read a matrix"){
		arma::mat out = conf.get<arma::mat>("trialDist"); 
		arma::mat correct; 
		correct << 0.4 << 0.3 << arma::endr << 0.2 << 0.1; 

		REQUIRE(all(vectorise(correct)==vectorise(out)));
	}
}

TEST_CASE("Write config to file"){
	Config conf; 

	conf.set<int>("eyeBrainLag", 10); 

	conf.set<double>("motorSd", 1.3); 
	
	conf.set<std::string>(std::string("version"), "testing"); 

	conf.set<std::string>("trialDistString", "0.4 0.3; 0.2 0.1"); 
	
	mat in; 
	in << 0.4 << 0.2 << 0.1 << arma::endr << 0.1 << 0.1 << 0.1; 
	conf.set<arma::mat>("trialDistMat", in);

	conf.save("testCfgSave.cfg");

	Config test;
	test.loadFromFile("testCfgSave.cfg");

	REQUIRE(test.get<int>("eyeBrainLag")==10);
	REQUIRE(test.get<double>("motorSd")==Approx(1.3));
	REQUIRE(test.get<string>("version").compare("testing")==0); 

	mat out; 

	out = test.get<mat>("trialDistString"); 

	REQUIRE(all(vectorise(mat("0.4 0.3; 0.2 0.1"))==vectorise(out)));

	out = test.get<mat>("trialDistMat"); 

	REQUIRE(all(vectorise(in)==vectorise(out)));
}

TEST_CASE("Write config to string and read it back out"){
	Config conf; 

	conf.set<int>("eyeBrainLag", 10); 

	conf.set<double>("motorSd", 1.3); 
	
	conf.set<std::string>(std::string("version"), "testing"); 

	conf.set<std::string>("trialDistString", "0.4 0.3; 0.2 0.1"); 
	
	mat in; 
	in << 0.4 << 0.2 << 0.1 << arma::endr << 0.1 << 0.1 << 0.1; 
	conf.set<arma::mat>("trialDistMat", in);

	std::string confStr = conf.stringRepr(); 

	Config test; 
	test.loadFromString(confStr); 

	REQUIRE(test.get<int>("eyeBrainLag")==10);
	REQUIRE(test.get<double>("motorSd")==Approx(1.3));
	REQUIRE(test.get<string>("version").compare("testing")==0); 

	mat out; 

	out = test.get<mat>("trialDistString"); 

	REQUIRE(all(vectorise(mat("0.4 0.3; 0.2 0.1"))==vectorise(out)));

	out = test.get<mat>("trialDistMat"); 

	REQUIRE(all(vectorise(in)==vectorise(out)));

}