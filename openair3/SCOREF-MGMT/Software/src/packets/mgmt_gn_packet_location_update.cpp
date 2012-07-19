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

	// todo serialise packet
	return true;
}
