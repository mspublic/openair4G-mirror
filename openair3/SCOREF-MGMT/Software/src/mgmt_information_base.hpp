/*
 * mgmt_information_base.hpp
 *
 *  Created on: May 3, 2012
 *      Author: demiray
 */

#ifndef MGMT_INFORMATION_BASE_HPP_
#define MGMT_INFORMATION_BASE_HPP_

#include "mgmt_comm_prof_manager.hpp"
#include "mgmt_its_key_manager.hpp"
#include "mgmt_gn_datatypes.hpp"
#include <sys/types.h>
#include <string>
#include <map>
using namespace std;

/**
 * A container to hold configuration parameters of Management entity
 */
class ManagementInformationBase {
	public:
		/**
		 * Constructor for ManagementInformationBase class
		 */
		ManagementInformationBase();
		/**
		 * Destructor for ManagementInformationBase class
		 */
		~ManagementInformationBase();

	public:
		/**
		 * Initialises ItsKeyManager member by defining configuration items
		 *
		 * @return true on success, false otherwise
		 */
		bool initialize();
		/**
		 * Returns value of relevant ITS key through ItsKeyManager methods
		 *
		 * @param itsKeyId ITS key ID
		 * @return Value of relevant ITS key
		 */
		ItsKeyValue getValue(ItsKeyID itsKeyId);
		/**
		 * Sets value of relevant ITS key through ItsKeyManager methods
		 *
		 * @param itsKeyId ITS key ID
		 * @param value Value to be set
		 * @return true on success, false otherwise
		 */
		bool setValue(ItsKeyID itsKeyId, ItsKeyValue value);
		/**
		 * Sets value of relevant ITS key through ItsKeyManager methods
		 *
		 * @param configurationItemName std::string name of configuration item
		 * @param value Value to be set
		 * @return true on success, false otherwise
		 */
		bool setValue(const string& configurationItemName, ItsKeyValue value);
		/**
		 * Returns DWORD length of relevant ITS key
		 *
		 * @param itsKeyId ITS key ID
		 * @return DWORD-length of relevant ITS key
		 */
		u_int8_t getLength(ItsKeyID itsKeyId) const;
		/**
		 * Returns ItsKeyManager container's reference
		 *
		 * @return ItsKeyManager reference
		 */
		ItsKeyManager& getItsKeyManager();
		/**
		 * Returns wireless state of relevant interface
		 *
		 * @param interfaceId Interface ID of the interface
		 * @return A reference to WirelessStateResponseItem of relevant interface
		 */
		WirelessStateResponseItem& getWirelessState(InterfaceID interfaceId);
		/**
		 * Returns the network state of this MIB
		 *
		 * @return A reference to the NetworkStateMessage object of this MIB
		 */
		NetworkStateMessage& getNetworkState();

	public:
		/**
		 * An object of ItsKeyManager class to keep track of ITS keys and their values
		 */
		ItsKeyManager itsKeyManager;
		/**
		 * Wireless and network state messages
		 */
		map<InterfaceID, WirelessStateResponseItem> wirelessStateMap;
		/**
		 * Network state of this MIB entity
		 */
		NetworkStateMessage networkState;
		/**
		 * Location Table map
		 */
		u_int8_t networkFlags;
		/**
		 * Location table that consists of a map of LocationTableItem objects
		 */
		map<GnAddress, LocationTableItem> locationTable;
		/**
		 * Communication profile manager
		 */
		CommunicationProfileManager communicationProfileManager;
};

#endif /* MGMT_INFORMATION_BASE_HPP_ */
