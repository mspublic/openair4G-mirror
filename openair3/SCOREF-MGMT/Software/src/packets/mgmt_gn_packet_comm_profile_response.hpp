/*
 * mgmt_gn_packet_comm_profile_response.hpp
 *
 *  Created on: 10 Jun 2012
 *      Author: barisd
 */

#ifndef MGMT_GN_PACKET_COMM_PROFILE_RESPONSE_HPP_
#define MGMT_GN_PACKET_COMM_PROFILE_RESPONSE_HPP_

#include "mgmt_gn_packet.hpp"

/**
 * Communication Profile Response
 */
struct CommunicationProfileResponse {
	MessageHeader header;

	u_int16_t communicationProfileCount;
	u_int16_t reserved;
	/* CommunicationProfileItem(s) follow(s)... */
} __attribute__((packed));

/**
 * A container for Communication Profile Response event
 */
class GeonetCommunicationProfileResponsePacket : public GeonetPacket {
	public:
		/**
		 * Constructor for GeonetCommunicationProfileResponsePacket class
		 *
		 * @param mib Management Information Base reference
		 * @param communicationProfileRequest Communication Profile Request
		 */
		GeonetCommunicationProfileResponsePacket(ManagementInformationBase& mib, u_int32_t communicationProfileRequest);
		/**
		 * Destructor for GeonetCommunicationProfileResponsePacket class
		 */
		~GeonetCommunicationProfileResponsePacket();

	public:
		/**
		 * Serialises the packet into given buffer
		 *
		 * @param Vector to be used to serialise the packet into
		 * @return true on success, false otherwise
		 */
		bool serialize(vector<unsigned char>& buffer) const;
		/**
		 * Returns std::string representation of packet
		 *
		 * @return String representation of packet
		 */
		string toString() const;

	private:
		/**
		 * Management Information Base reference to fetch
		 * necessary information to build this packet
		 */
		ManagementInformationBase& mib;
		/**
		 * Communication Profile Request flag set to determine
		 * requested options and respond accordingly
		 */
		u_int32_t communicationProfileRequest;
};

#endif /* MGMT_GN_PACKET_COMM_PROFILE_RESPONSE_HPP_ */
