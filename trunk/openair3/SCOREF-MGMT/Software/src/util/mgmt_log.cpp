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
 * \file mgmt_log.hpp
 * \brief A container for a basic logging utility
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include <boost/date_time.hpp>
#include "mgmt_log.hpp"
#include <iostream>
using namespace std;

Logger::Logger(const string& logFileName, Logger::LOG_LEVEL logLevel) {
	this->logFileName = logFileName;
	this->logLevel = logLevel;

	/**
	 * Open log file stream, if the file already exists then rename
	 * it appending the date and create a new one
	 */
	logFilePath = boost::filesystem::path(logFileName);

	if (boost::filesystem::exists(logFilePath)) {
		cout << "Log file already exists, renaming it..." << endl;

		/**
		 * Get the current date/time as string
		 */

		// boost::filesystem::rename(logFilePath, logFilePath + string("hede"));
	}

	if (logFileStream.open(logFileName.c_str(), ios_base::out)) {
		cerr << "Cannot open log file!" << endl;
	}

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

void Logger::setLogLevel(Logger::LOG_LEVEL logLevel) {
	this->logLevel = logLevel;
}
