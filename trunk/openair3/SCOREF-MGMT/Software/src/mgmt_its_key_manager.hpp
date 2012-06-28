/*
 * mgmt_its_key_manager.hpp
 *
 *  Created on: 8 Jun 2012
 *      Author: barisd
 */

#ifndef MGMT_ITS_KEY_MANAGER_HPP_
#define MGMT_ITS_KEY_MANAGER_HPP_

#include <map>
#include <string>
using namespace std;

#include <sys/types.h>
#include "mgmt_gn_datatypes.hpp"

typedef u_int32_t ItsKeyValue;
enum ItsKeyType {
	ITS_KEY_TYPE_COMMON = 0,
	ITS_KEY_TYPE_NET = 1,
	ITS_KEY_TYPE_FAC = 2,
	ITS_KEY_TYPE_ALL = 3
};

struct ItsKey {
	string name;
	ItsKeyType type;
	ItsKeyValue value;
};

class ItsKeyManager {
	public:
		ItsKeyManager();
		~ItsKeyManager();

	public:
		bool addKey(ItsKeyID id, const string& name, ItsKeyType type, ItsKeyValue value);
		ItsKeyValue getKey(ItsKeyID id);
		bool setKey(const string& name, ItsKeyValue value);
		bool setKey(ItsKeyID id, ItsKeyValue value);
		ItsKeyID findKeyId(const string& keyName) const;
		u_int16_t getNumberOfKeys(ItsKeyType type = ITS_KEY_TYPE_COMMON) const;

	public:
		map<ItsKeyID, ItsKeyValue> getSubset(ItsKeyType keyType) const;

	private:
		map<ItsKeyID, ItsKey> itsKeyMap;
};

#endif /* MGMT_ITS_KEY_MANAGER_HPP_ */
