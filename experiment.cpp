#include "experiment.h"

#include <string>
#include <vector>

using std::vector;
using std::string; 

/**
 * @brief Experiment Constructor. 
 * @param c Config containing (at minimum) \ref maxTrials
 * @param t A Task. 
 * @param r A Recorder. 
 * \warning Should always be called from child initializers. 
 */
Experiment::Experiment(Config * c, Task * t, Recorder * r): _config(c), _task(t), _recorder(r), _maxTrials(-1){
	_maxTrials = _config->get<int>("maxTrials"); 
}

/**
 * @brief Run trials in the task until maxTrials is hit or Recorder says we've had enough. 
 * \todo a simple place to parallelize with OpenMP is here, since trials can be run independently. 
 */
void Experiment::run(){
// this is where the #omp pragma parallel for can happen
	for (unsigned tr=0; tr<_maxTrials; tr++){
		_recorder->newTrial(); 
		_task->run(); 
		if (_recorder->recordedEnough()) break; 		
	}
}	

/**
 * @brief Constructor for BatchExperiment. 
 * @details TraceDatum and EventDatum are both set to DummyDatum to save space
 * and time in batch simulation. Only record trial-level summary information using
 * IncrementalMeanVarianceDatum. 
 * 
 * @param c Config, containing at least \ref maxTrials (required by parent), 
 * \ref nContexts, and \ref nTargets.
 * @param t A Task. 
 * @param r A recorder. 
 */
BatchExperiment::BatchExperiment(Config * c, Task * t, Recorder * r): Experiment(c, t, r) {
	int nContexts = _config->get<int>("nContexts");
	int nTargets = _config->get<int>("nTargets"); 
	vector<string> traceDatumNames = t->getTraceDatumNames(); 
	vector<string> summaryDatumNames = t->getSummaryDatumNames(); 
	vector<string> eventDatumNames = t->getEventDatumNames(); 
	for (unsigned i=0; i<traceDatumNames.size(); ++i){
		for (unsigned c = 0; c < nContexts; c++){
			for (unsigned t = 0; t< nTargets; t++){
				_recorder->registerDatum("Context" + to_string(c) + "_Target" + to_string(t) + "_" + traceDatumNames[i], DummyDatum<Timepoint>());
			}
		}
	}
	
	for (unsigned i=0; i<summaryDatumNames.size(); ++i){
		for (unsigned c = 0; c < nContexts; c++){
			for (unsigned t = 0; t< nTargets; t++){
				_recorder->registerDatum("Context" + to_string(c) + "_Target" + to_string(t) + "_" + summaryDatumNames[i], IncrementalMeanVarianceDatum<double>());
			}
		}
	}
	for (unsigned i=0; i<eventDatumNames.size(); ++i){
		for (unsigned c = 0; c < nContexts; c++){
			for (unsigned t = 0; t< nTargets; t++){
				_recorder->registerDatum("Context" + to_string(c) + "_Target" + to_string(t) + "_" + eventDatumNames[i], DummyDatum<Event>());
			}
		}
	}
}

/**
 * @brief Constructor for EventExperiment. 
 * @details This gives conditional RT distributions without storing belief traces. 
 * TraceDatum is set to DummyDatum to save space and time in simulation. 
 * Record events in EventDatum and trial-level summary information using IncrementalMeanVarianceDatum. 
 * 
 * @param c Config, containing at least \ref maxTrials (required by parent), 
 * \ref nContexts, and \ref nTargets.
 * @param t A Task. 
 * @param r A recorder. 
 */
EventExperiment::EventExperiment(Config * c, Task * t, Recorder * r): Experiment(c, t, r) {
	int nContexts = _config->get<int>("nContexts");
	int nTargets = _config->get<int>("nTargets"); 
	vector<string> traceDatumNames = t->getTraceDatumNames(); 
	vector<string> summaryDatumNames = t->getSummaryDatumNames(); 
	vector<string> eventDatumNames = t->getEventDatumNames(); 
	for (unsigned i=0; i<traceDatumNames.size(); ++i){
		for (unsigned c = 0; c < nContexts; c++){
			for (unsigned t = 0; t< nTargets; t++){
				_recorder->registerDatum("Context" + to_string(c) + "_Target" + to_string(t) + "_" + traceDatumNames[i], DummyDatum<Timepoint>());
			}
		}
	}
	
	for (unsigned i=0; i<summaryDatumNames.size(); ++i){
		for (unsigned c = 0; c < nContexts; c++){
			for (unsigned t = 0; t< nTargets; t++){
				_recorder->registerDatum("Context" + to_string(c) + "_Target" + to_string(t) + "_" + summaryDatumNames[i], RawVectorsDatum<double>());
			}
		}
	}
	for (unsigned i=0; i<eventDatumNames.size(); ++i){
		for (unsigned c = 0; c < nContexts; c++){
			for (unsigned t = 0; t< nTargets; t++){
				_recorder->registerDatum("Context" + to_string(c) + "_Target" + to_string(t) + "_" + eventDatumNames[i], EventDatum());
			}
		}
	}
}

/**
 * @brief Constructor for TraceExperiment. 
 * @details Records everything: belief traces, events, RTs, accuracies. Generates 
 * far more data than the others -- best to use for trial-level visualization but 
 * not for anything else. 

 * @param c Config, containing at least \ref maxTrials (required by parent), 
 * \ref nContexts, and \ref nTargets.
 * @param t A Task. 
 * @param r A recorder. 
 */
TraceExperiment::TraceExperiment(Config * c, Task * t, Recorder * r): Experiment(c, t, r) {
	int nContexts = _config->get<int>("nContexts");
	int nTargets = _config->get<int>("nTargets"); 
	vector<string> traceDatumNames = t->getTraceDatumNames(); 
	vector<string> summaryDatumNames = t->getSummaryDatumNames(); 
	vector<string> eventDatumNames = t->getEventDatumNames(); 
	for (unsigned i=0; i<traceDatumNames.size(); ++i){
		for (unsigned c = 0; c < nContexts; c++){
			for (unsigned t = 0; t< nTargets; t++){
				_recorder->registerDatum("Context" + to_string(c) + "_Target" + to_string(t) + "_" + traceDatumNames[i], TraceDatum());
			}
		}
	}
	
	for (unsigned i=0; i<summaryDatumNames.size(); ++i){
		for (unsigned c = 0; c < nContexts; c++){
			for (unsigned t = 0; t< nTargets; t++){
				_recorder->registerDatum("Context" + to_string(c) + "_Target" + to_string(t) + "_" + summaryDatumNames[i], RawVectorsDatum<double>());
			}
		}
	}
	for (unsigned i=0; i<eventDatumNames.size(); ++i){
		for (unsigned c = 0; c < nContexts; c++){
			for (unsigned t = 0; t< nTargets; t++){
				_recorder->registerDatum("Context" + to_string(c) + "_Target" + to_string(t) + "_" + eventDatumNames[i], EventDatum());
			}
		}
	}
}
