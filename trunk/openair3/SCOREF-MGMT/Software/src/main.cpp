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
#include "util/mgmt_udp_server.hpp"
#include "util/mgmt_util.hpp"

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
