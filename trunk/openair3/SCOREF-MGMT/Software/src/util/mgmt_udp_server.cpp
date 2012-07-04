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
 * \file mgmt_udp_server.cpp
 * \brief A wrapper container to maintain UDP socket connection
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "mgmt_udp_server.hpp"
#include "mgmt_util.hpp"

#include <iostream>
using namespace std;

UdpServer::UdpServer(u_int16_t portNumber) {
	socket = new udp::socket(ioService, udp::endpoint(udp::v4(), portNumber));
	cout << "A UDP server socket created for port " << portNumber << endl;
}

UdpServer::~UdpServer() {
	delete socket;
}

unsigned UdpServer::receive(vector<unsigned char>& rxBuffer) {
	boost::system::error_code error;
	unsigned bytesRead = 0;

	try {
		cout << "Reading..." << endl;
		bytesRead = socket->receive_from(boost::asio::buffer(rxBuffer), client, 0, error);
	} catch (std::exception& e) {
		cerr << e.what() << std::endl;
		return 0;
	}

	if (error && error != boost::asio::error::message_size)
		throw boost::system::system_error(error);

	rxBuffer.resize(bytesRead);

	cout << bytesRead << " byte(s) received from " << client.address().to_string() << endl;
	Util::printHexRepresentation(rxBuffer.data(), rxBuffer.size());

	return bytesRead;
}

bool UdpServer::send(vector<unsigned char>& txBuffer) {
	boost::system::error_code error;

	try {
		cout << "Writing..." << endl;
		socket->send_to(boost::asio::buffer(txBuffer), client, 0, error);
		cout << txBuffer.size() << " byte(s) sent" << endl;
		Util::printHexRepresentation(txBuffer.data(), txBuffer.size());
	} catch (std::exception& e) {
		cerr << e.what() << std::endl;
		return false;
	}

	return true;
}

bool UdpServer::send(const GeonetPacket& packet) {
	vector<unsigned char> txBuffer(TX_BUFFER_SIZE);

	if (packet.serialize(txBuffer))
		return send(txBuffer);

	return false;
}

const udp::endpoint& UdpServer::getClient() const {
	return this->client;
}
