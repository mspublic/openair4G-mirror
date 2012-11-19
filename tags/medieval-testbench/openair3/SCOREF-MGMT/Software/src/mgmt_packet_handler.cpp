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
 * \file mgmt_gn_packet_handler.cpp
 * \brief A container with packet handling functionality, all the packets read on the socket is passed here
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "mgmt_packet_handler.hpp"
#include "util/mgmt_exception.hpp"
#include <boost/lexical_cast.hpp>
#include "mgmt_types.hpp"
#include "util/mgmt_util.hpp"
#include <exception>
#include <iostream>
#include <cstring>
using namespace std;

PacketHandler::PacketHandler(ManagementInformationBase& mib, Logger& logger) :
	mib(mib), logger(logger) {
	try {
		this->packetFactory = new GeonetPacketFactory(mib, logger);
	} catch (...) {
		throw Exception("Cannot allocate a Geonet Packet Factory!", logger);
	}
}

PacketHandler::~PacketHandler() {
	delete packetFactory;
}

bool PacketHandler::handle(UdpServer& client, const vector<unsigned char>& packetBuffer) {
	if (packetBuffer.size() < sizeof(MessageHeader)) {
		logger.error("Buffer size (" + boost::lexical_cast<string>(packetBuffer.size()) + " byte(s)) is not enough to carry a message!");
		logger.warning("Discarding packet...");
		return false;
	}

	logger.info("Incoming packet size is " + boost::lexical_cast<string>(packetBuffer.size()) + " byte(s)");
	u_int16_t eventType = GeonetPacket::parseEventTypeOfPacketBuffer(packetBuffer);
	logger.info("Event field has the value " + boost::lexical_cast<string>(eventType));

	switch (eventType) {
		case MGMT_GN_EVENT_CONF_REQUEST:
		case MGMT_FAC_EVENT_CONF_REQUEST:
			return handleGetConfigurationEvent(client, new GeonetGetConfigurationEventPacket(packetBuffer, logger));

		case MGMT_GN_EVENT_STATE_WIRELESS_STATE_RESPONSE:
			if (handleWirelessStateResponseEvent(new GeonetWirelessStateResponseEventPacket(mib, packetBuffer, logger))) {
				logger.info("Wireless state event message processed");
				return NULL;
			}
			break;

		case MGMT_GN_EVENT_STATE_NETWORK_STATE:
			if (handleNetworkStateEvent(new GeonetNetworkStateEventPacket(mib, packetBuffer, logger))) {
				logger.info("Network state event message processed");
				/**
				 * todo this comment is no more functional, fix this!
				 * If the first message we have received from the client is a
				 * periodic network state then there was a configuration request
				 * that has been lost (cause the client was started before the
				 * server), so here we need to send them all
				 */
			}
			break;

		case MGMT_GN_EVENT_CONF_COMM_PROFILE_REQUEST:
		case MGMT_FAC_EVENT_CONF_COMM_PROFILE_REQUEST:
			return handleCommunicationProfileRequestEvent(client, new GeonetCommunicationProfileRequestPacket(packetBuffer, logger));

		case MGMT_GN_EVENT_LOCATION_TABLE_RESPONSE:
			if (handleLocationTableResponse(new GeonetLocationTableResponseEventPacket(mib, packetBuffer, logger))) {
				logger.info("Location table response packet processed");
			}
			break;

		case MGMT_FAC_EVENT_CONF_NOTIFICATION:
			if (handleConfigurationNotification(new FacConfigurationNotificationPacket(mib, packetBuffer, logger))) {
				logger.info("An incoming Configuration Notification packet has been processed");
			}
			break;

		/**
		 * Handle unexpected packets as well
		 */
		case MGMT_GN_EVENT_LOCATION_TABLE_REQUEST:
		case MGMT_GN_EVENT_CONF_UPDATE_AVAILABLE:
		case MGMT_GN_EVENT_CONF_CONT_RESPONSE:
		case MGMT_FAC_EVENT_CONF_CONT_RESPONSE:
		case MGMT_GN_EVENT_CONF_BULK_RESPONSE:
		case MGMT_FAC_EVENT_CONF_BULK_RESPONSE:
		case MGMT_GN_EVENT_CONF_COMM_PROFILE_RESPONSE:
		case MGMT_FAC_EVENT_CONF_COMM_PROFILE_RESPONSE:
		case MGMT_GN_EVENT_STATE_WIRELESS_STATE_REQUEST:
			logger.error("Unexpected packet (event: " + boost::lexical_cast<string>(eventType) + ") received, connected client is buggy");
			logger.error("Ignoring...");
			break;

		case MGMT_EVENT_ANY:
		default:
			logger.error("Unknown message received, ignoring...");
			break;
	}

	return NULL;
}

bool PacketHandler::handleGetConfigurationEvent(UdpServer& client, GeonetGetConfigurationEventPacket* request) {
	if (!request)
		return false;

	GeonetPacket* reply = NULL;

	/**
	 * Create a response according to the request and send to the client right away
	 */
	reply = this->packetFactory->createSetConfigurationEventPacket(static_cast<ItsKeyID> (request->getConfID()));

	if (client.send(*reply))
		logger.info("A reply for a Get Configuration packet has been sent");
	else
		logger.warning("Cannot send a Set Configuration in exchange for a Get Configuration!");

	/**
	 * Clean up
	 */
	delete request;
	delete reply;

	return true;
}

bool PacketHandler::handleNetworkStateEvent(GeonetNetworkStateEventPacket* request) {
	// Creation of a GeonetNetworkStateEventPacket is enough for processing...
	return true;
}

bool PacketHandler::handleWirelessStateResponseEvent(GeonetWirelessStateResponseEventPacket* request) {
	// Creation of a GeonetWirelessStateEventPacket is enough for processing...
	return true;
}

bool PacketHandler::handleLocationTableResponse(GeonetLocationTableResponseEventPacket* packet) {
	// Creation of a GeonetLocationTableResponseEventPacket is enough for processing...
	return true;
}

bool PacketHandler::handleConfigurationNotification(FacConfigurationNotificationPacket* packet) {
	// Creation of a FacConfigurationNotificationPacket is enough to handle it
	return true;
}

bool PacketHandler::handleCommunicationProfileRequestEvent(UdpServer& client, GeonetCommunicationProfileRequestPacket* request) {
	if (!request)
		return false;

	GeonetPacket* reply = NULL;

	/**
	 * Create a response according to the request and send to the client right away
	 */
	reply = this->packetFactory->createCommunicationProfileResponse(request);

	if (client.send(*reply))
		logger.info("A reply for a Communication Profile Request has been sent");
	else
		logger.warning("Cannot send a Communication Profile Response in exchange for a Communication Profile Request!");

	/**
	 * Clean up
	 */
	delete request;
	delete reply;

	return true;
}
