/*
 * main.cpp
 *
 *  Created on: Apr 25, 2012
 *      Author: demiray
 */

#include <ctime>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <boost/array.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::udp;

#include "mgmt_configuration.hpp"
#include "mgmt_gn_packet_handler.hpp"
#include "packets/mgmt_gn_packet_location_table_request.hpp"
#include "mgmt_udp_server.hpp"
#include "mgmt_util.hpp"

void printHelp(string binaryName) {
	cerr << binaryName << " <configurationFile>" << endl;
}

int main(int argc, char** argv) {
	// Location table update will be done at the beginning for once
	// todo this should be managed by ManagementClientState
	bool locationTableUpdated = false;

	// We expect the configuration file name as the only parameter
	if (argc != 2) {
		printHelp(argv[0]);
		exit(1);
	}

	ManagementInformationBase mib;
	GeonetMessageHandler packetHandler(mib);

	cout << "Starting Management & GeoNetworking Interface..." << endl;
	cout << "Reading configuration file..." << endl;

	Configuration configuration(argv[1]);
	if (!configuration.parseConfigurationFile(mib)) {
		cerr << "Cannot open/parse configuration file, exiting..." << endl;
		return -1;
	}

	UdpServer server(configuration.getServerPort());
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
				GeonetLocationTableRequestEventPacket locationTableRequest(0xffffffffffffffff);
				server.send(locationTableRequest);
				locationTableUpdated = true;
			}

			// Revert buffer sizes to initials
			rxBuffer.reserve(UdpServer::RX_BUFFER_SIZE);
			txBuffer.reserve(UdpServer::TX_BUFFER_SIZE);
		}
	} catch (std::exception& e) {
		cerr << e.what() << std::endl;
	}

	return 0;
}
