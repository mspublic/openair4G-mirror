/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2012 Eurecom

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
 * \file mgmt_fac_packet_comm_profile_selection_request.cpp
 * \brief A container for Communication Profile Selection Request event
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "mgmt_fac_packet_comm_profile_selection_request.hpp"
#include "../util/mgmt_util.hpp"

#include <sstream>

FacCommunicationProfileSelectionRequestPacket::FacCommunicationProfileSelectionRequestPacket(const vector<unsigned char>& packetBuffer, Logger& logger)
	: GeonetPacket(packetBuffer, logger) {
	parse(packetBuffer);
}

FacCommunicationProfileSelectionRequestPacket::~FacCommunicationProfileSelectionRequestPacket() {}

u_int8_t FacCommunicationProfileSelectionRequestPacket::getLatency() const {
	return packet.latency;
}

u_int8_t FacCommunicationProfileSelectionRequestPacket::getRelevance() const {
	return packet.relevance;
}

u_int8_t FacCommunicationProfileSelectionRequestPacket::getReliability() const {
	return packet.reliability;
}

bool FacCommunicationProfileSelectionRequestPacket::parse(const vector<unsigned char>& packetBuffer) {
	if (packetBuffer.size() < sizeof(CommunicationProfileSelectionRequest))
		return false;

	/* Parse octets */
	u_int8_t payloadIndex = sizeof(MessageHeader);
	packet.latency = packetBuffer[payloadIndex++];
	packet.relevance = packetBuffer[payloadIndex++];
	packet.reliability = packetBuffer[payloadIndex++];
	packet.reserved = packetBuffer[payloadIndex];

	return true;
}

string FacCommunicationProfileSelectionRequestPacket::toString() const {
	stringstream ss;

	ss << GeonetPacket::toString() << endl
		<< "Latency: " << hex << (int)packet.latency << endl
		<< "Relevance: " << hex << (int)packet.relevance << endl
		<< "Reliability: " << hex << (int)packet.reliability << endl;

	return ss.str();
}
