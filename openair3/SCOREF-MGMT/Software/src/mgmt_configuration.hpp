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
