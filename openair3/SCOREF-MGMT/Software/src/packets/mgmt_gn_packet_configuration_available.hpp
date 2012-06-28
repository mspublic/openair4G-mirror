/*
 * mgmt_gn_packet_configuration_available.hpp
 *
 *  Created on: May 10, 2012
 *      Author: demiray
 */

#ifndef MGMT_GN_PACKET_CONFIGURATION_AVAILABLE_HPP_
#define MGMT_GN_PACKET_CONFIGURATION_AVAILABLE_HPP_

#include <vector>
#include "mgmt_gn_packet.hpp"
#include "../mgmt_information_base.hpp"

/**
 * A container for Configuration Available Event
 */
class GeonetConfigurationAvailableEventPacket : public GeonetPacket {
	public:
		/**
		 * Constructor for GeonetConfigurationAvailableEventPacket class
		 *
		 * @param mib Management Information Base reference to fetch necessary
		 * information to build this packet
		 */
		GeonetConfigurationAvailableEventPacket(ManagementInformationBase& mib);
		/**
		 * Destructor for GeonetConfigurationAvailableEventPacket class
		 */
		~GeonetConfigurationAvailableEventPacket();

	public:
		/**
		 * Serialises packet header and payload into given buffer
		 *
		 * @param buffer Vector buffer that the packet will be serialised into
		 * @return true on success, false otherwise
		 */
		bool serialize(vector<unsigned char>& buffer);
		/**
		 * Returns string representation of the packet
		 *
		 * @return std::string representation of the packet
		 */
		string toString() const;

	private:
		/**
		 * Management Information Base reference to fetch necessary
		 * information to build this packet
		 */
		ManagementInformationBase& mib;
};

#endif /* MGMT_GN_PACKET_CONFIGURATION_AVAILABLE_HPP_ */
