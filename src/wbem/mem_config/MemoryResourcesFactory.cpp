/*
 * Copyright (c) 2015, Intel Corporation
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
 * This file contains the provider for the MemoryResources instances which
 * represents system NVM capacity in total regardless of how or whether
 * it is accessible to the host operating system.
 */

#include <nvm_management.h>

#include <LogEnterExit.h>

#include <intel_cim_framework/ExceptionBadParameter.h>
#include <intel_cim_framework/ExceptionNoMemory.h>
#include "MemoryResourcesFactory.h"
#include <server/BaseServerFactory.h>
#include <sstream>
#include <mem_config/PoolViewFactory.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>

wbem::mem_config::MemoryResourcesFactory::MemoryResourcesFactory()
	throw (wbem::framework::Exception)
{
}

wbem::mem_config::MemoryResourcesFactory::~MemoryResourcesFactory()
{
}

void wbem::mem_config::MemoryResourcesFactory::populateAttributeList(
	framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(wbem::INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(wbem::ELEMENTNAME_KEY);
	attributes.push_back(wbem::PRIMORDIAL_KEY);
	attributes.push_back(wbem::POOLID_KEY);
	attributes.push_back(wbem::CAPACITY_KEY);
	attributes.push_back(wbem::RESOURCETYPE_KEY);
	attributes.push_back(wbem::ALLOCATIONUNITS_KEY);
	attributes.push_back(wbem::RESERVED_KEY);
	attributes.push_back(wbem::VOLATILECAPACITY_KEY);
	attributes.push_back(wbem::PERSISTENTCAPACITY_KEY);
	attributes.push_back(wbem::BLOCKCAPACITY_KEY);
	attributes.push_back(wbem::UNCONFIGUREDCAPACITY_KEY);
	attributes.push_back(wbem::INACCESSIBLECAPACITY_KEY);
	attributes.push_back(wbem::RESERVEDCAPACITY_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::mem_config::MemoryResourcesFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);

		// get host server name
		std::string hostName = wbem::server::getHostName();

		// check the object path is valid
		framework::Attribute instanceID = path.getKeyValue(INSTANCEID_KEY);
		if (instanceID.stringValue().compare(hostName + MEMORYRESOURCES_INSTANCEID) != 0)
		{
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
		}

		// get aggregate capacities
		struct device_capacities capacities;
		int rc = m_pApi->getNvmCapacities(&capacities);
		if (rc != NVM_SUCCESS)
		{
			// couldn't retrieve the capacities
			throw exception::NvmExceptionLibError(rc);
		}

		// Element Name = "Platform NVM Primordial Pool"
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			framework::Attribute attrElementName(MEMORYRESOURCES_ELEMENTNAME, false);
			pInstance->setAttribute(ELEMENTNAME_KEY, attrElementName, attributes);
		}

		// Primordial = true
		if (containsAttribute(PRIMORDIAL_KEY, attributes))
		{
			framework::Attribute attrPrimodial(true, false);
			pInstance->setAttribute(PRIMORDIAL_KEY, attrPrimodial, attributes);
		}

		// PoolID - NVMPool1  (arbitrary ID)
		if (containsAttribute(POOLID_KEY, attributes))
		{
			framework::Attribute a(MEMORYRESOURCES_POOLID, false);
			pInstance->setAttribute(POOLID_KEY, a, attributes);
		}

		// Total Capacity
		if (containsAttribute(CAPACITY_KEY, attributes))
		{
			framework::Attribute attrCapacity(capacities.capacity, false);
			pInstance->setAttribute(CAPACITY_KEY, attrCapacity, attributes);
		}

		// ResourceType = 34, "Multi-mode memory"
		if (containsAttribute(RESOURCETYPE_KEY, attributes))
		{
			framework::Attribute attrResourceType((NVM_UINT16)MEMORYRESOURCES_RESOURCETYPE_VAL, false);
			pInstance->setAttribute(RESOURCETYPE_KEY, attrResourceType, attributes);
		}

		// AllocationUnits - bytes
		if (containsAttribute(ALLOCATIONUNITS_KEY, attributes))
		{
			framework::Attribute attrAllocationUnitsType(allocationUnitsToStr(capacities.capacity), false);
			pInstance->setAttribute(ALLOCATIONUNITS_KEY, attrAllocationUnitsType, attributes);
		}

		// Reserved - Capacity allocated from the pool
		if (containsAttribute(RESERVED_KEY, attributes))
		{
			framework::Attribute attrReservedType(getCapacityAllocatedFromPool(), false);
			pInstance->setAttribute(RESERVED_KEY, attrReservedType, attributes);
		}

		// TotalVolatileCapacity - Bytes of NVM capacity currently allocated for use as volatile memory
		if (containsAttribute(VOLATILECAPACITY_KEY, attributes))
		{
			framework::Attribute attrVolCapacity(capacities.volatile_capacity, false);
			pInstance->setAttribute(VOLATILECAPACITY_KEY, attrVolCapacity, attributes);
		}

		// TotalPMCapableCapacity - Bytes of NVM capacity currently allocated for use as persistent memory
		if (containsAttribute(PERSISTENTCAPACITY_KEY, attributes))
		{
			framework::Attribute attrPmCapacity(capacities.persistent_capacity, false);
			pInstance->setAttribute(PERSISTENTCAPACITY_KEY, attrPmCapacity, attributes);
		}

		// TotalBlockCapableCapacity - Bytes of NVM capacity allocated as PM that can be accessed using a block aperture
		if (containsAttribute(BLOCKCAPACITY_KEY, attributes))
		{
			framework::Attribute attrBlockCapacity(capacities.block_capacity, false);
			pInstance->setAttribute(BLOCKCAPACITY_KEY, attrBlockCapacity, attributes);
		}

		// TotalUnconfiguredCapacity
		if (containsAttribute(UNCONFIGUREDCAPACITY_KEY, attributes))
		{
			framework::Attribute attrUnconfiguredCapacity(capacities.unconfigured_capacity, false);
			pInstance->setAttribute(UNCONFIGUREDCAPACITY_KEY, attrUnconfiguredCapacity, attributes);
		}

		// TotalInaccessibleCapacity
		if (containsAttribute(INACCESSIBLECAPACITY_KEY, attributes))
		{
			framework::Attribute attrInaccessibleCapacity(capacities.inaccessible_capacity, false);
			pInstance->setAttribute(INACCESSIBLECAPACITY_KEY, attrInaccessibleCapacity, attributes);
		}

		// TotalReservedCapacity
		if (containsAttribute(RESERVEDCAPACITY_KEY, attributes))
		{
			framework::Attribute attrReservedCapacity(capacities.reserved_capacity, false);
			pInstance->setAttribute(RESERVEDCAPACITY_KEY, attrReservedCapacity, attributes);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pInstance;
		throw;
	}

	return pInstance;
}

/*
 * Return the object paths for the BaseServer class.  Should only
 * be one server.
 */
wbem::framework::instance_names_t* wbem::mem_config::MemoryResourcesFactory::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		framework::attributes_t keys;

		// InstanceID = host name + NVM Pool
		std::string hostName = wbem::server::getHostName();
		std::string InstanceIdStr = hostName + MEMORYRESOURCES_INSTANCEID;
		framework::Attribute attrInstanceID(InstanceIdStr, true);
		keys.insert(std::pair<std::string, framework::Attribute>(
				INSTANCEID_KEY, attrInstanceID));

		// generate the ObjectPath for the instance (only one per server)
		framework::ObjectPath path(hostName, NVM_NAMESPACE,
				MEMORYRESOURCES_CREATIONCLASSNAME, keys);
		pNames->push_back(path);
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}

	return pNames;
}

/*
 * Helper function to convert block size to allocation units
 */
std::string wbem::mem_config::MemoryResourcesFactory::allocationUnitsToStr(
		const NVM_UINT32 &blockSize)
{
	std::stringstream sizeStr;
	sizeStr << "bytes*" << blockSize;
	return sizeStr.str();
}

/*
 * Helper function to get get capacity allocated from pool
 */
NVM_UINT64 wbem::mem_config::MemoryResourcesFactory::getCapacityAllocatedFromPool()
{
	NVM_UINT64 reservedCapacity = 0;

	std::vector<struct pool> pools = PoolViewFactory::getPoolList(false);
	for (std::vector<struct pool>::const_iterator iter = pools.begin();
			iter != pools.end(); iter++)
	{
		struct pool pool = (*iter);
		reservedCapacity += pool.capacity;
	}

	return reservedCapacity;
}
