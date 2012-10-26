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
 * \file mgmt_packet_handler.hpp
 * \brief A container with packet handling functionality, all the packets read on the socket is passed here
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
 */

#ifndef MGMT_PACKET_HANDLER_HPP_
#define MGMT_PACKET_HANDLER_HPP_

#include <boost/array.hpp>
using namespace boost;

#include "packets/mgmt_fac_packet_configuration_notification.hpp"
#include "packets/mgmt_gn_packet_location_table_response.hpp"
#include "packets/mgmt_gn_packet_wireless_state_response.hpp"
#include "packets/mgmt_gn_packet_comm_profile_request.hpp"
#include "packets/mgmt_gn_packet_get_configuration.hpp"
#include "packets/mgmt_gn_packet_network_state.hpp"
#include "mgmt_information_base.hpp"
#include "util/mgmt_udp_server.hpp"
#include "mgmt_packet_factory.hpp"
#include "util/mgmt_log.hpp"
#include "mgmt_client.hpp"

/**
 * A container with packet handling functionality, all the packets read on
 * the socket is passed here
 */
class PacketHandler {
	public:
		/**
		 * Constructor for PacketHandler class
		 *
		 * @param mib ManagementInformationBase reference
		 * @param logger Logger object reference
		 */
		PacketHandler(ManagementInformationBase& mib, Logger& logger);
		/**
		 * Destructor for PacketHandler class
		 */
		~PacketHandler();

	private:
		/**
		 * Copy constructor to prevent the usage of default one
		 */
		PacketHandler(const PacketHandler& packetHandler);

	public:
		/**
		 * Takes buffer of a packet and processes accordingly
		 *
		 * @param client Sender client
		 * @param packetBuffer Packet buffer
		 * @return true on success, false otherwise
		 */
		bool handle(UdpServer& client, const vector<unsigned char>& packetBuffer);

	private:
		/**
		 * Handles a Get Configuration message creating its reply utilizing relevant
		 * PacketFactory method
		 *
		 * @param client Sender of GetConfiguration packet
		 * @param packet Pointer to Get Configuration packet object
		 * @return true on success, false otherwise
		 */
		bool handleGetConfigurationEvent(UdpServer& client, GeonetGetConfigurationEventPacket* packet);
		/**
		 * Handles a Network State message and triggers an update at MIB
		 *
		 * @param packet Pointer to Network State packet
		 * @return true on success, false otherwise
		 */
		static bool handleNetworkStateEvent(GeonetNetworkStateEventPacket* packet);
		/**
		 * Handles a Wireless State Response message and triggers an update at MIB
		 *
		 * @param packet Pointer to incoming Wireless State Response packet
		 * @return true on success, false otherwise
		 */
		static bool handleWirelessStateResponseEvent(GeonetWirelessStateResponseEventPacket* packet);
		/**
		 * Handles a Location Table Response packet
		 *
		 * @param Pointer to a Location Table Response packet
		 * @return true on success, false otherwise
		 */
		static bool handleLocationTableResponse(GeonetLocationTableResponseEventPacket* packet);
		/**
		 * Handles a Configuration Notification packet
		 *
		 * @param Pointer to a Configuration Notification packet
		 * @return true on success, false otherwise
		 */
		static bool handleConfigurationNotification(FacConfigurationNotificationPacket* packet);
		/**
		 * Handles a Communication Profile Request event message and creates a
		 * Communication Profile Response packet
		 *
		 * @param client Sender of CoummunicationProfileRequest packet
		 * @param Pointer to a Communication Profile Request packet
		 * @return true on success, false otherwise
		 */
		bool handleCommunicationProfileRequestEvent(UdpServer& client, GeonetCommunicationProfileRequestPacket* packet);

	private:
		/**
		 * GeonetPacketFactory object to hide packet generation details from PacketHandler class
		 */
		ManagementPacketFactory* packetFactory;
		/**
		 * ManagementInformationBase object to fetch necessary information when needed
		 */
		ManagementInformationBase& mib;
		/**
		 * Logger object reference
		 */
		Logger& logger;
};

#endif /* MGMT_PACKET_HANDLER_HPP_ */
