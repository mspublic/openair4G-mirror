/*
 * mgmt_gn_packets.h
 *
 *  Created on: May 2, 2012
 *      Author: demiray
 */

#ifndef MGMT_GN_PACKET_HPP_
#define MGMT_GN_PACKET_HPP_

#include "../mgmt_information_base.hpp"
#include "../mgmt_gn_datatypes.hpp"
#include <string>
#include <vector>
using namespace std;

/**
 * Superclass for all Management-Geonet messages
 */
class GeonetPacket {
	public:
		/**
		 * Constructor for GeonetPacket class
		 *
		 * @param extendedMessage Indicates if this an extended (vendor specific) message
		 * @param validity Indicates non-existent data
		 * @param version Version number
		 * @param priority Priority
		 * @param eventType 16-bit Event Type and Event Subtype information
		 */
		GeonetPacket(bool extendedMessage, bool validity, u_int8_t version, u_int8_t priority, u_int16_t eventType);
		/**
		 * Buffer-parser constructor for GeonetPacket class
		 *
		 * @param packetBuffer Buffer carrying GeonetPacket
		 */
		GeonetPacket(const vector<unsigned char>& packetBuffer);
		/**
		 * Virtual destructor for GeonetPacket class
		 */
		virtual ~GeonetPacket();

	public:
		/**
		 * Parses buffer and fills in incoming header structure
		 *
		 * @param headerBuffer Buffer carrying message header
		 * @param header Structure to be filled in
		 * @return true on success, false otherwise
		 */
		static bool parseHeaderBuffer(const vector<unsigned char>& headerBuffer, MessageHeader* header);
		/**
		 * Serialises header fields onto given buffer
		 * This method is called by every subclass::serialize()
		 *
		 * @param buffer Buffer to serialise buffer on
		 * @return true on success, false otherwise
		 */
		virtual bool serialize(vector<unsigned char>& buffer) const;
		/**
		 * Returns string representation of relevant GeonetPacket object
		 *
		 * @return std::string representation
		 */
		virtual string toString() const;

	protected:
		/**
		 * Header is kept in superclass for every Geonet* subclass
		 */
		MessageHeader header;
};

#endif /* MGMT_GN_PACKET_HPP_ */
