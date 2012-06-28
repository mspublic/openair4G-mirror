/*
 * mgmt_gn_packet_location_table_response.hpp
 *
 *  Created on: May 21, 2012
 *      Author: demiray
 */

#ifndef MGMT_GN_PACKET_LOCATION_TABLE_RESPONSE_HPP_
#define MGMT_GN_PACKET_LOCATION_TABLE_RESPONSE_HPP_

#include "mgmt_gn_packet.hpp"
#include "../mgmt_information_base.hpp"
#include <string>
using namespace std;

/**
 * A container for Location Table Response Event packet
 */
class GeonetLocationTableResponseEventPacket: public GeonetPacket {
	public:
		/**
		 * Constructor of GeonetLocationTableResponseEventPacket class
		 *
		 * @param mib Management Information Base reference to keep it up-to-date
		 * with incoming information
		 * @param packetBuffer std::vector containing packet data
		 */
		GeonetLocationTableResponseEventPacket(ManagementInformationBase& mib,
				const vector<unsigned char>& packetBuffer);

	public:
		/**
		 * Returns string representation of this packet
		 *
		 * @return std::string representation of this packet
		 */
		string toString() const;

	private:
		/**
		 * Parses incoming packet buffer and updates MIB with this information
		 *
		 * @param packetBuffer std::vector containing packet data
		 * @return true on success, false otherwise
		 */
		bool parse(const vector<unsigned char>& packetBuffer);

	private:
		/**
		 * Management Information Base reference to keep it up-to-date with incoming
		 * Location Table information
		 */
		ManagementInformationBase& mib;
};

#endif /* MGMT_GN_PACKET_LOCATION_TABLE_RESPONSE_HPP_ */
