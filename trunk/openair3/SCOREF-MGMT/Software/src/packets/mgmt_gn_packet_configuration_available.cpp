/*
 * mgmt_gn_packet_configuration_available.cpp
 *
 *  Created on: May 10, 2012
 *      Author: demiray
 */

#include "mgmt_gn_packet_configuration_available.hpp"
#include <iostream>
#include <sstream>
using namespace std;

GeonetConfigurationAvailableEventPacket::GeonetConfigurationAvailableEventPacket(ManagementInformationBase& mib)
	: GeonetPacket(false, true, 0x00, 0x00, MGMT_GN_EVENT_CONF_UPDATE_AVAILABLE), mib(mib) {
}

GeonetConfigurationAvailableEventPacket::~GeonetConfigurationAvailableEventPacket() {
}

bool GeonetConfigurationAvailableEventPacket::serialize(vector<unsigned char>& buffer) {
	if (buffer.size() < sizeof(ConfigureAvailableMessage)) {
		cerr << "Incoming buffer' size is not sufficient!" << endl;
		return false;
	}

	// Get some help from superclass to place header into given buffer
	if (!GeonetPacket::serialize(buffer)) {
		cerr << "Cannot serialise header into given buffer!" << endl;
		return false;
	}

	u_int8_t bodyIndex = sizeof(MessageHeader);
	u_int16_t keyCount = mib.getItsKeyManager().getNumberOfKeys();

	// encode `reserved' field
	buffer[bodyIndex] = 0x00;
	buffer[bodyIndex + 1] = 0x00;
	// encode `key count' field
	buffer[bodyIndex + 2] = ((keyCount >> 8) & 0xff);
	buffer[bodyIndex + 3] = (keyCount & 0xff);

	return true;
}

string GeonetConfigurationAvailableEventPacket::toString() const {
	stringstream ss;

	ss << "Key count: " << mib.getItsKeyManager().getNumberOfKeys() << endl;

	return ss.str();
}

