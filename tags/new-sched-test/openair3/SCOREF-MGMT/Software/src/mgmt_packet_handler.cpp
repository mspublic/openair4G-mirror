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
  Forums       : http://forums.eurecom.fr/openairinterface
  Address      : EURECOM, Campus SophiaTech, 450 Route des Chappes, 06410 Biot FRANCE

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
		this->packetFactory = new ManagementPacketFactory(mib, logger);
	} catch (...) {
		throw Exception("Cannot allocate a Geonet Packet Factory!", logger);
	}
}

PacketHandler::PacketHandler(const PacketHandler& packetHandler) :
	mib(packetHandler.mib), logger(packetHandler.logger) {
	throw Exception("Copy constructor for a PacketHandler object is called!", logger);
}

PacketHandler::~PacketHandler() {
	delete packetFactory;
}

PacketHandlerResult* PacketHandler::handle(const vector<unsigned char>& packetBuffer) {
	if (packetBuffer.size() < sizeof(MessageHeader)) {
		logger.error("Buffer size (" + boost::lexical_cast<string>(packetBuffer.size()) + " byte(s)) is not enough to carry a message!");
		logger.warning("Discarding packet...");
		return new PacketHandlerResult(PacketHandlerResult::INVALID_PACKET, NULL);
	}

	logger.info("Incoming packet size is " + boost::lexical_cast<string>(packetBuffer.size()) + " byte(s)");
	u_int16_t eventType = GeonetPacket::parseEventTypeOfPacketBuffer(packetBuffer);
	logger.info("Event field has the value " + boost::lexical_cast<string>(eventType));

	switch (eventType) {
		case MGMT_GN_EVENT_CONF_REQUEST:
		case MGMT_FAC_EVENT_CONF_REQUEST:
			logger.info("GET_CONFIGURATION packet of size " + boost::lexical_cast<string>(packetBuffer.size()) + " has been received");
			return handleGetConfigurationEvent(new GeonetGetConfigurationEventPacket(packetBuffer, logger));

		case MGMT_GN_EVENT_STATE_NETWORK_STATE:
			logger.info("NETWORK_STATE packet of size " + boost::lexical_cast<string>(packetBuffer.size()) + " has been received");
			return handleNetworkStateEvent(new GeonetNetworkStateEventPacket(mib, packetBuffer, logger));

		case MGMT_GN_EVENT_STATE_WIRELESS_STATE_RESPONSE:
			logger.info("WIRELESS_STATE_RESPONSE packet of size " + boost::lexical_cast<string>(packetBuffer.size()) + " has been received");
			return handleWirelessStateResponseEvent(new GeonetWirelessStateResponseEventPacket(mib, packetBuffer, logger));

		case MGMT_GN_EVENT_CONF_COMM_PROFILE_REQUEST:
		case MGMT_FAC_EVENT_CONF_COMM_PROFILE_REQUEST:
			logger.info("COMMUNICATION_PROFILE_REQUEST packet of size " + boost::lexical_cast<string>(packetBuffer.size()) + " has been received");
			return handleCommunicationProfileRequestEvent(new GeonetCommunicationProfileRequestPacket(packetBuffer, logger));

		case MGMT_GN_EVENT_LOCATION_TABLE_RESPONSE:
			logger.info("LOCATION_TABLE_RESPONSE packet of size " + boost::lexical_cast<string>(packetBuffer.size()) + " has been received");
			return handleLocationTableResponse(new GeonetLocationTableResponseEventPacket(mib, packetBuffer, logger));

		case MGMT_FAC_EVENT_CONF_NOTIFICATION:
			logger.info("CONFIGURATION_NOTIFICATION packet of size " + boost::lexical_cast<string>(packetBuffer.size()) + " has been received");
			return handleConfigurationNotification(new FacConfigurationNotificationPacket(mib, packetBuffer, logger));

		case MGMT_FAC_EVENT_LOCATION_UPDATE:
			logger.info("LOCATION_UPDATE packet of size " + boost::lexical_cast<string>(packetBuffer.size()) + "has been received");
			return handleLocationUpdate(new GeonetLocationUpdateEventPacket(mib, packetBuffer, logger));

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
			return new PacketHandlerResult(PacketHandlerResult::INVALID_PACKET, NULL);

		case MGMT_EVENT_ANY:
		default:
			logger.error("Unknown message received, ignoring...");
			return new PacketHandlerResult(PacketHandlerResult::INVALID_PACKET, NULL);
	}

	return new PacketHandlerResult(PacketHandlerResult::DISCARD_PACKET, NULL);
}

PacketHandlerResult* PacketHandler::handleGetConfigurationEvent(GeonetGetConfigurationEventPacket* request) {
	if (!request)
		return new PacketHandlerResult(PacketHandlerResult::INVALID_PACKET, NULL);

	/**
	 * Create a response according to the request
	 */
	GeonetPacket* reply = this->packetFactory->createSetConfigurationEventPacket(static_cast<ItsKeyID> (request->getConfID()));

	/**
	 * Clean up
	 */
	delete request;

	return new PacketHandlerResult(PacketHandlerResult::DELIVER_PACKET, reply);
}

PacketHandlerResult* PacketHandler::handleNetworkStateEvent(GeonetNetworkStateEventPacket* request) {
	delete request;
	/*
	 * Creation of a GeonetNetworkStateEventPacket is enough for processing...
	 */
	return new PacketHandlerResult(PacketHandlerResult::DISCARD_PACKET, NULL);
}

PacketHandlerResult* PacketHandler::handleWirelessStateResponseEvent(GeonetWirelessStateResponseEventPacket* request) {
	delete request;
	/*
	 * Creation of a GeonetWirelessStateEventPacket is enough for processing...
	 */
	return new PacketHandlerResult(PacketHandlerResult::DISCARD_PACKET, NULL);
}

PacketHandlerResult* PacketHandler::handleLocationTableResponse(GeonetLocationTableResponseEventPacket* packet) {
	delete packet;
	/*
	 * Creation of a GeonetLocationTableResponseEventPacket is enough for processing...
	 */
	return new PacketHandlerResult(PacketHandlerResult::DISCARD_PACKET, NULL);
}

PacketHandlerResult* PacketHandler::handleConfigurationNotification(FacConfigurationNotificationPacket* packet) {
	// TODO Update MIB with incoming ITS key configuration update
	return new PacketHandlerResult(PacketHandlerResult::DISCARD_PACKET, NULL);
}

PacketHandlerResult* PacketHandler::handleCommunicationProfileRequestEvent(GeonetCommunicationProfileRequestPacket* request) {
	if (!request)
		return new PacketHandlerResult(PacketHandlerResult::INVALID_PACKET, NULL);

	/**
	 * Create a response according to the request and send to the client right away
	 */
	GeonetPacket* reply = this->packetFactory->createCommunicationProfileResponse(request);

	/**
	 * Clean up
	 */
	delete request;

	return new PacketHandlerResult(PacketHandlerResult::DELIVER_PACKET, reply);
}

PacketHandlerResult* PacketHandler::handleLocationUpdate(GeonetLocationUpdateEventPacket* packet) {
	delete packet;
	/*
	 * Creation of a GeonetWirelessStateEventPacket is enough for processing...
	 */
	return new PacketHandlerResult(PacketHandlerResult::DISCARD_PACKET, NULL);
}
