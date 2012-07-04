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
 * \file mgmt_client.hpp
 * \brief A container to hold information about Management clients
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#ifndef MGMT_CLIENT_HPP_
#define MGMT_CLIENT_HPP_

#include <boost/asio.hpp>
using boost::asio::ip::udp;

/**
 * A container to hold information about Management clients, mostly used
 * in Message Handler code
 */
class ManagementClient {
	public:
		/**
		 * Client state
		 */
		enum ManagementClientState {
			/**
			 * No client has been connected
			 */
			OFFLINE = 0,
			/**
			 * A client has been connected but has not yet received configuration
			 */
			ONLINE = 1,
			/**
			 * A client has been connected and has received configuration
			 */
			CONNECTED = 2
		};

	public:
		/**
		 * Constructor for ManagementClient class
		 *
		 * @param client Socket information of relevant client
		 */
		ManagementClient(const udp::endpoint& client) {
			this->client = client;
		}
		/**
		 * Destructor for ManagementClient class
		 */
		~ManagementClient() {}

	public:
		/**
		 * Getter for IP address of this client
		 *
		 * @return IP address of this client
		 */
		boost::asio::ip::address getAddress() const {
			return this->client.address();
		}
		/**
		 * Getter for port number of this client
		 *
		 * @return Port number of this client
		 */
		unsigned short int getPort() const {
			return this->client.port();
		}

		/**
		 * Overloaded == operator to use ManagementClient type as a std::map key
		 *
		 * @param client Client that is going to be compared with
		 * @return true if clients are the same, false otherwise
		 */
		bool operator==(const ManagementClient& client) const {
			if (this->client.address() == client.getAddress())
				return true;

			return false;
		}
		/**
		 * Overloaded < operator to use ManagementClient type as a std::map key
		 *
		 * @param client Client that is going to be compared with
		 * @return true if host object's IP address is smaller, false otherwise
		 */
		bool operator<(const ManagementClient& client) const {
			if (this->client.address() < client.getAddress())
				return true;

			return false;
		}

	private:
		/**
		 * Client's UDP socket information
		 */
		udp::endpoint client;
};

#endif /* MGMT_CLIENT_HPP_ */
