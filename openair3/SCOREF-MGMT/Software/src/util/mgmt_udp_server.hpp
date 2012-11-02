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
 * \file mgmt_udp_server.hpp
 * \brief A wrapper container to maintain UDP socket connection
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#ifndef MGMT_UDP_SERVER_H_
#define MGMT_UDP_SERVER_H_

#include <vector>
using namespace std;

#include <boost/thread.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::udp;

#include "../packets/mgmt_gn_packet.hpp"
#include "../util/mgmt_log.hpp"

/**
 * A wrapper container to maintain UDP socket connection
 */
class UdpServer {
	public:
		/**
		 * Receive buffer size in bytes
		 */
		static const u_int16_t RX_BUFFER_SIZE = 1024;
		/**
		 * Transmit buffer size in bytes
		 */
		static const u_int16_t TX_BUFFER_SIZE = 1024;

	public:
		/**
		 * Constructor for UdpServer class
		 *
		 * @param portNumber UDP port number that will be listened for client connections
		 * @param logger Logger object reference for logging purposes
		 */
		UdpServer(u_int16_t portNumber, Logger& logger);
		/**
		 * Destructor for UdpServer class
		 */
		~UdpServer();

	private:
		/**
		 * Copy constructor to prevent the usage of default copy constructor
		 */
		UdpServer(const UdpServer& udpServer);

	public:
		/**
		 * Reads available data from socket into given buffer
		 *
		 * @param rxBuffer RX buffer that read data will be put into
		 * @return Number of bytes read
		 */
		unsigned receive(vector<unsigned char>& rxBuffer);
		/**
		 * Writes given data through socket
		 *
		 * @param txBuffer TX buffer that will be sent
		 * @return true on success, false otherwise
		 */
		bool send(vector<unsigned char>& txBuffer);
		/**
		 * Serialises given Geonet packet and writes onto socket
		 *
		 * @param packet GeonetPacket reference that will be serialised
		 * and sent through socket
		 * @return true on success, false otherwise
		 */
		bool send(const GeonetPacket& packet);
		/**
		 * Returns the reference of udp::endpoint
		 *
		 * @return The reference of udp::endpoint
		 */
		const udp::endpoint& getClient() const;
		/**
		 * Returns string representation of this connection
		 *
		 * @return String representation of this class of type std::string
		 */
		string toString() const;

	private:
		/**
		 * The io_service object that the datagram socket will use to dispatch
		 * handlers for any asynchronous operations performed on the socket
		 */
		boost::asio::io_service ioService;
		/**
		 * Mutexes to coordinate I/O on UDP server socket
		 */
		boost::mutex readMutex;
		boost::mutex writeMutex;
		/**
		 * udp::socket of Boost library
		 */
		udp::socket* socket;
		/**
		 * UDP client
		 */
		udp::endpoint client;
		/**
		 * Logger object reference for logging purposes
		 */
		Logger& logger;
};

#endif /* MGMT_UDP_SERVER_H_ */
