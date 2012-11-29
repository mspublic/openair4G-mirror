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
 * \file mgmt_packet_factory.cpp
 * \brief A container with necessary (mostly responses) packet generation functionality
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "packets/mgmt_gn_packet_comm_profile_response.hpp"
#include "packets/mgmt_gn_packet_set_configuration.hpp"
#include "mgmt_packet_factory.hpp"
#include <iostream>
using namespace std;

ManagementPacketFactory::ManagementPacketFactory(ManagementInformationBase& mib, Logger& logger) :
	mib(mib), logger(logger) {
}

GeonetPacket* ManagementPacketFactory::createSetConfigurationEventPacket(ItsKeyID itsKeyID) {
	return new GeonetSetConfigurationEventPacket(mib, logger, itsKeyID);
}

GeonetPacket* ManagementPacketFactory::createCommunicationProfileResponse(GeonetCommunicationProfileRequestPacket* request) {
	return new GeonetCommunicationProfileResponsePacket(mib, request->getCommunicationProfileRequestSet(), logger);
}
