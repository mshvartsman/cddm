#include "recorder.h"
#include <armadillo> 
#include <iostream>
#include <sys/stat.h>

using arma::mat; 
using arma::vec; 
/**
 * @brief Return true if we have enough data. 
 * @details Subclass from this if you want to stop sampling when some condition is hit
 * (for example, standard errors shrink enough). 
 * @return false -- might return true in subclasses
 */
bool Recorder::recordedEnough(){ 
	return false; 
} 

/**
 * @brief Tell all the datums we started a new trial. 
 * @details Some types of data we record care about the start of a new trial
 * (e.g. trajectories) so this is where we tell them about it. 
 */
void Recorder::newTrial(){
	for (umapi it = _contents.begin(); it != _contents.end(); ++it){
		it->second->newTrial(); 
	}
}

/**
 * @brief Return all the keys (and therefore data) recorder knows.
 */
void Recorder::printKnownKeys(){
	for (umapi it = _contents.begin(); it != _contents.end(); ++it){
		std::cout << it->first << " "; 
	}	
}

/**
 * @brief Dump all datums in recorder to CSV. 
 * @details This calls getStringRepr on each datum. 
 * 
 * @param basedir directory of where all the CSVs go. 
 */
void Recorder::writeToFiles(string basedir){
	std::ofstream f; 
	// I think this creates the dir as permission 775
	mkdir(basedir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	for (umapi it = _contents.begin(); it != _contents.end(); ++it){
		std::string filename = basedir + "/" + it->first + ".csv"; 
		f.open(filename); 
		f << it->second->getStringRepr(); 
		f.close(); 
	}
}

/**
 * @brief Delete both the data and known data types. 
 */
void Recorder::reset(){
	_contents.clear(); 
}

/**
 * @param t the time of the observation. 
 * @param v the vector-valued observation.
 */
Timepoint::Timepoint(double t, vec v): time(t), value(v) {} 

/**
 * @param start event start time
 * @param end event end time
 * \todo link from here to docs on sequence diagrams when there's something
 * standardized for creating them. 
 */
Event::Event(double start, double end): startTime(start), endTime(end) {
	#ifndef DISABLE_ERROR_CHECKS
		if(start > end) throw fatal_error() << "Start time is later than end time! Start: " << start << ", end: " << end; 
	#endif
}

/**
 * @brief Record an event. 
 */
void EventDatum::record(Event val){
	_traceIds.push_back(_latestTraceId);
	_startTimes.push_back(val.startTime);
	_endTimes.push_back(val.endTime);
}

/**
 * @brief Return the times of all the events this EventDatum knows. 
 * @details This is really useful for being able to dump out all 
 * the known events. 
 * @return A matrix with 3 columns and as many rows as we have events. 
 * First column is the trace (trial) the event is in, second column
 * is the start, and third column is the end. 
 */
mat EventDatum::getEventTimes(){
	if (_startTimes.empty()){
		return mat(); 
	}
	mat out(_startTimes.size(), 3); 
	for (unsigned i=0; i<_startTimes.size(); ++i){
		out(i,0) = _traceIds[i];
		out(i,1) = _startTimes[i];
		out(i,2) = _endTimes[i]; 
	}
	return out; 
}

/**
 * @brief Tell EventDatum that we started a new trial. 
 */
void EventDatum::newTrial(){
	++_latestTraceId; 
}

/**
 * @brief Return a string representation of this event datum. 
 * @details Used in data dumps. 
 * @return A CSV dump of the output of EventDatum::getEventTimes(). 
 */
std::string EventDatum::getStringRepr(){
	mat tmp = getEventTimes(); 
	std::ostringstream out; 
	tmp.save(out, arma::csv_ascii); 
	return out.str(); 
}

/**
 * @brief Return a matrix representation of the datum. 
 * @details Aliased to EventDatum::getMatRepr() here but 
 * might be used differently in other Datums. 
 */
mat EventDatum::getMatRepr(){
	return getEventTimes(); 
}

/**
 * @brief Record a new timepoint to our trace. 
 * @details Records the timepoint and the trace it came from. 
 * @param val Timepoint to record. 
 */
void TraceDatum::record(Timepoint val){
	_traceIds.push_back(_latestTraceId);
	_times.push_back(val.time); 
	_values.push_back(val.value); 	
	// if (_traceIds.empty()){
	// 	_traceIds.push_back(0); 
	// 	return; 
	// }
	// int traceId = _traceIds.back(); 
	// if(_times.back() >= val.time){ // if our time index is equal or lower than the last index then we start a new trace:
	// 	_traceIds.push_back(traceId+1); 
	// }  else {
	// 	_traceIds.push_back(traceId); 
	// }
	// _times.push_back(val.time); 
	// #ifndef DISABLE_ERROR_CHECKS
	// 	if(val.value.size() != _values.back().size()) throw fatal_error() << "ERROR: current value for this TraceDatum is of length " << val.value.size() << " but the pervious one recorded was of length" << _values.back().size() << "! That is almost certainly a mistake and TraceDatum's getTraces() method can't handle it!";
	// #endif
	// _values.push_back(val.value); 
}

/**
 * @brief Get all the traces this datum knows about. 
 * @return a matrix with as many rows as timepoints, and as many columns as 
 * there are values in the TimePoint vector for this TraceDatum, +2. The 
 * first two columns are the trace (trial) ID and trial time, and the remaining
 * columns are the vectors at each timepoint (for example, posteriors). 
 */
mat TraceDatum::getTraces(){
	// this is not the most efficient way to do this... but is the fastest to write and this will at most get called a handful of times
	// #ifndef DISABLE_ERROR_CHECKS
		// if (_traceIds.empty()) throw fatal_error() << "Attempting to get traces but none were recorded!";
	// #endif
	if (_traceIds.empty()){
		return mat(); 
	}
	mat out(_times.size(), 2+_values.back().size()); 
	for (unsigned i=0; i<_times.size(); ++i){
		out(i,0) = _traceIds[i]; 
		out(i,1) = _times[i];
		for (unsigned j=0; j < _values.back().size(); ++j){
			out(i,2+j) = _values[i][j]; 
		}
	}
	return out; 
}

/**
 * @brief Return a string (CSV) representation of this TraceDatum. 
 * @details A string dump of TraceDatum::getTraces() 
 */
std::string TraceDatum::getStringRepr(){
	mat tmp = getTraces(); 
	std::ostringstream out; 
	tmp.save(out, arma::csv_ascii); 
	return out.str(); 
}


/**
 * @brief Tell TraceDatum we started a new trial. 
 * @details Increments the current traceID
 */
void TraceDatum::newTrial(){
	++_latestTraceId; 
}

/**
 * @brief Return a matrix representation of the datum. 
 * @details Aliased to TraceDatum::getMatRepr() here but 
 * might be used differently in other Datums. 
 */
mat TraceDatum::getMatRepr(){
	return getTraces(); 
}

/**
 * @brief A datum that estimates a gaussian mixture model from its incoming data stream. 
 * @details This provides a compact nonparametric distribution of anything we might want 
 * to record (e.g. RTs). 
 * 
 * @param ngauss number of gaussians. 
 * @param expectedNObs number of observations we expect (a good guess helps save us some memory allocations).
 */
GMMDatum::GMMDatum(int ngauss, int expectedNObs): _ngauss(ngauss), _estimateIsFresh(false), _n(0){
	_rawData = rowvec(expectedNObs); 
	_model = arma::gmm_diag();
}


/**
 * @brief Return the mean of the current observation set. 
 * @return Mean of the observations (using kahan sum to keep floats precise)
 */
double GMMDatum::getMean(){
	return utils::mean(_rawData); // using kahan sum on the back end
}

/**
 * @brief Return the variance of the current observation set. 
 * @return Variance of the observations (using kahan sum to keep floats precise)
 */
double GMMDatum::getVariance(){
	return utils::variance(_rawData); 
}

/**
 * @brief Record a new value. 
 * @details Also marks our latest GMM estimate as not fresh, 
 * and resizes the observation vector if needed. 
 */
void GMMDatum::record(double val){
	// if we run out of space, double the space
	if (_rawData.n_elem == _n) _rawData.resize(_n*2); 
	_rawData[_n++] = val; 
	_estimateIsFresh = false; 
}

/**
 * @brief returns CSV string representation of the GMM
 * @details Returns a CSV with a header and one row per gaussian, 
 * with columns mean, variance, weight.
 */
std::string GMMDatum::getStringRepr(){
	_estimateModel();
	std::ostringstream out; 
	out << "mean,variance,weight" << std::endl; 
	for (int i = 0; i<_ngauss; i++){
		out << _model.means[i] << "," << _model.dcovs[i] << "," << _model.hefts[i] << std::endl; 
	}	
	return out.str(); 
}

/**
 * @brief Return the number of observations in this datum. 
 */
int GMMDatum::getN(){
	return _n; 
}

/**
 * @brief Estimate a gaussian mixture model for the observations seen so far. 
 * @details Uses armadillo's gmm_diag. 
 */
void GMMDatum::_estimateModel(){
	if(_estimateIsFresh) return; 
	_model.learn(_rawData(arma::span(0,_n-1)), _ngauss, arma::maha_dist, arma::random_subset, 15, 15, 1e-10, false); 
	_estimateIsFresh = true; 
}

/**
 * @brief Returns the means of the gaussians estimated.
 */
rowvec GMMDatum::getGaussMeans(){
	_estimateModel();
	return _model.means; 
}

/**
 * @brief Returns the variances of the gaussians estimated.
 */
rowvec GMMDatum::getGaussVars(){
	_estimateModel();
	return _model.dcovs;
}

/**
 * @brief Returns the weights of the gaussians estimated.
 */
rowvec GMMDatum::getGaussWeights(){
	_estimateModel();
	return _model.hefts; 
}

/**
 * @brief Returns the observations seen so far. 
 */
rowvec GMMDatum::getRawData(){
	return _rawData(arma::span(0,_n-1)); 
}