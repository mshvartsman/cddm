#include "catch_main.h"
#include "../task.h"
#include "../config.h"
#include "../recorder.h"
#include <armadillo>

using arma::mat; 

class DummyTask : public Task{
public: 
	DummyTask(Config * c, Recorder * r); 
	void run(); 
};

DummyTask::DummyTask(Config * c, Recorder * r): Task(c, r){}
void DummyTask::run(){
	std::cout << "I ran"; 
}

TEST_CASE("Task auxiliary methods"){
	arma::arma_rng::set_seed_random();
	Config conf; 
	mat correctTrialDist;
	correctTrialDist << 0.25 << 0.25 << arma::endr << 0.25 << 0.25;
	mat out(2,2); 
	conf.set("trialDist", correctTrialDist); 
	Recorder r; 
	DummyTask t(&conf, &r); 

	SECTION("drawTrials() asymptotically recovers trialDist"){
		for (unsigned i=0; i<10000; ++i){
			t.drawTrialType(); 
			out(t.getCurrentContext(), t.getCurrentTarget()) += 1;
		}
		out = out / 10000; 

		arma::uvec correctbool(4); 
		arma::vec correct = vectorise(correctTrialDist); 
		arma::vec actual = vectorise(out); 
		for (unsigned i=0; i<correctbool.size(); ++i){
			correctbool[i] = correct[i] == Approx(actual[i]).epsilon(0.01);
		} 
		REQUIRE(arma::all(correctbool==1));
	}
}
