/*
 * mgmt_gn_packet_handler.h
 *
 *  Created on: Apr 30, 2012
 *      Author: demiray
 */

#ifndef MGMT_GN_PACKET_HANDLER_HPP_
#define MGMT_GN_PACKET_HANDLER_HPP_

#include <boost/array.hpp>
using namespace boost;

#include "packets/mgmt_gn_packet_comm_profile_request.hpp"
#include "packets/mgmt_gn_packet_location_table_response.hpp"
#include "packets/mgmt_gn_packet_get_configuration.hpp"
#include "packets/mgmt_gn_packet_wireless_state_response.hpp"
#include "packets/mgmt_gn_packet_network_state.hpp"
#include "mgmt_gn_packet_factory.hpp"
#include "mgmt_information_base.hpp"
#include "mgmt_client.hpp"

/**
 * A container with packet handling functionality, all the packets read on
 * the socket is passed here
 */
class GeonetMessageHandler {
	public:
		/**
		 * Constructor for GeonetMessageHandler class
		 *
		 * @param mib ManagementInformationBase reference
		 */
		GeonetMessageHandler(ManagementInformationBase& mib);
		/**
		 * Destructor for GeonetMessageHandler class
		 */
		~GeonetMessageHandler();

	public:
		/**
		 * Takes buffers of Geonet messages and dispatches them to relevant private
		 * methods after building a GeonetPacket object out of them
		 *
		 * @param packetBuffer Buffer carrying Geonet message
		 * @param client Socket information of sender client
		 * @return pointer to the response of type GeonetMessage
		 */
		GeonetPacket* handleGeonetMessage(const vector<unsigned char>& packetBuffer, const udp::endpoint& client);

	private:
		/**
		 * Handles a Get Configuration message creating its reply utilizing relevant
		 * PacketFactory method
		 *
		 * @param packet Pointer to Get Configuration packet object
		 * @return Reply to Get Configuration message
		 */
		GeonetPacket* handleGetConfigurationEvent(GeonetGetConfigurationEventPacket* packet);
		/**
		 * Handles a Network State message and triggers an update at MIB
		 *
		 * @param packet Pointer to Network State packet
		 * @return true on success, false otherwise
		 */
		bool handleNetworkStateEvent(GeonetNetworkStateEventPacket* packet);
		/**
		 * Handles a Wireless State Response message and triggers an update at MIB
		 *
		 * @param packet Pointer to incoming Wireless State Response packet
		 * @return true on success, false otherwise
		 */
		bool handleWirelessStateResponseEvent(GeonetWirelessStateResponseEventPacket* packet);
		/**
		 * Handles a Location Table Response packet
		 *
		 * @param Pointer to a Location Table Response packet
		 * @return true on success, false otherwise
		 */
		bool handleLocationTableResponse(GeonetLocationTableResponseEventPacket* packet);
		/**
		 * Handles a Communication Profile Request event message and creates a
		 * Communication Profile Response packet
		 *
		 * @param Pointer to a Communication Profile Request packet
		 * @return Pointer to a Communication Profile Response packet
		 */
		GeonetPacket* handleCommunicationProfileRequestEvent(GeonetCommunicationProfileRequestPacket* packet);

	private:
		/**
		 * GeonetPacketFactory object to hide packet generation details from PacketHandler class
		 */
		GeonetPacketFactory* packetFactory;
		/**
		 * ManagementInformationBase object to fetch necessary information when needed
		 */
		ManagementInformationBase& mib;
		/**
		 * State map holding clients' states
		 */
		map<ManagementClient, ManagementClient::ManagementClientState> clientState;
};

#endif /* MGMT_GN_PACKET_HANDLER_HPP_ */
