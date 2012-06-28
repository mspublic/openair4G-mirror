/*
 * mgmt_gn_packet_handler.cpp
 *
 *  Created on: Apr 30, 2012
 *      Author: demiray
 */

#include "mgmt_gn_packet_handler.hpp"
#include "mgmt_gn_datatypes.hpp"
#include <iostream>
#include <exception>
#include <cstring>
#include <arpa/inet.h>
using namespace std;

GeonetMessageHandler::GeonetMessageHandler(ManagementInformationBase& mib) :
	mib(mib) {
	try {
		this->packetFactory = new GeonetPacketFactory(mib);
	} catch (std::exception& e) {
		cerr << e.what() << endl;
	}
}

GeonetMessageHandler::~GeonetMessageHandler() {
	delete packetFactory;
}

GeonetPacket* GeonetMessageHandler::handleGeonetMessage(const vector<unsigned char>& packetBuffer, const udp::endpoint& client) {
	if (packetBuffer.size() < sizeof(MessageHeader))
		return NULL; // todo throw an exception here

	cout << "Incoming message size is " << packetBuffer.size() << " byte(s)" << endl;

	MessageHeader* header = (MessageHeader*) packetBuffer.data();

	u_int16_t eventType = header->eventType;
	eventType <<= 8;
	eventType |= header->eventSubtype;

	cout << "Event field has the value " << eventType << endl;

	switch (eventType) {
		case MGMT_GN_EVENT_CONF_REQUEST:
			clientState[ManagementClient(client)] = ManagementClient::CONNECTED;
			return handleGetConfigurationEvent(new GeonetGetConfigurationEventPacket(packetBuffer));

		case MGMT_GN_EVENT_STATE_WIRELESS_STATE_RESPONSE:
			clientState[ManagementClient(client)] = ManagementClient::ONLINE;
			if (handleWirelessStateResponseEvent(new GeonetWirelessStateResponseEventPacket(mib, packetBuffer))) {
				cout << "Wireless state event message processed" << endl;
				return NULL;
			}
			break;

		case MGMT_GN_EVENT_STATE_NETWORK_STATE:
			if (handleNetworkStateEvent(new GeonetNetworkStateEventPacket(mib, packetBuffer))) {
				cout << "Network state event message processed" << endl;
				/*
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
			return handleCommunicationProfileRequestEvent(new GeonetCommunicationProfileRequestPacket(packetBuffer));

		case MGMT_GN_EVENT_LOCATION_TABLE_RESPONSE:
			if (handleLocationTableResponse(new GeonetLocationTableResponseEventPacket(mib, packetBuffer))) {
				cout << "Location table response packet processed" << endl;
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
			cerr << "Unexpected packet (event: " << eventType << ") received, connected client is buggy" << endl;
			cerr << "Ignoring..." << endl;
			break;

		case MGMT_GN_EVENT_ANY:
		default:
			cerr << "Unknown message received, ignoring..." << endl;
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
