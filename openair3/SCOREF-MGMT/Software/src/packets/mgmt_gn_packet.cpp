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
 * \file mgmt_gn_packets.cpp
 * \brief Superclass for all Management-Geonet messages
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "mgmt_gn_packet.hpp"
#include "../util/mgmt_util.hpp"
#include <sstream>
#include <iostream>
using namespace std;

GeonetPacket::GeonetPacket(bool extendedMessage, bool validity, u_int8_t version, u_int8_t priority,
    u_int16_t eventType, Logger& logger) : logger(logger) {
	Util::resetBuffer(reinterpret_cast<unsigned char*>(&header), sizeof(MessageHeader));

	if (extendedMessage)
		this->header.version |= 0x80;
	if (validity)
		this->header.version |= 0x40;

	this->header.version |= (version & 0x0f);
	this->header.priority = priority;
	this->header.priority <<= 5;
	this->header.eventType = (eventType >> 8);
	this->header.eventSubtype = (eventType & 0x0f);
}

GeonetPacket::GeonetPacket(const vector<unsigned char>& packetBuffer, Logger& logger)
	: logger(logger) {
	parseHeaderBuffer(packetBuffer, &this->header);
	logger.info(toString());
}

GeonetPacket::~GeonetPacket() {
}

bool GeonetPacket::parseHeaderBuffer(const vector<unsigned char>& headerBuffer, MessageHeader* header) {
	if (headerBuffer.size() < sizeof(MessageHeader) || !header)
		return false;

	header->version = headerBuffer[0] & 0x0f;
	header->priority = (headerBuffer[1] >> 5);
	header->eventType = headerBuffer[2];
	header->eventSubtype = headerBuffer[3];

	return true;
}

bool GeonetPacket::serialize(vector<unsigned char>& buffer) const {
	logger.debug("Serialising header...");

	buffer[0] = header.version;
	buffer[0] |= 0x40; // encode Validity flag as 1
	buffer[1] = header.priority;
	buffer[1] <<= 5;
	buffer[2] = header.eventType;
	buffer[3] = header.eventSubtype;

	return true;
}

string GeonetPacket::toString() const {
	stringstream ss;

	// todo write extended message and validity fields here as well
	ss << "GeonetHeader[version:" << (int) header.version << ", priority:" << (int) header.priority
		<< ", event:" << (int) (header.eventType * 100 + header.eventSubtype) << "]";

	return ss.str();
}
