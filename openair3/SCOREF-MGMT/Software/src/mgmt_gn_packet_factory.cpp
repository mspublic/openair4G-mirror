/*
 * mgmt_gn_packet_factory.cpp
 *
 *  Created on: May 2, 2012
 *      Author: demiray
 */

#include "packets/mgmt_gn_packet_comm_profile_response.hpp"
#include "packets/mgmt_gn_packet_set_configuration.hpp"
#include "mgmt_gn_packet_factory.hpp"
#include <iostream>
using namespace std;

GeonetPacketFactory::GeonetPacketFactory(ManagementInformationBase& mib) :
	mib(mib) {
}

GeonetPacket* GeonetPacketFactory::createSetConfigurationEventPacket(ItsKeyID itsKeyID) {
	return new GeonetSetConfigurationEventPacket(mib, itsKeyID);
}

GeonetPacket* GeonetPacketFactory::createCommunicationProfileResponse(GeonetCommunicationProfileRequestPacket* request) {
	return new GeonetCommunicationProfileResponsePacket(mib, request->getCommunicationProfileRequestSet());
}
