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

#include "mgmt_gn_packet_handler.hpp"
#include "mgmt_gn_datatypes.hpp"
#include "util/mgmt_util.hpp"
#include <arpa/inet.h>
#include <exception>
#include <iostream>
#include <cstring>
using namespace std;

GeonetMessageHandler::GeonetMessageHandler(ManagementInformationBase& mib, Logger& logger) :
	mib(mib), logger(logger) {
	try {
		this->packetFactory = new GeonetPacketFactory(mib, logger);
	} catch (std::exception& e) {
		logger.error(e.what());
	}
}

GeonetMessageHandler::~GeonetMessageHandler() {
	delete packetFactory;
}

GeonetPacket* GeonetMessageHandler::handleGeonetMessage(const vector<unsigned char>& packetBuffer, const udp::endpoint& client) {
	if (packetBuffer.size() < sizeof(MessageHeader)) {
		logger.error("Buffer size is not enough to carry a Geonet message!");
		return NULL;
	}

	logger.info("Incoming packet size in bytes is " + packetBuffer.size());

	MessageHeader* header = (MessageHeader*) packetBuffer.data();

	u_int16_t eventType = header->eventType;
	eventType <<= 8;
	eventType |= header->eventSubtype;

	logger.info("Event field has the value " + eventType);

	switch (eventType) {
		case MGMT_GN_EVENT_CONF_REQUEST:
			clientState[ManagementClient(client)] = ManagementClient::CONNECTED;
			return handleGetConfigurationEvent(new GeonetGetConfigurationEventPacket(packetBuffer, logger));

		case MGMT_GN_EVENT_STATE_WIRELESS_STATE_RESPONSE:
			clientState[ManagementClient(client)] = ManagementClient::ONLINE;
			if (handleWirelessStateResponseEvent(new GeonetWirelessStateResponseEventPacket(mib, packetBuffer, logger))) {
				logger.info("Wireless state event message processed");
				return NULL;
			}
			break;

		case MGMT_GN_EVENT_STATE_NETWORK_STATE:
			if (handleNetworkStateEvent(new GeonetNetworkStateEventPacket(mib, packetBuffer, logger))) {
				logger.info("Network state event message processed");
				/**
				 * If the first message we have received from the client is a
				 * periodic network state then there was a configuration request
				 * that has been lost (cause the client was started before the
				 * server), so here we need to send them all
				 */
				if (clientState[ManagementClient(client)] == ManagementClient::OFFLINE) {
					clientState[ManagementClient(client)] = ManagementClient::ONLINE;
					return packetFactory->createSetConfigurationEventPacket(MGMT_GN_ITSKEY_ALL);
				} else {
					clientState[ManagementClient(client)] = ManagementClient::ONLINE;
					return NULL;
				}
			}
			break;

		case MGMT_GN_EVENT_CONF_COMM_PROFILE_REQUEST:
			clientState[ManagementClient(client)] = ManagementClient::ONLINE;
			return handleCommunicationProfileRequestEvent(new GeonetCommunicationProfileRequestPacket(packetBuffer, logger));

		case MGMT_GN_EVENT_LOCATION_TABLE_RESPONSE:
			if (handleLocationTableResponse(new GeonetLocationTableResponseEventPacket(mib, packetBuffer, logger))) {
				logger.info("Location table response packet processed");
			}
			clientState[ManagementClient(client)] = ManagementClient::ONLINE;
			break;

			/*
			 * Unexpected message handling
			 */
		case MGMT_GN_EVENT_LOCATION_TABLE_REQUEST:
		case MGMT_GN_EVENT_CONF_UPDATE_AVAILABLE:
		case MGMT_GN_EVENT_CONF_CONT_RESPONSE:
		case MGMT_GN_EVENT_CONF_BULK_RESPONSE:
		case MGMT_GN_EVENT_CONF_COMM_PROFILE_RESPONSE:
		case MGMT_GN_EVENT_STATE_WIRELESS_STATE_REQUEST:
			// todo logger.error("Unexpected packet (event: " + eventType + ") received, connected client is buggy");
			logger.error("Ignoring...");
			break;

		case MGMT_GN_EVENT_ANY:
		default:
			logger.error("Unknown message received, ignoring...");
			break;
	}

	return NULL;
}

GeonetPacket* GeonetMessageHandler::handleGetConfigurationEvent(GeonetGetConfigurationEventPacket* request) {
	if (!request)
		return NULL;

	GeonetPacket* reply = NULL;

	reply = this->packetFactory->createSetConfigurationEventPacket(static_cast<ItsKeyID> (request->getConfID()));

	delete request;
	return reply;
}

bool GeonetMessageHandler::handleNetworkStateEvent(GeonetNetworkStateEventPacket* request) {
	// Creation of a GeonetNetworkStateEventPacket is enough for processing...
	return true;
}

bool GeonetMessageHandler::handleWirelessStateResponseEvent(GeonetWirelessStateResponseEventPacket* request) {
	// Creation of a GeonetWirelessStateEventPacket is enough for processing...
	return true;
}

bool GeonetMessageHandler::handleLocationTableResponse(GeonetLocationTableResponseEventPacket* packet) {
	// Creation of a GeonetLocationTableResponseEventPacket is enough for processing...
	return true;
}

GeonetPacket* GeonetMessageHandler::handleCommunicationProfileRequestEvent(
		GeonetCommunicationProfileRequestPacket* request) {
	if (!request)
		return NULL;

	GeonetPacket* reply = NULL;

	reply = this->packetFactory->createCommunicationProfileResponse(request);

	delete request;
	return reply;
}
