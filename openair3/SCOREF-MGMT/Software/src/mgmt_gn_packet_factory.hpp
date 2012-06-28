/*
 * mgmt_gn_packet_factory.hpp
 *
 *  Created on: Apr 30, 2012
 *      Author: demiray
 */

#ifndef MGMT_GN_PACKET_FACTORY_HPP_
#define MGMT_GN_PACKET_FACTORY_HPP_

#include "packets/mgmt_gn_packet_comm_profile_request.hpp"
#include "packets/mgmt_gn_packet.hpp"
#include "mgmt_gn_datatypes.hpp"
#include "mgmt_information_base.hpp"

/**
 * A container with necessary (mostly responses) packet generation functionality
 */
class GeonetPacketFactory {
	public:
		/**
		 * Constructor for GeonetPacketFactory class
		 *
		 * @param mib Management Information Base reference
		 */
		GeonetPacketFactory(ManagementInformationBase& mib);

	public:
		/**
		 * Creates and returns a Set Configuration packet whose type is determined
		 * by the default parameter
		 *
		 * @param itsKeyID ITS key ID, if this has the default value MGMT_GN_ITSKEY_ALL
		 * then this method creates a bulk response, otherwise generated packet will carry
		 * specified ITS key only
		 * @return Pointer to the GeonetPacket created
		 */
		GeonetPacket* createSetConfigurationEventPacket(ItsKeyID itsKeyID = MGMT_GN_ITSKEY_ALL);
		/**
		 * Creates a Communication Profile Response packet according to what
		 * was asked in relevant request packet
		 *
		 * @param request Pointer to the Communication Profile Request packet
		 * @return Pointer to the Communication Profile Response packet
		 */
		GeonetPacket* createCommunicationProfileResponse(GeonetCommunicationProfileRequestPacket* request);

	private:
		/**
		 * ManagementInformationBase reference used to fetch necessary information
		 * to create certain messages/replies
		 */
		ManagementInformationBase& mib;
};

#endif /* MGMT_GN_PACKET_FACTORY_HPP_ */
