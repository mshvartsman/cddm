// include guard
#ifndef CONFIG_H
#define CONFIG_H

// no forward declarations needed

// includes we need
#include <armadillo>
#include <map>
#include <string>
#include "fatal_error.h"
#include "external/ConfigFile/ConfigFile.h"

using arma::mat; 
using std::string; 

/**
 * @brief Configuration class. 
 * @details Wraps ConfigFile and augments it to be able to save/load matrices, 
 * and to save/load from strings. 
 */
class Config{
public:
    
    Config();
    void loadFromFile(string filename, string delimiter = "=", string comment = "#", string sentry = "EndConfigFile" );
    void loadFromString( string conf);
    
    void save(std::string filename);
    std::string stringRepr(); 

    template<typename T> void set(const string key, const T & value);
    
    template<typename T> T get(const string key) const;

    void unset(const string & key); 

    bool keyExists(const string & key) const;
    

protected:
    
    ConfigFile _cf; ///< internal handle to the ConfigFile object

};

/**
 * @brief Templated setter for the underlying ConfigFile. 
 */
template<typename T> void Config::set(string key, const T & value){
    _cf.add<T>(key, value);
}

/**
 * @brief Templated getter for the underlying ConfigFile. 
 */
template<typename T> T Config::get(string key) const{
    return _cf.read<T>(key);
}



/**
 * @brief provides specialization for ConfigFile::string_as_T for mat
 * @details Allows loading armadillo matrices from ConfigFile. 
 * 
 * @param s matlab-style string representation of an armadillo matrix: 
 *  val, val; 
 *  val, val;
 */
template<>
inline mat ConfigFile::string_as_T( const string& s )
{
    // Convert from a string to a matrix (using arma initialization from matlab-style notation)
    return mat(s); 
}


/**
 * @brief provides specialization for ConfigFile::T_as_string for mat
 * @details Allows saving armadillo matrices into ConfigFile. 
 */
template<>
inline string ConfigFile::T_as_string( const mat& t )
{
    std::ostringstream ost;
    for (unsigned i=0; i<t.n_rows; ++i){
        for (unsigned j=0; j<t.n_cols; ++j){
            ost << t(i,j);
            if (j < (t.n_cols-1)) ost << ", "; 
        }
        if (i < (t.n_rows-1)) ost << "; "; 
    }
    return ost.str(); 
}

#endif