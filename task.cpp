#include "task.h"
#include <armadillo>
#include <string>
#include "rng.h"
#include "config.h"
#include "architecture.h"
#include "recorder.h"
#include "utils.h"

using arma::mat; 
using std::to_string; 

/**
 * @brief Draw a context and a target based on \ref trialDist. 
 */
void Task::drawTrialType(){
    // categorical draw:
    double p = RNG::runif(1);
    double probSoFar = 0;
    for (unsigned c = 0; c < _trialDist.n_rows; ++c){
        for (unsigned t = 0; t < _trialDist.n_cols; ++t){
            if (p <= (_trialDist(c, t) + probSoFar)){
                _context = c;
                _target = t;
                _trialLabel = "Context" + to_string(_context) + "_Target" + to_string(_target) + "_"; 
                return; 
            }
            probSoFar += _trialDist(c, t);
        }
    }
    #ifndef DISABLE_ERROR_CHECKS
    throw fatal_error() << "drawTrialType() fallthrough case! Should never get here! Currently p = " << p << ", probSoFar = " << probSoFar;
    #endif
}

/**
 * @brief Getter for context for current trial. 
 */
int Task::getCurrentContext(){
    return _context; 
}

/**
 * @brief Getter for target for current trial. 
 */
int Task::getCurrentTarget(){
    return _target; 
}

/**
 * @brief Getter for names of things we will record as TraceDatum
 */
std::vector<std::string> Task::getTraceDatumNames(){
    return _traceDatumNames;
}

/**
 * @brief Getter for names of things we will record as SummaryDatum
 */
std::vector<std::string> Task::getSummaryDatumNames(){
    return _summaryDatumNames;
}

/**
 * @brief Getter for names of things we will record as EventDatum
 */
std::vector<std::string> Task::getEventDatumNames(){
    return _eventDatumNames;
}

/**
 * @brief Verify that the distribution of trial (context,target) types sums to 1, else throw error
 */
void Task::_checkTrialDistProperness(){
    if (utils::kahanSum(_trialDist) != 1) throw fatal_error() << "trialDist is not proper! Actual sum: " << utils::kahanSum(_trialDist) << ", actual trialDist " << _trialDist; 
}

/**
 * @brief Constructor for Task. 
 * @param c Config, containing at least \ref trialDist. 
 * * @param r Recorder (empty). 
 */
Task::Task(const Config * c, Recorder * r):_context(-1), _target(-1){
    _config = c; 
    _recorder = r; 
    _trialDist = c->get<mat>("trialDist");
    #ifndef DISABLE_ERROR_CHECKS
    _checkTrialDistProperness(); 
    #endif
}

/**
 * @brief Constructor for TaskStub does nothing. 
 */
TaskStub::TaskStub(const Config * c, Recorder * r): Task(c, r) {}

/**
 * @brief Does nothing (used for testing Experiment)
 */
void TaskStub::run(){
    return; 
}