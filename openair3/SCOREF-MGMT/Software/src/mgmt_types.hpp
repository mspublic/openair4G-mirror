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
 * \file mgmt_gn_packets.h
 * \brief Definitions of common data types used in SCOREF Management Module
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#ifndef MGMT_GN_DATATYPES_HPP_
#define MGMT_GN_DATATYPES_HPP_

#include <sys/types.h>
#include <sstream>
#include <string>
using namespace std;

/**
 * Configuration Item IDs
 */
enum ItsKeyID {
	/*
	 * Common ITS keys
	 */
	MGMT_GN_ALL_ITSKEY_ID_STATION_TYPE 						= 0,
	MGMT_GN_ALL_ITSKEY_ID_STATION_SUBTYPE					= 1,
	/*
	 * Network ITS keys
	 */
	MGMT_GN_NET_ITSKEY_ID_GN_LOCAL_ADD_CONF_METHOD			= 1000,
	MGMT_GN_NET_ITSKEY_ID_DEFAULT_HOP_LIMIT					= 1001,
	MGMT_GN_NET_ITSKEY_ID_GN_MAX_PKT_LIFETIME				= 1002,
	MGMT_GN_NET_ITSKEY_ID_GN_MIN_PKT_REPETITION_INTERVAL	= 1003,
	MGMT_GN_NET_ITSKEY_ID_GN_GEO_BCAST_FORWARDING_ALG		= 1010,
	MGMT_GN_NET_ITSKEY_ID_GN_GEO_UCAST_FORWARDING_ALG		= 1011,
	MGMT_GN_NET_ITSKEY_ID_GN_TRAFFIC_CLASS_RELEVANCE		= 1020,
	MGMT_GN_NET_ITSKEY_ID_GN_TRAFFIC_CLASS_RELIABILITY		= 1021,
	MGMT_GN_NET_ITSKEY_ID_GN_TRAFFIC_CLASS_LATENCY			= 1022,
	MGMT_GN_NET_ITSKEY_ID_GN_CBF_MIN_TTS					= 1030,
	MGMT_GN_NET_ITSKEY_ID_GN_CBF_MAX_TTS					= 1031,
	MGMT_GN_NET_ITSKEY_ID_GN_MAX_COMM_RANGE					= 1040,
	MGMT_GN_NET_ITSKEY_ID_GN_DEF_TX_POWER					= 1050,
	MGMT_GN_NET_ITSKEY_ID_GN_DEF_BITRATE					= 1051,
	MGMT_GN_NET_ITSKEY_ID_GN_DEF_CHANNEL					= 1052,
	MGMT_GN_NET_ITSKEY_ID_GN_DEF_PRIORITY					= 1053,
	MGMT_GN_NET_ITSKEY_ID_GN_DEF_CHANNEL_BW					= 1054,
	MGMT_GN_NET_ITSKEY_ID_SEC_ALLOW_UNSECURE				= 2000,
	MGMT_GN_NET_ITSKEY_ID_SEC_END_2_END						= 2001,
	MGMT_GN_NET_ITSKEY_ID_SEC_PSEUDONYM						= 2002,
	/*
	 * FACilities ITS keys
	 */
	MGMT_GN_FAC_ITSKEY_ID_VEHICLE_WIDTH						= 2,
	MGMT_GN_FAC_ITSKEY_ID_VEHICLE_LENGTH					= 3,
	MGMT_GN_FAC_ITSKEY_ID_CAM_BTP_PORT						= 3010,
	MGMT_GN_FAC_ITSKEY_ID_DENM_BTP_PORT						= 3011,
	MGMT_GN_FAC_ITSKEY_ID_LDM_GARBAGE_COLLECTION_INTERVAL	= 3020,
	/*
	 * Configuration set ITS keys
	 */
	MGMT_GN_ITSKEY_SET_NET									= 0xaaaa,
	MGMT_GN_ITSKEY_SET_FAC									= 0xbbbb,
	MGMT_GN_ITSKEY_ALL										= 0xffff
};

/**
 * Message & Event Types
 */
enum EventType {
	/**
	 * Any
	 */
	MGMT_EVENT_ANY = 0x000,
	/**
	 * Location
	 */
	MGMT_GN_EVENT_LOCATION_UPDATE = 0x100,
	MGMT_GN_EVENT_LOCATION_TABLE_REQUEST = 0x101,
	MGMT_FAC_EVENT_LOCATION_TABLE_REQUEST = 0x103,
	MGMT_GN_EVENT_LOCATION_TABLE_RESPONSE = 0x102,
	MGMT_FAC_EVENT_LOCATION_TABLE_RESPONSE = 0x104,
	/**
	 * Configuration
	 */
	MGMT_GN_EVENT_CONF_UPDATE_AVAILABLE = 0x300,
	MGMT_GN_EVENT_CONF_REQUEST = 0x301,
	MGMT_FAC_EVENT_CONF_REQUEST = 0x311,
	MGMT_GN_EVENT_CONF_CONT_RESPONSE = 0x302,
	MGMT_FAC_EVENT_CONF_CONT_RESPONSE = 0x312,
	MGMT_GN_EVENT_CONF_BULK_RESPONSE = 0x303,
	MGMT_FAC_EVENT_CONF_BULK_RESPONSE = 0x313,
	MGMT_GN_EVENT_CONF_COMM_PROFILE_REQUEST = 0x304,
	MGMT_FAC_EVENT_CONF_COMM_PROFILE_REQUEST = 0x314,
	MGMT_GN_EVENT_CONF_COMM_PROFILE_RESPONSE = 0x305,
	MGMT_FAC_EVENT_CONF_COMM_PROFILE_RESPONSE = 0x315,
	/**
	 * State
	 */
	MGMT_GN_EVENT_STATE_WIRELESS_STATE_REQUEST = 0x402,
	MGMT_GN_EVENT_STATE_WIRELESS_STATE_RESPONSE = 0x403,
	MGMT_GN_EVENT_STATE_NETWORK_STATE = 0x404
};

/**
 * Configuration Message / Transmission Type
 */
enum ConfigurationTransmissionMode {
	C2X_MGMT_GN_CONF_TX_CONT = 0,
	C2X_MGMT_GN_CONF_TX_BULK = 1
};

/**
 * Message Header
 */
struct MessageHeader {
	u_int8_t version;
	u_int8_t priority;
	u_int8_t eventType;
	u_int8_t eventSubtype;
} __attribute__((packed));

/**
 * Location Information
 */
struct LocationInformation {
	u_int32_t timestamp; /* Time in milliseconds */
	u_int32_t latitude;  /* Latitude in 1/10 micro-degree */
	u_int32_t longitude; /* Longitude in 1/10 micro-degree */
	u_int16_t speed;	 /* Speed in signed units of 1 meter */
	u_int16_t heading;
	u_int16_t altitude;

	unsigned TAcc:4;
	unsigned PosAcc:4;
	unsigned SAcc:2;
	unsigned Hacc:3;
	unsigned AltAcc:3;
} __attribute__((packed));

/**
 * Update Location Event
 */
struct LocationUpdateMessage {
	MessageHeader header;

	LocationInformation location;
} __attribute__((packed));

typedef u_int64_t GnAddress;

/**
 * Query Location Table Event Message
 */
struct LocationTableRequest {
	MessageHeader header;

	GnAddress gnAddress;
} __attribute__((packed));

/**
 * Location Table Response Item
 */
struct LocationTableItem {
	GnAddress gnAddress;
	u_int32_t timestamp; /* Time in milliseconds */
	u_int32_t latitude;  /* Latitude in 1/10 micro-degree */
	u_int32_t longitude; /* Longitude in 1/10 micro-degree */
	u_int16_t speed;	 /* Speed in signed units of 1 meter */
	u_int16_t heading;
	u_int16_t altitude;
	u_int16_t acceleration;
	u_int16_t sequenceNumber;
	u_int8_t lpvFlags;
	u_int8_t reserved;

	string toString() const {
		stringstream ss;

		ss << "[gnAddr:" << gnAddress << ", "
			<< "timestamp:" << timestamp << ", "
			<< "lat.:" << latitude << ", "
			<< "long.:" << longitude << ", "
			<< "speed:" << speed << ", "
			<< "heading:" << heading << ", "
			<< "alt.:" << altitude << ", "
			<< "accel.:" << acceleration << ", "
			<< "seqNum:" << sequenceNumber << ", "
			<< "lpvFlags:" << (int)lpvFlags << ", "
			<< "res.:" << (int)reserved << "]";

		return ss.str();
	}
} __attribute__((packed));

/**
 * Reply Location Event
 */
struct LocationTableResponse {
	MessageHeader header;

	u_int16_t lpvCount;
	u_int8_t networkFlags;
	u_int8_t reserved;

	// Location table items will follow
} __attribute__((packed));

/**
 * Wireless State Request Message
 */
struct WirelessStateRequestMessage {
	MessageHeader header;
} __attribute__((packed));

/**
 * Wireless State Response Message
 */
struct WirelessStateResponseMessage {
	MessageHeader header;

	u_int8_t interfaceCount;
	u_int8_t reserved_first8;
	u_int16_t reserved_last16;
} __attribute__((packed));

/**
 * Wireless State of a Certain Interface
 */
typedef u_int16_t InterfaceID;
struct WirelessStateResponseItem {
	InterfaceID interfaceId;
	u_int16_t accessTechnology;
	u_int16_t channelFrequency;
	u_int16_t bandwidth;
	u_int8_t channelBusyRatio;
	u_int8_t status;
	u_int8_t averageTxPower;
	u_int8_t reserved;

	string toString() const {
		stringstream ss;

		ss << "Interface ID: " << interfaceId << endl
			<< "Access Technology: " << accessTechnology << endl
			<< "Channel Frequency: " << channelFrequency << endl
			<< "Bandwidth: " << bandwidth << endl
			<< "Channel Busy Ratio: " << (int)channelBusyRatio << endl
			<< "Status: " << (int)status << endl
			<< "Average TX Power: " << (int)averageTxPower << endl
			<< "Reserved: " << (int)reserved << endl;

		return ss.str();
	}
} __attribute__((packed));

/**
 * Network State Message
 */
struct NetworkStateMessage {
	MessageHeader header;

	u_int32_t rxPackets;
	u_int32_t rxBytes;
	u_int32_t txPackets;
	u_int32_t txBytes;
	u_int32_t toUpperLayerPackets;
	u_int32_t discardedPackets;
	u_int32_t duplicatePackets;
	u_int32_t forwardedPackets;
} __attribute__((packed));

/**
 * Configuration Available Message
 */
struct ConfigureAvailableMessage {
	MessageHeader header;

	u_int16_t reserved;
	u_int16_t keyCount;
} __attribute__((packed));

/**
 * Configuration Request Message
 */
struct ConfigurationRequestMessage {
	MessageHeader header;

	u_int16_t configurationId;
	u_int16_t transmissionMode;
} __attribute__((packed));

/**
 * Configuration Item
 *
 * This struct holds the common "configuration item" fields of
 * continuous and bulk configuration set messages
 */
struct ConfigurationItem {
	u_int16_t configurationId;
	u_int16_t length; /* # of DWORDs */
	u_int32_t configurationValue;
} __attribute__((packed));

/**
 * Set Configuration Event (Continuous)
 */
struct ContinuousConfigurationResponse {
	MessageHeader header;

	ConfigurationItem configurationItem;
} __attribute__((packed));

/**
 * Set Configuration Event (Bulk)
 *
 * This struct is processed with a std::vector<ConfigurationItem>
 * See GeonetSetConfigurationEventPacket.{hpp|cpp}
 */
struct BulkConfigurationResponse {
	MessageHeader header;

	u_int16_t reserved;
	u_int16_t keyCount;
} __attribute__((packed));

#endif /* MGMT_GN_DATATYPES_HPP_ */
