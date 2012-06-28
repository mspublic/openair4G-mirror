/*
 * mgmt_gn_packet_wireless_state.cpp
 *
 *  Created on: May 11, 2012
 *      Author: demiray
 */

#include "mgmt_gn_packet_wireless_state_response.hpp"
#include "../mgmt_util.hpp"
#include <sstream>

GeonetWirelessStateResponseEventPacket::GeonetWirelessStateResponseEventPacket(ManagementInformationBase& mib, const vector<unsigned char>& packetBuffer)
	: GeonetPacket(packetBuffer), mib(mib) {
	if (this->parse(packetBuffer)) {
		cout << "MIB is updated with incoming wireless state information" << endl;
	}
}

GeonetWirelessStateResponseEventPacket::~GeonetWirelessStateResponseEventPacket() {
}

string GeonetWirelessStateResponseEventPacket::toString() const {
	stringstream ss;

	return ss.str();
}

bool GeonetWirelessStateResponseEventPacket::parse(const vector<unsigned char> packetBuffer) {
	if (packetBuffer.size() < sizeof(WirelessStateResponseMessage))
		return false;

	// Parse interface count first
	u_int8_t interfaceCount = packetBuffer.data()[sizeof(MessageHeader)];
	cout << "There are " << interfaceCount << " interface(s)" << endl;

	// Then traverse the buffer to get the state for every interface...
	u_int16_t itemIndex = sizeof(WirelessStateResponseMessage);
	for (; interfaceCount != 0; interfaceCount--) {
		WirelessStateResponseItem item;

		Util::parse2byteInteger(packetBuffer.data() + itemIndex, &item.interfaceId); itemIndex += sizeof(InterfaceID);
		Util::parse2byteInteger(packetBuffer.data() + itemIndex, &item.accessTechnology); itemIndex += 2;
		Util::parse2byteInteger(packetBuffer.data() + itemIndex, &item.channelFrequency); itemIndex += 2;
		Util::parse2byteInteger(packetBuffer.data() + itemIndex, &item.bandwidth); itemIndex += 2;
		item.channelBusyRatio = static_cast<u_int8_t>(packetBuffer.data()[itemIndex]); ++itemIndex;
		item.status = static_cast<u_int8_t>(packetBuffer.data()[itemIndex]); ++itemIndex;
		item.averageTxPower = static_cast<u_int8_t>(packetBuffer.data()[itemIndex]); ++itemIndex;
		item.reserved = static_cast<u_int8_t>(packetBuffer.data()[itemIndex]); ++itemIndex;

		// Update MIB with this record
		mib.wirelessStateMap.insert(mib.wirelessStateMap.end(), pair<InterfaceID, WirelessStateResponseItem>(item.interfaceId, item));

		cout << "Management Information Base has been updated with following wireless state entry: " << endl;
		cout << item.toString() << endl;

		// itemIndex shows the next record now, if there's any
	}

	return true;
}
