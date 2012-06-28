/*
 * mgmt_udp_server.cpp
 *
 *  Created on: Jun 6, 2012
 *      Author: demiray
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
