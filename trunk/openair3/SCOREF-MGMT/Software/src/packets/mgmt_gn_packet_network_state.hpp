/*
 * mgmt_gn_packet_network_state.hpp
 *
 *  Created on: May 11, 2012
 *      Author: demiray
 */

#ifndef MGMT_GN_PACKET_NETWORK_STATE_HPP_
#define MGMT_GN_PACKET_NETWORK_STATE_HPP_

#include "mgmt_gn_packet.hpp"
#include "../mgmt_information_base.hpp"

/**
 * A container for Network State Event packet
 */
class GeonetNetworkStateEventPacket: public GeonetPacket {
	public:
		/**
		 * Constructor of GeonetNetworkStateEventPacket class
		 *
		 * @param mib Management Information Base reference to keep it up-to-date
		 * with incoming information
		 * @param packetBuffer Buffer containing Network State Event packet (which
		 * is going to be parsed using parse())
		 */
		GeonetNetworkStateEventPacket(ManagementInformationBase& mib, vector<unsigned char> packetBuffer);
		/**
		 * Destructor of GeonetNetworkStateEventPacket class
		 */
		~GeonetNetworkStateEventPacket();

	public:
		/**
		 * Returns string representation of this packet
		 *
		 * @return std::string representation of this packet
		 */
		string toString() const;

	private:
		/**
		 * Parses incoming packet buffer and updates MIB
		 *
		 * @param packetBuffer Buffer containing packet data
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

#endif /* MGMT_GN_PACKET_NETWORK_STATE_HPP_ */

