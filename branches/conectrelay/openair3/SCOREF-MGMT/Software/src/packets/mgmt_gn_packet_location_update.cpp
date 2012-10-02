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
 * \file mgmt_gn_packet_location_update.cpp
 * \brief A container for Location Update Event packet
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "mgmt_gn_packet_location_update.hpp"
#include <boost/lexical_cast.hpp>
#include <ctime>

GeonetLocationUpdateEventPacket::GeonetLocationUpdateEventPacket(ManagementInformationBase& mib, Logger& logger)
	: GeonetPacket(false, true, 0x00, 0x00, MGMT_GN_EVENT_LOCATION_UPDATE, logger), mib(mib), logger(logger) {
}

GeonetLocationUpdateEventPacket::~GeonetLocationUpdateEventPacket() {
}

bool GeonetLocationUpdateEventPacket::serialize(vector<unsigned char>& buffer) const {
	if (buffer.size() < sizeof(LocationUpdateMessage)) {
		logger.warning("Incoming buffer (size:" + boost::lexical_cast<string>(buffer.size()) + ") is not enough to carry a Location Update packet!");
		return false;
	}

	/**
	 * Serialise header first..
	 */
	if (!GeonetPacket::serialize(buffer)) {
		logger.error("Cannot serialise packet header");
		return false;
	}

	/**
	 * ..and then the body
	 */
	u_int8_t payloadIndex = sizeof(MessageHeader);
	LocationInformation location = mib.getLocation();
	/**
	 * Update time-stamp
	 */
	location.timestamp = time(NULL); // todo timestamp's calculation is explained in lot 3 slides

	Util::encode4byteInteger(buffer, payloadIndex, location.timestamp); payloadIndex += 4;
	Util::encode4byteInteger(buffer, payloadIndex, location.latitude); payloadIndex += 4;
	Util::encode4byteInteger(buffer, payloadIndex, location.longitude); payloadIndex += 4;
	Util::encode2byteInteger(buffer, payloadIndex, location.speed); payloadIndex += 2;
	Util::encode2byteInteger(buffer, payloadIndex, location.heading); payloadIndex += 2;
	Util::encode2byteInteger(buffer, payloadIndex, location.altitude); payloadIndex += 2;
	/**
	 * Encode last two octets carrying 5 flags
	 */
	u_int8_t octet = 0x00;
	Util::encodeBits(octet, 0, location.TAcc, 4);
	Util::encodeBits(octet, 4, location.PosAcc, 4);
	buffer[payloadIndex++] = octet;
	Util::encodeBits(octet, 0, location.SAcc, 2);
	Util::encodeBits(octet, 2, location.Hacc, 3);
	Util::encodeBits(octet, 5, location.AltAcc, 3);
	buffer[payloadIndex] = octet;

	buffer.resize(sizeof(LocationUpdateMessage));
	return true;
}
