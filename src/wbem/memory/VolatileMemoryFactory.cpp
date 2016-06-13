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
 * This file contains the provider for the VolatileMemory instances
 * which represent all NVM-DIMM Memory Mode capacity in the system.
 */

#include "VolatileMemoryFactory.h"

#include <LogEnterExit.h>
#include <nvm_management.h>
#include <uid/uid.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <framework_interface/NvmAssociationFactory.h>
#include <memory/RawMemoryFactory.h>
#include <mem_config/MemoryAllocationSettingsFactory.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>
#include <core/device/DeviceHelper.h>

wbem::memory::VolatileMemoryFactory::VolatileMemoryFactory()
{
}

wbem::memory::VolatileMemoryFactory::~VolatileMemoryFactory()
{
}

void wbem::memory::VolatileMemoryFactory::populateAttributeList(
	framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(wbem::SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(wbem::SYSTEMNAME_KEY);
	attributes.push_back(wbem::CREATIONCLASSNAME_KEY);
	attributes.push_back(wbem::DEVICEID_KEY);

	// add non-key attributes
	attributes.push_back(wbem::NUMBEROFBLOCKS_KEY);
	attributes.push_back(wbem::BLOCKSIZE_KEY);
	attributes.push_back(wbem::VOLATILE_KEY);
	attributes.push_back(wbem::REPLICATION_KEY);
	attributes.push_back(wbem::PROCESSORAFFINITY_KEY);
	attributes.push_back(wbem::ACCESSGRANULARITY_KEY);
	attributes.push_back(wbem::HEALTHSTATE_KEY);
	attributes.push_back(wbem::OPERATIONALSTATUS_KEY);
	attributes.push_back(wbem::PRIMORDIAL_KEY);
}

/*
 * Get volatile memory capacity for the whole system
 */
wbem::framework::UINT64 wbem::memory::VolatileMemoryFactory::getMemoryCapacity()
	throw (wbem::framework::Exception)
{
	device_capacities capacities;
	int rc = nvm_get_nvm_capacities(&capacities);
	if (rc != NVM_SUCCESS)
	{
		// couldn't retrieve the capacity info
		throw exception::NvmExceptionLibError(rc);
	}

	return capacities.memory_capacity;
}

/*
 * helper function to determine health state incrementally by comparing the current health state
 * of the whole system to that of the current dimm
 */
void wbem::memory::VolatileMemoryFactory::updateHealthStateIncrementally(NVM_UINT16 &currentHealthState,
	const NVM_UINT16 dimmHealthState) throw (wbem::framework::Exception)
{
	// nothing more to do if the health state is already critical!
	if (currentHealthState != VOLATILEMEMORY_HEALTHSTATE_CRITICAL_FAILURE)
	{
		if ((dimmHealthState == DEVICE_HEALTH_CRITICAL) ||
			(dimmHealthState == DEVICE_HEALTH_FATAL))
		{
			currentHealthState = VOLATILEMEMORY_HEALTHSTATE_CRITICAL_FAILURE;
		}
		else if (dimmHealthState == DEVICE_HEALTH_NONCRITICAL)
		{
			currentHealthState = VOLATILEMEMORY_HEALTHSTATE_DEGRADED;
		}
		// if the health state of the current dimm is normal, health state of the whole
		// system is set to unknown only if the current health state is also unknown
		// (i.e., current health state is already set correctly)
		else if (dimmHealthState == DEVICE_HEALTH_NORMAL)
		{
			 /* Do Nothing */
		}
		// if the health state of the current dimm is unknown, health state of the whole
		// system is set to unknown only if the current health state is ok
		else
		{
			if (dimmHealthState != DEVICE_HEALTH_UNKNOWN) // unexpected health - unknown
			{
				COMMON_LOG_WARN_F("Unexpected health state: %d", dimmHealthState);
			}
			if (currentHealthState == VOLATILEMEMORY_HEALTHSTATE_OK)
			{
				currentHealthState = VOLATILEMEMORY_HEALTHSTATE_UNKNOWN;
			}
		}
	}
}

/*
 * Get health state of the whole system
 */
wbem::framework::UINT16 wbem::memory::VolatileMemoryFactory::getHealthState()
throw (wbem::framework::Exception)
{
	NVM_UINT16 currentHealthState = VOLATILEMEMORY_HEALTHSTATE_OK;
	int dev_count = nvm_get_device_count();
	if (dev_count == 0)
	{
		COMMON_LOG_WARN("Couldn't get any devices");
	}
	else if (dev_count > 0)
	{
		// get device_discovery information of all dimms
		struct device_discovery dimms[dev_count];
		dev_count = nvm_get_devices(dimms, dev_count);
		if (dev_count > 0)
		{
			for (int device_index = 0; device_index < dev_count; device_index++)
			{
				if (dimms[device_index].manageability == MANAGEMENT_VALIDCONFIG)
				{
					// ignore dimm if it does not have any Memory Mode capacity
					NVM_UID uidStr;
					uid_copy(dimms[device_index].uid, uidStr);
					NVM_UINT64 volatileCapacity = getDimmMemoryCapacity(uidStr);
					if (volatileCapacity > 0)
					{
						struct device_status status;
						int rc = nvm_get_device_status(dimms[device_index].uid, &status);
						if (rc != NVM_SUCCESS)
						{
							// couldn't retrieve device status
							currentHealthState = VOLATILEMEMORY_HEALTHSTATE_UNKNOWN;
						}
						else
						{
							updateHealthStateIncrementally(currentHealthState, status.health);
						}
					}
				}
			}
		}
		else
		{
			COMMON_LOG_WARN("Couldn't get any devices");
		}
	}
	return currentHealthState;
}

/*
 * Get operational status for the whole system
 */
wbem::framework::UINT16 wbem::memory::VolatileMemoryFactory::getOperationalStatus(NVM_UINT16 currentHealthState)
throw (wbem::framework::Exception)
{
	NVM_UINT16 currentOperationalStatus = VOLATILEMEMORY_OPERATIONALSTATUS_UNKNOWN;
	switch (currentHealthState)
	{
	case VOLATILEMEMORY_HEALTHSTATE_UNKNOWN:
		currentOperationalStatus = VOLATILEMEMORY_OPERATIONALSTATUS_UNKNOWN;
		break;
	case VOLATILEMEMORY_HEALTHSTATE_OK:
		currentOperationalStatus = VOLATILEMEMORY_OPERATIONALSTATUS_OK;
		break;
	case VOLATILEMEMORY_HEALTHSTATE_DEGRADED:
		currentOperationalStatus = VOLATILEMEMORY_OPERATIONALSTATUS_DEGRADED;
		break;
	case VOLATILEMEMORY_HEALTHSTATE_CRITICAL_FAILURE:
		currentOperationalStatus = VOLATILEMEMORY_OPERATIONALSTATUS_SUPPORTINGENTITYINERROR;
		break;
	default:
		break;
	}
	return currentOperationalStatus;
}

/*
 * Get volatile memory capacity for a single DIMM
 */
wbem::framework::UINT64 wbem::memory::VolatileMemoryFactory::getDimmMemoryCapacity(std::string uidStr)
	throw (wbem::framework::Exception)
{
	NVM_UINT64 volatileCapacity = 0;
	if (!core::device::isUidValid(uidStr))
	{
		throw framework::ExceptionBadParameter("UID");
	}

	NVM_UID uid;
	uid_copy(uidStr.c_str(), uid);

	struct device_details details;
	int rc = nvm_get_device_details(uid, &details);
	if (rc != NVM_SUCCESS && rc != NVM_ERR_NOTMANAGEABLE)
	{
		// couldn't retrieve the capacity info
		throw exception::NvmExceptionLibError(rc);
	}
	else if (details.discovery.manageability == MANAGEMENT_VALIDCONFIG)
	{
		volatileCapacity = details.capacities.memory_capacity;
	}
	return volatileCapacity;
}

/*
 * Determine if the VolatileMemory has a complex association.
 */
bool wbem::memory::VolatileMemoryFactory::isAssociated(
		const std::string &associationClass,
		framework::Instance* pAntInstance,
		framework::Instance* pDepInstance)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool result = false;

	// Association: BasedOn
	if (associationClass == wbem::framework_interface::ASSOCIATION_CLASS_BASEDON)
	{
		// Antecedent: RawMemory
		// Dependent: VolatileMemory
		if ((pDepInstance->getClass() == wbem::memory::VOLATILEMEMORY_CREATIONCLASSNAME)
			&& (pAntInstance->getClass() == wbem::memory::RAWMEMORY_CREATIONCLASSNAME))
		{
			framework::Attribute uidAttr;
			if (pAntInstance->getAttribute(DEVICEID_KEY, uidAttr) == framework::SUCCESS)
			{
				try
				{
					// Look up DIMM by UID
					NVM_UINT64 memoryCapacity = getDimmMemoryCapacity(uidAttr.stringValue());

					// Does it have any Memory Mode capacity?
					if (memoryCapacity > 0)
					{
						result = true;
					}
				}
				catch (framework::Exception &e)
				{
					COMMON_LOG_ERROR_F("Error determining BasedOn association: %s", e.what());
				}
			}
			else
			{
				COMMON_LOG_WARN("Couldn't get DeviceID attribute from RawMemory instance");
			}
		}
		else // unrecognized instance classes
		{
			COMMON_LOG_WARN("Incorrect antecedent and dependent class instances.");
		}
	}
	// Association: ElementSettingData
	else if (associationClass == wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTSETTINGDATA)
	{
		// Antecedent: VolatileMemory
		// Dependent: MemoryAllocationSettings
		if ((pAntInstance->getClass() == VOLATILEMEMORY_CREATIONCLASSNAME)
			&& (pDepInstance->getClass() == mem_config::MEMORYALLOCATIONSETTINGS_CREATIONCLASSNAME))
		{
			// this association is implemented in MemoryAllocationSettings
			result = true;
		}
		else // unrecognized instance classes
		{
			COMMON_LOG_WARN("Incorrect antecedent and dependent class instances.");
		}
	}
	else // unknown association class - the other class can decide whether they're associated
	{
		result = true;
	}

	return result;
}

/*
 * Verify the object path is valid for VolatileMemory instance
 */
void wbem::memory::VolatileMemoryFactory::validateObjectPath(framework::ObjectPath &path)
	throw (wbem::framework::Exception)
{
	// Inspect key attributes from object path
	// Note there's only one instance of VolatileMemory per system

	// SystemCreationClassName - BaseServer
	framework::Attribute sysClassName = path.getKeyValue(SYSTEMCREATIONCLASSNAME_KEY);
	if (sysClassName.stringValue() != VOLATILEMEMORY_SYSTEMCREATIONCLASSNAME)
	{
		throw framework::ExceptionBadParameter(SYSTEMCREATIONCLASSNAME_KEY.c_str());
	}

	// SystemName - host name
	framework::Attribute sysName = path.getKeyValue(SYSTEMNAME_KEY);
	std::string hostName = wbem::server::getHostName();
	if (sysName.stringValue() != hostName)
	{
		throw framework::ExceptionBadParameter(SYSTEMNAME_KEY.c_str());
	}

	// CreationClassName - VolatileMemory
	framework::Attribute className = path.getKeyValue(CREATIONCLASSNAME_KEY);
	if(className.stringValue() != VOLATILEMEMORY_CREATIONCLASSNAME)
	{
		throw framework::ExceptionBadParameter(CREATIONCLASSNAME_KEY.c_str());
	}

	// DeviceID - NVDIMM Volatile Capacity
	framework::Attribute devId = path.getKeyValue(DEVICEID_KEY);
	if(devId.stringValue() != VOLATILEMEMORY_DEVICEID)
	{
		throw framework::ExceptionBadParameter(DEVICEID_KEY.c_str());
	}
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::memory::VolatileMemoryFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	if (pInstance != NULL)
	{
		try
		{
			checkAttributes(attributes);
			validateObjectPath(path);

			// Fill out the non-key attributes

			// NumberOfBlocks - actually number of bytes
			if (containsAttribute(NUMBEROFBLOCKS_KEY, attributes))
			{
				NVM_UINT64 capacity = getMemoryCapacity(); // in bytes

				framework::Attribute attrNumBlocks(capacity, false);
				pInstance->setAttribute(NUMBEROFBLOCKS_KEY, attrNumBlocks, attributes);
			}

			// BlockSize - constant = 1 byte
			if (containsAttribute(BLOCKSIZE_KEY, attributes))
			{
				framework::Attribute attrBlockSize(VOLATILEMEMORY_BLOCKSIZE, false);
				pInstance->setAttribute(BLOCKSIZE_KEY, attrBlockSize, attributes);
			}

			// Volatile - true
			if(containsAttribute(VOLATILE_KEY, attributes))
			{
				framework::Attribute attrVolatile((framework::BOOLEAN)true, false);
				pInstance->setAttribute(VOLATILE_KEY, attrVolatile, attributes);
			}

			// Replication - "Unknown" Not settable for volatile memory
			if(containsAttribute(REPLICATION_KEY, attributes))
			{
				framework::Attribute attrReplication(VOLATILEMEMORY_REPLICATION_UNKNOWN_VAL, false);
				pInstance->setAttribute(REPLICATION_KEY, attrReplication, attributes);
			}

			// ProcessorAffinity - socket identifier
			if(containsAttribute(PROCESSORAFFINITY_KEY, attributes))
			{
				framework::Attribute attrProcessorAffinity(VOLATILEMEMORY_PROCESSORAFFINITY_NONE, false);
				pInstance->setAttribute(PROCESSORAFFINITY_KEY, attrProcessorAffinity, attributes);
			}

			// AccessGranularity - "Byte Addressable"
			if(containsAttribute(ACCESSGRANULARITY_KEY, attributes))
			{
				framework::Attribute attrAccessGranularity(VOLATILEMEMORY_ACCESSGRANULARITY_BYTE_ADDRESSABLE, false);
				pInstance->setAttribute(ACCESSGRANULARITY_KEY, attrAccessGranularity, attributes);
			}

			// HealthState
			NVM_UINT16 healthState = getHealthState();
			if(containsAttribute(HEALTHSTATE_KEY, attributes))
			{
				framework::Attribute attrHealthState(healthState, false);
				pInstance->setAttribute(HEALTHSTATE_KEY, attrHealthState, attributes);
			}

			// OperationalStatus
			if(containsAttribute(OPERATIONALSTATUS_KEY, attributes))
			{
				framework::UINT16_LIST opStatus;
				opStatus.push_back(getOperationalStatus(healthState));
				framework::Attribute attrOpStatus(opStatus, false);
				pInstance->setAttribute(OPERATIONALSTATUS_KEY, attrOpStatus, attributes);
			}

			// Primordial - false
			if(containsAttribute(PRIMORDIAL_KEY, attributes))
			{
				framework::Attribute attrPrimordial((framework::BOOLEAN)false, false);
				pInstance->setAttribute(PRIMORDIAL_KEY, attrPrimordial, attributes);
			}
		}
		catch (framework::Exception &) // clean up and re-throw
		{
			delete pInstance;
			throw;
		}
	}
	else
	{
		throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__, "Failed to allocate Instance");
	}

	return pInstance;
}

/*
 * Return the object path for the VolatileMemory instance.
 */
wbem::framework::instance_names_t* wbem::memory::VolatileMemoryFactory::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	if (pNames != NULL)
	{
		try
		{
			// If there is any Memory Mode capacity, there is one VolatileMemory instance.
			if(getMemoryCapacity() > 0)
			{
				framework::attributes_t keys;

				// SystemCreationClassName - class for our BaseServer
				framework::Attribute systemCreationClassName(VOLATILEMEMORY_SYSTEMCREATIONCLASSNAME, true);
				keys[SYSTEMCREATIONCLASSNAME_KEY] = systemCreationClassName;

				// SystemName - host name
				std::string hostName = wbem::server::getHostName();
				framework::Attribute systemName(hostName, true);
				keys[SYSTEMNAME_KEY] = systemName;

				// CreationClassName
				framework::Attribute creationClassName(VOLATILEMEMORY_CREATIONCLASSNAME, true);
				keys[CREATIONCLASSNAME_KEY] = creationClassName;

				// DeviceID - constant string
				framework::Attribute deviceId(VOLATILEMEMORY_DEVICEID, true);
				keys[DEVICEID_KEY] = deviceId;

				// generate the ObjectPath
				framework::ObjectPath path(hostName, NVM_NAMESPACE,
						VOLATILEMEMORY_CREATIONCLASSNAME, keys);
				pNames->push_back(path);
			}
		}
		catch (framework::Exception &) // clean up and re-throw
		{
			delete pNames;
			throw;
		}
	}
	else
	{
		throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__, "Failed to allocate instance_names_t");
	}

	return pNames;
}
