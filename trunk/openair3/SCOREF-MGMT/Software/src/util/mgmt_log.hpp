/*
 * mgmt_log.hpp
 *
 *  Created on: 30 Jun 2012
 *      Author: barisd
 */

#ifndef MGMT_LOG_HPP_
#define MGMT_LOG_HPP_

#include <string>
#include <map>
using namespace std;

/**
 * A container for a basic logging utility
 */
class Logger {
	public:
		enum LOG_LEVEL {
			DEBUG = 0,
			INFO = 1,
			WARNING = 2,
			ERROR = 3
		};

	public:
		/**
		 * Constructor for Logger class
		 *
		 * @param logLevel Initial log level (default is INFO)
		 */
		Logger(Logger::LOG_LEVEL logLevel = Logger::INFO);
		/**
		 * Destructor for Logger class
		 */
		~Logger();

	public:
		/**
		 * Prints given log message at DEBUG level
		 *
		 * @param message Log message
		 */
		void debug(const string& message);
		/**
		 * Prints given log message at INFO level
		 *
		 * @param message Log message
		 */
		void info(const string& message);
		/**
		 * Prints given log message at WARNING level
		 *
		 * @param message Log message
		 */
		void warning(const string& message);
		/**
		 * Prints given log message at ERROR level
		 *
		 * @param message Log message
		 */
		void error(const string& message);
		/**
		 * Prints given log message at given level
		 *
		 * @param message Log message
		 * @param level Log level
		 */
		void log(const string& message, Logger::LOG_LEVEL level);

	private:
		/**
		 * Configured log level
		 */
		Logger::LOG_LEVEL logLevel;
		/**
		 * Log level string map
		 */
		map<LOG_LEVEL, string> logLevelString;
};

#endif /* MGMT_LOG_HPP_ */
