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
 * \file mgmt_client_manager.hpp
 * \brief A container for a manager for Management clients
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
 */

#ifndef MGMT_CLIENT_MANAGER_HPP_
#define MGMT_CLIENT_MANAGER_HPP_

#include "util/mgmt_udp_server.hpp"
#include "mgmt_configuration.hpp"
#include "mgmt_types.hpp"
#include "util/mgmt_log.hpp"
#include "mgmt_client.hpp"

#include <vector>
using namespace std;

/**
 * A container for a manager for Management clients
 */
class ManagementClientManager {
	public:
		/**
		 * Constructor for ManagementClientManager class
		 *
		 * @param mib Management Information Base reference
		 * @param configuration Management configuration to pass to/to manage client objects
		 * @param logger Logger object reference
		 */
		ManagementClientManager(ManagementInformationBase& mib, Configuration& configuration, Logger& logger);
		/**
		 * Destructor for ManagementClientManager class
		 */
		~ManagementClientManager();

	public:
		/**
		 * Handles incoming data and updates client vector accordingly, by creating
		 * a new client object if necessary or updating its state if there's one
		 * defined for sender source address
		 *
		 * @param clientConnection Sender of relevant packet
		 * @param eventType Type/subtype of event the packet was sent for
		 * @return true if success, false otherwise
		 */
		bool updateManagementClientState(UdpServer& clientConnection, EventType eventType);
		/**
		 * Sends CONFIGURATION UPDATE AVAILABLE to all those clients connected
		 *
		 * @return true on success, false otherwise
		 */
		bool sendConfigurationUpdateAvailable();
		/**
		 * Returns the string representation of Client Manager and the clients it manages
		 *
		 * @param none
		 * @return std::string representation of this class
		 */
		string toString();

	private:
		/**
		 * Management Information Base reference
		 */
		ManagementInformationBase& mib;
		/**
		 * Client vector holding clients of connected/online (see ManagementClientState) state
		 */
		vector<ManagementClient*> clientVector;
		/**
		 * Configuration object reference to fetch necessary configuration information
		 */
		Configuration& configuration;
		/**
		 * Logger object reference
		 */
		Logger& logger;
};

#endif /* MGMT_CLIENT_MANAGER_HPP_ */
