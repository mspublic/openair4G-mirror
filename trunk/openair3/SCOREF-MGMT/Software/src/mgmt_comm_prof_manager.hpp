/*
 * mgmt_comm_prof_manager.hpp
 *
 *  Created on: Jun 29, 2012
 *      Author: demiray
 */

#ifndef MGMT_COMM_PROF_MANAGER_HPP_
#define MGMT_COMM_PROF_MANAGER_HPP_

#include <map>
#include <string>
using namespace std;

#include "mgmt_gn_datatypes.hpp"

/**
 * Communication Profile Item
 */
typedef u_int32_t CommunicationProfileID;
struct CommunicationProfileItem {
	CommunicationProfileID id;
	u_int8_t transport;
	u_int8_t network;
	u_int8_t access;
	u_int8_t channel;
} __attribute__((packed));

class CommunicationProfileManager {
	public:
		/**
		 * Constructor for CommunicationProfileManager class
		 */
		CommunicationProfileManager();
		/**
		 * Destructor for CommunicationProfileManager class
		 */
		~CommunicationProfileManager();

	public:
		/**
		 * Inserts given communication profile information into the table
		 *
		 * @param profileIdString Communication profile ID string
		 * @param profileDefinitionString Communication profile details
		 * @return true on success, false otherwise
		 */
		bool insert(const string& profileIdString, const string& profileDefinitionString);
		/**
		 * Returns string representation of Communication Profile Table
		 *
		 * return std::string representation of table
		 */
		string toString() const;
		/**
		 * Returns the number of profiles present
		 *
		 * @return Number of communication profiles, ie. table size
		 */
		u_int8_t getProfileCount() const;
		/**
		 * Returns communication profile map
		 *
		 * @return std::map of Communication Profile Table
		 */
		map<CommunicationProfileID, CommunicationProfileItem> getProfileMap() const;

	private:
		/**
		 * Initialises profile item strings
		 */
		void initialise();

		/**
		 * Parses comma-separated Communication Profile string and returns
		 * a CommunicationProfileItem structure filled accordingly
		 *
		 * @param profileString Comma-separated profile definition
		 * @return CommunicationProfileItem having parsed information
		 */
		CommunicationProfileItem parse(const string& profileString);

	private:
		/**
		 * Communication profile map
		 */
		map<CommunicationProfileID, CommunicationProfileItem> communicationProfileMap;
		/**
		 * Communication profile string and bitmap index map
		 */
		map<string, u_int8_t> communicationProfileStringMap;
};

#endif /* MGMT_COMM_PROF_MANAGER_HPP_ */
