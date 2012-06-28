/*
 * mgmt_gn_packet.cpp
 *
 *  Created on: May 2, 2012
 *      Author: demiray
 */

#include "mgmt_gn_packet.hpp"
#include "../mgmt_util.hpp"
#include <sstream>
#include <iostream>
using namespace std;

GeonetPacket::GeonetPacket(bool extendedMessage, bool validity, u_int8_t version, u_int8_t priority,
    u_int16_t eventType) {
	Util::resetBuffer(&header, sizeof(MessageHeader));

	if (extendedMessage)
		this->header.version |= 0x80;
	if (validity)
		this->header.version |= 0x40;

	this->header.version |= (version & 0x0f);
	this->header.priority = priority;
	this->header.priority <<= 5;
	this->header.eventType = (eventType >> 8);
	this->header.eventSubtype = (eventType & 0x0f);
}

GeonetPacket::GeonetPacket(const vector<unsigned char>& packetBuffer) {
	parseHeaderBuffer(packetBuffer, &this->header);
	cout << toString() << endl;
}

GeonetPacket::~GeonetPacket() {
}

bool GeonetPacket::parseHeaderBuffer(const vector<unsigned char>& headerBuffer, MessageHeader* header) {
	if (headerBuffer.size() < sizeof(MessageHeader) || !header)
		return false;

	header->version = headerBuffer[0] & 0x0f;
	header->priority = (headerBuffer[1] >> 5);
	header->eventType = headerBuffer[2];
	header->eventSubtype = headerBuffer[3];

	return true;
}

bool GeonetPacket::serialize(vector<unsigned char>& buffer) const {
	cout << "Serializing header..." << endl;

	buffer[0] = header.version;
	buffer[0] |= 0x40; // encode Validity flag as 1
	buffer[1] = header.priority;
	buffer[1] <<= 5;
	buffer[2] = header.eventType;
	buffer[3] = header.eventSubtype;

	return true;
}

string GeonetPacket::toString() const {
	stringstream ss;

	ss << "Version: " << (int) header.version << endl << "Priority: " << (int) header.priority << endl << "Event Type: "
	    << (int) (header.eventType * 100 + header.eventSubtype);

	return ss.str();
}
