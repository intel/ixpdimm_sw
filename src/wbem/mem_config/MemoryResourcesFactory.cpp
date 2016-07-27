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
 * This file contains the provider for the MemoryResources instances which
 * represents system NVM capacity in total regardless of how or whether
 * it is accessible to the host operating system.
 */

#include <nvm_management.h>

#include <LogEnterExit.h>

#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include "MemoryResourcesFactory.h"
#include <server/BaseServerFactory.h>
#include <sstream>
#include <mem_config/PoolViewFactory.h>
#include <exception/NvmExceptionLibError.h>
#include <core/exceptions/LibraryException.h>
#include <core/exceptions/NoMemoryException.h>
#include <NvmStrings.h>

#include "framework_interface/FrameworkExtensions.h"

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
	attributes.push_back(wbem::MEMORYCAPACITY_KEY);
	attributes.push_back(wbem::APP_DIRECT_CAPACITY_KEY);
	attributes.push_back(wbem::STORAGECAPACITY_KEY);
	attributes.push_back(wbem::UNCONFIGUREDCAPACITY_KEY);
	attributes.push_back(wbem::INACCESSIBLECAPACITY_KEY);
	attributes.push_back(wbem::RESERVEDCAPACITY_KEY);
}

void wbem::mem_config::MemoryResourcesFactory::toInstance(core::system::SystemMemoryResources &memoryResourcesInfo,
		wbem::framework::Instance &instance, wbem::framework::attribute_names_t attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	ADD_ATTRIBUTE(instance, attributes, ELEMENTNAME_KEY, framework::STR, MEMORYRESOURCES_ELEMENTNAME);
	ADD_ATTRIBUTE(instance, attributes, PRIMORDIAL_KEY, framework::BOOLEAN, true);
	ADD_ATTRIBUTE(instance, attributes, POOLID_KEY, framework::STR, MEMORYRESOURCES_POOLID);
	ADD_ATTRIBUTE(instance, attributes, RESERVED_KEY, framework::UINT64, getCapacityAllocatedFromPool());
	ADD_ATTRIBUTE(instance, attributes, RESOURCETYPE_KEY, framework::UINT16, (NVM_UINT16)MEMORYRESOURCES_RESOURCETYPE_VAL);
	ADD_ATTRIBUTE(instance, attributes, ALLOCATIONUNITS_KEY, framework::STR, MEMORYRESOURCES_ALLOCATIONUNITS_VAL);
	ADD_ATTRIBUTE(instance, attributes, CAPACITY_KEY, framework::UINT64, memoryResourcesInfo.getTotalCapacity());
	ADD_ATTRIBUTE(instance, attributes, MEMORYCAPACITY_KEY, framework::UINT64, memoryResourcesInfo.getTotalMemoryCapacity());
	ADD_ATTRIBUTE(instance, attributes, APP_DIRECT_CAPACITY_KEY, framework::UINT64, memoryResourcesInfo.getTotalAppDirectCapacity());
	ADD_ATTRIBUTE(instance, attributes, STORAGECAPACITY_KEY, framework::UINT64, memoryResourcesInfo.getTotalStorageCapacity());
	ADD_ATTRIBUTE(instance, attributes, UNCONFIGUREDCAPACITY_KEY, framework::UINT64, memoryResourcesInfo.getTotalUnconfiguredCapacity());
	ADD_ATTRIBUTE(instance, attributes, INACCESSIBLECAPACITY_KEY, framework::UINT64, memoryResourcesInfo.getTotalInaccessibleCapacity());
	ADD_ATTRIBUTE(instance, attributes, RESERVEDCAPACITY_KEY, framework::UINT64, memoryResourcesInfo.getTotalReservedCapacity());
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
	framework::Instance *pResult = new framework::Instance(path);
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

		core::system::SystemService &service = core::system::SystemService::getService();
		core::Result<core::system::SystemMemoryResources> r = service.getMemoryResources();

		toInstance(r.getValue(), *pResult, attributes);
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pResult;
		throw;
	}
	catch (core::LibraryException&e)
	{
		delete pResult;
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}
	catch (core::NoMemoryException)
	{
		delete pResult;
		throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__, "Could not allocate memory");
	}

	return pResult;
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
 * Helper function to get get capacity allocated from pool
 */
NVM_UINT64 wbem::mem_config::MemoryResourcesFactory::getCapacityAllocatedFromPool()
{
	NVM_UINT64 reservedCapacity = 0;

	std::vector<struct pool> pools = PoolViewFactory::getPoolList(false);
	for (std::vector<struct pool>::const_iterator iter = pools.begin();
			iter != pools.end(); iter++)
	{
		reservedCapacity += iter->capacity;
	}

	return reservedCapacity;
}
