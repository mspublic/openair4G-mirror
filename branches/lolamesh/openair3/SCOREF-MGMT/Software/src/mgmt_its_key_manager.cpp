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
 * \file mgmt_its_key_manager.cpp
 * \brief ITS keys and relevant configuration information is maintained in this container
 * \company EURECOM
 * \date 2012
 * \author Baris Demiray
 * \email: baris.demiray@eurecom.fr
 * \note none
 * \bug none
 * \warning none
*/

#include "mgmt_its_key_manager.hpp"

ItsKeyManager::ItsKeyManager() {}

ItsKeyManager::~ItsKeyManager() {
	itsKeyMap.empty();
}

ItsKeyID ItsKeyManager::findKeyId(const string& keyName) const {
	map<ItsKeyID, ItsKey>::const_iterator iterator = itsKeyMap.begin();

	while (iterator != this->itsKeyMap.end()) {
		if (!iterator->second.name.compare(keyName))
			return iterator->first;

		++iterator;
	}

	return static_cast<ItsKeyID>(0xeeee);
}

map<ItsKeyID, ItsKeyValue> ItsKeyManager::getSubset(ItsKeyType keyType) const {
	map<ItsKeyID, ItsKey>::const_iterator iterator = itsKeyMap.begin();
	map<ItsKeyID, ItsKeyValue> subset;

	while (iterator != this->itsKeyMap.end()) {
		// Add every ITS key which is common and is of requested type into the map
		if (iterator->second.type == keyType || iterator->second.type == ITS_KEY_TYPE_COMMON || keyType == ITS_KEY_TYPE_ALL)
			subset.insert(subset.end(), std::make_pair(iterator->first, iterator->second.value));

		++iterator;
	}

	return subset;
}

bool ItsKeyManager::addKey(ItsKeyID id, const string& name, ItsKeyType type, ItsKeyValue value) {
	if (name.empty())
		return false;

	ItsKey itsKey = {name, type, value};
	itsKeyMap.insert(itsKeyMap.end(), std::make_pair(id, itsKey));

	return true;
}

ItsKeyValue ItsKeyManager::getKey(ItsKeyID id) {
	return itsKeyMap[id].value;
}

bool ItsKeyManager::setKey(const string& name, ItsKeyValue value) {
	map<ItsKeyID, ItsKey>::iterator iterator = itsKeyMap.begin();

	while (iterator != this->itsKeyMap.end()) {
		if (!iterator->second.name.compare(name)) {
			iterator->second.value = value;
			return true;
		}

		++iterator;
	}

	return false;
}

bool ItsKeyManager::setKey(ItsKeyID id, ItsKeyValue value) {
	itsKeyMap[id].value = value;

	return true;
}

u_int16_t ItsKeyManager::getNumberOfKeys(ItsKeyType type) const {
	map<ItsKeyID, ItsKey>::const_iterator iterator = itsKeyMap.begin();
	u_int16_t numberOfKeys = 0;

	while (iterator != itsKeyMap.end()) {
		if (type == ITS_KEY_TYPE_COMMON || iterator->second.type == type)
			++numberOfKeys;

		++iterator;
	}

	return numberOfKeys;
}
