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
 * This file contains the provider for the PersistentMemoryPool instances
 * which represent NVM-DIMM hosted persistent capacity with a given set of QoS attributes.
 */

#include <string>

#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <libinvm-cim/ExceptionNotSupported.h>
#include <server/BaseServerFactory.h>
#include <LogEnterExit.h>
#include <nvm_management.h>
#include <uid/uid.h>

#include "PersistentMemoryPoolFactory.h"
#include "mem_config/PoolViewFactory.h"
#include <pmem_config/NamespaceSettingsFactory.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>
#include <core/Helper.h>

wbem::pmem_config::PersistentMemoryPoolFactory::PersistentMemoryPoolFactory()
	throw (wbem::framework::Exception)
{
	m_GetAvailablePersistentSizeRange = nvm_get_available_persistent_size_range;
}

wbem::pmem_config::PersistentMemoryPoolFactory::~PersistentMemoryPoolFactory()
{
}

void wbem::pmem_config::PersistentMemoryPoolFactory::populateAttributeList(
	framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(wbem::INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(wbem::ELEMENTNAME_KEY);
	attributes.push_back(wbem::RESERVED_KEY);
	attributes.push_back(wbem::CAPACITY_KEY);
	attributes.push_back(wbem::PRIMORDIAL_KEY);
	attributes.push_back(wbem::POOLID_KEY);
	attributes.push_back(wbem::ALLOCATIONUNITS_KEY);
	attributes.push_back(wbem::RESOURCETYPE_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::pmem_config::PersistentMemoryPoolFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	struct pool *pPool = NULL;

	if (pInstance != NULL)
	{
		try
		{
			checkAttributes(attributes);

			// InstanceID is the pool UID we should fetch
			framework::Attribute instanceID = path.getKeyValue(INSTANCEID_KEY);
			std::string uidStr = instanceID.stringValue();
			if (uidStr.length() != (NVM_MAX_UID_LEN - 1))
			{
				throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
			}

			pPool = wbem::mem_config::PoolViewFactory::getPool(uidStr);

			// Element Name = "NVMDIMM Available Capacity Pool"
			if (containsAttribute(ELEMENTNAME_KEY, attributes))
			{
				framework::Attribute attrElementName(PERSISTENTMEMORYPOOL_ELEMENTNAME, false);
				pInstance->setAttribute(ELEMENTNAME_KEY, attrElementName, attributes);
			}

			// Allocated capacity in bytes
			if (containsAttribute(RESERVED_KEY, attributes))
			{
				framework::Attribute attrRemainingManagedSpace(pPool->capacity - pPool->free_capacity, false);
				pInstance->setAttribute(RESERVED_KEY, attrRemainingManagedSpace, attributes);
			}

			// Number of bytes in the pool
			if (containsAttribute(CAPACITY_KEY, attributes))
			{
				framework::Attribute attrManagedSpace(pPool->capacity, false);
				pInstance->setAttribute(CAPACITY_KEY, attrManagedSpace, attributes);
			}

			// Primordial = true
			if (containsAttribute(PRIMORDIAL_KEY, attributes))
			{
				framework::Attribute attrPrimodial(true, false);
				pInstance->setAttribute(PRIMORDIAL_KEY, attrPrimodial, attributes);
			}

			// PoolID = "NVMDIMM Available Capacity"
			if (containsAttribute(POOLID_KEY, attributes))
			{
				framework::Attribute attrPoolId(PERSISTENTMEMORYPOOL_POOLID, false);
				pInstance->setAttribute(POOLID_KEY, attrPoolId, attributes);
			}

			// AllocationUnits = "bytes"
			if (containsAttribute(ALLOCATIONUNITS_KEY, attributes))
			{
				framework::Attribute attrAllocationUnits(PERSISTENTMEMORYPOOL_ALLOCATIONUNITS, false);
				pInstance->setAttribute(ALLOCATIONUNITS_KEY, attrAllocationUnits, attributes);
			}

			// ResourceType = "Non-Volatile Memory"
			if (containsAttribute(RESOURCETYPE_KEY, attributes))
			{
				framework::Attribute attrResourceType(PERSISTENTMEMORYPOOL_RESOURCETYPE, false);
				pInstance->setAttribute(RESOURCETYPE_KEY, attrResourceType, attributes);
			}
			delete pPool;
		}
		catch (framework::Exception &) // clean up and re-throw
		{
			delete pInstance;
			if (pPool)
			{
				delete pPool;
			}
			throw;
		}
	}
	else // Couldn't allocate memory
	{
		throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__, "Failed to allocate Instance");
	}

	return pInstance;
}

/*
 * Return the object paths for the PersistentMemoryPool instances.
 */
wbem::framework::instance_names_t* wbem::pmem_config::PersistentMemoryPoolFactory::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::instance_names_t *pNames = new framework::instance_names_t();
	if (pNames == NULL)
	{
		throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__,
				"Failed to allocate instance_names_t");
	}
	try
	{
		std::vector<struct pool> pools = mem_config::PoolViewFactory::getPoolList(true);
		for (std::vector<struct pool>::const_iterator iter = pools.begin();
				iter != pools.end(); iter++)
		{
			framework::attributes_t keys;

			// InstanceID = pool UUID
			NVM_UID poolUidStr;
			uid_copy((*iter).pool_uid, poolUidStr);
			framework::Attribute attrInstanceID(poolUidStr, true);
			keys[INSTANCEID_KEY] = attrInstanceID;

			// generate the ObjectPath for the instance
			framework::ObjectPath path(wbem::server::getHostName(), NVM_NAMESPACE,
					PERSISTENTMEMORYPOOL_CREATIONCLASSNAME, keys);
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
 * Helper function to retrieve the largest and smallest namespace that can be created
 */
struct possible_namespace_ranges wbem::pmem_config::PersistentMemoryPoolFactory::getSupportedSizeRange(
		const std::string &poolUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct possible_namespace_ranges range;
	NVM_UID uid;
	uid_copy(poolUid.c_str(), uid);
	int rc = m_GetAvailablePersistentSizeRange(uid, &range);
	if (rc < NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
	return range;
}

void wbem::pmem_config::PersistentMemoryPoolFactory::getSupportedSizeRange(
		const std::string &poolUid,
		COMMON_UINT64 &largestPossibleAdNs,
		COMMON_UINT64 &smallestPossibleAdNs,
		COMMON_UINT64 &adIncrement,
		COMMON_UINT64 &largestPossibleStorageNs,
		COMMON_UINT64 &smallestPossibleStorageNs,
		COMMON_UINT64 &storageIncrement)
{
	struct possible_namespace_ranges range = getSupportedSizeRange(poolUid);
	largestPossibleAdNs = range.largest_possible_app_direct_ns;
	smallestPossibleAdNs = range.smallest_possible_app_direct_ns;
	adIncrement = range.app_direct_increment;

	largestPossibleStorageNs = range.largest_possible_storage_ns;
	smallestPossibleStorageNs = range.smallest_possible_storage_ns;
	storageIncrement = range.storage_increment;
}

wbem::framework::UINT32 wbem::pmem_config::PersistentMemoryPoolFactory::executeMethod(
		wbem::framework::UINT32 &wbemRc,
		const std::string method,
		wbem::framework::ObjectPath &object,
		wbem::framework::attributes_t &inParms,
		wbem::framework::attributes_t &outParms)
{
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;
	wbemRc = framework::MOF_ERR_SUCCESS;
	COMMON_LOG_ENTRY_PARAMS("methodName: %s, number of in params: %u", method.c_str(), inParms.size());

	try
	{
		if (method == PERSISTENTMEMORYPOOL_GETSUPPORTEDSIZERANGE)
		{
			/*
			    GetSupportedSizeRange(
				string Goal, // embedded instance of CIM_NamespaceSettings
				uint64 MinimumNamespaceSize,
				uint64 MaximumNamespaceSize,
				uint64 NamespaceSizeDivisor);
			 */

			// get pool UID from object path
			std::string poolUidStr;
			try
			{
				poolUidStr = object.getKeyValue(wbem::INSTANCEID_KEY).stringValue();
			}
			catch (framework::ExceptionBadParameter &)
			{
				COMMON_LOG_ERROR("Invalid Object Path, missing instance ID");
				httpRc = framework::CIM_ERR_INVALID_PARAMETER;
				throw;
			}
			if (!core::Helper::isValidPoolUid(poolUidStr))
			{
				COMMON_LOG_ERROR_F("Invalid pool uid in object path: %s", poolUidStr.c_str());
				httpRc = framework::CIM_ERR_INVALID_PARAMETER;
				throw framework::ExceptionBadParameter(wbem::INSTANCEID_KEY.c_str());
			}

			// check namespace settings goal
			std::string goalString = inParms[PERSISTENTMEMORYPOOL_GOAL].stringValue();
			if (goalString.empty())
			{
				COMMON_LOG_ERROR_F("%s is required.", PERSISTENTMEMORYPOOL_GOAL.c_str());
				httpRc = framework::CIM_ERR_INVALID_PARAMETER;
				throw framework::ExceptionBadParameter(PERSISTENTMEMORYPOOL_GOAL.c_str());
			}

			// check namespace type
			wbem::framework::Instance *pGoalInstance = new framework::Instance(goalString);
			wbem::framework::Attribute namespaceTypeAttribute;
			pGoalInstance->getAttribute(wbem::RESOURCETYPE_KEY, namespaceTypeAttribute);
			NVM_UINT16 type = namespaceTypeAttribute.uintValue();
			if ((type != wbem::pmem_config::NS_RESOURCETYPE_BLOCK_ADDRESSABLE) &&
				(type != wbem::pmem_config::NS_RESOURCETYPE_BYTE_ADDRESSABLE))
			{
				delete pGoalInstance;
				COMMON_LOG_ERROR_F("Invalid namespace type in object path: %d", type);
				httpRc = framework::CIM_ERR_INVALID_PARAMETER;
				throw framework::ExceptionBadParameter(PERSISTENTMEMORYPOOL_GOAL.c_str());
			}
			delete pGoalInstance;

			// get supported namespace size range
			struct possible_namespace_ranges p_range = getSupportedSizeRange(poolUidStr);
			wbemRc = wbem::framework::SUCCESS;

			// send back sizes based on specified namespace type
			if (type == wbem::pmem_config::NS_RESOURCETYPE_BLOCK_ADDRESSABLE)
			{
				outParms[PERSISTENTMEMORYPOOL_MIN_NS_SIZE] =
						framework::Attribute(p_range.smallest_possible_storage_ns, false);
				outParms[PERSISTENTMEMORYPOOL_MAX_NS_SIZE] =
						framework::Attribute(p_range.largest_possible_storage_ns, false);
				outParms[PERSISTENTMEMORYPOOL_NS_SIZE_DIVISOR] =
						framework::Attribute(p_range.storage_increment, false);
			}
			else
			{
				outParms[PERSISTENTMEMORYPOOL_MIN_NS_SIZE] =
						framework::Attribute(p_range.smallest_possible_app_direct_ns, false);
				outParms[PERSISTENTMEMORYPOOL_MAX_NS_SIZE] =
						framework::Attribute(p_range.largest_possible_app_direct_ns, false);
				outParms[PERSISTENTMEMORYPOOL_NS_SIZE_DIVISOR] =
						framework::Attribute(p_range.app_direct_increment, false);
			}
		}
		else
		{
			httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
			COMMON_LOG_ERROR_F("methodName %s not supported", method.c_str());
		}
	}
	catch (wbem::framework::ExceptionBadParameter &)
	{
		wbemRc = PERSISTENTMEMORYPOOL_ERR_INVALID_PARAMETER;
	}
	catch (wbem::exception::NvmExceptionLibError &)
	{
		wbemRc = PERSISTENTMEMORYPOOL_ERR_FAILED;
	}
	catch (wbem::framework::ExceptionNoMemory &)
	{
		wbemRc = PERSISTENTMEMORYPOOL_ERR_INSUFFICIENT_RESOURCES;
	}
	catch (wbem::framework::ExceptionNotSupported &)
	{
		wbemRc = PERSISTENTMEMORYPOOL_ERR_NOT_SUPPORTED;
	}
	catch (wbem::framework::Exception &)
	{
		wbemRc = PERSISTENTMEMORYPOOL_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN("httpRc: %u, wbemRc: %u", httpRc, wbemRc);
	return httpRc;
}
