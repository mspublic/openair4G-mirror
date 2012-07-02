/*
 * mgmt_udp_server.h
 *
 *  Created on: Apr 30, 2012
 *      Author: demiray
 */

#ifndef MGMT_UDP_SERVER_H_
#define MGMT_UDP_SERVER_H_

#include <vector>
using namespace std;

#include <boost/array.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::udp;

#include "../packets/mgmt_gn_packet.hpp"

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
		 */
		UdpServer(u_int16_t portNumber);
		/**
		 * Destructor for UdpServer class
		 */
		~UdpServer();

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

	private:
		/**
		 * The io_service object that the datagram socket will use to dispatch
		 * handlers for any asynchronous operations performed on the socket
		 */
		boost::asio::io_service ioService;
		/**
		 * udp::socket of Boost library
		 */
		udp::socket* socket;
		/**
		 * UDP client
		 */
		udp::endpoint client;
};

#endif /* MGMT_UDP_SERVER_H_ */
