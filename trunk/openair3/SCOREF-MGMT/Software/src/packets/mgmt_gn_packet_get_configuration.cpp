/*
 * mgmt_gn_packet_get_configuration.cpp
 *
 *  Created on: May 9, 2012
 *      Author: demiray
 */

#include "mgmt_gn_packet_get_configuration.hpp"
#include <iostream>
#include <sstream>
using namespace std;

GeonetGetConfigurationEventPacket::GeonetGetConfigurationEventPacket(const vector<unsigned char>& packetBuffer) :
	GeonetPacket(packetBuffer) {
	parse(packetBuffer);
	cout << toString() << endl;
}

u_int16_t GeonetGetConfigurationEventPacket::getConfID() const {
	return packet.configurationId;
}

u_int16_t GeonetGetConfigurationEventPacket::getTxMode() const {
	return packet.transmissionMode;
}

string GeonetGetConfigurationEventPacket::toString() const {
	stringstream ss;

	ss << "Configuration ID: " << packet.configurationId << endl << "Transmission Mode: " << packet.transmissionMode;

	return ss.str();
}

bool GeonetGetConfigurationEventPacket::parse(const vector<unsigned char>& packetBuffer) {
	if (packetBuffer.size() < sizeof(ConfigurationRequestMessage))
		return false;

	u_int8_t payloadIndex = sizeof(MessageHeader);

	packet.configurationId = packetBuffer[payloadIndex];
	packet.configurationId <<= 8;
	packet.configurationId |= packetBuffer[payloadIndex + 1];

	packet.transmissionMode = packetBuffer[payloadIndex + 2];
	packet.transmissionMode <<= 8;
	packet.transmissionMode |= packetBuffer[payloadIndex + 3];

	return true;
}
