/*
 * mgmt_gn_packet_get_configuration.hpp
 *
 *  Created on: May 9, 2012
 *      Author: demiray
 */

#ifndef MGMT_GN_PACKET_GET_CONFIGURATION_HPP_
#define MGMT_GN_PACKET_GET_CONFIGURATION_HPP_

#include <string>
#include <vector>
#include "mgmt_gn_packet.hpp"
using namespace std;

/**
 * A container for Get Configuration Event packet
 */
class GeonetGetConfigurationEventPacket: public GeonetPacket {
	public:
		/**
		 * Constructor of GeonetGetConfigurationEventPacket class
		 *
		 * @param packetBuffer Incoming packet buffer that is going to be parsed
		 */
		GeonetGetConfigurationEventPacket(const vector<unsigned char>& packetBuffer);

	public:
		/**
		 * Returns configuration ID that is requested
		 *
		 * @return Requested configuration ID
		 */
		u_int16_t getConfID() const;
		/**
		 * Returns TX mode that is requested
		 *
		 * @return Requested TX mode (bulk or single)
		 */
		u_int16_t getTxMode() const;
		/**
		 * Returns string representation of this packet
		 *
		 * @return std::string representation of this packet
		 */
		string toString() const;

	private:
		/**
		 * Parses incoming packet data and updates `packet' member
		 *
		 * @param packetBuffer std::vector carrying packet data
		 * @return true on success, false otherwise
		 */
		bool parse(const vector<unsigned char>& packetBuffer);

	private:
		/**
		 * Packet that will hold parsed information
		 */
		ConfigurationRequestMessage packet;
};

#endif /* MGMT_GN_PACKET_GET_CONFIGURATION_HPP_ */
