#ifndef RECORDER_H
#define RECORDER_H

#include <string>
#include <memory>
#include <unordered_map>
#include <type_traits>
#include <numeric>
#include <cmath>
#include <armadillo> 

#include "utils.h"
#include "fatal_error.h"

using std::string;
using arma::rowvec;

/**
 * @brief A class that lets us abstract from the type of observations Recorder holds. 
 * @details IDatum is not templated so we can stick it in std::map, even though we're 
 * actually sticking its templated subclasses in. 
 * \sa http://stackoverflow.com/questions/24702235/c-stdmap-holding-any-type-of-value for the cleverness used. 
 */
class IDatum {
public: 
	~IDatum() = default;
	/// Record the beginning of a new trial (in subclasses)
	virtual void newTrial() {}; 
	/// return a string holding CSV of the datum (in subclasses)
	virtual std::string getStringRepr() = 0;
}; 

/**
 * @brief Templated abstract base class for things we might put into Recorder. 
 * @tparam T A type of observation we wrap in a Datum. This might be POD, or a
 * more complex struct (like Event or Timepoint).
 */
template<typename T>
class Datum : public IDatum {
public:
	/// Record this datum (implemented in subclasses). 
	virtual void record(T val) = 0;
};

/**
 * @brief Datum that keeps track of summaries (mean and variance for now) of incoming observations. 
 * @details Should work transparentely for POD types for which we can
 * compute means and variances using summation, division etc. Can specialize
 * for other types we can summarize with mean and variance. 
 * 
 * @tparam T A type we can summarize with mean and variance. 
 */
template<typename T>
class SummaryDatum : public Datum<T> {
public:
	virtual T getMean() = 0; ///< Return the mean of this set of observations. Defined in subclasses 
	virtual T getVariance() = 0; ///< Return the variance of this set of observations. Defined in subclasses 
	virtual int getN() = 0; ///< Return the number of observations in this set. Defined in subclasses 
};

/**
 * @brief Learns a gaussian mixture model of incoming double observations. 
 * @details Keeps mean and variance, but also adds a gaussian mixture model 
 * of incoming observations using armadillo's gmm_diag (assuming diagonal 
 * covariance between the Gaussians). 
 */
class GMMDatum : public SummaryDatum<double> {
public:
	GMMDatum(int ngauss=2, int expectedNObs=1000); 
	virtual double getMean(); 
	virtual double getVariance();
	virtual void record(double val); 
	virtual std::string getStringRepr(); 
	virtual int getN(); 
	virtual rowvec getGaussMeans(); 
	virtual rowvec getGaussVars(); 
	virtual rowvec getGaussWeights(); 
	virtual rowvec getRawData(); 
protected: 
	void _estimateModel(); 
	int _n; ///< number of observations
	rowvec _rawData;  ///< the raw observations
	arma::gmm_diag _model; ///< the GMM object
	int _ngauss; ///< number of gaussians to fit
	bool _estimateIsFresh; ///< has the GMM been updated since the latest observation? 
};

/**
 * @brief Datum to store its raw vector of observations. 
 * @details This is useful for generating traces, but should mostly be
 * avoided in large scale simulation. 
 * @tparam T Some type of observation we can store in a vector. 
 */
template<typename T>
class RawVectorsDatum : public SummaryDatum<T> {
public: 
	RawVectorsDatum();
	virtual void record(T val); 
	virtual T getMean(); 
	virtual T getVariance(); 
	virtual int getN(); 
	vector<T> getRawData(); 
	virtual std::string getStringRepr(); 
	void newTrial();

protected: 
	vector<T> _rawData; ///< stores the raw data 
	vector<int> _traceIds; ///< Trial/trace IDs associated with the individual data points
	int _latestTraceId = -1; ///< ID of the latest trial (initialized at -1 because newTrial will be called to set it to 0)
};

/**
 * @brief Does nothing. 
 * @details This is useful for disabling recording of things we don't need in a 
 * way that is transparent to task implementations. 
 * 
 * @tparam T anything 
 */
template<typename T>
class DummyDatum : public Datum<T> {
public: 
	virtual void record(T val); 
	virtual std::string getStringRepr(); 
};

/**
 * @brief Computes mean and variance incrementally without storing all the observations. 
 * @details Keeps track of the mean and sum of square differences over time, so mean and 
 * variance can be computed at any timepoint. 
 * 
 * @tparam T arithmetic type. 
 */
template<typename T>
class IncrementalMeanVarianceDatum : public SummaryDatum<T> {
public: 
	IncrementalMeanVarianceDatum(); 
	virtual void record(T val); 
	virtual T getMean(); 
	virtual T getVariance(); 
	virtual int getN(); 
	virtual std::string getStringRepr(); 

protected: 
	T _mean; ///< mean so far
	T _ssq;  ///< sum of square deviations so far
	int _n;  ///< number of observations so far. 
};

/**
 * @brief Container class for vector-valued data stored at each timepoint. 
 * @details One good thing to store in this are posteriors at each timepoint. 
 */
class Timepoint {
public: 
	Timepoint(double t, arma::vec v); 
	double time; ///< timestamp (in ms) of this timepoint
	arma::vec value; ///< a recorded vector value at this timepoint (e.g. a posterior)
};

/**
 * @brief Container class for events. 
 * @details Events have starts and ends. Use this to be able to create sequence 
 * diagram visualizations of the model. 
 */
class Event {
public: 
	Event(double start, double end); 
	double startTime; ///< Time (ms) this event starts
	double endTime; ///< Time (ms) this event ends
};

/**
 * @brief Holds events ([start_time,end_time] pairs) 
 */
class EventDatum : public Datum<Event>{
public:
	virtual void record(Event val);
	arma::mat getEventTimes(); 
	virtual std::string getStringRepr(); 
	virtual void newTrial(); 
	arma::mat getMatRepr(); 
protected:
	vector<double> _startTimes;  ///< event start times
	vector<double> _endTimes;  ///< event end times
	vector<int> _traceIds; ///< trace (trial) IDs associated with the start-end pairs
	int _latestTraceId = -1;  ///< ID of the latest trial (initialized at -1 because newTrial will be called to set it to 0)
};

/**
 * @brief Holds timepoint traces (each a vector, indexed by timepoint)
 */
class TraceDatum : public Datum<Timepoint>{
public:
	virtual void record(Timepoint val);
	arma::mat getTraces();
	virtual std::string getStringRepr();  
	virtual void newTrial(); 
	arma::mat getMatRepr(); 
protected:
	vector<arma::vec> _values; ///< the raw timepoint traces. Outer is a std::vector for efficient push_back(), inner arma::vec to capture vectorise()'d belief matrices. 
	vector<double> _times; ///< timestamps of the timepoints 
	vector<int> _traceIds; ///< trace (trial) ids of the timepoints
	int _latestTraceId = -1;  ///< ID of the latest trial (initialized at -1 because newTrial will be called to set it to 0)
};

/**
 * @brief Recorder supports recording and storing arbitrary types from the simulator. 
 */
class Recorder {
public:
	template<typename T> void registerDatum(const string & key, const T & ex); 
	template<typename T> T getDatum(const string & key); 
	template<typename T> void updateDatum(const string & key, const T & val); 
	void printKnownKeys(); ///< mostly for debugging
	void writeToFiles(string basedir); 
	virtual bool recordedEnough(); 
	void newTrial(); 
	void reset(); 

protected:
	typedef std::unordered_map<string,std::unique_ptr< IDatum > >::iterator umapi; ///< iterator for our map of datums
	std::unordered_map<string,std::unique_ptr< IDatum > > _contents; ///< a hash map of IDatum holding all of our templated Datums. 
	bool _empty; ///< \todo remove _empty, it is not used
}; 

/**
 * @brief Tell recorder about a datum. 
 * @details Recorder needs to know the data it will be recording, identified
 * by a string label and a Datum type. 
 * 
 * @param key The string key by which this datum can be accessed for read/write. 
 * @param ex An empty example of the kind of Datum this key points to 
 * (e.g. EventDatum()) so that we can do memory allocation. 
 */
template<typename T>
void Recorder::registerDatum(const string & key, const T & ex){
	#ifndef DISABLE_ERROR_CHECKS
	if (_contents.find(key) != _contents.end()) throw fatal_error() << "ERROR: attempting to register datum " << key << " which was already registered!"; 
	#endif
	_contents[key] = std::unique_ptr<T>(new T); 
}

/**
 * @brief Returns a datum by name. 
 * @param key a string-valued name for the datum we want
 * @tparam T the type we should return. Casting up or down the Datum class 
 * hierarchy is supported and intended. 
 */
template<typename T>
T Recorder::getDatum(const string & key){
	#ifndef DISABLE_ERROR_CHECKS
	if (_contents.find(key)==_contents.end()) throw fatal_error() << "ERROR: attempting to get datum " << key << " which was not registered to the recorder!"; 
	#endif

	return *static_cast<T*>(_contents[key].get());
}

/**
 * @brief Update a datum with a new value. 
 * @param key name of the datum
 * @param val value to record
 */
template<typename T>
void Recorder::updateDatum(const string& key, const T & val){
	#ifndef DISABLE_ERROR_CHECKS
	if (_contents.find(key)==_contents.end()) throw fatal_error() << "ERROR: attempting to update datum " << key << " which was not registered to the recorder!"; 
	#endif
	static_cast<Datum<T>*>(_contents[key].get())->record(val); 
}

/**
 * @brief Does nothing.
 */
template<typename T>
void DummyDatum<T>::record(T val){};

/**
 * @brief Does nothing.
 * @return an empty string. 
 */
template<typename T>
std::string DummyDatum<T>::getStringRepr(){
	return std::string(""); 
};

/**
 * @brief Initialize a vector of T
 * @tparam T type that can be stored in a std::vector
 */
template<typename T>
RawVectorsDatum<T>::RawVectorsDatum() {
	_rawData = vector<T>(); 
}

/**
 * @brief Return the mean of the observation vector. 
 * @details Uses the kahan summation algorithm, should be pretty accurate. 
 */
template<typename T>
T RawVectorsDatum<T>::getMean(){
	return utils::mean(_rawData);
	
}

/**
 * @brief Return the variance of the observation vector
 * @details Uses the kahan summation algorithm, should be pretty accurate. 
 */
template<typename T>
T RawVectorsDatum<T>::getVariance(){
	return utils::variance(_rawData);
}

/**
 * @brief Add the current value to the vector. 
 */
template<typename T>
void RawVectorsDatum<T>::record(T val){
	_traceIds.push_back(_latestTraceId);
	_rawData.push_back(val); 
}

/**
 * @brief Return the number of observations so far. 
 */
 template<typename T>
int RawVectorsDatum<T>::getN(){
	return _rawData.size();  
}

/**
 * @brief Return the raw observation vector so far. 
 */
template<typename T>
vector<T> RawVectorsDatum<T>::getRawData(){
	return _rawData; 
}

/**
 * @brief Return a comma-separated string representation of the observation vector. 
 */
template<typename T>
std::string RawVectorsDatum<T>::getStringRepr(){
	std::ostringstream out; 
	// assume one per trial ID
	for (unsigned i = 0; i<_rawData.size(); ++i){
		out << _traceIds[i] << ","<< _rawData[i] << std::endl; 
	}
	return out.str(); 
}

/**
 * @brief Register a new trial.
 * @details Increments traceID. 
 */
template<typename T>
void RawVectorsDatum<T>::newTrial(){
	++_latestTraceId; 
}

template<typename T>
IncrementalMeanVarianceDatum<T>::IncrementalMeanVarianceDatum(): _mean(0), _ssq(0), _n(0) {}

/**
 * @brief Return the mean of the observations so far. 
 */
template<typename T>
T IncrementalMeanVarianceDatum<T>::getMean(){
	return _mean; 
}

/**
 * @brief Return the variance of the observations so far. 
 */
template<typename T>
T IncrementalMeanVarianceDatum<T>::getVariance(){
	return _ssq / (_n-1); 
}

/**
 * @brief Update the estimates of mean and variance so far
 * @details \sa https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
 */
template<typename T>
void IncrementalMeanVarianceDatum<T>::record(T val){
	T oldMean = _mean; 
	++_n;
	_mean += _n == 0 ? val : (val - oldMean) / _n;
	_ssq += (val - oldMean) * (val - _mean);
}

/**
 * @brief Return the number of observations in the datum. 
 */
template<typename T>
int  IncrementalMeanVarianceDatum<T>::getN(){
	return _n; 
}

/**
 * @brief Return a CSV string representation of the datum. 
 * @return "mean,variance,n"
 */
template<typename T>
std::string IncrementalMeanVarianceDatum<T>::getStringRepr(){
	std::ostringstream out; 
	out << _mean << "," << (_ssq / (_n-1)) << "," << _n << std::endl; 
	return out.str(); 
}


#endif