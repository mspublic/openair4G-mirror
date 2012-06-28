/*
 * mgmt_gn_packet_set_configuration.hpp
 *
 *  Created on: May 10, 2012
 *      Author: demiray
 */

#ifndef MGMT_GN_PACKET_SET_CONFIGURATION_HPP_
#define MGMT_GN_PACKET_SET_CONFIGURATION_HPP_

#include "mgmt_gn_packet.hpp"

/**
 * A container for Set Configuration Event packet
 */
class GeonetSetConfigurationEventPacket: public GeonetPacket {
	public:
		/**
		 * Constructor for GeonetSetConfigurationEventPacket class
		 *
		 * @param mib Management Information Base reference to fetch necessary
		 * configuration information
		 * @param itsKeyId Requested ITS key or key set
		 */
		GeonetSetConfigurationEventPacket(ManagementInformationBase& mib, ItsKeyID itsKeyID = MGMT_GN_ITSKEY_ALL);
		/**
		 * Destructor of GeonetSetConfigurationEventPacket class
		 */
		virtual ~GeonetSetConfigurationEventPacket();

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
		 * Serialises given configuration item into given buffer
		 *
		 * @param buffer Unsigned char array of buffer to serialise configuration into
		 * @param configurationItem Configuration item that'll be serialised into given buffer
		 * @return true on success, false otherwise
		 */
		bool encodeConfigurationItem(unsigned char* buffer, const ConfigurationItem* configurationItem) const;
		/**
		 * Creates a ConfigurationItem by fetching necessary information from
		 * Management Information Base
		 */
		ConfigurationItem buildConfigurationItem(ItsKeyID itsKey) const;

	private:
		/**
		 * Boolean for request type: true for bulk request, false for a request
		 * for a single ITS key
		 */
		bool isBulk;
		/**
		 * Requested ITS key
		 */
		ItsKeyID requestedItsKey;
		/**
		 * Requested ITS key type
		 */
		ItsKeyType requestedItsKeyType;
		/**
		 * Management Information Base reference
		 */
		ManagementInformationBase& mib;
};

#endif /* MGMT_GN_PACKET_SET_CONFIGURATION_HPP_ */
