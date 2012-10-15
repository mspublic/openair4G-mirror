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
  Forums       : http://forums.eurecom.fr/openairinterface
  Address      : EURECOM, Campus SophiaTech, 450 Route des Chappes, 06410 Biot FRANCE

*******************************************************************************/

/*!
 * \file mgmt_client.cpp
 * \brief A container to hold information about Management clients
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
 */

#include "packets/mgmt_gn_packet_location_table_request.hpp"
#include <boost/lexical_cast.hpp>
#include "mgmt_client.hpp"

ManagementClient::ManagementClient(ManagementInformationBase& mib, UdpServer& clientConnection, u_int8_t wirelessStateUpdateInterval, u_int8_t locationUpdateInterval, Logger& logger)
	: mib(mib), logger(logger) {
	this->client = clientConnection.getClient();

	/**
	 * Initialise state strings map
	 */
	clientStateStringMap.insert(std::make_pair(ManagementClient::OFFLINE, "OFFLINE"));
	clientStateStringMap.insert(std::make_pair(ManagementClient::ONLINE, "ONLINE"));
	clientStateStringMap.insert(std::make_pair(ManagementClient::CONNECTED, "CONNECTED"));
	/**
	 * Initialise type strings map
	 */
	clientTypeStringMap.insert(std::make_pair(ManagementClient::GN, "GeoNetworking"));
	clientTypeStringMap.insert(std::make_pair(ManagementClient::FAC, "Facilities"));
	/**
	 * Initialise this client's state and type
	 */
	state = ManagementClient::OFFLINE;
	type = ManagementClient::UNKNOWN;
	/**
	 * Update location table
	 */
	GeonetLocationTableRequestEventPacket locationTableRequest(0xffffffffffffffff, logger);
	clientConnection.send(locationTableRequest);

	/**
	 * Initialise InquiryThread object for Wireless State updates
	 */
	// todo who is going to join() this thread?
	inquiryThreadObject = new InquiryThread(mib, clientConnection, wirelessStateUpdateInterval, locationUpdateInterval, logger);
	inquiryThread = new boost::thread(*inquiryThreadObject);
}

ManagementClient::~ManagementClient() {
	clientTypeStringMap.clear();

	delete inquiryThreadObject;
	delete inquiryThread;
}

boost::asio::ip::address ManagementClient::getAddress() const {
	return client.address();
}

unsigned short int ManagementClient::getPort() const {
	return client.port();
}

ManagementClient::ManagementClientState ManagementClient::getState() const {
	return state;
}

bool ManagementClient::setState(ManagementClient::ManagementClientState state) {
	if (this->state == state) {
		logger.info("State change is not necessary, client is already " + clientStateStringMap[state]);
		return true;
	}

	/**
	 * Verify state change
	 */
	if ((this->state == OFFLINE && state == ONLINE)
			|| (this->state == OFFLINE && state == CONNECTED)
			|| (this->state == ONLINE && state == CONNECTED)) {
		logger.debug("State change is valid");
	} else {
		logger.error("Requested state change from " + clientStateStringMap[this->state] + " to " + clientStateStringMap[state] + " is invalid!");
		logger.info("Ignoring state change request...");
		return false;
	}

	this->state = state;
	logger.info("State has changed from " + clientStateStringMap[this->state] + " to " + clientStateStringMap[state]);

	return true;
}

bool ManagementClient::operator==(const ManagementClient& client) const {
	if (this->client.address() == client.getAddress())
		return true;

	return false;
}

bool ManagementClient::operator<(const ManagementClient& client) const {
	if (this->client.address() < client.getAddress())
		return true;

	return false;
}

string ManagementClient::toString() {
	stringstream ss;

	ss << "ManagementClient[ip:" << client.address().to_string()
		<< ", port:" << boost::lexical_cast<string>(client.port())
		<< ", state:" << clientStateStringMap[state] << "]";

	return ss.str();
}
