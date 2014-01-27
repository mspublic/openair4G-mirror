/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2012 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*!
 * \file mgmt_configuration.hpp
 * \brief A container with configuration file parsing capability
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#ifndef MGMT_CONFIGURATION_HPP_
#define MGMT_CONFIGURATION_HPP_

#include "mgmt_information_base.hpp"
#include "util/mgmt_log.hpp"
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
		/**
		 * Parameter string for repetition interval (in seconds)
		 * for Wireless State Request message
		 */
		static const string CONF_WIRELESS_STATE_UPDATE_INTERVAL;
		/**
		 * Parameter string for repetition interval (in seconds)
		 * for Location Update message
		 */
		static const string CONF_LOCATION_UPDATE_INTERVAL;

	public:
		/**
		 * Constructor for Configuration class
		 *
		 * @param configurationFileNameList Configuration files that shall be parsed
		 * @param logger Logger object reference
		 */
		Configuration(const vector<string>& confgurationFileNameVector, Logger& logger);
		/**
		 * Destructor for Configuration class
		 */
		~Configuration();

	public:
		/**
		 * Parses given set of configuration files and updates MIB accordingly
		 *
		 * @param mib Management Information Base reference
		 * @return true on success, false otherwise
		 */
		bool parseConfigurationFiles(ManagementInformationBase& mib);
		/**
		 * Returns configuration file name vector
		 *
		 * @param none
		 * @return Configuration file name vector
		 */
		const vector<string>& getConfigurationFileVector() const;
		/**
		 * Adds a configuration file name to the vector
		 *
		 * @param configurationFileName New configuration file's name
		 * @return none
		 */
		void addConfigurationFile(const string& configurationFileName);
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
		/**
		 * Returns Wireless State Update interval (in seconds)
		 *
		 * @return Wireless State Update interval in seconds
		 */
		u_int8_t getWirelessStateUpdateInterval() const;
		/**
		 * Sets Wireless State Update interval
		 *
		 * @param interval Wireless State Update interval in seconds
		 * @return none
		 */
		void setWirelessStateUpdateInterval(u_int8_t interval);
		/**
		 * Returns Location Update interval (in seconds)
		 *
		 * @return Location Update interval in seconds
		 */
		u_int8_t getLocationUpdateInterval() const;
		/**
		 * Sets Location Update interval
		 *
		 * @param interval Location Update interval in seconds
		 * @return none
		 */
		void setLocationUpdateInterval(u_int8_t interval);

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
		 * Parses IHM configuration IDs of type <configurationItemName|configurationItemID>
		 *
		 * @param param Parameter string (input)
		 * @param parameterString "configurationItemName" part of the parameter string (output)
		 * @param parameterID "configurationItemID" part of the parameter string (output)
		 * @return bool true on success, false otherwise
		 */
		bool parseParameterId(const string& param, string& parameterString, u_int16_t& parameterId);
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
		 * Configuration files name vector
		 */
		vector<string> configurationFileNameVector;
		/**
		 * UDP server port number
		 */
		int serverPort;
		/**
		 * Wireless State Update interval (in seconds)
		 */
		u_int8_t wirelessStateUpdateInterval;
		/**
		 * Location Update interval (in seconds)
		 */
		u_int8_t locationUpdateInterval;
		/**
		 * Logger object reference
		 */
		Logger& logger;
};

#endif /* MGMT_CONFIGURATION_HPP_ */