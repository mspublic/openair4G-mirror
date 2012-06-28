/*
 * mgmt_configuration.hpp
 *
 *  Created on: May 4, 2012
 *      Author: demiray
 */

#ifndef MGMT_CONFIGURATION_HPP_
#define MGMT_CONFIGURATION_HPP_

#include "mgmt_information_base.hpp"
#include <string>
using namespace std;

/**
 * A container with configuration file parsing capability, this class is utilised
 * to update ManagementInformationBase class with configuration file content
 */
class Configuration {
	public:
		/**
		 * Parameter string for UDP server port
		 */
		static const string CONF_SERVER_PORT_PARAMETER;

	public:
		/**
		 * Constructor for Configuration class
		 *
		 * @param configurationFile Configuration file name
		 */
		Configuration(string configurationFile);
		/**
		 * Destructor for Configuration class
		 */
		~Configuration();

	public:
		/**
		 * Parses configuration file and updates MIB thru passed reference
		 *
		 * @param mib Management Information Base reference
		 * @return true on success, false otherwise
		 */
		bool parseConfigurationFile(ManagementInformationBase& mib);
		/**
		 * Returns configuration file name
		 *
		 * @param none
		 * @return Configuration file name
		 */
		string getConfigurationFile() const;
		/**
		 * Sets configuration file name
		 *
		 * @param New configuration file name
		 * @return none
		 */
		void setConfigurationFile(string configurationFile);
		/**
		 * Returns UDP server port number
		 *
		 * @param none
		 * @return UDP server port number
		 */
		int getServerPort() const;
		/**
		 * Sets UDP server port number
		 *
		 * @param new port number
		 * @return none
		 */
		void setServerPort(int serverPort);

	private:
		/**
		 * Parses incoming string of format "parameter = value" and fills in
		 * passed parameter and value variables
		 *
		 * @param line Configuration file line
		 * @param parameter Parameter's name
		 * @param value Parameter's value
		 * @return true on success, false otherwise
		 */
		bool parseLine(const string& line, string& parameter, string& value);
		/**
		 * Sets configuration parameter's value with given value
		 *
		 * @param parameter Parameter name
		 * @param value Parameter value
		 * @return true on success, false otherwise
		 */
		bool setValue(const string& parameter, const string& value);

	private:
		/**
		 * Configuration file name
		 */
		string configurationFile;
		/**
		 * UDP server port number
		 */
		int serverPort;
};

#endif /* MGMT_CONFIGURATION_HPP_ */
