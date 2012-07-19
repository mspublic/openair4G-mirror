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

#include "mgmt_client_manager.hpp"
#include <boost/lexical_cast.hpp>

ManagementClientManager::ManagementClientManager(Configuration& configuration, Logger& logger)
	: configuration(configuration), logger(logger) {
}

ManagementClientManager::~ManagementClientManager() {
	while(!clientVector.empty()) {
		delete clientVector.back();
		clientVector.pop_back();
	}
}

bool ManagementClientManager::updateManagementClientState(UdpServer& clientConnection, EventType eventType) {
	vector<ManagementClient*>::iterator it = clientVector.begin();
	bool clientExists = false;
	ManagementClient* client;

	while (it++ != clientVector.end()) {
		if ((*it)->getAddress() == clientConnection.getClient().address() && (*it)->getPort() == clientConnection.getClient().port()) {
			logger.trace("A client object for " + clientConnection.getClient().address().to_string() + ":" + boost::lexical_cast<string>(clientConnection.getClient().port()) + " if found");
			client = *it;
			clientExists = true;
		}
	}

	/**
	 * Create a new client object if we couldn't find one
	 */
	if (!clientExists) {
		ManagementClient* newClient = new ManagementClient(clientConnection, configuration.getWirelessStateUpdateInterval(), logger);
		clientVector.push_back(newClient);

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

	return true;
}
