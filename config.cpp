#include "config.h"

using std::string;

Config::Config(): _cf(ConfigFile()){}  

/**
 * @brief Load configuration from a comma separated string. 
 * @param conf a string containing key=value pairs separated by commas. 
 */
void Config::loadFromString(string conf){
	// lazy solution... paying this copy and replace cost once per parameter combo... 
	// TODO: test if this is affordable
	std::replace( conf.begin(), conf.end(), ',', '\n'); // replace all commas with newlines... now this should be a format ConfigFile likes
	std::stringstream tmp; 
	tmp << conf;
	_cf = ConfigFile(); 
	tmp >> _cf; 
}

/**
 * @brief Load configuration from INI-style file
 * @details Relying on ConfigFile for all the heavy lifting. 
 * @param filename name of config file
 * @param delimiter delimiter between keys and values (default =)
 * @param comment comment character (default #)
 * @param sentry end of config marker (default "EndOfFile")
 */
void Config::loadFromFile( string filename, string delimiter, string comment, string sentry){
	_cf = ConfigFile(filename, delimiter, comment, sentry);	
} 

/**
 * @brief Save configuration to an INI file. 
 */
void Config::save(std::string filename){
	std::ofstream f; 
	std::stringstream tmp; 
	tmp << _cf; 
	f.open(filename); 
	f << tmp.str(); 
	f.close(); 
}

/**
 * @brief Delete a key in config. 
 */
void Config::unset(const string & key){
	#ifndef DISABLE_ERROR_CHECKS
	if (!_cf.keyExists(key)) throw fatal_error() << "Trying to delete a key that doesn't exist! ("<<key << ")"; 
	#endif
	_cf.remove(key);
}

/**
 * @brief Dump a CSV representation of the config. 
 * @return "key=value,key=value"
 */
std::string Config::stringRepr(){
	std::stringstream tmp; 
	tmp << _cf; 
	std::string out = tmp.str(); 
	std::replace(out.begin(), out.end(), '\n', ',');
	return out; 
}

/**
 * @brief Check if this key is set. 
 * @details Useful for throwing informative errors (rather than
 * relying on Config throwing).
 */
bool Config::keyExists(const string & key) const{
	return _cf.keyExists(key); 
}