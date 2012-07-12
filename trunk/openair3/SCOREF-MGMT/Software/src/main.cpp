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
 * \file main.cpp
 * \brief SCOREF Management module's hometown
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include <iostream>
#include <vector>
#include <string>
#include <ctime>
using namespace std;

#include <boost/program_options.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::udp;

#include "packets/mgmt_gn_packet_location_table_request.hpp"
#include "mgmt_gn_packet_handler.hpp"
#include "util/mgmt_udp_server.hpp"
#include "mgmt_inquiry_thread.hpp"
#include "mgmt_configuration.hpp"
#include "util/mgmt_util.hpp"
#include "util/mgmt_log.hpp"

void printHelp(string binaryName) {
	cerr << binaryName << " <configurationFile>" << endl;
	// todo explain other commandline options here
}

const string CONF_HELP_PARAMETER_STRING = "help";
const string CONF_LOG_LEVEL_PARAMETER_STRING = "loglevel";

int main(int argc, char** argv) {
	Logger logger(Logger::DEBUG);

#ifdef BOOST_VERSION_1_50
	/**
	 * Define and parse command-line parameters
	 */
	po::options_description commandLineOptions("Command-line options");
	desc.add_options()
			(CONF_HELP_PARAMETER_STRING, "Print help message")
			(CONF_LOG_LEVEL_PARAMETER_STRING, po::value<int>(), "Set log level (DEBUG=0, INFO=1, WARNING=2, ERROR=3)");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, commandLineOptions), vm);
	po::notify(vm);

	if (vm.count(CONF_HELP_PARAMETER_STRING)) {
		printHelp(argv[0]);
		return 1;
	}

	if (vm.count(CONF_LOG_LEVEL_PARAMETER_STRING)) {
		logger.info("Log level was set to " + vm[CONF_LOG_LEVEL_PARAMETER_STRING].as<int>());
		logger.setLogLevel(vm[CONF_LOG_LEVEL_PARAMETER_STRING.as<int>]);
	} else {
		logger.info("Compression level was not set, default is DEBUG");
	}
#endif

	// Location table update will be done at the beginning for once
	// todo this should be managed by ManagementClientState
	bool locationTableUpdated = false;

	// We expect the configuration file name as the only parameter
	if (argc != 2) {
		printHelp(argv[0]);
		exit(1);
	}

	ManagementInformationBase mib(logger);
	GeonetMessageHandler packetHandler(mib, logger);

	logger.info("Starting Management & GeoNetworking Interface...");
	logger.info("Reading configuration file...");

	Configuration configuration(argv[1], logger);
	if (!configuration.pars23 May 2012eConfigurationFile(mib)) {
		logger.error("Cannot open/parse configuration file, exiting...");
		return -1;
	}

	UdpServer server(configuration.getServerPort(), logger);

	/**
	 * Initialise InquiryThread object for Wireless State updates
	 */
	InquiryThread inquiryThreadObject(server, configuration.getWirelessStateUpdateInterval(), logger);
	boost::thread inquiryThread(inquiryThreadObject);

	vector<unsigned char> rxBuffer(UdpServer::RX_BUFFER_SIZE);
	vector<unsigned char> txBuffer(UdpServer::TX_BUFFER_SIZE);

	try {
		for (;;) {
			if (server.receive(rxBuffer)) {
				GeonetPacket* reply = NULL;

				try {
					reply = packetHandler.handleGeonetMessage(rxBuffer, server.getClient());
				} catch (std::exception& e) {
					cerr << e.what() << endl;
				}

				if (reply)
					server.send(*reply);
			}

			if (!locationTableUpdated) {
				// Initialise location table
				GeonetLocationTableRequestEventPacket locationTableRequest(0xffffffffffffffff, logger);
				server.send(locationTableRequest);
				locationTableUpdated = true;
			}

			// Revert buffer sizes to initials
			rxBuffer.reserve(UdpServer::RX_BUFFER_SIZE);
			txBuffer.reserve(UdpServer::TX_BUFFER_SIZE);
		}
	} catch (std::exception& e) {
		logger.error(e.what());
	}

	/**
	 * Wait for inquiry thread to finish its job
	 */
	inquiryThread.join();

	return 0;
}
