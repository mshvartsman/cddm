#include "catch_main.h"
#include "../task.h"
#include "../recorder.h"
#include "../experiment.h"
#include "../config.h"
// #include "experiments/parallelAxcpt.h"

TEST_CASE("Batch Experiment test"){

	Config conf; 

	conf.set("timePerStep", 10.3); 
	conf.set("retentionIntervalDur", 20.6);
	conf.set("maxTrials", 1000); 
	conf.set("maxSamps", 100); 
	conf.set("contextNoise", 1); 
	conf.set("targetNoise", 1); 
	conf.set("decisionThresh", 0.95); 
	conf.set("eblMean", 50); 
	conf.set("motorPlanMean", 125);
	conf.set("motorExecMean", 150);
	conf.set("eblSd", 20); 
	conf.set("motorSd", 50); 
	conf.set("urPrior", "0.4 0.3; 0.2 0.1"); 
	conf.set("trialDist", "0.4 0.3; 0.2 0.1"); 
	conf.set("nContexts", "2"); 
	conf.set("nTargets", "2"); 
	

	Recorder r; 

	TaskStub t(&conf, &r); 

	BatchExperiment be(&conf, &t, &r); 

	be.run(); 

	// r.writeToFiles("BatchTest"); 

	conf.set("maxTrials", 5); 	

	r.reset(); 
	
	TraceExperiment te(&conf, &t, &r); 

	te.run(); 

	// r.writeToFiles("TraceTest"); 

}