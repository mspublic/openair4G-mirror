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
#include "util/mgmt_exception.hpp"
#include <boost/lexical_cast.hpp>
#include "mgmt_client.hpp"

ManagementClient::ManagementClient(ManagementInformationBase& mib, udp::endpoint& clientEndpoint, u_int8_t wirelessStateUpdateInterval, u_int8_t locationUpdateInterval, Logger& logger)
	: mib(mib), clientEndpoint(clientEndpoint), logger(logger) {
	/**
	 * Check that source port is not an ephemeral port which would
	 * change every time a client sendto()s to MGMT
	 * TODO Ephemeral port range could be read from /proc/sys/net/ipv4/ip_local_port_range
	 */
	if (clientEndpoint.port() >= 32768 && clientEndpoint.port() <= 61000) {
		throw Exception("Client has an ephemeral port number that will change every time it sends data and this will screw ManagementClientManager's state", logger);
	}

	/**
	 * Initialise state strings map
	 */
	clientStateStringMap.insert(std::make_pair(ManagementClient::OFFLINE, "OFFLINE"));
	clientStateStringMap.insert(std::make_pair(ManagementClient::ONLINE, "ONLINE"));
	clientStateStringMap.insert(std::make_pair(ManagementClient::CONNECTED, "CONNECTED"));
	/**
	 * Initialise type strings map
	 */
	clientTypeStringMap.insert(std::make_pair(ManagementClient::UNKNOWN, "Unknown"));
	clientTypeStringMap.insert(std::make_pair(ManagementClient::GN, "GeoNetworking"));
	clientTypeStringMap.insert(std::make_pair(ManagementClient::FAC, "Facilities"));
	/**
	 * Initialise this client's state and type
	 */
	state = ManagementClient::OFFLINE;
	type = ManagementClient::UNKNOWN;
}

ManagementClient::ManagementClient(const ManagementClient& managementClient)
	: mib(managementClient.mib), clientEndpoint(managementClient.clientEndpoint), logger(managementClient.logger) {
	throw Exception("Copy constructor is called for a ManagementClient object!", logger);
}

ManagementClient::~ManagementClient() {
	clientTypeStringMap.clear();
}

boost::asio::ip::address ManagementClient::getAddress() const {
	return clientEndpoint.address();
}

unsigned short int ManagementClient::getPort() const {
	return clientEndpoint.port();
}

const udp::endpoint& ManagementClient::getEndpoint() const {
	return this->clientEndpoint;
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
		logger.warning("Requested state change from " + clientStateStringMap[this->state] + " to " + clientStateStringMap[state] + " is either invalid or unnecessary");
		logger.info("Ignoring state change request...");
		return false;
	}

	this->state = state;
	logger.info("State has changed from " + clientStateStringMap[this->state] + " to " + clientStateStringMap[state]);

	return true;
}

ManagementClient::ManagementClientType ManagementClient::getType() const {
	return this->type;
}

bool ManagementClient::setType(ManagementClient::ManagementClientType type) {
	this->type = type;

	return true;
}

bool ManagementClient::operator==(const ManagementClient& client) const {
	if (this->clientEndpoint.address() == client.getAddress())
		return true;

	return false;
}

bool ManagementClient::operator<(const ManagementClient& client) const {
	if (this->clientEndpoint.address() < client.getAddress())
		return true;

	return false;
}

string ManagementClient::toString() {
	stringstream ss;

	ss << "ManagementClient[ip:" << clientEndpoint.address().to_string()
		<< ", port:" << boost::lexical_cast<string>(clientEndpoint.port())
		<< ", type:" << clientTypeStringMap[type] << ", state:" << clientStateStringMap[state] << "]";

	return ss.str();
}
