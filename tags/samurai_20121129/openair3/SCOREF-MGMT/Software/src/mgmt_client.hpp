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

#include "mgmt_inquiry_thread.hpp"
#include "util/mgmt_log.hpp"

#include <boost/thread.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::udp;

#include <string>
#include <map>

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
			 * Client is not connected
			 */
			OFFLINE = 0,
			/**
			 * A client is connected but has not yet received configuration
			 */
			ONLINE = 1,
			/**
			 * A client is connected and has received configuration
			 */
			CONNECTED = 2
		};

		/**
		 * Client type
		 */
		enum ManagementClientType {
			/**
			 * GeoNetworking client
			 */
			GN = 0,
			/**
			 * Facilities client
			 */
			FAC = 1
		};

	public:
		/**
		 * Constructor for ManagementClient class
		 *
		 * @param mib Management Information Base reference
		 * @param clientConnection Connected socket for client connection
		 * @param wirelessStateUpdateInterval Determines how frequent the wireless state update will be performed
		 * @param locationUpdateInterval Determines how frequent the location update will be performed
		 * @logger Logger object reference
		 */
		ManagementClient(ManagementInformationBase& mib, UdpServer& clientConnection, u_int8_t wirelessStateUpdateInterval, u_int8_t locationUpdateInterval, Logger& logger);
		/**
		 * Destructor for ManagementClient class
		 */
		~ManagementClient();

	public:
		/**
		 * Getter for IP address of this client
		 *
		 * @return IP address of this client
		 */
		boost::asio::ip::address getAddress() const;
		/**
		 * Getter for port number of this client
		 *
		 * @return Port number of this client
		 */
		unsigned short int getPort() const;
		/**
		 * Returns the state of this client
		 *
		 * @return ManagementClientState value for this client
		 */
		ManagementClientState getState() const;
		// XXX setType() and getType()
		/**
		 * Sets the state of this client with given state
		 *
		 * @param state New ManagementClientState for this client
		 * @return true on success, false otherwise
		 */
		bool setState(ManagementClientState state);
		/**
		 * Overloaded == operator to use ManagementClient type as a std::map key
		 *
		 * @param client Client that is going to be compared with
		 * @return true if clients are the same, false otherwise
		 */
		bool operator==(const ManagementClient& client) const;
		/**
		 * Overloaded < operator to use ManagementClient type as a std::map key
		 *
		 * @param client Client that is going to be compared with
		 * @return true if host object's IP address is smaller, false otherwise
		 */
		bool operator<(const ManagementClient& client) const;
		/**
		 * Returns string representation of this client
		 *
		 * @return std::string representation of this client
		 */
		string toString();

	private:
		/**
		 * Management Information Base reference
		 */
		ManagementInformationBase& mib;
		/**
		 * Client's UDP socket information
		 */
		udp::endpoint client;
		/**
		 * Client's connection state with Management module
		 */
		ManagementClient::ManagementClientState state;
		/**
		 * Client type
		 */
		ManagementClient::ManagementClientType type;
		/**
		 * InquiryThread object for Wireless State updates
		 */
		InquiryThread* inquiryThreadObject;
		/**
		 * InquiryThread runner for Wireless State updates
		 */
		boost::thread* inquiryThread;
		/**
		 * Logger object reference
		 */
		Logger& logger;
		/**
		 * String representations for Management Client states
		 */
		map<ManagementClient::ManagementClientState, string> clientStateStringMap;
		/**
		 * String representations for Management Client types
		 */
		map<ManagementClient::ManagementClientType, string> clientTypeStringMap;
};

#endif /* MGMT_CLIENT_HPP_ */