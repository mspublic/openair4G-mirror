/*
 * mgmt_comm_prof_manager.cpp
 *
 *  Created on: Jun 29, 2012
 *      Author: demiray
 */

#include "mgmt_comm_prof_manager.hpp"
#include "mgmt_util.hpp"

#include <cstdlib>

CommunicationProfileManager::CommunicationProfileManager() {
	initialise();
}

CommunicationProfileManager::~CommunicationProfileManager() {
	communicationProfileMap.empty();
	communicationProfileStringMap.empty();
}

bool CommunicationProfileManager::insert(const string& profileIdString, const string& profileDefinitionString) {
	if (profileIdString.empty() || profileDefinitionString.empty())
		return false;

	u_int8_t profileID = atoi(profileIdString.substr(profileIdString.find("CP") + 2, profileIdString.length() - 2).c_str());
	cout << "Comm. Profile ID = " << (int)profileID << endl;
	communicationProfileMap.insert(communicationProfileMap.end(), std::make_pair(profileID, parse(profileDefinitionString)));

	return true;
}

string CommunicationProfileManager::toString() const {
	stringstream ss;

	ss << "Communication profile count: " << communicationProfileMap.size() << endl;

	map<CommunicationProfileID, CommunicationProfileItem>::iterator iterator;
	while (iterator != communicationProfileMap.end()) {
		ss << "Communication Profile [ID:" << iterator->second.id
			<< ", transport:" << iterator->second.transport
			<< ", network:" << iterator->second.network
			<< ", access: " << iterator->second.access
			<< ", channel: " << iterator->second.channel << "]" << endl;
	}

	return ss.str();
}

u_int8_t CommunicationProfileManager::getProfileCount() const {
	return communicationProfileMap.size();
}

map<CommunicationProfileID, CommunicationProfileItem> CommunicationProfileManager::getProfileMap() const {
	return communicationProfileMap;
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

CommunicationProfileItem CommunicationProfileManager::parse(const string& profileString) {
	CommunicationProfileItem communicationProfileItem;

	vector<string> profileItemVector = Util::split(profileString, ',');
	cout << "There are " << profileItemVector.size() << " item(s)" << endl;

	/*
	 * Fill transport, network, access, and channel fields respectively
	 */
	Util::setBit(communicationProfileItem.transport, static_cast<u_int8_t>(communicationProfileStringMap[profileItemVector[0]]));
	Util::setBit(communicationProfileItem.network, static_cast<u_int8_t>(communicationProfileStringMap[profileItemVector[1]]));
	Util::setBit(communicationProfileItem.access, static_cast<u_int8_t>(communicationProfileStringMap[profileItemVector[2]]));
	/*
	 * For access methods `3G' and `Ethernet' this information is not relevant; for `11n'
	 * the choise is made by the Access Point, here parse accordingly
	 */
	if (!profileItemVector.data()->compare("3G")
			|| !profileItemVector.data()->compare("Ethernet")
			|| !profileItemVector.data()->compare("11n")) {
		cout << "Access type is either 3G, or Ethernet, or 11n. Skipping channel information" << endl;
	} else {
		Util::setBit(communicationProfileItem.channel, static_cast<u_int8_t>(communicationProfileStringMap[profileItemVector[3]]));
	}

	return communicationProfileItem;
}
