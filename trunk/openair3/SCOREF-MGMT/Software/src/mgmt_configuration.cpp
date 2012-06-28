/*
 * mgmt_configuration.cpp
 *
 *  Created on: May 4, 2012
 *      Author: demiray
 */

#include "mgmt_configuration.hpp"
#include <boost/tokenizer.hpp>
#include <fstream>
#include <iostream>
using namespace boost;
using namespace std;

// Initialise configuration parameter strings
const string Configuration::CONF_SERVER_PORT_PARAMETER("CONF_SERVER_PORT");

Configuration::Configuration(string configurationFile) {
	this->configurationFile = configurationFile;

	// Set default values
	this->serverPort = 1402;
}

Configuration::~Configuration() {
}

bool Configuration::parseConfigurationFile(ManagementInformationBase& mib) {
	ifstream configurationFile;
	string parameter, value;
	string line;
	string netParameterPrefix("MIB_GN_NET");
	string facParameterPrefix("MIB_GN_FAC");
	string commonParameterPrefix("MIB_GN_ALL");
	string confParameterPrefix("CONF_");

	configurationFile.open(this->configurationFile.c_str());

	if (configurationFile.is_open()) {
		// Traverse the lines till the end
		while (!configurationFile.eof()) {
			getline(configurationFile, line);

			if (parseLine(line, parameter, value)) {
				// NETwork and FACilities parameters are sent to MIB
				if (!line.compare(0, netParameterPrefix.length(), netParameterPrefix) ||
						!line.compare(0, facParameterPrefix.length(), facParameterPrefix) ||
						!line.compare(0, commonParameterPrefix.length(), commonParameterPrefix)) {
					mib.setValue(parameter, atoi(value.c_str()));
					// Others must be general configuration parameters
				} else if (!line.compare(0, confParameterPrefix.size(), confParameterPrefix)) {
					setValue(parameter, value);
				}
			}
		}
	} else {
		cerr << "Cannot open configuration file" << endl;
		return false;
	}

	configurationFile.close();
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

bool Configuration::setValue(const string& parameter, const string& value) {
	if (!parameter.compare(0, CONF_SERVER_PORT_PARAMETER.length(), CONF_SERVER_PORT_PARAMETER)) {
		setServerPort(atoi(value.c_str()));
	}

	return true;
}

string Configuration::getConfigurationFile() const {
	return configurationFile;
}

void Configuration::setConfigurationFile(string configurationFile) {
	this->configurationFile = configurationFile;
}

int Configuration::getServerPort() const {
	return serverPort;
}

void Configuration::setServerPort(int serverPort) {
	this->serverPort = serverPort;
}
