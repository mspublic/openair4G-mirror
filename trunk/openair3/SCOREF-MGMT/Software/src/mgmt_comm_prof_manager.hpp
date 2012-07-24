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
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*!
 * \file mgmt_comm_prof_manager.hpp
 * \brief Communication Profiles list is kept and maintained by this container
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#ifndef MGMT_COMM_PROF_MANAGER_HPP_
#define MGMT_COMM_PROF_MANAGER_HPP_

#include "mgmt_gn_datatypes.hpp"
#include "util/mgmt_util.hpp"
#include "util/mgmt_log.hpp"
#include <string>
#include <map>
using namespace std;

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

	/**
	 * Constructor for CommunicationProfileItem to
	 * initialise variables
	 */
	CommunicationProfileItem() {
		id = 0;
		transport = 0;
		network = 0;
		access = 0;
		channel = 0;
	}

	string toString() {
		stringstream ss;

		ss << "[id:" << id
			<< " transport:" << Util::getBinaryRepresentation(transport)
			<< " network:" << Util::getBinaryRepresentation(network)
			<< " access:" << Util::getBinaryRepresentation(access)
			<< " channel:" << Util::getBinaryRepresentation(channel) << "]";

		return ss.str();
	}
} __attribute__((packed));

/**
 * Communication Profiles list is kept and maintained by this container
 */
class CommunicationProfileManager {
	public:
		/**
		 * Constructor for CommunicationProfileManager class
		 *
		 * @param logger Logger object reference
		 */
		CommunicationProfileManager(Logger& logger);
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
		/**
		 * Returns communication profile map subset filtered by incoming request map
		 *
		 * @param filter 32-bit filter part of a Communication Profile Request packet
		 * @return Filtered subset of communication profile
		 */
		map<CommunicationProfileID, CommunicationProfileItem> getProfileMapSubset(u_int32_t filter) const;

	private:
		/**
		 * Initialises profile item strings
		 */
		void initialise();

		/**
		 * Parses comma-separated Communication Profile string and returns
		 * a CommunicationProfileItem structure filled accordingly
		 *
		 * @param profileIdString Profile ID string (CP1, CP2, etc.)
		 * @param profileDefinitionString Comma-separated profile definition
		 * @return CommunicationProfileItem having parsed information
		 */
		CommunicationProfileItem parse(const string& profileIdString, const string& profileDefinitionString);
		/**
		 * A helper method to set any bits given in particular communication
		 * profile string, e.g. if a string "IPv4/v6:DSMIPv4/v6" is given then
		 * this method will set both 'IPv4/v6' and 'DSMIPv4/v6' flags in given
		 * octet
		 *
		 * @param octet Octet that the found out flags will be set
		 * @return true on success, false otherwise
		 */
		bool setFlags(const string& configuration, u_int8_t& octet);

	private:
		/**
		 * Communication profile map
		 */
		map<CommunicationProfileID, CommunicationProfileItem> communicationProfileMap;
		/**
		 * Communication profile string and bitmap index map
		 */
		map<string, u_int8_t> communicationProfileStringMap;
		/**
		 * Logger object reference
		 */
		Logger& logger;

};

#endif /* MGMT_COMM_PROF_MANAGER_HPP_ */
