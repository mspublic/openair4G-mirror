/*
 * mgmt_gn_packet_comm_profile_response.cpp
 *
 *  Created on: 10 Jun 2012
 *      Author: barisd
 */

#include "mgmt_gn_packet_comm_profile_response.hpp"
#include "../mgmt_util.hpp"

GeonetCommunicationProfileResponsePacket::GeonetCommunicationProfileResponsePacket(ManagementInformationBase& mib,
		u_int32_t communicationProfileRequest) :
	GeonetPacket(false, true, 0x00, 0x00, MGMT_GN_EVENT_CONF_COMM_PROFILE_RESPONSE), mib(mib) {
	this->communicationProfileRequest = communicationProfileRequest;
}

GeonetCommunicationProfileResponsePacket::~GeonetCommunicationProfileResponsePacket() {
}

bool GeonetCommunicationProfileResponsePacket::serialize(vector<unsigned char>& buffer) const {
	if (buffer.size() < sizeof(CommunicationProfileResponse))
		return false;

	// Serialise header first...
	GeonetPacket::serialize(buffer);
	// ...then append communication profile item count
	u_int8_t payloadIndex = sizeof(MessageHeader);
	Util::encode2byteInteger(buffer, payloadIndex, mib.communicationProfileMap.size());
	payloadIndex += 2;
	// ...and `reserved' field
	Util::encode2byteInteger(buffer, payloadIndex, 0x0000);
	payloadIndex += 2;

	// ...and communication profile item(s)
	map<CommunicationProfileID, CommunicationProfileResponseItem>::iterator iterator;
	while (iterator != mib.communicationProfileMap.end()) {
		Util::encode4byteInteger(buffer, payloadIndex, iterator->second.id);
		payloadIndex += 4;

		buffer[payloadIndex++] = iterator->second.transport;
		buffer[payloadIndex++] = iterator->second.network;
		buffer[payloadIndex++] = iterator->second.access;
		buffer[payloadIndex++] = iterator->second.channel;

		// Now `payloadIndex' points to the next available place
		++iterator;
	}

	// Resize incoming buffer
	buffer.resize(sizeof(CommunicationProfileResponse) + mib.communicationProfileMap.size()
			* sizeof(CommunicationProfileResponseItem));

	return true;
}

string GeonetCommunicationProfileResponsePacket::toString() const {
	stringstream ss;

	ss << GeonetPacket::toString() << endl;
	ss << "Communication profile count: " << mib.communicationProfileMap.size() << endl;

	map<CommunicationProfileID, CommunicationProfileResponseItem>::iterator iterator;
	while (iterator != mib.communicationProfileMap.end()) {
		ss << "Communication Profile [ID:" << iterator->second.id
			<< ", transport:" << iterator->second.transport
			<< ", network:" << iterator->second.network
			<< ", access: " << iterator->second.access
			<< ", channel: " << iterator->second.channel << "]" << endl;
	}

	return ss.str();
}
