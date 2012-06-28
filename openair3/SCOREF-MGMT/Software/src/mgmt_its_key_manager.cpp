/*
 * mgmt_its_key_manager.cpp
 *
 *  Created on: 8 Jun 2012
 *      Author: barisd
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
