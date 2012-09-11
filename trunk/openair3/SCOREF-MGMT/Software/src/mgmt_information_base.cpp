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
 * \file mgmt_information_base.cpp
 * \brief A container to hold configuration parameters of Management entity
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "mgmt_information_base.hpp"
#include "util/mgmt_exception.hpp"
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

ManagementInformationBase::ManagementInformationBase(Logger& logger)
	: itsKeyManager(logger), communicationProfileManager(logger), logger(logger) {
	logger.info("Management Information Base has been created");
}

ManagementInformationBase::~ManagementInformationBase() {}

bool ManagementInformationBase::initialise() {
	try {
		// Common Parameters
		itsKeyManager.addKey(MGMT_GN_ALL_ITSKEY_ID_STATION_TYPE, "MIB_GN_ALL_STATION_TYPE", ITS_KEY_TYPE_COMMON, 1, 1, 30);
		itsKeyManager.addKey(MGMT_GN_ALL_ITSKEY_ID_STATION_SUBTYPE, "MIB_GN_ALL_STATION_SUBTYPE", ITS_KEY_TYPE_COMMON, 1, 0, 1);
		// NETwork Parameters
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_LOCAL_ADD_CONF_METHOD, "MIB_GN_NET_LOCAL_ADDR_CONF_METHOD", ITS_KEY_TYPE_NET, 0, 0, 1);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_DEFAULT_HOP_LIMIT, "MIB_GN_NET_DEFAULT_HOP_LIMIT", ITS_KEY_TYPE_NET, 1, 0, 255);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_MAX_PKT_LIFETIME, "MIB_GN_NET_MAX_PACKET_LIFETIME", ITS_KEY_TYPE_NET, 20000, 1, 6300000);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_MIN_PKT_REPETITION_INTERVAL, "MIB_GN_NET_MIN_PACKET_REPETITION_INTERVAL", ITS_KEY_TYPE_NET, 1000);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_GEO_BCAST_FORWARDING_ALG, "MIB_GN_NET_GEO_BCAST_FORWARDING_ALGORITHM", ITS_KEY_TYPE_NET, 0, 0, 2);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_GEO_UCAST_FORWARDING_ALG, "MIB_GN_NET_GEO_UCAST_FORWARDING_ALGORITHM", ITS_KEY_TYPE_NET, 0, 0, 3);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_TRAFFIC_CLASS_RELEVANCE, "MIB_GN_NET_TRAFFIC_CLASS_RELEVANCE", ITS_KEY_TYPE_NET, 5, 0, 7);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_TRAFFIC_CLASS_RELIABILITY, "MIB_GN_NET_TRAFFIC_CLASS_RELIABILITY", ITS_KEY_TYPE_NET, 2, 0, 3);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_TRAFFIC_CLASS_LATENCY, "MIB_GN_NET_TRAFFIC_CLASS_LATENCY", ITS_KEY_TYPE_NET, 2, 0, 3);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_CBF_MIN_TTS, "MIB_GN_NET_CBF_MIN_TTS", ITS_KEY_TYPE_NET, 100);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_CBF_MAX_TTS, "MIB_GN_NET_CBF_MAX_TTS", ITS_KEY_TYPE_NET, 500);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_MAX_COMM_RANGE, "MIB_GN_NET_MAX_COMM_RANGE", ITS_KEY_TYPE_NET, 1000);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_DEF_TX_POWER, "MIB_GN_NET_DEF_TX_POWER", ITS_KEY_TYPE_NET, 5);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_DEF_BITRATE, "MIB_GN_NET_DEF_BITRATE", ITS_KEY_TYPE_NET, 12);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_DEF_CHANNEL, "MIB_GN_NET_DEF_CHANNEL", ITS_KEY_TYPE_NET, 178);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_DEF_PRIORITY, "MIB_GN_NET_DEF_PRIORITY", ITS_KEY_TYPE_NET, 5, 0, 7);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_GN_DEF_CHANNEL_BW, "MIB_GN_NET_DEF_CHANNEL_BW", ITS_KEY_TYPE_NET, 30);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_SEC_ALLOW_UNSECURE, "MIB_GN_NET_SEC_ALLOW_UNSECURE", ITS_KEY_TYPE_NET, 1);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_SEC_END_2_END, "MIB_GN_NET_SEC_END2END", ITS_KEY_TYPE_NET, 0);
		itsKeyManager.addKey(MGMT_GN_NET_ITSKEY_ID_SEC_PSEUDONYM, "MIB_GN_NET_SEC_PSEUDONYM", ITS_KEY_TYPE_NET, 0);
		// FACilities Parameters
		itsKeyManager.addKey(MGMT_GN_FAC_ITSKEY_ID_VEHICLE_WIDTH, "MIB_GN_FAC_VEHICLE_WIDTH", ITS_KEY_TYPE_FAC, 3);
		itsKeyManager.addKey(MGMT_GN_FAC_ITSKEY_ID_VEHICLE_LENGTH, "MIB_GN_FAC_VEHICLE_LENGTH", ITS_KEY_TYPE_FAC, 7);
		itsKeyManager.addKey(MGMT_GN_FAC_ITSKEY_ID_CAM_BTP_PORT, "MIB_GN_FAC_CAM_BTP_PORT", ITS_KEY_TYPE_FAC, 2000);
		itsKeyManager.addKey(MGMT_GN_FAC_ITSKEY_ID_DENM_BTP_PORT, "MIB_GN_FAC_DENM_BTP_PORT", ITS_KEY_TYPE_FAC, 3000);
		itsKeyManager.addKey(MGMT_GN_FAC_ITSKEY_ID_LDM_GARBAGE_COLLECTION_INTERVAL, "MIB_GN_FAC_LDM_GARBAGE_COLLECTION_INTERVAL", ITS_KEY_TYPE_FAC, 1000);
	} catch (Exception& e) {
		e.updateStackTrace("Cannot define ITS key");
		throw e;
	}

	logger.info("Management Information Base has been initialised");
	logger.info("Number of ITS key elements defined is " + boost::lexical_cast<string>(itsKeyManager.getNumberOfKeys()));

	return true;
}

bool ManagementInformationBase::setValue(ItsKeyID id, ItsKeyValue value) {
	try {
		itsKeyManager.setKeyValue(id, value);
	} catch (Exception& e) {
		e.updateStackTrace("Cannot set ITS key using its ID");
		throw e;
	}

	return true;
}

bool ManagementInformationBase::setValue(const string& name, ItsKeyValue value) {
	if (name.empty())
		throw Exception("Incoming parameter name is empty!", logger);

	try {
		itsKeyManager.setKeyValue(name, value);
	} catch (Exception& e) {
		e.updateStackTrace("Cannot set ITS key using its name");
		throw e;
	}

	return true;
}

ItsKeyValue ManagementInformationBase::getItsKeyIntegerValue(ItsKeyID id) {
	return itsKeyManager.getKeyIntegerValue(id);
}

u_int8_t ManagementInformationBase::getLength(ItsKeyID itsKey) const {
	// This is the DWORD-length so it's 1
	return 1;
}

ItsKeyManager& ManagementInformationBase::getItsKeyManager() {
	return this->itsKeyManager;
}

WirelessStateResponseItem& ManagementInformationBase::getWirelessState(InterfaceID interfaceId) {
	return wirelessStateMap[interfaceId];
}

bool ManagementInformationBase::updateWirelessState(InterfaceID interfaceId, WirelessStateResponseItem wirelessState) {
	wirelessStateMap.insert(wirelessStateMap.end(), pair<InterfaceID, WirelessStateResponseItem>(interfaceId, wirelessState));

	return true;
}

NetworkStateMessage& ManagementInformationBase::getNetworkState() {
	return networkState;
}

CommunicationProfileManager& ManagementInformationBase::getCommunicationProfileManager() {
	return communicationProfileManager;
}

bool ManagementInformationBase::updateLocationTable(LocationTableItem& locationTableItem) {
	locationTable.insert(locationTable.end(), pair<GnAddress, LocationTableItem>(locationTableItem.gnAddress, locationTableItem));

	return true;
}

LocationInformation ManagementInformationBase::getLocation() {
	/**
	 * todo this is temporary, location information will be received somewhere else later on
	 */
	srand(time(NULL));
	location.latitude = rand() % 20 + 10;
	location.longitude = rand() % 10 + 5;
	location.speed = rand() % 20;
	location.heading = 0;
	location.altitude = rand() % 1000 + 10;

	return location;
}

bool ManagementInformationBase::setNetworkFlags(const u_int8_t& networkFlags) {
	this->networkFlags = networkFlags;

	return true;
}
