/*
 * mgmt_gn_packet_wireless_state.hpp
 *
 *  Created on: May 10, 2012
 *      Author: demiray
 */

#ifndef MGMT_GN_PACKET_WIRELESS_STATE_RESPONSE_HPP_
#define MGMT_GN_PACKET_WIRELESS_STATE_RESPONSE_HPP_

#include "mgmt_gn_packet.hpp"
#include "../mgmt_information_base.hpp"

/**
 * A container for Wireless State Event Response packet
 */
class GeonetWirelessStateResponseEventPacket : public GeonetPacket {
	public:
		/**
		 * Constructor for GeonetWirelessStateResponseEventPacket class
		 *
		 * @param mib Management Information Base reference to update it
		 * with incoming information
		 * @param packetBuffer Packet data as a vector
		 */
		GeonetWirelessStateResponseEventPacket(ManagementInformationBase& mib, const vector<unsigned char>& packetBuffer);
		/**
		 * Destructor for GeonetWirelessStateEventPacket class
		 */
		~GeonetWirelessStateResponseEventPacket();

	public:
		/**
		 * Returns string representation of this packet
		 *
		 * @return std::string representation of this packet
		 */
		string toString() const;

	private:
		/**
		 * Parses given buffer and updates Geonet::header and Management Information Base
		 *
		 * @param packetBuffer std::vector keeping packet data
		 * @return true on success, false otherwise
		 */
		bool parse(const vector<unsigned char> packetBuffer);

	private:
		/**
		 * Management Information Base reference to keep it up-to-date with
		 * incoming information
		 */
		ManagementInformationBase& mib;
};

#endif /* MGMT_GN_PACKET_WIRELESS_STATE_RESPONSE_HPP_ */
