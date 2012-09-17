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
 * \file mgmt_its_key_manager.hpp
 * \brief ITS keys and relevant configuration information is maintained in this container
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#ifndef MGMT_ITS_KEY_MANAGER_HPP_
#define MGMT_ITS_KEY_MANAGER_HPP_

#include <map>
#include <string>
#include <climits>
using namespace std;

#include "mgmt_gn_datatypes.hpp"
#include "util/mgmt_log.hpp"
#include <sys/types.h>

/**
 * ITS key types according to module they belong to
 *
 * ITS_KEY_TYPE_COMMON: ITS keys common for NETwork and FACilities,
 * ITS_KEY_TYPE_NET: NETwork ITS keys,
 * ITS_KEY_TYPE_FAC: FACilities ITS keys,
 * ITS_KEY_TYPE_ALL: All ITS keys defined
 */
enum ItsKeyType {
	ITS_KEY_TYPE_COMMON = 0,
	ITS_KEY_TYPE_NET = 1,
	ITS_KEY_TYPE_FAC = 2,
	ITS_KEY_TYPE_ALL = 3
};
/**
 * ITS key values are limited to 32-bit values so
 * we define this name definition for readability
 */
typedef u_int32_t ItsKeyValue;
/**
 * ITS key structure holding every property of an ITS key
 */
struct ItsKey {
	string name;
	ItsKeyType type;
	ItsKeyValue value;
	ItsKeyValue minValue;
	ItsKeyValue maxValue;
};

/**
 * ITS keys and relevant configuration information is maintained in this container
 */
class ItsKeyManager {
	public:
		/**
		 * Constructor for ItsKeyManager class
		 *
		 * @param logger Logger object reference
		 */
		ItsKeyManager(Logger& logger);
		/**
		 * Destructor for ItsKeyManager class
		 */
		~ItsKeyManager();

	public:
		/**
		 * Enqueues given ITS key, name and value to ITS keys map
		 *
		 * @param id ITS key ID of new key to be added
		 * @param name Name of new key to be added
		 * @param value Value of new key to be added
		 * @param minValue Minimum value of new key
		 * @param maxValue Maximum value of new key
		 * @return true on success, false otherwise
		 */
		bool addKey(ItsKeyID id, const string& name, ItsKeyType type, ItsKeyValue value, ItsKeyValue minValue = 0, ItsKeyValue maxValue = INT_MAX);
		/**
		 * Returns value of the key with given ITS key ID
		 *
		 * @param id ITS key ID of the key
		 * @return Value of ITS key
		 */
		ItsKeyValue getKey(ItsKeyID id);
		/**
		 * Sets the value of ITS key given its name
		 *
		 * @param name Name of ITS key to be reset
		 * @param value Value to be set as the new value of relevant ITS key
		 * @return true on success, false otherwise
		 */
		bool setKey(const string& name, ItsKeyValue value);
		/**
		 * Sets the value of ITS key given its ITS key ID
		 *
		 * @param id ITS key ID of ITS key to be reset
		 * @param value Value to be set as the new value of relevant ITS key
		 * @return true on success, false otherwise
		 */
		bool setKey(ItsKeyID id, ItsKeyValue value);
		/**
		 * Returns ITS key ID of ITS key given its name
		 *
		 * @param keyName Name of the ITS key being searched
		 * @return ITS key ID of the ITS key if found, 0xeeee otherwise
		 */
		ItsKeyID findKeyId(const string& keyName) const;
		/**
		 * Returns the number of ITS keys defined in ITS key map
		 *
		 * @param type ITS Key type, may take following values,
		 * ITS_KEY_TYPE_COMMON = Returns ITS keys common for NETwork and FACilities,
		 * ITS_KEY_TYPE_NET = Returns NETwork ITS keys,
		 * ITS_KEY_TYPE_FAC = Returns FACilities ITS keys,
		 * ITS_KEY_TYPE_ALL = Returns all ITS keys defined
		 */
		u_int16_t getNumberOfKeys(ItsKeyType type = ITS_KEY_TYPE_COMMON) const;

	public:
		/**
		 * Returns the subset of ITS key map having ITS keys of given type
		 *
		 * @param keyType ITS key type to filter ITS keys
		 * @return List of type std::map containing ITS keys of asked type
		 */
		map<ItsKeyID, ItsKeyValue> getSubset(ItsKeyType keyType) const;

	private:
		/**
		 * List of type std::map for 'ITS key ID to ITS key value' mapping
		 */
		map<ItsKeyID, ItsKey> itsKeyMap;
		/**
		 * Logger object reference
		 */
		Logger& logger;
};

#endif /* MGMT_ITS_KEY_MANAGER_HPP_ */
