/*
 * mgmt_gn_packet_wireless_state_request.hpp
 *
 *  Created on: Jun 22, 2012
 *      Author: demiray
 */

#ifndef MGMT_GN_PACKET_WIRELESS_STATE_REQUEST_HPP_
#define MGMT_GN_PACKET_WIRELESS_STATE_REQUEST_HPP_

#include "mgmt_gn_packet.hpp"

/**
 * A container for Wireless State Event Request packet
 */
class GeonetWirelessStateRequestEventPacket : public GeonetPacket {
	public:
		/**
		 * Constructor for GeonetWirelessStateRequestEventPacket class
		 */
		GeonetWirelessStateRequestEventPacket();
		/**
		 * Destructor for GeonetWirelessStateRequestEventPacket class
		 */
		~GeonetWirelessStateRequestEventPacket();

	public:
		/**
		 * Serialises packet information into incoming buffer
		 *
		 * @param buffer std::vector that packet information will be serialised into
		 * @return true on success, false otherwise
		 */
		bool serialize(vector<unsigned char>& buffer) const;
		/**
		 * Returns string representation of this packet
		 *
		 * @return std::string representation of this packet
		 */
		string toString() const;
};

#endif /* MGMT_GN_PACKET_WIRELESS_STATE_REQUEST_HPP_ */
