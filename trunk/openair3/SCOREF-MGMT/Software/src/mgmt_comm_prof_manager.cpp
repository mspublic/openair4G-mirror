/*
 * mgmt_comm_prof_manager.cpp
 *
 *  Created on: Jun 29, 2012
 *      Author: demiray
 */

#include "mgmt_comm_prof_manager.hpp"
#include "util/mgmt_util.hpp"

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
	cout << "Comm. Profile ID = " << (int)profileID << endl << endl;
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

	/*
	 * Parse communication profile string and get tokens for each layer
	 */
	vector<string> profileItemVector = Util::split(profileString, ',');
	const string transport = profileItemVector[0];
	const string network = profileItemVector[1];
	const string access = profileItemVector[2];
	string channel;
	/*
	 * For access methods `3G' and `Ethernet' this information is not relevant; for `11n'
	 * the choice is made by the Access Point, here parse accordingly
	 */
	cout << "access = '" << access << "'" << endl;
	if (!access.compare(0, 2, "3G") || !access.compare(0, 8, "Ethernet") || !access.compare(0, 3, "11n")) {
		channel = "";
	} else {
		channel = profileItemVector[3];
	}

	cout << "There are " << profileItemVector.size() << " item(s)" << endl;

	/*
	 * Fill transport, network, access, and channel fields respectively
	 */
	setFlags(profileItemVector[0], communicationProfileItem.transport);
	cout << "transport = " << profileItemVector[0] << endl;
	setFlags(profileItemVector[1], communicationProfileItem.network);
	cout << "network = " << profileItemVector[1] << endl;
	setFlags(profileItemVector[2], communicationProfileItem.access);
	cout << "access = " << profileItemVector[2] << endl;

	if (channel.empty()) {
		cout << "Access type is either 3G, or Ethernet, or 11n. Skipping channel information" << endl;
	} else {
		cout << "lenght = " << access.length() << endl;
		cout << "Encoding channel information" << endl;
		setFlags(profileItemVector[3], communicationProfileItem.channel);
		cout << "channel = " << profileItemVector[3] << endl;
	}

	return communicationProfileItem;
}

bool CommunicationProfileManager::setFlags(const string& configuration, u_int8_t& octet) {
	if (configuration.empty())
		return false;

	vector<string> profileStrings = Util::split(configuration, ':');
	vector<string>::iterator iterator = profileStrings.begin();

	while (iterator != profileStrings.end()) {
		cout << "Setting '" << *iterator << "' flag..." << endl;
		Util::setBit(octet, static_cast<u_int8_t>(communicationProfileStringMap[*iterator]));

		++iterator;
	}

	return true;
}
