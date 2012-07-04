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
 * \file mgmt_gn_packet_location_table_response.hpp
 * \brief A container for Location Table Response Event packet
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "mgmt_gn_packet_location_table_response.hpp"
#include "../util/mgmt_util.hpp"
#include <sstream>

GeonetLocationTableResponseEventPacket::GeonetLocationTableResponseEventPacket(ManagementInformationBase& mib, const vector<unsigned char>& packetBuffer)
	: GeonetPacket(packetBuffer), mib(mib) {
	parse(packetBuffer);
}

string GeonetLocationTableResponseEventPacket::toString() const {
	stringstream ss;

	return ss.str();
}

bool GeonetLocationTableResponseEventPacket::parse(const vector<unsigned char>& packetBuffer) {
	u_int16_t lpvCount = 0;
	u_int16_t packetBufferIndex = sizeof(MessageHeader);

	// Parse LPV Count...
	if (Util::parse2byteInteger(packetBuffer.data() + packetBufferIndex, &lpvCount)) {
		cout << "Location table response has " << lpvCount << " entr{y|ies}" << endl;
	} else {
		cerr << "Cannot parse location table entry count" << endl;
		return false;
	}
	// ...and Network Flags
	mib.networkFlags = packetBuffer[2];

	u_int16_t itemIndex = packetBufferIndex;
	for (; lpvCount != 0; lpvCount--) {
		LocationTableItem item;

		Util::parse8byteInteger(packetBuffer.data() + itemIndex, &item.gnAddress); itemIndex += sizeof(GnAddress);
		Util::parse4byteInteger(packetBuffer.data() + itemIndex, &item.timestamp); itemIndex += sizeof(u_int32_t);
		Util::parse4byteInteger(packetBuffer.data() + itemIndex, &item.latitude); itemIndex += sizeof(u_int32_t);
		Util::parse4byteInteger(packetBuffer.data() + itemIndex, &item.longitude); itemIndex += sizeof(u_int32_t);
		Util::parse2byteInteger(packetBuffer.data() + itemIndex, &item.speed); itemIndex += sizeof(u_int16_t);
		Util::parse2byteInteger(packetBuffer.data() + itemIndex, &item.heading); itemIndex += sizeof(u_int16_t);
		Util::parse2byteInteger(packetBuffer.data() + itemIndex, &item.altitude); itemIndex += sizeof(u_int16_t);
		Util::parse2byteInteger(packetBuffer.data() + itemIndex, &item.acceleration); itemIndex += sizeof(u_int16_t);
		Util::parse2byteInteger(packetBuffer.data() + itemIndex, &item.sequenceNumber); itemIndex += sizeof(u_int16_t);
		item.lpvFlags = packetBuffer.data()[itemIndex++];
		item.reserved = packetBuffer.data()[itemIndex++];

		// Update MIB with this record
		mib.locationTable.insert(mib.locationTable.end(), pair<GnAddress, LocationTableItem>(item.gnAddress, item));

		cout << "Management Information Base has been updated with following location table entry: " << endl;
		cout << item.toString() << endl;

		// itemIndex shows the next record now, if there's any
	}

	return true;
}
