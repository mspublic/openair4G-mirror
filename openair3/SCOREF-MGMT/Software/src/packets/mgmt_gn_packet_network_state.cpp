/*
 * mgmt_gn_packet_network_state.cpp
 *
 *  Created on: May 11, 2012
 *      Author: demiray
 */

#include "mgmt_gn_packet_network_state.hpp"
#include "../util/mgmt_util.hpp"
#include <sstream>

GeonetNetworkStateEventPacket::GeonetNetworkStateEventPacket(ManagementInformationBase& mib, vector<unsigned char> packetBuffer)
	: GeonetPacket(packetBuffer), mib(mib) {
	if (parse(packetBuffer)) {
		cout << "MIB is updated with incoming network state information" << endl;
	}
}

GeonetNetworkStateEventPacket::~GeonetNetworkStateEventPacket() {
}

string GeonetNetworkStateEventPacket::toString() const {
	stringstream ss;

	return ss.str();
}

bool GeonetNetworkStateEventPacket::parse(const vector<unsigned char> packetBuffer) {
	if (packetBuffer.size() < sizeof(NetworkStateMessage))
		return false;

	unsigned int dataIndex = sizeof(MessageHeader);
	Util::parse4byteInteger(packetBuffer.data() + dataIndex, &mib.getNetworkState().rxPackets); dataIndex += 4;
	Util::parse4byteInteger(packetBuffer.data() + dataIndex, &mib.getNetworkState().rxBytes); dataIndex += 4;
	Util::parse4byteInteger(packetBuffer.data() + dataIndex, &mib.getNetworkState().txPackets); dataIndex += 4;
	Util::parse4byteInteger(packetBuffer.data() + dataIndex, &mib.getNetworkState().txBytes); dataIndex += 4;
	Util::parse4byteInteger(packetBuffer.data() + dataIndex, &mib.getNetworkState().toUpperLayerPackets); dataIndex += 4;
	Util::parse4byteInteger(packetBuffer.data() + dataIndex, &mib.getNetworkState().discardedPackets); dataIndex += 4;
	Util::parse4byteInteger(packetBuffer.data() + dataIndex, &mib.getNetworkState().duplicatePackets); dataIndex += 4;
	Util::parse4byteInteger(packetBuffer.data() + dataIndex, &mib.getNetworkState().forwardedPackets);

	return true;
}

