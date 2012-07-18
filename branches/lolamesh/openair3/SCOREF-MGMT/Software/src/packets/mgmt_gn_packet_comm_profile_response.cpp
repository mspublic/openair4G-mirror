/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2010 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*!
 * \file mgmt_gn_packet_comm_profile_response.cpp
 * \brief A container for Communication Profile Response event
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "mgmt_gn_packet_comm_profile_response.hpp"
#include "../util/mgmt_util.hpp"

GeonetCommunicationProfileResponsePacket::GeonetCommunicationProfileResponsePacket(ManagementInformationBase& mib,
		u_int32_t communicationProfileRequest, Logger& logger) :
	GeonetPacket(false, true, 0x00, 0x00, MGMT_GN_EVENT_CONF_COMM_PROFILE_RESPONSE, logger), mib(mib) {
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
	Util::encode2byteInteger(buffer, payloadIndex, mib.communicationProfileManager.getProfileCount());
	payloadIndex += 2;
	// ...and `reserved' field
	Util::encode2byteInteger(buffer, payloadIndex, 0x0000);
	payloadIndex += 2;

	// ...and communication profile item(s)
	map<CommunicationProfileID, CommunicationProfileItem>::iterator iterator;
	while (iterator != mib.communicationProfileManager.getProfileMap().end()) {
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
	buffer.resize(sizeof(CommunicationProfileResponse) + mib.communicationProfileManager.getProfileCount()
			* sizeof(CommunicationProfileItem));

	return true;
}

string GeonetCommunicationProfileResponsePacket::toString() const {
	stringstream ss;

	ss << GeonetPacket::toString() << endl;
	ss << mib.communicationProfileManager.toString() << endl;

	return ss.str();
}
