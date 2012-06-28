/*
 * mgmt_gn_packet_location_table_request.hpp
 *
 *  Created on: May 21, 2012
 *      Author: demiray
 */

#ifndef MGMT_GN_PACKET_LOCATION_TABLE_REQUEST_HPP_
#define MGMT_GN_PACKET_LOCATION_TABLE_REQUEST_HPP_

#include "mgmt_gn_packet.hpp"
#include <string>
using namespace std;

/**
 * A container for Location Table Request Event packet
 */
class GeonetLocationTableRequestEventPacket: public GeonetPacket {
	public:
		/**
		 * Constructor of GeonetLocationTableRequestEventPacket class
		 *
		 * @param address GN Address that is going to be requested
		 */
		GeonetLocationTableRequestEventPacket(GnAddress address);

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

	private:
		/**
		 * GN address that is going to be questioned
		 */
		GnAddress gnAddress;
};

#endif /* MGMT_GN_PACKET_LOCATION_TABLE_REQUEST_HPP_ */
