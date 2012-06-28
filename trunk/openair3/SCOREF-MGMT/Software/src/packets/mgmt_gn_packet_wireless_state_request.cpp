/*
 * mgmt_gn_packet_wireless_state_request.cpp
 *
 *  Created on: Jun 22, 2012
 *      Author: demiray
 */

#include "mgmt_gn_packet_wireless_state_request.hpp"
#include <sstream>

GeonetWirelessStateRequestEventPacket::GeonetWirelessStateRequestEventPacket()
	: GeonetPacket(false, true, 0x00, 0x00, MGMT_GN_EVENT_STATE_WIRELESS_STATE_REQUEST) {
}

GeonetWirelessStateRequestEventPacket::~GeonetWirelessStateRequestEventPacket() {}

bool GeonetWirelessStateRequestEventPacket::serialize(vector<unsigned char>& buffer) const {
	/* This packet is an only-header packet */
	return GeonetPacket::serialize(buffer);
}

string GeonetWirelessStateRequestEventPacket::toString() const {
	/* This packet is an only-header packet */
	return GeonetPacket::toString();
};
