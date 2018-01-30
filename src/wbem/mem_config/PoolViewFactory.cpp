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

wbem::mem_config::PoolViewFactory::PoolViewFactory(core::NvmLibrary &lib)
throw(wbem::framework::Exception) : m_nvmLib(lib)
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
	attributes.push_back(PERSISTENTMEMORYTYPE_KEY);
	attributes.push_back(CAPACITY_KEY);
	attributes.push_back(FREECAPACITY_KEY);
	attributes.push_back(ENCRYPTIONCAPABLE_KEY);
	attributes.push_back(ENCRYPTIONENABLED_KEY);
	attributes.push_back(ERASECAPABLE_KEY);
	attributes.push_back(SOCKETID_KEY);
	attributes.push_back(APPDIRECTNAMESPACE_MAX_SIZE_KEY);
	attributes.push_back(APPDIRECTNAMESPACE_MIN_SIZE_KEY);
	attributes.push_back(APPDIRECTNAMESPACE_COUNT_KEY);
	attributes.push_back(HEALTHSTATE_KEY);
	attributes.push_back(ACTIONREQUIRED_KEY);
	attributes.push_back(ACTIONREQUIREDEVENTS_KEY);
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
			// List of underlying types of PM- AppDirect,AppDirectNotInterleaved
			if (containsAttribute(PERSISTENTMEMORYTYPE_KEY, attributes))
			{
				framework::Attribute a(getPersistentMemoryType(pPool), false);
				pInstance->setAttribute(PERSISTENTMEMORYTYPE_KEY, a, attributes);
			}
			possible_namespace_ranges ranges = getAvailablePersistentSizeRange(pPool->pool_uid, INTERLEAVE_WAYS_0);

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
				bool encryptionCapable = getEncryptionCapable(pPool);
				framework::Attribute a(encryptionCapable, false);
				pInstance->setAttribute(ENCRYPTIONCAPABLE_KEY, a, attributes);
			}

			if (containsAttribute(ENCRYPTIONENABLED_KEY, attributes))
			{
				bool encryptionEnabled = getEncryptionEnabled(pPool);
				framework::Attribute a(encryptionEnabled, false);
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

			// Health State = enum + string
			if (containsAttribute(HEALTHSTATE_KEY, attributes))
			{
				framework::Attribute a((NVM_UINT16)pPool->health,
						poolHealthToStr(pPool->health), false);
				pInstance->setAttribute(HEALTHSTATE_KEY, a, attributes);
			}

			// ActionRequired = true if any unacknowledged action required events for this pool
			if (containsAttribute(ACTIONREQUIRED_KEY, attributes))
			{
				framework::Attribute a(isActionRequiredForPool(pPool), false);
				pInstance->setAttribute(ACTIONREQUIRED_KEY, a, attributes);
			}
			// ActionRequiredEvents = list of action required events ids and messages
			if (containsAttribute(ACTIONREQUIREDEVENTS_KEY, attributes))
			{
				framework::Attribute a(getActionRequiredEvents(pPool), false);
				pInstance->setAttribute(ACTIONREQUIREDEVENTS_KEY, a, attributes);
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

struct event_filter wbem::mem_config::PoolViewFactory::getPoolActionRequiredFilterForDimm(
		NVM_UID dimm_uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	event_filter poolEventFilter;
	memset(&poolEventFilter, 0, sizeof (poolEventFilter));

	poolEventFilter.filter_mask = NVM_FILTER_ON_AR | NVM_FILTER_ON_UID;
	poolEventFilter.action_required = true;
	memmove(poolEventFilter.uid, dimm_uid, NVM_MAX_UID_LEN);

	return poolEventFilter;
}

bool wbem::mem_config::PoolViewFactory::isActionRequiredForPool(pool *pPool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	int eventCount = 0;
	try
	{
		for (int i = 0; i < pPool->dimm_count; i++)
		{
			event_filter filter = getPoolActionRequiredFilterForDimm(pPool->dimms[i]);
			eventCount += m_nvmLib.getEventCount(filter);
		}
	}
	catch (core::LibraryException &e)
	{
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}

	return eventCount > 0;
}

std::string wbem::mem_config::PoolViewFactory::getActionRequiredEvents(pool *pPool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<event> actionRequiredEvents;
	std::string formattedEventStr = POOL_ACTIONREQUIRED_EVENTS_NA;
	event_filter filter;
	try{
		for (int i = 0; i < pPool->dimm_count; i++)
		{
			filter = getPoolActionRequiredFilterForDimm(pPool->dimms[i]);
			actionRequiredEvents = m_nvmLib.getEvents(filter);
			if (!(actionRequiredEvents.empty()))
			{
				if(formattedEventStr == POOL_ACTIONREQUIRED_EVENTS_NA)
				{
					formattedEventStr = core::Helper::getFormattedEventList(actionRequiredEvents);
				}
				else
				{
					formattedEventStr.append(", ");
					formattedEventStr.append(core::Helper::getFormattedEventList(
							actionRequiredEvents));
				}
			}
		}
	}
	catch (core::LibraryException &e)
	{
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}
	return formattedEventStr;
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
		std::string hostName = m_nvmLib.getHostName();
		std::vector<struct pool> pools = getPoolList(true);
		for (std::vector<struct pool>::const_iterator iter = pools.begin();
				iter != pools.end(); iter++)
		{
			framework::attributes_t keys;

			NVM_UID poolUid;
			uid_copy((*iter).pool_uid, poolUid);
			keys[POOLID_KEY] = framework::Attribute(std::string(poolUid), true);

			framework::ObjectPath path(hostName, NVM_NAMESPACE,
					INTEL_POOLVIEW_CREATIONCLASSNAME, keys);
			pNames->push_back(path);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}
	catch (core::LibraryException &e)
	{
		delete pNames;
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}
	return pNames;
}

/*
 * Helper function to retrieve a list of pools
 */
std::vector<struct pool> wbem::mem_config::PoolViewFactory::getPoolList(bool pmOnly)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::vector<struct pool> poolList;

	try
	{
		poolList = m_nvmLib.getPools();
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
	}
	catch (core::LibraryException &e)
	{
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}
	// return the vector
	return poolList;
}

/*
 * Helper function to retrieve a specific pool.
 */
struct pool *wbem::mem_config::PoolViewFactory::getPool(const std::string &poolUidStr)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UID poolUid;
	struct pool *pPool = NULL;
	uid_copy(poolUidStr.c_str(), poolUid);
	try
	{
		pPool = m_nvmLib.getPool(poolUid);
	}
	catch (core::LibraryException &e)
	{
		delete pPool;
		throw exception::NvmExceptionLibError(e.getErrorCode());
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

bool wbem::mem_config::PoolViewFactory::getEncryptionCapable(pool *pPool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return pPool->encryption_capable;
}

bool wbem::mem_config::PoolViewFactory::getEncryptionEnabled(const struct pool *pPool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool result = false;

	try
	{
		for (NVM_UINT16 i = 0; i < pPool->dimm_count && result == false; i++)
		{
			struct device_discovery device = m_nvmLib.getDeviceDiscovery(pPool->dimms[i]);

			switch (device.lock_state)
			{
			case LOCK_STATE_UNLOCKED:
			case LOCK_STATE_LOCKED:
			case LOCK_STATE_FROZEN:
			case LOCK_STATE_PASSPHRASE_LIMIT:
				result = true;
				break;
			case LOCK_STATE_DISABLED:
			case LOCK_STATE_UNKNOWN:
			default:
				break;
			}
		}
	}
	catch (core::LibraryException &e)
	{
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}

	return result;
}

bool wbem::mem_config::PoolViewFactory::PoolHasAppDirectInterleaved(const struct pool *pPool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool result = false;

	if ((pPool->type == POOL_TYPE_PERSISTENT_MIRROR) ||
			(pPool->type == POOL_TYPE_PERSISTENT))
	{
		for (NVM_UINT16 i = 0; i < pPool->ilset_count; i++)
		{
			if (pPool->ilsets[i].settings.ways > INTERLEAVE_WAYS_1) // INTERLEAVE_WAYS_0 is don't care
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

bool wbem::mem_config::PoolViewFactory::PoolHasAppDirectByOne(const struct pool *pPool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool result = false;

	if (pPool->type == POOL_TYPE_PERSISTENT)
	{
		for (NVM_UINT16 i = 0; i < pPool->ilset_count; i++)
		{
			if (pPool->ilsets[i].settings.ways == INTERLEAVE_WAYS_1)
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

wbem::framework::STR_LIST wbem::mem_config::PoolViewFactory::getPersistentMemoryType(
		const struct pool *pPool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::STR_LIST pmType;

	if (PoolHasAppDirectByOne(pPool))
	{
		pmType.push_back(wbem::mem_config::PMTYPE_APPDIRECT_NOTINTERLEAVED);
	}

	if (PoolHasAppDirectInterleaved(pPool))
	{
		pmType.push_back(wbem::mem_config::PMTYPE_APPDIRECT);
	}

	return pmType;
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

	try
	{
		if (m_nsCache.size() == 0)
		{
			int rc = m_nvmLib.getNamespaceCount();

			if (rc > 0)
			{
				std::vector<struct namespace_discovery> namespaces;
				namespaces = m_nvmLib.getNamespaces();

				for (std::vector<struct namespace_discovery>::const_iterator iter = namespaces.begin();
						iter != namespaces.end(); iter++)
				{
					struct namespace_details nsDetails = m_nvmLib.getNamespaceDetails((*iter).namespace_uid);
					m_nsCache.push_back(nsDetails);
				}
			}
		}
	}
	catch (core::LibraryException &e)
	{
		throw exception::NvmExceptionLibError(e.getErrorCode());
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
		case POOL_HEALTH_PENDING:
			healthStr = POOL_HEALTH_STR_PENDING;
			break;
		case POOL_HEALTH_ERROR:
			healthStr = POOL_HEALTH_STR_ERROR;
			break;
		case POOL_HEALTH_LOCKED:
			healthStr = POOL_HEALTH_STR_LOCKED;
			break;
		default:
			healthStr = POOL_HEALTH_STR_UNKNOWN;
			break;
	}

	return healthStr;
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

struct possible_namespace_ranges wbem::mem_config::PoolViewFactory::getAvailablePersistentSizeRange(NVM_UID pool_uid, const NVM_UINT8 ways)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct possible_namespace_ranges ranges ;
	memset(&ranges, 0, sizeof (possible_namespace_ranges));

	try
	{
		ranges = m_nvmLib.getAvailablePersistentSizeRange(pool_uid, ways);
	}
	catch (core::LibraryException &e)
	{
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}

	return ranges;
}
