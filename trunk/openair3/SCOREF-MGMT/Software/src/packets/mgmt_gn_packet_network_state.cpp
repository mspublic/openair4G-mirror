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
 * \file mgmt_gn_packet_network_state.cpp
 * \brief A container for Network State Event packet
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
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

