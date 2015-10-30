#include "catch_main.h"
#include "../recorder.h"
#include "../rng.h" // for testing the GMM

#include <string>
#include <armadillo>
#include <vector>
#include <algorithm> // std::equal

using std::vector; 

TEST_CASE("Tests for DummyDatum"){
	DummyDatum<double> d; 
	REQUIRE_NOTHROW(d.record(3.5)); 
}

TEST_CASE("Tests for RawVectorsDatum"){
	SECTION("double data"){
		RawVectorsDatum<double> d; 

		vector<double> inputVec{0.1, 2, 3, 4.1, 5.7, 6.4}; 

		for (auto i: inputVec){
			d.newTrial(); 
			d.record(i); 
		}

		SECTION("Record mean correctly"){
			REQUIRE(d.getMean() == Approx(3.55)); 
		}

		SECTION("Record variance correctly"){
			REQUIRE(d.getVariance() == Approx(5.531)); 	
		}

		SECTION("Record N correctly"){
			REQUIRE(d.getN() == 6); 
		}

		SECTION("Record raw data vector correctly"){
			vector<double> outputVec = d.getRawData(); 
			REQUIRE(std::equal(inputVec.begin(), inputVec.end(), outputVec.begin())==true);
		}

		SECTION("String repr is as we expect"){
			std::string actual = d.getStringRepr(); 
			std::string expected = "0,0.1\n1,2\n2,3\n3,4.1\n4,5.7\n5,6.4\n"; 
			INFO("Expected: " << expected << "Actual: " << actual); 
			REQUIRE(actual==expected); 
		}
	}

	SECTION("float data"){
		RawVectorsDatum<float> d; 

		vector<float> inputVec{0.1, 2, 3, 4.1, 5.7, 6.4}; 

		for (auto i: inputVec){
			d.newTrial(); 
			d.record(i); 
		}
		SECTION("Record mean correctly"){
			REQUIRE(d.getMean() == Approx(3.55)); 
		}

		SECTION("Record variance correctly"){
			REQUIRE(d.getVariance() == Approx(5.531)); 	
		}

		SECTION("Record N correctly"){
			REQUIRE(d.getN() == 6); 
		}

		SECTION("Record raw data vector correctly"){
			vector<float> outputVec = d.getRawData(); 
			REQUIRE(std::equal(inputVec.begin(), inputVec.end(), outputVec.begin())==true);
		}

		SECTION("String repr is as we expect"){
			std::string actual = d.getStringRepr(); 
			std::string expected = "0,0.1\n1,2\n2,3\n3,4.1\n4,5.7\n5,6.4\n"; 
			INFO("Expected: " << expected << "Actual: " << actual); 
			REQUIRE(actual==expected); 
		}
	}

	SECTION("long double data"){
		RawVectorsDatum<long double> d; 

		vector<long double> inputVec{0.1, 2, 3, 4.1, 5.7, 6.4}; 

		for (auto i: inputVec){
			d.newTrial(); 
			d.record(i); 
		}

		SECTION("Record mean correctly"){
			REQUIRE(d.getMean() == Approx(3.55)); 
		}

		SECTION("Record variance correctly"){
			REQUIRE(d.getVariance() == Approx(5.531)); 	
		}

		SECTION("Record N correctly"){
			REQUIRE(d.getN() == 6); 
		}

		SECTION("Record raw data vector correctly"){
			vector<long double> outputVec = d.getRawData(); 
			REQUIRE(std::equal(inputVec.begin(), inputVec.end(), outputVec.begin())==true);
		}

		SECTION("String repr is as we expect"){
			std::string actual = d.getStringRepr(); 
			std::string expected = "0,0.1\n1,2\n2,3\n3,4.1\n4,5.7\n5,6.4\n"; 
			INFO("Expected: " << expected << "Actual: " << actual); 
			REQUIRE(actual==expected); 
		}
	}

}

TEST_CASE("Tests for IncrementalMeanVarianceDatum"){
	SECTION("double data"){
		IncrementalMeanVarianceDatum<double> d; 

		vector<double> inputVec{0.1, 2, 3, 4.1, 5.7, 6.4}; 

		for (auto i: inputVec){
			d.record(i); 
		}

		SECTION("Record mean correctly"){
			REQUIRE(d.getMean() == Approx(3.55)); 
		}

		SECTION("Record variance correctly"){
			REQUIRE(d.getVariance() == Approx(5.531)); 	
		}

		SECTION("Record N correctly"){
			REQUIRE(d.getN() == 6); 
		}

		SECTION("String repr is as we expect"){
			std::string actual = d.getStringRepr(); 
			std::string expected = "3.55,5.531,6\n"; 
			INFO("Expected: " << expected << "Actual: " << actual); 
			REQUIRE(actual==expected); 
		}
	}
	SECTION("long double data"){
		IncrementalMeanVarianceDatum<long double> d; 

		vector<long double> inputVec{0.1, 2, 3, 4.1, 5.7, 6.4}; 

		for (auto i: inputVec){
			d.record(i); 
		}

		SECTION("Record mean correctly"){
			REQUIRE(d.getMean() == Approx(3.55)); 
		}

		SECTION("Record variance correctly"){
			REQUIRE(d.getVariance() == Approx(5.531)); 	
		}

		SECTION("Record N correctly"){
			REQUIRE(d.getN() == 6); 
		}
		SECTION("String repr is as we expect"){
			std::string actual = d.getStringRepr(); 
			std::string expected = "3.55,5.531,6\n"; 
			INFO("Expected: " << expected << "Actual: " << actual); 
			REQUIRE(actual==expected); 
		}
	}
	SECTION("float data"){
		IncrementalMeanVarianceDatum<float> d; 

		vector<float> inputVec{0.1, 2, 3, 4.1, 5.7, 6.4}; 

		for (auto i: inputVec){
			d.record(i); 
		}

		SECTION("Record mean correctly"){
			REQUIRE(d.getMean() == Approx(3.55)); 
		}

		SECTION("Record variance correctly"){
			REQUIRE(d.getVariance() == Approx(5.531)); 	
		}

		SECTION("Record N correctly"){
			REQUIRE(d.getN() == 6); 
		}
		SECTION("String repr is as we expect"){
			std::string actual = d.getStringRepr(); 
			std::string expected = "3.55,5.531,6\n"; 
			INFO("Expected: " << expected << "Actual: " << actual); 
			REQUIRE(actual==expected); 
		}
	}


}

TEST_CASE("Tests for TraceDatum"){
	TraceDatum d;
	
	arma::vec times{0, 1, 2, 3, 4.4, 6, 3.5, 4, 6, 1, 2, 3}; 
	vector<arma::vec> vals(12); 
	arma::vec traceIds{0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2};
	arma::mat correctOut(12, 6); 
	correctOut.col(0) = traceIds;
	correctOut.col(1) = times; 
	for (unsigned i = 0; i < vals.size(); ++i){
		vals[i].randu(4); 
		for (unsigned j = 0; j < 4; ++j){
			correctOut(i, 2+j) = vals[i][j]; 
		}
	}
	d.newTrial(); 
	d.record(Timepoint(times[0], vals[0])); 

	for (unsigned i = 1; i< times.size(); ++i){
		if (traceIds[i] > traceIds[i-1]) d.newTrial(); 
		d.record(Timepoint(times[i], vals[i])); 
	}
	
	arma::mat out = d.getTraces();

	INFO("Correct =\n" << correctOut);
	INFO("Actual =\n" << out);

	REQUIRE(all(vectorise(correctOut)==vectorise(out)));

	SECTION("String repr test"){
		std::string actual = d.getStringRepr(); 
		std::ostringstream tmp;
		correctOut.save(tmp, arma::csv_ascii); 
		std::string expected = tmp.str();
		INFO("Expected: " << expected << "Actual: " << actual); 
		REQUIRE(actual==expected); 
	}
	
}

TEST_CASE("Tests for EventDatum"){

	SECTION("Event throws if end before start but not if equal"){
		REQUIRE_THROWS(Event(3, 1)); 
		REQUIRE_NOTHROW(Event(3, 3)); 
	}
	EventDatum d;
	
	arma::vec starts1{100, 200}; 
	arma::vec ends1{101.5, 200}; 
	
	d.newTrial(); 
	
	arma::mat correctOut; 
	correctOut << 0 << 0 << 1 <<  1 << arma::endr << 100 << 200 << 300.5 << 400.5 << arma::endr << 101.5 << 200 << 350.5 << 10000 ;
	correctOut = correctOut.t(); 
	for (unsigned i=0; i< starts1.size(); ++i){
		d.record(Event(starts1[i], ends1[i])); 
	}

	arma::vec starts2{300.5, 400.5}; 
	arma::vec ends2{350.5, 10000}; 

	d.newTrial(); 

	for (unsigned i=0; i< starts2.size(); ++i){
		d.record(Event(starts2[i], ends2[i])); 
	}
	
	arma::mat out = d.getEventTimes();

	INFO("Correct =\n" << correctOut);
	INFO("Actual =\n" << out);

	REQUIRE(all(vectorise(correctOut)==vectorise(out)));

	SECTION("String repr test"){
		std::string actual = d.getStringRepr(); 
		std::ostringstream tmp;
		correctOut.save(tmp, arma::csv_ascii); 
		std::string expected = tmp.str();
		INFO("Expected: " << expected << "Actual: " << actual); 
		REQUIRE(actual==expected); 
	}
	
}

TEST_CASE("GMMDatum"){

	SECTION("Correctly hang onto raw vector"){
		GMMDatum d; 
		vector<double> inputVec{0.1, 2, 3, 4.1, 5.7, 6.4}; 
		for (auto i: inputVec){
			d.record(i); 
		}
		rowvec outputVec = d.getRawData(); 
		REQUIRE(std::equal(inputVec.begin(), inputVec.end(), outputVec.begin())==true);
	}

	SECTION("Correctly hang onto raw vector when resizing _rawData"){
		GMMDatum d(2, 10);
		vector<double> inputVec(20); 
		for (unsigned i=0; i<20; i++){
			d.record(i); 
			inputVec[i] = i; 
		}		
		rowvec outputVec = d.getRawData(); 
		REQUIRE(std::equal(inputVec.begin(), inputVec.end(), outputVec.begin())==true);
	}

	SECTION("Correctly estimate two fed-in gaussians"){
		GMMDatum d(2, 2000); 
		for (unsigned i=0; i<10000; i++){
			// 3:1 ratio, so hefts should be .25 and .75
			d.record(RNG::rnorm(0, 1)); 
			d.record(RNG::rnorm(0, 1)); 
			d.record(RNG::rnorm(0, 1)); 
			d.record(RNG::rnorm(5, 2)); 
		}
		rowvec means = d.getGaussMeans(); 
		// means can be in either order
		bool firstMeanIs0 = means[0] == Approx(0).epsilon(0.1);
		if (firstMeanIs0){
			REQUIRE(means[1] == Approx(5).epsilon(0.1)); 
		} else {
			REQUIRE(means[1] == Approx(0).epsilon(0.1)); 
		}
		rowvec vars = d.getGaussVars(); 
		// vars can be in either order
		bool firstVarIs1 = vars[0] == Approx(1).epsilon(0.1);
		if (firstVarIs1){
			REQUIRE(vars[1] == Approx(4).epsilon(0.1)); 
		} else {
			REQUIRE(vars[1] == Approx(1).epsilon(0.1)); 
		}
		rowvec hefts = d.getGaussWeights(); 
		bool firstHeftIs34 = hefts[0] == Approx(0.75).epsilon(0.01);
		if (firstHeftIs34){
			REQUIRE(hefts[1] == Approx(0.25).epsilon(0.01)); 
		} else {
			REQUIRE(hefts[1] == Approx(0.75).epsilon(0.01)); 
		}
	}
}


TEST_CASE("Tests for Recorder"){
	Recorder r; 
	
	SECTION("Throws trying to record unregistered data"){
		REQUIRE_THROWS(r.updateDatum("bad", 10.0)); 
	}

	SECTION("Throws trying to get unregistered data"){
		REQUIRE_THROWS(r.getDatum<RawVectorsDatum<double> >("bad")); 
	}

	SECTION("Throws trying to register already registered data of same kind"){
		r.registerDatum("d", RawVectorsDatum<double>());
		REQUIRE_THROWS(r.registerDatum("d", RawVectorsDatum<double>()));
	}

	SECTION("Throws trying to register already registered data of different kind"){
		r.registerDatum("d", RawVectorsDatum<double>());
		REQUIRE_THROWS(r.registerDatum("d", DummyDatum<double>())); 
	}

	SECTION("DummyDatum"){
		r.registerDatum("rvd", DummyDatum<double>()); 
		vector<double> inputVec{0.1, 2, 3, 4.1, 5.7, 6.4}; 

		for (auto i: inputVec){
			r.updateDatum("rvd", i); 
		}

	}

	SECTION("RawVectorDatum"){

		r.registerDatum("rvd", RawVectorsDatum<double>()); 
		vector<double> inputVec{0.1, 2, 3, 4.1, 5.7, 6.4}; 

		for (auto i: inputVec){
			r.updateDatum("rvd", i); 
		}

		RawVectorsDatum<double> d; 

		d = r.getDatum<RawVectorsDatum<double> >("rvd"); 

		SECTION("Record mean correctly"){
			REQUIRE(d.getMean() == Approx(3.55)); 
		}

		SECTION("Record variance correctly"){
			REQUIRE(d.getVariance() == Approx(5.531)); 	
		}

		SECTION("Record N correctly"){
			REQUIRE(d.getN() == 6); 
		}	

	}

	SECTION("IncrementalMeanVarianceDatum"){

		r.registerDatum("rvd", IncrementalMeanVarianceDatum<double>()); 
		vector<double> inputVec{0.1, 2, 3, 4.1, 5.7, 6.4}; 

		for (auto i: inputVec){
			r.updateDatum("rvd", i); 
		}

		IncrementalMeanVarianceDatum<double> d; 

		d = r.getDatum<IncrementalMeanVarianceDatum<double> >("rvd"); 

		SECTION("Record mean correctly"){
			REQUIRE(d.getMean() == Approx(3.55)); 
		}

		SECTION("Record variance correctly"){
			REQUIRE(d.getVariance() == Approx(5.531)); 	
		}

		SECTION("Record N correctly"){
			REQUIRE(d.getN() == 6); 
		}	

	}

	SECTION("TraceDatum"){
		r.registerDatum("td", TraceDatum());
		TraceDatum d;

		arma::vec times{0, 1, 2, 3, 4.4, 6, 3.5, 4, 6, 1, 2, 3}; 
		vector<arma::vec> vals(12); 
		arma::vec traceIds{0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2};
		arma::mat correctOut(12, 6); 
		correctOut.col(0) = traceIds;
		correctOut.col(1) = times; 
		for (unsigned i = 0; i < vals.size(); ++i){
			vals[i].randu(4); 
			for (unsigned j = 0; j < 4; ++j){
				correctOut(i, 2+j) = vals[i][j]; 
			}
		}
		r.newTrial(); 
		r.updateDatum("td", Timepoint(times[0], vals[0])); 

		for (unsigned i = 1; i< times.size(); ++i){
			if (traceIds[i] > traceIds[i-1]) r.newTrial(); 
			r.updateDatum("td", Timepoint(times[i], vals[i])); 
		}
		d = r.getDatum<TraceDatum>("td"); 
		arma::mat out = d.getTraces();



		INFO("Correct =\n" << correctOut);
		INFO("Actual =\n" << out);

		REQUIRE(all(vectorise(correctOut)==vectorise(out)));

	}
	SECTION("EventDatum"){
		r.registerDatum("ed", EventDatum());
		EventDatum d;

		arma::vec starts1{100, 200}; 
		arma::vec ends1{101.5, 200}; 

		r.newTrial(); 

		arma::mat correctOut; 
		correctOut << 0 << 0 << 1 <<  1 << arma::endr << 100 << 200 << 300.5 << 400.5 << arma::endr << 101.5 << 200 << 350.5 << 10000 ;

		for (unsigned i=0; i< starts1.size(); ++i){
			r.updateDatum("ed", Event(starts1[i], ends1[i])); 
		}

		arma::vec starts2{300.5, 400.5}; 
		arma::vec ends2{350.5, 10000}; 

		r.newTrial(); 

		for (unsigned i=0; i< starts2.size(); ++i){
			r.updateDatum("ed", Event(starts2[i], ends2[i])); 
		}

		d = r.getDatum<EventDatum>("ed"); 
		arma::mat out = d.getEventTimes();

		INFO("Correct =\n" << correctOut.t());
		INFO("Actual =\n" << out);

		REQUIRE(all(vectorise(correctOut.t())==vectorise(out)));
	}
}