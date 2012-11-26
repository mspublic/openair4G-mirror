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
 * \file mgmt_client_manager.cpp
 * \brief A container for a manager for Management clients
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
 */

#include "packets/mgmt_gn_packet_configuration_available.hpp"
#include "mgmt_client_manager.hpp"
#include "util/mgmt_exception.hpp"
#include <boost/lexical_cast.hpp>

ManagementClientManager::ManagementClientManager(ManagementInformationBase& mib, Configuration& configuration, Logger& logger)
	: mib(mib), configuration(configuration), logger(logger) {
}

ManagementClientManager::~ManagementClientManager() {
	clientVector.clear();
}

bool ManagementClientManager::updateManagementClientState(UdpSocket& clientConnection, EventType eventType) {
	bool clientExists = false;
	ManagementClient* client = NULL;

	/**
	 * Traverse client list and check if we already have this client
	 */
	for (vector<ManagementClient*>::iterator it = clientVector.begin(); it != clientVector.end(); ++it) {
		logger.debug("Comparing IP addresses " + (*it)->getAddress().to_string() + " and " + clientConnection.getRecipient().address().to_string());
		logger.debug("Comparing UDP ports " + boost::lexical_cast<string>((*it)->getPort()) + " and " + boost::lexical_cast<string>(clientConnection.getRecipient().port()));

		if ((*it)->getAddress() == clientConnection.getRecipient().address() && (*it)->getPort() == clientConnection.getRecipient().port()) {
			logger.trace("A client object for " + clientConnection.getRecipient().address().to_string() + ":" + boost::lexical_cast<string>(clientConnection.getRecipient().port()) + " is found");
			client = *it;
			clientExists = true;
		}
	}

	/**
	 * Create a new client object if we couldn't find one
	 */
	if (!clientExists) {
		ManagementClient* newClient = NULL;

		try {
			newClient = new ManagementClient(mib, clientConnection, configuration.getWirelessStateUpdateInterval(), configuration.getLocationUpdateInterval(), logger);
		} catch (Exception& e) {
			e.updateStackTrace("Cannot create a ManagementClient object!");
			throw;
		}

		clientVector.push_back(newClient);
		logger.info("A client object for " + clientConnection.getRecipient().address().to_string() + ":" + boost::lexical_cast<string>(clientConnection.getRecipient().port()) + " is created");

		client = newClient;
	}

	/**
	 * Update client's (the one either found or created) state according
	 * to the event type
	 */
	switch (eventType) {
		case MGMT_GN_EVENT_CONF_REQUEST:
			client->setState(ManagementClient::CONNECTED);
			break;

		case MGMT_GN_EVENT_STATE_WIRELESS_STATE_RESPONSE:
		case MGMT_GN_EVENT_STATE_NETWORK_STATE:
		case MGMT_GN_EVENT_CONF_COMM_PROFILE_REQUEST:
		case MGMT_GN_EVENT_LOCATION_TABLE_RESPONSE:
			client->setState(ManagementClient::ONLINE);
			break;

		/**
		 * Any other packet doesn't cause a state change for clients
		 */
		default:
			break;
	}

	logger.info(toString());

	return true;
}

bool ManagementClientManager::sendConfigurationUpdateAvailable() {
	if (clientVector.empty())
		return false;

	/**
	 * Create a CONFIGURATION_UPDATE_AVAILABLE packet
	 */
	GeonetConfigurationAvailableEventPacket* packet;
	vector<unsigned char> packetBuffer;

	try {
		packet = new GeonetConfigurationAvailableEventPacket(mib, logger);
	} catch (...) {
		throw Exception("Cannot create a CONFIGURATION_UPDATE_AVAILABLE packet!", logger);
	}

	/**
	 * Serialize...
	 */
	if (!packet->serialize(packetBuffer)) {
		logger.error("Cannot serialize CONFIGURATION_UPDATE_AVAILABLE packet!");
		return false;
	}

	/**
	 * ...and send
	 */
	boost::asio::io_service ioService;
	udp::socket* clientSocket = NULL;
	boost::system::error_code error;
	for (vector<ManagementClient*>::iterator it = clientVector.begin(); it != clientVector.end(); ++it) {
		clientSocket = new udp::socket(ioService, udp::endpoint(udp::v4(), (*it)->getPort()));
//LEFT_HERE		clientSocket->send_to(boost::asio::buffer(packetBuffer), (*it)->get, 0, error);
		delete clientSocket;
	}
	return true;
}

string ManagementClientManager::toString() {
	stringstream ss;

	ss << "Current status of client(s):" << endl;
	ss << "Client count is " << clientVector.size() << endl;
	for (vector<ManagementClient*>::iterator it = clientVector.begin(); it != clientVector.end(); ++it)
		ss << (*it)->toString();

	return ss.str();
}
