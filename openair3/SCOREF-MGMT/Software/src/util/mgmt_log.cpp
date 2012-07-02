/*
 * mgmt_log.cpp
 *
 *  Created on: 30 Jun 2012
 *      Author: barisd
 */

#include "mgmt_log.hpp"
#include <iostream>
using namespace std;

Logger::Logger(Logger::LOG_LEVEL logLevel) {
	this->logLevel = logLevel;

	logLevelString.insert(logLevelString.end(), std::make_pair(DEBUG, "DEBUG"));
	logLevelString.insert(logLevelString.end(), std::make_pair(INFO, "INFO"));
	logLevelString.insert(logLevelString.end(), std::make_pair(WARNING, "WARNING"));
	logLevelString.insert(logLevelString.end(), std::make_pair(ERROR, "ERROR"));
}

Logger::~Logger() {}

void Logger::debug(const string& message) {
	log(message, DEBUG);
}

void Logger::info(const string& message) {
	log(message, INFO);
}

void Logger::warning(const string& message) {
	log(message, WARNING);
}

void Logger::error(const string& message) {
	log(message, ERROR);
}

void Logger::log(const string& message, LOG_LEVEL level) {
	cout << logLevelString[level] << ": " << message << endl;
}
