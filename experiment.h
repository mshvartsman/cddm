#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <armadillo>
#include <string>
#include "config.h"
#include "recorder.h"
#include "task.h"

using arma::mat; 
using std::to_string; 

/**
 * @brief Superclass for experiments. 
 * @details All experiment types subclass from here, and differ in (a) 
 * how they set up the Recorder, and (b) how they batch out Trial%s. For example, 
 * to parallelize things you might write an Experiment subclass whose run() method
 * does #pragma omp parallel for or some MPI cleverness. Likewise, you might put
 * together an Experiment that only keeps track of observations you care about, 
 * dumping the rest. 
 */
class Experiment{
public: 
	void run(); 

protected:
	Experiment(Config * conf, Task * t, Recorder * r);  // instantiate me via my children who set up the recorder
	Config * _config; ///< pointer to the configuration object
	Task * _task; ///< pointer to a subclass of Task
	Recorder * _recorder; ///< pointer to the recorder
	int _maxTrials; ///< stop when this many trials are run or Recorder says stop. 
};

/**
 * @brief Subclass for "batch" experiments. 
 * @details This records things as minimally and possible: SummaryDatum%s only, for 
 * trial-level variables (RT, accuracy, etc) only. For use in batch simulation / 
 * parameter fitting. 
 */
class BatchExperiment : public Experiment {
public: 
	BatchExperiment(Config * conf, Task * t, Recorder * r); 


};

/**
 * @brief Subclass for trace experiments. 
 * @details Records raw vectors for everything, including posterior trajectories. 
 * For use in inspection/debugging and visualization. 
 */
class TraceExperiment : public Experiment {
public: 
	TraceExperiment(Config * conf, Task * t, Recorder * r); 
};


/**
 * @brief Subclass for event experiments. 
 * @details Records all events but not posterior trajectories. This is useful for 
 * looking at raw conditional RT distributions (and other things like motor planning
 * times) but not necessarily trajectories. 
 */
class EventExperiment : public Experiment {
public: 
	EventExperiment(Config * conf, Task * t, Recorder * r); 


};


#endif