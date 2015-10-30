#ifndef TASK_H
#define TASK_H

class Config;
class Recorder;

#include <armadillo>
#include <string>
#include "architecture.h"
#include "belief.h"

using arma::mat; 
using std::to_string; 

/**
 * @brief Task parent class. 
 * @details Subclass from here. Two things are needed for a task: 
 *  first, an event loop in run(). Second, _traceDatumNames, 
 *  _summaryDatumNames and _eventDatumNames for things that run() will record. 
 *  The relevant subclass of Experiment will take care of registering those 
 *  Datum's with Recorder. 
 */
class Task{
public:
    Task(const Config * c, Recorder * r);
    virtual void run() = 0; ///< define me in children
    void drawTrialType();
    int getCurrentContext();
    int getCurrentTarget();
    std::vector<std::string> getTraceDatumNames();
    std::vector<std::string> getSummaryDatumNames();
    std::vector<std::string> getEventDatumNames();

protected: 
    void _checkTrialDistProperness();
    const Config * _config; ///< pointer to the config object
    Recorder * _recorder; ///< pointer to recorder 
    mat _trialDist; ///< distribution of stimuli to show in trials
    int _context; ///< holds context for current trial
    int _target; ///< holds target for current trial
    std::string _trialLabel; ///< string label for current trial type (used to access Datums in Recorder)
    std::vector<std::string> _traceDatumNames = {}; ///< names of datums that should use TraceDatum() (Experiment uses them to set up the Recorder)
    std::vector<std::string> _summaryDatumNames = {}; ///< names of datums that should use some SummaryDatum() (Experiment uses them to set up the Recorder)
    std::vector<std::string> _eventDatumNames = {}; ///< names of datums that should use some EventDatum() (Experiment uses them to set up the Recorder)

};

/**
 * @brief Task stub for testing. 
 */
class TaskStub: public Task{
public:
    TaskStub(const Config * c, Recorder * r);
    virtual void run(); 
};


#endif
