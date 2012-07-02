/*
 * mgmt_gn_packet_location_table_request.cpp
 *
 *  Created on: May 21, 2012
 *      Author: demiray
 */

#include "mgmt_gn_packet_location_table_request.hpp"
#include "../util/mgmt_util.hpp"
#include <iostream>
#include <sstream>
using namespace std;

GeonetLocationTableRequestEventPacket::GeonetLocationTableRequestEventPacket(GnAddress address)
	: GeonetPacket(false, true, 0x00, 0x00, MGMT_GN_EVENT_LOCATION_TABLE_REQUEST) {
	this->gnAddress = address;
}

bool GeonetLocationTableRequestEventPacket::serialize(vector<unsigned char>& buffer) const {
	if (buffer.size() < sizeof(LocationTableRequest))
		return false;

	// Encode header first
	if (!GeonetPacket::serialize(buffer)) {
		cerr << "Cannot serialise header into given buffer!" << endl;
		return false;
	}

	// Then the GN address follows
	if (!Util::encode8byteInteger(buffer, sizeof(MessageHeader), this->gnAddress)) {
		cerr << "Cannot serialise GN address into given buffer!" << endl;
		return false;
	}

	buffer.resize(sizeof(LocationTableRequest));

	return true;
}

string GeonetLocationTableRequestEventPacket::toString() const {
	stringstream ss;

	ss << GeonetPacket::toString();
	ss << "GN Address: " << gnAddress << endl;

	return ss.str();
}
