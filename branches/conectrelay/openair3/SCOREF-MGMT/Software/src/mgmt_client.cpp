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

#include "mgmt_client.hpp"

ManagementClient::ManagementClient(UdpServer& clientConnection, u_int8_t wirelessStateUpdateInterval, Logger& logger)
	: logger(logger) {
	this->client = client;

	/**
	 * Initialise InquiryThread object for Wireless State updates
	 */
	// todo who is going to join() this thread?
	inquiryThreadObject = new InquiryThread(clientConnection, wirelessStateUpdateInterval, logger);
	inquiryThread = new boost::thread(*inquiryThreadObject);
}

ManagementClient::~ManagementClient() {
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
	// todo check state changes (state machine)
	this->state = state;
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
