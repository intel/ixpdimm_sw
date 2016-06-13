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
 * This file contains the provider for the NamespaceView instances which is
 * an internal only view class for the show namespace CLI command.
 */

#include <LogEnterExit.h>
#include <nvm_management.h>
#include <utility.h>
#include <libinvm-cim/Attribute.h>
#include <server/BaseServerFactory.h>
#include <uid/uid.h>
#include <string/s_str.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <sstream>
#include "NamespaceViewFactory.h"
#include <string.h>
#include <mem_config/InterleaveSet.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>
#include <core/Helper.h>

wbem::pmem_config::NamespaceViewFactory::NamespaceViewFactory()
throw(wbem::framework::Exception)
{
}

wbem::pmem_config::NamespaceViewFactory::~NamespaceViewFactory()
{
}

void wbem::pmem_config::NamespaceViewFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw(wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(NAMESPACEID_KEY);

	// add non-key attributes
	attributes.push_back(POOLID_KEY);
	attributes.push_back(HEALTHSTATE_KEY);
	attributes.push_back(NAME_KEY);
	attributes.push_back(ENABLEDSTATE_KEY);
	attributes.push_back(CAPACITY_KEY);
	attributes.push_back(NUMBEROFBLOCKS_KEY);
	attributes.push_back(BLOCKSIZE_KEY);
	attributes.push_back(TYPE_KEY);
	attributes.push_back(OPTIMIZE_KEY);
	attributes.push_back(ACTIONREQUIRED_KEY);
	attributes.push_back(ACTIONREQUIREDEVENTS_KEY);
	attributes.push_back(ENCRYPTIONENABLED_KEY);
	attributes.push_back(ERASECAPABLE_KEY);
	attributes.push_back(APP_DIRECT_SETTINGS_KEY);
	attributes.push_back(REPLICATION_KEY);
	attributes.push_back(MEMORYPAGEALLOCATION_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance *wbem::pmem_config::NamespaceViewFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);

		std::string nsUidStr = path.getKeyValue(NAMESPACEID_KEY).stringValue();
		if (!core::Helper::isValidNamespaceUid(nsUidStr))
		{
			if (pInstance)
			{
				delete pInstance;
				pInstance = NULL;
			}
			throw framework::ExceptionBadParameter(NAMESPACEID_KEY.c_str());
		}

		struct namespace_details ns = getNamespaceDetails(nsUidStr);

		// Name = Friendly Name
		if (containsAttribute(NAME_KEY, attributes))
		{
			framework::Attribute a(ns.discovery.friendly_name, false);
			pInstance->setAttribute(NAME_KEY, a, attributes);
		}

		// PoolID = Pool UID
		if (containsAttribute(POOLID_KEY, attributes))
		{
			NVM_UID poolUidStr;
			uid_copy(ns.pool_uid, poolUidStr);
			framework::Attribute a(poolUidStr, false);
			pInstance->setAttribute(POOLID_KEY, a, attributes);
		}

		// Type = namespace type
		if (containsAttribute(TYPE_KEY, attributes))
		{
			framework::Attribute a((NVM_UINT16)ns.type,
					namespaceTypeToStr(ns.type), false);
			pInstance->setAttribute(TYPE_KEY, a, attributes);
		}

		// Capacity = NumberOfBlocks * Block Size in bytes
		if (containsAttribute(CAPACITY_KEY, attributes))
		{
			NVM_UINT64 capacity;
			if (ns.block_size == 0) // unspecified - assume block size of 1
			{
				capacity = (NVM_UINT64)ns.block_count;
			}
			else
			{
				capacity = adjust_namespace_size(ns.block_size, ns.block_count);
			}
			framework::Attribute a(capacity, false);
			pInstance->setAttribute(CAPACITY_KEY, a, attributes);
		}

		// NumberOfBlocks
		if (containsAttribute(NUMBEROFBLOCKS_KEY, attributes))
		{
			framework::Attribute a(ns.block_count, false);
			pInstance->setAttribute(NUMBEROFBLOCKS_KEY, a, attributes);
		}

		// BlockSize
		if (containsAttribute(BLOCKSIZE_KEY, attributes))
		{
			framework::Attribute a((NVM_UINT64)ns.block_size, false);
			pInstance->setAttribute(BLOCKSIZE_KEY, a, attributes);
		}

		// Health State = enum + string
		if (containsAttribute(HEALTHSTATE_KEY, attributes))
		{
			framework::Attribute a((NVM_UINT16)ns.health,
					namespaceHealthToStr(ns.health), false);
			pInstance->setAttribute(HEALTHSTATE_KEY, a, attributes);
		}

		// Enabled State = enum + string
		if (containsAttribute(ENABLEDSTATE_KEY, attributes))
		{
			framework::Attribute a((NVM_UINT16)ns.enabled,
					namespaceEnableStateToStr(ns.enabled), false);
			pInstance->setAttribute(ENABLEDSTATE_KEY, a, attributes);
		}

		// Optimize = btt
		if (containsAttribute(OPTIMIZE_KEY, attributes))
		{
			framework::Attribute a(namespaceOptimizeToValue(ns.btt),
					namespaceOptimizeToStr(ns.btt), false);
			pInstance->setAttribute(OPTIMIZE_KEY, a, attributes);
		}

		if (containsAttribute(ACTIONREQUIRED_KEY, attributes) ||
				containsAttribute(ACTIONREQUIREDEVENTS_KEY, attributes))
		{
			struct event_filter filter;
			memset(&filter, 0, sizeof (filter));
			filter.filter_mask = NVM_FILTER_ON_AR | NVM_FILTER_ON_UID;
			filter.action_required = true;
			memmove(filter.uid, ns.discovery.namespace_uid, NVM_MAX_UID_LEN);
			int eventCount = nvm_get_event_count(&filter);
			if (eventCount < 0)
			{
				COMMON_LOG_ERROR_F("Failed to retrieve events for namespace %s, error %d",
						nsUidStr.c_str(), eventCount);
				throw exception::NvmExceptionLibError(eventCount);
			}

			// ActionRequired = true if any unacknowledged action required events for this namespace
			if (containsAttribute(ACTIONREQUIRED_KEY, attributes))
			{
				framework::Attribute a(eventCount > 0 ? true : false, false);
				pInstance->setAttribute(ACTIONREQUIRED_KEY, a, attributes);
			}

			// ActionRequiredEvents = list of action required events ids and messages
			if (containsAttribute(ACTIONREQUIREDEVENTS_KEY, attributes))
			{
				framework::STR_LIST arEventList;
				if (eventCount > 0)
				{
					// get the events
					struct event events[eventCount];
					eventCount = nvm_get_events(&filter, events, eventCount);
					if (eventCount < 0)
					{
						COMMON_LOG_ERROR_F("Failed to retrieve events for namespace %s, error %d",
								nsUidStr.c_str(), eventCount);
						throw exception::NvmExceptionLibError(eventCount);
					}

					for (int i = 0; i < eventCount; i++)
					{
						std::stringstream eventMsg;
						eventMsg << "Event " << events[i].event_id;
						char msg[NVM_EVENT_MSG_LEN + (3 * NVM_EVENT_ARG_LEN)];
						s_snprintf(msg, (NVM_EVENT_MSG_LEN + (3 * NVM_EVENT_ARG_LEN)),
								events[i].message,
								events[i].args[0],
								events[i].args[1],
								events[i].args[2]);
						eventMsg << " - " << msg;
						arEventList.push_back(eventMsg.str());
					}
				}
				framework::Attribute a(arEventList, false);
				pInstance->setAttribute(ACTIONREQUIREDEVENTS_KEY, a, attributes);
			}
		}

		if (containsAttribute(ENCRYPTIONENABLED_KEY, attributes))
		{
			pInstance->setAttribute(ENCRYPTIONENABLED_KEY,
				framework::Attribute((NVM_UINT16)ns.security_features.encryption, false));
		}

		if (containsAttribute(ERASECAPABLE_KEY, attributes))
		{
			pInstance->setAttribute(ERASECAPABLE_KEY,
				framework::Attribute((NVM_UINT16)ns.security_features.erase_capable, false));
		}

		if (containsAttribute(APP_DIRECT_SETTINGS_KEY, attributes))
		{
			std::string format =
					wbem::mem_config::InterleaveSet::getInterleaveFormatString(&ns.interleave_format);

			pInstance->setAttribute(APP_DIRECT_SETTINGS_KEY,
			framework::Attribute(format, false));
		}

		if (containsAttribute(REPLICATION_KEY, attributes))
		{
			pInstance->setAttribute(REPLICATION_KEY,
					framework::Attribute((bool)ns.mirrored, false));
		}

		if (containsAttribute(MEMORYPAGEALLOCATION_KEY, attributes))
		{
			framework::Attribute a((NVM_UINT16)ns.memory_page_allocation,
					namespaceMemoryPageAllocationToStr(ns.memory_page_allocation), false);
			pInstance->setAttribute(MEMORYPAGEALLOCATION_KEY, a, attributes);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		if (pInstance)
		{
			delete pInstance;
			pInstance = NULL;
		}
		throw;
	}

	return pInstance;
}

/*
 * Return the object paths for the Intel_NamespaceViewFactory class.
 */
wbem::framework::instance_names_t *wbem::pmem_config::NamespaceViewFactory::getInstanceNames()
throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		std::vector<std::string> nsList = getNamespaceUidList();
		for (std::vector<std::string>::const_iterator iter = nsList.begin();
				iter != nsList.end(); iter++)
		{
			framework::attributes_t keys;
			std::string uidStr = *iter;
			keys[NAMESPACEID_KEY] = framework::Attribute(*iter, true);

			framework::ObjectPath path(server::getHostName(), NVM_NAMESPACE,
					NAMESPACEVIEW_CREATIONCLASSNAME, keys);
			pNames->push_back(path);
		}
	}
	catch (framework::Exception &)
	{
		delete pNames;
		throw;
	}
	return pNames;
}

/*
 * Helper function to retrieve a list of namespaces.
 */
std::vector<std::string>
	wbem::pmem_config::NamespaceViewFactory::getNamespaceUidList()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<std::string> nsList;
	int nsCount = nvm_get_namespace_count();
	if (nsCount < 0)
	{
		throw exception::NvmExceptionLibError(nsCount);
	}
	if (nsCount > 0)
	{
		// Note: allocate namespace list on the heap to avoid OpenPegasus issues
		struct namespace_discovery *namespaces =
				(struct namespace_discovery *)calloc(nsCount, sizeof (struct namespace_discovery));
		if (namespaces)
		{
			nsCount = nvm_get_namespaces(namespaces, nsCount);
			if (nsCount < 0)
			{
				free (namespaces);
				throw exception::NvmExceptionLibError(nsCount);
			}
			for (int i = 0; i < nsCount; i++)
			{
				NVM_UID uidStr;
				uid_copy(namespaces[i].namespace_uid, uidStr);
				nsList.push_back(std::string(uidStr));
			}
			free(namespaces);
		}
		else
		{
			throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__,
					"Allocating the namespace_discovery array failed");
		}
	}
	return nsList;
}

/*
 * Helper function to retrieve details about a specific namespace.
 */
struct namespace_details
	wbem::pmem_config::NamespaceViewFactory::getNamespaceDetails(const std::string &nsUidStr)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UID nsUid;
	uid_copy(nsUidStr.c_str(), nsUid);

	struct namespace_details details;
	memset(&details, 0, sizeof (details));
	int rc = nvm_get_namespace_details(nsUid, &details);
	if (rc < NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}

	return details;
}

/*
 * Helper function to convert namespace health to a string.
 */
std::string wbem::pmem_config::NamespaceViewFactory::namespaceHealthToStr(
		const enum namespace_health &health)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string healthStr;
	switch (health)
	{
		case NAMESPACE_HEALTH_NORMAL:
			healthStr = NS_HEALTH_STR_NORMAL;
			break;
		case NAMESPACE_HEALTH_NONCRITICAL:
			healthStr = NS_HEALTH_STR_WARN;
			break;
		case NAMESPACE_HEALTH_CRITICAL:
			healthStr = NS_HEALTH_STR_ERR;
			break;
		case NAMESPACE_HEALTH_BROKENMIRROR:
			healthStr = NS_HEALTH_STR_BROKENMIRROR;
			break;
		default:
			healthStr = NS_HEALTH_STR_UNKNOWN;
			break;
	}
	return healthStr;
}

/*
 * Helper function to convert namespace enable state to a string.
 */
std::string wbem::pmem_config::NamespaceViewFactory::namespaceEnableStateToStr(
		const enum namespace_enable_state &enableState)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string enableStateStr;
	switch (enableState)
	{
		case NAMESPACE_ENABLE_STATE_ENABLED:
			enableStateStr = "Enabled";
			break;
		case NAMESPACE_ENABLE_STATE_DISABLED:
			enableStateStr = "Disabled";
			break;
		default:
			enableStateStr = "Unknown";
			break;
	}
	return enableStateStr;
}

/*
 * Helper function to convert namespace type to a string
 */
std::string wbem::pmem_config::NamespaceViewFactory::namespaceTypeToStr(const enum namespace_type &type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string typeStr;
	switch (type)
	{
		case NAMESPACE_TYPE_STORAGE:
			typeStr = NS_TYPE_STR_STORAGE;
			break;
		case NAMESPACE_TYPE_APP_DIRECT:
			typeStr = NS_TYPE_STR_APPDIRECT;
			break;
		default:
			typeStr = NS_TYPE_STR_UNKNOWN;
			break;
	}
	return typeStr;
}


/*
 * Helper function to convert namespace btt to an optimize string
 */
std::string wbem::pmem_config::NamespaceViewFactory::namespaceOptimizeToStr(const NVM_BOOL &btt)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string optimizeStr;
	if (btt)
	{
		optimizeStr = "CopyOnWrite";
	}
	else
	{
		optimizeStr = "None";
	}
	return optimizeStr;
}

/*
 * Helper function to convert namespace btt to an optimize value
 */
NVM_UINT16 wbem::pmem_config::NamespaceViewFactory::namespaceOptimizeToValue(const NVM_BOOL &btt)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT16 optimize = NS_OPTIMIZE_NONE;
	if (btt)
	{
		optimize = NS_OPTIMIZE_COPYONWRITE;
	}
	return optimize;
}

/*
 * Helper function to convert namespace memory page allocation attribute to a memory page allocation string
 */
std::string wbem::pmem_config::NamespaceViewFactory::namespaceMemoryPageAllocationToStr(
		const enum namespace_memory_page_allocation allocation)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string str = NS_MEMORY_PAGE_ALLOCATION_STR_NONE;
	switch (allocation)
	{
	case NAMESPACE_MEMORY_PAGE_ALLOCATION_APP_DIRECT:
		str = NS_MEMORY_PAGE_ALLOCATION_STR_APP_DIRECT;
		break;
	case NAMESPACE_MEMORY_PAGE_ALLOCATION_DRAM:
		str = NS_MEMORY_PAGE_ALLOCATION_STR_DRAM;
		break;
	default:
		break;
	}

	return str;
}
