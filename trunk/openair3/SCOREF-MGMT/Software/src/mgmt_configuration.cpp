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
 * \file mgmt_configuration.cpp
 * \brief A container with configuration file parsing capability
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "util/mgmt_exception.hpp"
#include "mgmt_configuration.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <fstream>
#include <iostream>
using namespace boost;
using namespace std;

// Initialise configuration parameter strings
const string Configuration::CONF_SERVER_PORT_PARAMETER("CONF_SERVER_PORT");
const string Configuration::CONF_WIRELESS_STATE_UPDATE_INTERVAL("CONF_WIRELESS_STATE_UPDATE_INTERVAL");
const string Configuration::CONF_LOCATION_UPDATE_INTERVAL("CONF_LOCATION_UPDATE_INTERVAL");

Configuration::Configuration(const vector<string>& configurationFileNameVector, Logger& logger)
	: logger(logger) {
	this->configurationFileNameVector = configurationFileNameVector;
	logger.info("There are " + boost::lexical_cast<string>(configurationFileNameVector.size()) + " configuration file(s) to parse");

	/**
	 * Set default values
	 */
	this->serverPort = 1402;
	this->wirelessStateUpdateInterval = 10;
	this->locationUpdateInterval = 20;
}

Configuration::~Configuration() {
}

bool Configuration::parseConfigurationFiles(ManagementInformationBase& mib) {
	ifstream configurationFileStream;
	string parameter, value;
	string line;
	string netParameterPrefix("MIB_GN_NET");
	string facParameterPrefix("MIB_GN_FAC");
	string ihmParameterPrefix("MIB_MC002");
	string commonParameterPrefix("MIB_GN_ALL");
	string confParameterPrefix("CONF_");
	string communicationProfilePrefix("CP");

	/*
	 * Here we traverse configuration files and let MIB know about
	 * every single parameter/[parameterID/]value pair we find
	 */
	for (vector<string>::const_iterator it = configurationFileNameVector.begin(); it != configurationFileNameVector.end(); ++it) {
		string configurationFile = *it;
		logger.info("Parsing configuration file '" + configurationFile + "'");
		configurationFileStream.open(configurationFile.c_str());

		if (configurationFileStream.is_open()) {
			/**
			 * Traverse and parse configuration file lines
			 */
			while (!configurationFileStream.eof()) {
				getline(configurationFileStream, line);

				if (parseLine(line, parameter, value)) {
					value = Util::trim(value, '\0');

					/*
					 * NETwork and FACilities parameters are sent to MIB
					 */
					if (!line.compare(0, netParameterPrefix.length(), netParameterPrefix) ||
							!line.compare(0, facParameterPrefix.length(), facParameterPrefix) ||
							!line.compare(0, commonParameterPrefix.length(), commonParameterPrefix)) {
						try {
							mib.setValue(parameter, atoi(value.c_str()));
						} catch (Exception& e) {
							e.updateStackTrace("Cannot set MIB ITS key using value given in the configuration file");
							throw e;
						}
					/*
					 * General configuration parameters are handled locally in this class
					 */
					} else if (!line.compare(0, confParameterPrefix.size(), confParameterPrefix)) {
						setValue(parameter, value);
					/*
					 * Communication profiles are sent to MIB
					 */
					} else if (!line.compare(0, communicationProfilePrefix.size(), communicationProfilePrefix)) {
						try {
							mib.getCommunicationProfileManager().insert(parameter, value);
						} catch (Exception& e) {
							e.updateStackTrace("Cannot process communication profile string");
							throw e;
						}
					/**
					 * Point de Charge Vehicule Electrique (PCVE) IHM Parameters
					 */
					} else if (!line.compare(0, ihmParameterPrefix.size(), ihmParameterPrefix)) {
						/**
						 * Parse parameter of format <configurationParameter|configurationParameterID>
						 */
						string parameterName;
						u_int16_t parameterId;
						parseParameterId(parameter, parameterName, parameterId);

						/**
						 * Update MIB and add new ITS key but before check ITS data type (it
						 * can be integer and string)
						 */
						if (Util::isNumeric(value)) {
							logger.info("Adding IHM integer parameter [name:" + parameterName + ", id:" + boost::lexical_cast<string>(parameterId) + ", value:" + value + "]");
							mib.getItsKeyManager().addKey(static_cast<ItsKeyID>(parameterId), parameterName, ITS_KEY_TYPE_IHM, atoi(value.c_str()));
						} else {
							logger.info("Adding IHM string parameter [name:" + parameterName + ", id:" + boost::lexical_cast<string>(parameterId) + ", value:" + value + "]");
							mib.getItsKeyManager().addKey(static_cast<ItsKeyID>(parameterId), parameterName, ITS_KEY_TYPE_IHM, value);
						}
					} else {
						logger.warning("Unrecognized configuration item: '" + parameter + "'");
					}
				}
			}
			logger.debug("Closing file stream for '" + configurationFile + "'");
			configurationFileStream.close();
		} else {
			configurationFileStream.close();
			throw Exception("Cannot open configuration file '" + configurationFile + "'!", logger);
		}
	}

	return true;
}

bool Configuration::parseLine(const string& line, string& parameter, string& value) {
	if (line.find('=') == string::npos)
		return false;

	// parse the line according to the place of equal sign
	parameter = line.substr(0, line.find("="));
	value = line.substr(line.find("=") + 1, line.length());

	// trim strings
	remove(parameter.begin(), parameter.end() + 1, ' ');
	remove(value.begin(), value.end() + 1, ' ');

	return true;
}

bool Configuration::parseParameterId(const string& param, string& parameterString, u_int16_t& parameterId) {
	if (param.find('|') == string::npos)
		return false;

	/**
	 * Parse the string according to the separator '|'
	 */
	parameterString = param.substr(0, param.find('|'));
	stringstream parameterIdStringStream;
	parameterIdStringStream << hex << param.substr(param.find('|') + 1, param.length());
	/**
	 * Cast parameter ID string and update incoming variable reference
	 */
	parameterIdStringStream >> parameterId;

	return true;
}


bool Configuration::setValue(const string& parameter, const string& value) {
	if (!parameter.compare(0, CONF_SERVER_PORT_PARAMETER.length(), CONF_SERVER_PORT_PARAMETER)) {
		setServerPort(atoi(value.c_str()));
	} else if (!parameter.compare(0, CONF_WIRELESS_STATE_UPDATE_INTERVAL.length(), CONF_WIRELESS_STATE_UPDATE_INTERVAL)) {
		setWirelessStateUpdateInterval(atoi(value.c_str()));
	} else if (!parameter.compare(0, CONF_LOCATION_UPDATE_INTERVAL.length(), CONF_LOCATION_UPDATE_INTERVAL)) {
		setLocationUpdateInterval(atoi(value.c_str()));
	}

	return true;
}

const vector<string>& Configuration::getConfigurationFileVector() const {
	return configurationFileNameVector;
}

void Configuration::addConfigurationFile(const string& configurationFileName) {
	this->configurationFileNameVector.push_back(configurationFileName);
}

int Configuration::getServerPort() const {
	return serverPort;
}

void Configuration::setServerPort(int serverPort) {
	if (serverPort > 0 && serverPort < 9000)
		this->serverPort = serverPort;
	/**
	 * Keep default value otherwise
	 */
}

u_int8_t Configuration::getWirelessStateUpdateInterval() const {
	return wirelessStateUpdateInterval;
}

void Configuration::setWirelessStateUpdateInterval(u_int8_t interval) {
	/**
	 * Verify incoming value for wireless state update interval
	 * Keep default value if incoming is invalid
	 */
	if (interval >= 10 && interval <= 120) {
		logger.info("Setting Wireless State Update Interval to " + boost::lexical_cast<string>((int)interval) + " seconds");
		wirelessStateUpdateInterval = interval;
	} else
		logger.warning("Parsed value (" + boost::lexical_cast<string>((int)interval) + ") of Wireless State Update Interval is invalid [min=10,max=120], keeping default value (" + boost::lexical_cast<string>((int)wirelessStateUpdateInterval) + ")");
}

u_int8_t Configuration::getLocationUpdateInterval() const {
	return locationUpdateInterval;
}

void Configuration::setLocationUpdateInterval(u_int8_t interval) {
	/**
	 * Verify incoming value for location update interval
	 * Keep default value if incoming is invalid
	 */
	if (interval >= 20 && interval <= 120) {
		logger.info("Setting Location Update Interval to " + boost::lexical_cast<string>((int)interval) + " seconds");
		locationUpdateInterval = interval;
	} else
		logger.warning("Parsed value (" + boost::lexical_cast<string>((int)interval) + ") of Location Update Interval is invalid [min=20,max=120], keeping default value (" + boost::lexical_cast<string>((int)locationUpdateInterval) + ")");
}
