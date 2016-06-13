/*
 * Copyright (c) 2015 2016, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Intel Corporation nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This file contains the provider for the PoolView instances. PoolView
 * is an internal view class for the Show -pool CLI command.
 */

#include <LogEnterExit.h>
#include <nvm_management.h>
#include <libinvm-cim/Attribute.h>
#include <server/BaseServerFactory.h>
#include <server/SystemCapabilitiesFactory.h>
#include <uid/uid.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <sstream>
#include "PoolViewFactory.h"

#include <exception/NvmExceptionLibError.h>
#include <lib_interface/NvmApi.h>
#include <NvmStrings.h>
#include <core/Helper.h>
#include "InterleaveSet.h"

wbem::mem_config::PoolViewFactory::PoolViewFactory()
throw(wbem::framework::Exception)
{
}

wbem::mem_config::PoolViewFactory::~PoolViewFactory()
{
}

void wbem::mem_config::PoolViewFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw(wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(POOLID_KEY);

	// add non-key attributes
	attributes.push_back(POOLTYPE_KEY);
	attributes.push_back(CAPACITY_KEY);
	attributes.push_back(FREECAPACITY_KEY);
	attributes.push_back(ENCRYPTIONCAPABLE_KEY);
	attributes.push_back(ENCRYPTIONENABLED_KEY);
	attributes.push_back(ERASECAPABLE_KEY);
	attributes.push_back(SOCKETID_KEY);
	attributes.push_back(APPDIRECTNAMESPACE_MAX_SIZE_KEY);
	attributes.push_back(APPDIRECTNAMESPACE_MIN_SIZE_KEY);
	attributes.push_back(APPDIRECTNAMESPACE_COUNT_KEY);
	attributes.push_back(STORAGENAMESPACE_MAX_SIZE_KEY);
	attributes.push_back(STORAGENAMESPACE_MIN_SIZE_KEY);
	attributes.push_back(STORAGENAMESPACE_COUNT_KEY);
	attributes.push_back(HEALTHSTATE_KEY);
	attributes.push_back(APP_DIRECT_SETTINGS_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance *wbem::mem_config::PoolViewFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	struct pool *pPool = NULL;
	try
	{
		checkAttributes(attributes);

		std::string poolUidStr = path.getKeyValue(POOLID_KEY).stringValue();
		if (!core::Helper::isValidPoolUid(poolUidStr))
		{
			throw framework::ExceptionBadParameter(POOLID_KEY.c_str());
		}

		pPool = getPool(poolUidStr);
		bool isVolatilePool = pPool->type == POOL_TYPE_VOLATILE;

		if (!isVolatilePool)
		{
			possible_namespace_ranges ranges;
			int rc;
			if ((rc = nvm_get_available_persistent_size_range(pPool->pool_uid, &ranges)) != NVM_SUCCESS)
			{
				throw exception::NvmExceptionLibError(rc);
			}

			// PoolType - The type of pool. One of:  Volatile, Persistent, Mirrored
			if (containsAttribute(POOLTYPE_KEY, attributes))
			{
				framework::Attribute a(getPoolType(pPool), false);
				pInstance->setAttribute(POOLTYPE_KEY, a, attributes);
			}

			// Capacity - Total usable capacity, both allocated and unallocated in bytes.
			if (containsAttribute(CAPACITY_KEY, attributes))
			{
				framework::Attribute a(pPool->capacity, false);
				pInstance->setAttribute(CAPACITY_KEY, a, attributes);
			}

			// FreeCapacity - Remaining usable capacity in bytes.
			if (containsAttribute(FREECAPACITY_KEY, attributes))
			{
				framework::Attribute a(pPool->free_capacity, false);
				pInstance->setAttribute(FREECAPACITY_KEY, a, attributes);
			}

			if (containsAttribute(ENCRYPTIONCAPABLE_KEY, attributes))
			{
				std::string encryption = getEncryptionCapable(pPool);
				framework::Attribute a(encryption, false);
				pInstance->setAttribute(ENCRYPTIONCAPABLE_KEY, a, attributes);
			}

			if (containsAttribute(ENCRYPTIONENABLED_KEY, attributes))
			{
				std::string encryption = getEncryptionEnabled(pPool);
				framework::Attribute a(encryption, false);
				pInstance->setAttribute(ENCRYPTIONENABLED_KEY, a, attributes);
			}

			if (containsAttribute(ERASECAPABLE_KEY, attributes))
			{
				std::string erase = getEraseCapable(pPool);
				framework::Attribute a(erase, false);
				pInstance->setAttribute(ERASECAPABLE_KEY, a, attributes);
			}

			if (containsAttribute(SOCKETID_KEY, attributes))
			{
				framework::Attribute a(getString(pPool->socket_id), false);
				pInstance->setAttribute(SOCKETID_KEY, a, attributes);
			}

			// AppDirectNamespaceMaxSize - Largest AD namespace that can be created
			if (containsAttribute(APPDIRECTNAMESPACE_MAX_SIZE_KEY, attributes))
			{
				framework::Attribute a(ranges.largest_possible_app_direct_ns, false);
				pInstance->setAttribute(APPDIRECTNAMESPACE_MAX_SIZE_KEY, a, attributes);
			}

			// AppDirectNamespaceMinSize - Smallest AD namespace that can be created (smallest alignment size)
			if (containsAttribute(APPDIRECTNAMESPACE_MIN_SIZE_KEY, attributes))
			{
				framework::Attribute a(ranges.smallest_possible_app_direct_ns, false);
				pInstance->setAttribute(APPDIRECTNAMESPACE_MIN_SIZE_KEY, a, attributes);
			}

			// AppDirectNamespaceCount - Current number of AD namespaces
			if (containsAttribute(APPDIRECTNAMESPACE_COUNT_KEY, attributes))
			{
				framework::Attribute a(getString(countNamespaces(pPool, NAMESPACE_TYPE_APP_DIRECT)), false);
				pInstance->setAttribute(APPDIRECTNAMESPACE_COUNT_KEY, a, attributes);
			}

			// StorageNamespaceMaxSize - Largest Storage namespace that can be created
			if (containsAttribute(STORAGENAMESPACE_MAX_SIZE_KEY, attributes))
			{
				framework::Attribute a(ranges.largest_possible_storage_ns, false);
				pInstance->setAttribute(STORAGENAMESPACE_MAX_SIZE_KEY, a, attributes);
			}

			// StorageNamespaceMinSize - Smallest Storage namespace that can be created (smallest block size)
			if (containsAttribute(STORAGENAMESPACE_MIN_SIZE_KEY, attributes))
			{
				framework::Attribute a(ranges.smallest_possible_storage_ns, false);
				pInstance->setAttribute(STORAGENAMESPACE_MIN_SIZE_KEY, a, attributes);
			}

			// StorageNamespacesCount - Current number of Storage namespaces
			if (containsAttribute(STORAGENAMESPACE_COUNT_KEY, attributes))
			{
				framework::Attribute a(getString(countNamespaces(pPool, NAMESPACE_TYPE_STORAGE)), false);
				pInstance->setAttribute(STORAGENAMESPACE_COUNT_KEY, a, attributes);
			}

			// Health State = enum + string
			if (containsAttribute(HEALTHSTATE_KEY, attributes))
			{
				framework::Attribute a((NVM_UINT16)pPool->health,
						poolHealthToStr(pPool->health), false);
				pInstance->setAttribute(HEALTHSTATE_KEY, a, attributes);
			}

			if (containsAttribute(APP_DIRECT_SETTINGS_KEY, attributes))
			{
				framework::Attribute attr(getAppDirectSettings(pPool), false);
				pInstance->setAttribute(APP_DIRECT_SETTINGS_KEY, attr, attributes);
			}
		}
		delete pPool;
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		if (pInstance)
		{
			delete pInstance;
		}
		if (pPool)
		{
			delete pPool;
		}
		throw;
	}

	return pInstance;
}

/*
 * Return the object paths for the Intel_PoolView class.
 */
wbem::framework::instance_names_t *wbem::mem_config::PoolViewFactory::getInstanceNames()
	throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		std::vector<struct pool> pools = getPoolList(true);
		for (std::vector<struct pool>::const_iterator iter = pools.begin();
				iter != pools.end(); iter++)
		{
			framework::attributes_t keys;

			NVM_UID poolUid;
			uid_copy((*iter).pool_uid, poolUid);
			keys[POOLID_KEY] = framework::Attribute(std::string(poolUid), true);

			framework::ObjectPath path(server::getHostName(), NVM_NAMESPACE,
					INTEL_POOLVIEW_CREATIONCLASSNAME, keys);
			pNames->push_back(path);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}
	return pNames;
}

/*
 * Helper function to retrieve a list of pools
 */
std::vector<struct pool> wbem::mem_config::PoolViewFactory::getPoolList(bool pmOnly)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::vector<struct pool> poolList;

	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();
	pApi->getPools(poolList);

	// remove non-persistent pools if necessary
	if (pmOnly)
	{
		for (std::vector<struct pool>::iterator iter = poolList.begin();
				iter != poolList.end(); )
		{
			if (iter->type != POOL_TYPE_PERSISTENT && iter->type != POOL_TYPE_PERSISTENT_MIRROR)
			{
				// returns iterator to the next item - no need to increment
				iter = poolList.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}

	// return the vector
	return poolList;
}

/*
 * Helper function to retrieve a specific pool.
 */
struct pool *wbem::mem_config::PoolViewFactory::getPool(const std::string &poolUidStr)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UID poolUid;
	uid_copy(poolUidStr.c_str(), poolUid);

	struct pool *pPool = new struct pool;
	int rc = nvm_get_pool(poolUid, pPool);
	if (rc != NVM_SUCCESS)
	{
		if (pPool)
		{
			delete pPool;
		}
		throw exception::NvmExceptionLibError(rc);
	}

	return pPool;
}

std::string wbem::mem_config::PoolViewFactory::getEraseCapable(pool *pPool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string result = NO;
	if (pPool->erase_capable)
	{
		result = YES;
	}

	return result;
}

std::string wbem::mem_config::PoolViewFactory::getEncryptionCapable(pool *pPool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string result = NO;
	if (pPool->encryption_capable)
	{
		result = YES;
	}

	return result;
}

std::string wbem::mem_config::PoolViewFactory::getEncryptionEnabled(const struct pool *pPool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string result = NO;
	for (NVM_UINT16 i = 0; i < pPool->dimm_count && result == NO; i++)
	{
		struct device_discovery device;
		int rc = nvm_get_device_discovery(pPool->dimms[i], &device);
		if (rc != NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(rc);
		}

		switch (device.lock_state)
		{
			case LOCK_STATE_UNLOCKED:
			case LOCK_STATE_LOCKED:
			case LOCK_STATE_FROZEN:
			case LOCK_STATE_PASSPHRASE_LIMIT:
				result = YES;
				break;
			case LOCK_STATE_DISABLED:
			case LOCK_STATE_UNKNOWN:
			default:
				break;
		}
	}

	return result;
}

std::string wbem::mem_config::PoolViewFactory::getPoolType(struct pool *pPool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string poolType;
	switch (pPool->type)
	{
		case POOL_TYPE_PERSISTENT:
			poolType = wbem::mem_config::POOLTYPE_APPDIRECT;
			break;
		case POOL_TYPE_VOLATILE:
			poolType = wbem::mem_config::POOLTYPE_VOLATILE;
			break;
		case POOL_TYPE_PERSISTENT_MIRROR:
			poolType = wbem::mem_config::POOLTYPE_MIRRORED;
			break;
		default:
			poolType = wbem::mem_config::POOLTYPE_UNKNOWN;
			break;
	}
	return poolType;
}

/*
 * Get namespaces for pool.
 * For each namespace increment count if it matches pool_type.
 * If can't get namespaces for pool, throw an exception
 */
NVM_UINT32 wbem::mem_config::PoolViewFactory::countNamespaces(const struct pool *pPool,
		namespace_type const type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	NVM_UINT32 result = 0;

	lazyInitNs();

	for (size_t n = 0; n < m_nsCache.size(); n++)
	{
		if (uid_cmp(pPool->pool_uid, m_nsCache[n].pool_uid) &&
				m_nsCache[n].type == type)
		{
			result++;
		}
	}
	return result;
}

/*
 * Create a cache of namespaces to be used in populating the attributes.
 */
void wbem::mem_config::PoolViewFactory::lazyInitNs()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (m_nsCache.size() == 0)
	{
		int rc = nvm_get_namespace_count();
		if (rc < NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(rc);
		}
		int nsCount = rc;
		if (nsCount > 0)
		{
			struct namespace_discovery namespaces[nsCount];
			rc = nvm_get_namespaces(namespaces, nsCount);
			if (rc < NVM_SUCCESS)
			{
				throw exception::NvmExceptionLibError(rc);
			}
			nsCount = rc;
			for (int n = 0; n < nsCount; n++)
			{
				struct namespace_details nsDetails;
				rc = nvm_get_namespace_details(namespaces[n].namespace_uid, &nsDetails);
				if (rc != NVM_SUCCESS)
				{
					throw exception::NvmExceptionLibError(rc);
				}
				m_nsCache.push_back(nsDetails);
			}
		}
	}
}

/*
 * convert an integer to string
 */
std::string wbem::mem_config::PoolViewFactory::getString(const NVM_UINT64 value)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::stringstream ss;
	ss << value;
	return ss.str();
}

/*
 * Helper function to convert pool health to a string.
 */
std::string wbem::mem_config::PoolViewFactory::poolHealthToStr(
		const enum pool_health &health)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string healthStr;
	switch (health)
	{
		case POOL_HEALTH_NORMAL:
			healthStr = POOL_HEALTH_STR_NORMAL;
			break;
		case POOL_HEALTH_WARNING:
		case POOL_HEALTH_DEGRADED:
			healthStr = POOL_HEALTH_STR_DEGRADED;
			break;
		case POOL_HEALTH_FAILED:
			healthStr = POOL_HEALTH_STR_FAILED;
			break;
		default:
			healthStr = POOL_HEALTH_STR_UNKNOWN;
			break;
	}

	return healthStr;
}

/*
 * convert bytes to MB and append MB suffix string
 */
std::string wbem::mem_config::PoolViewFactory::getMbString(const NVM_UINT64 bytes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getString(bytes / BYTES_PER_MB) + wbem::MB_SUFFIX;
}

std::string wbem::mem_config::PoolViewFactory::getInterleaveSetFormatStr(
	const struct interleave_format &format)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::stringstream formatStr;
	// output format
	formatStr << wbem::mem_config::InterleaveSet::getInterleaveFormatString(&format);
	// input format
	formatStr << " (";
	formatStr << wbem::mem_config::InterleaveSet::getInterleaveFormatInputString(&format, false);
	formatStr << ")";

	return formatStr.str();
}

wbem::framework::STR_LIST wbem::mem_config::PoolViewFactory::getAppDirectSettings(
		const struct pool *pPool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::STR_LIST appDirectSettings;

	if (pPool->ilset_count > 0)
	{
		for (NVM_UINT16 i = 0; i < pPool->ilset_count; i++)
		{
			server::SystemCapabilitiesFactory::addFormatStringIfNotInList(appDirectSettings,
					pPool->ilsets[i].settings, false);
		}
	}
	else
	{ // storage-only capacity
		appDirectSettings.push_back(wbem::NA);
	}

	return appDirectSettings;
}
