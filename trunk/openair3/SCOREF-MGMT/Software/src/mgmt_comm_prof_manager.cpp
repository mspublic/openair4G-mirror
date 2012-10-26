/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2012 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fr/openairinterface
  Address      : EURECOM, Campus SophiaTech, 450 Route des Chappes, 06410 Biot FRANCE

*******************************************************************************/

/*!
 * \file mgmt_comm_prof_manager.cpp
 * \brief Communication Profiles list is kept and maintained by this container
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "mgmt_comm_prof_manager.hpp"
#include "util/mgmt_exception.hpp"
#include <boost/lexical_cast.hpp>

CommunicationProfileManager::CommunicationProfileManager(Logger& logger)
	: logger(logger) {
	initialise();
}

CommunicationProfileManager::~CommunicationProfileManager() {
	communicationProfileMap.clear();
	communicationProfileStringMap.clear();
}

bool CommunicationProfileManager::insert(const string& profileIdString, const string& profileDefinitionString) {
	if (profileIdString.empty() || profileDefinitionString.empty())
		return false;

	/**
	 * Parse communication profiles defined in the configuration file and
	 * insert them into the communication profile map
	 */
	CommunicationProfileItem communicationProfileItem;
	/**
	 * std::string as a map key should not have a NULL at
	 * the end so here we trim it
	 */
	string trimmedProfileDefinitionString = Util::trim(profileDefinitionString, '\0');

	try {
		communicationProfileItem = parse(profileIdString, trimmedProfileDefinitionString);
	} catch (Exception& e) {
		e.updateStackTrace("Cannot parse Communication Profile definitions");
		throw;
	}

	communicationProfileMap.insert(communicationProfileMap.end(), std::make_pair(communicationProfileItem.id, communicationProfileItem));
	logger.debug("Communication profile: " + profileIdString + ":" + profileDefinitionString);
	logger.info("Communication profile: " + communicationProfileItem.toString());

	return true;
}

string CommunicationProfileManager::toString() const {
	stringstream ss;

	ss << "Communication profile count: " << communicationProfileMap.size() << endl;

	map<CommunicationProfileID, CommunicationProfileItem>::const_iterator it;
	while (it != communicationProfileMap.end()) {
		ss << "Communication Profile [ID:" << it->second.id
			<< ", transport:" << it->second.transport
			<< ", network:" << it->second.network
			<< ", access: " << it->second.access
			<< ", channel: " << it->second.channel << "]" << endl;

		++it;
	}

	return ss.str();
}

u_int8_t CommunicationProfileManager::getProfileCount() const {
	return communicationProfileMap.size();
}

map<CommunicationProfileID, CommunicationProfileItem> CommunicationProfileManager::getProfileMap() const {
	return communicationProfileMap;
}

map<CommunicationProfileID, CommunicationProfileItem> CommunicationProfileManager::getProfileMapSubset(u_int32_t filter) const {
	/**
	 * If we're asked everything, return everything
	 */
	if (filter == 0xFFFFFFFF)
		return getProfileMap();

	map<CommunicationProfileID, CommunicationProfileItem> filteredProfileMap;
	map<CommunicationProfileID, CommunicationProfileItem>::const_iterator it = communicationProfileMap.begin();

	while (it != communicationProfileMap.end()) {
		// todo Intelligent code goes here
		++it;
	}

	return filteredProfileMap;
}

void CommunicationProfileManager::initialise() {
	/*
	 * Transport string & index map
	 */
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("BTP_A", 1));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("BTP_B", 2));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("TCP", 3));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("UDP", 4));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("RTP", 5));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("STCP", 6));
	/*
	 * Network string & index map
	 */
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("GN", 1));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("IPv6_GN", 2));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("IPv6", 3));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("IPv4", 4));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("IPv4/v6", 5));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("DSMIPv4/v6", 6));
	/*
	 * Access string & index map
	 */
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("ITSG5", 1));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("3G", 2));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("11n", 3));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("Ethernet", 4));
	/*
	 * Channel string & index map
	 */
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("CCH", 1));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("SCH1", 2));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("SCH2", 3));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("SCH3", 4));
	communicationProfileStringMap.insert(communicationProfileStringMap.end(), std::make_pair("SCH4", 5));
}

CommunicationProfileItem CommunicationProfileManager::parse(const string& profileIdString, const string& profileDefinitionString) {
	CommunicationProfileItem communicationProfileItem;
	u_int8_t profileID = 0x00;

	try {
		profileID = (u_int8_t)boost::lexical_cast<unsigned short>(profileIdString.substr(profileIdString.find("CP") + 2, profileIdString.length() - 2).c_str());
		communicationProfileItem.id = profileID;
	} catch (...) {
		throw Exception("Cannot parse Communication Profile ID string '" + profileIdString + "' in configuration file", logger);
	}

	/*
	 * Parse communication profile string and get tokens for each layer
	 */
	vector<string> profileItemVector = Util::split(profileDefinitionString, ',');
	const string access = profileItemVector[2];
	string channel;
	/*
	 * For access methods `3G' and `Ethernet' this information is not relevant; for `11n'
	 * the choice is made by the Access Point, here parse accordingly
	 */
	if (!access.compare(0, 2, "3G") || !access.compare(0, 8, "Ethernet") || !access.compare(0, 3, "11n")) {
		channel = "";
	} else {
		channel = profileItemVector[3];
	}

	/*
	 * Fill transport, network, access, and channel fields respectively
	 */
	setFlags(profileItemVector[0], communicationProfileItem.transport);
	setFlags(profileItemVector[1], communicationProfileItem.network);
	setFlags(profileItemVector[2], communicationProfileItem.access);

	if (channel.empty()) {
		logger.info("Access type is either 3G, or Ethernet, or 11n. Skipping channel information");
	} else {
		setFlags(profileItemVector[3], communicationProfileItem.channel);
	}

	return communicationProfileItem;
}

bool CommunicationProfileManager::setFlags(const string& configuration, u_int8_t& octet) {
	if (configuration.empty())
		return false;

	vector<string> profileStrings = Util::split(configuration, ':');
	vector<string>::iterator iterator = profileStrings.begin();

	while (iterator != profileStrings.end()) {
		/**
		 * Verify incoming communication profile definition string item and handle "BTP" exception
		 */
		if (iterator->compare("BTP") == 0) {
			logger.debug("Communication profile string 'BTP' has found, both BTP_A and BTP_B flags will be set");

			/**
			 * If communication profile includes BTP then we should set both BTP_A and BTP_B
			 * bits since they'll both be defined and available
			 *
			 * Bit indexes start from 1 in 'MNGT to CM-GN Interface' paper but it
			 * corresponds to bit 0 for Util::setBit() so here we subtract 1 to
			 * find index against 0 as the first
			 */
			Util::setBit(octet, static_cast<u_int8_t>(communicationProfileStringMap["BTP_A"] - 1));
			Util::setBit(octet, static_cast<u_int8_t>(communicationProfileStringMap["BTP_B"] - 1));
		} else if (communicationProfileStringMap[*iterator] != 0) {
			/*
			 * If index is valid than set the bit at that index
			 */
			Util::setBit(octet, static_cast<u_int8_t>(communicationProfileStringMap[*iterator] - 1));
		} else if (communicationProfileStringMap[*iterator] == 0) {
			/*
			 * Invalid strings are ignored
			 */
			logger.warning("Communication profile definition string '" + *iterator + "' is not valid!");
			logger.info("Check SCOREF-MGMT_Configuration.pdf file for valid configuration settings");
		}

		++iterator;
	}

	return true;
}
