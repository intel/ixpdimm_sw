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
 * This file contains the provider for the RawMemory instances
 * which model the capacity of a given DIMM.
 */

#include <string.h>
#include <uid/uid.h>
#include <nvm_management.h>
#include <LogEnterExit.h>
#include <server/BaseServerFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include "RawMemoryFactory.h"

#include <exception/NvmExceptionLibError.h>
#include "MemoryControllerFactory.h"

wbem::memory::RawMemoryFactory::RawMemoryFactory()
	throw (wbem::framework::Exception)
{
}

wbem::memory::RawMemoryFactory::~RawMemoryFactory()
{
}

void wbem::memory::RawMemoryFactory::populateAttributeList(
	framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(SYSTEMNAME_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(DEVICEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(BLOCKSIZE_KEY);
	attributes.push_back(NUMBEROFBLOCKS_KEY);
	attributes.push_back(OPERATIONALSTATUS_KEY);
	attributes.push_back(HEALTHSTATE_KEY);
	attributes.push_back(MEMORYCONTROLLERID_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::memory::RawMemoryFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);

		path.checkKey(CREATIONCLASSNAME_KEY, RAWMEMORY_CREATIONCLASSNAME);
		path.checkKey(SYSTEMCREATIONCLASSNAME_KEY, server::BASESERVER_CREATIONCLASSNAME);
		path.checkKey(SYSTEMNAME_KEY, server::getHostName());

		// extract the UID from the object path
		framework::Attribute attrDeviceID = path.getKeyValue(DEVICEID_KEY);
		NVM_UID dimmUid;
		uid_copy((char *) attrDeviceID.stringValue().c_str(), dimmUid);

		// get dimm discovery info
		struct device_discovery dimmDiscovery;
		int rc = nvm_get_device_discovery(dimmUid, &dimmDiscovery);

		if (rc != NVM_SUCCESS)
		{
			// couldn't retrieve the dimm info
			throw exception::NvmExceptionLibError(rc);
		}

		// Element Name = RAWMEMORY_ELEMENTNAME_prefix + UID
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			NVM_UID uidStr;
			uid_copy(dimmDiscovery.uid, uidStr);
			framework::Attribute attrElementName(
					RAWMEMORY_ELEMENTNAME_prefix + std::string(uidStr), false);
			pInstance->setAttribute(ELEMENTNAME_KEY, attrElementName, attributes);
		}

		// BlockSize = 1 (block concept is not valid)
		if (containsAttribute(BLOCKSIZE_KEY, attributes))
		{
			framework::Attribute attrBlockSize((NVM_UINT64)1, false);
			pInstance->setAttribute(BLOCKSIZE_KEY, attrBlockSize, attributes);
		}

		// NumberOfBlocks = DIMM raw capacity in bytes
		if (containsAttribute(NUMBEROFBLOCKS_KEY, attributes))
		{
			framework::Attribute attrNumberOfBlocks(dimmDiscovery.capacity, false);
			pInstance->setAttribute(NUMBEROFBLOCKS_KEY, attrNumberOfBlocks, attributes);
		}

		// get dimm details
		struct device_details dimmDetails;
		memset(&dimmDetails, 0, sizeof(dimmDetails));
		int getDeviceDetailsReturnCode = nvm_get_device_details(dimmDiscovery.uid, &dimmDetails);

		// OperationalStatus
		if (containsAttribute(OPERATIONALSTATUS_KEY, attributes))
		{
			framework::UINT16_LIST operationalStatusList;
			if (NVM_SUCCESS == getDeviceDetailsReturnCode &&
					dimmDiscovery.manageability == MANAGEMENT_VALIDCONFIG)
			{
				if (dimmDetails.status.is_missing)
				{
					operationalStatusList.push_back(OPSTATUS_NOCONTACT);
				}
				else if (dimmDetails.status.is_new)
				{
					operationalStatusList.push_back(OPSTATUS_DORMANT);
				}
				else if ((SENSOR_CRITICAL == dimmDetails.sensors[SENSOR_WEARLEVEL].current_state) ||
						(SENSOR_CRITICAL == dimmDetails.sensors[SENSOR_SPARECAPACITY].current_state) ||
						(SENSOR_CRITICAL == dimmDetails.sensors[SENSOR_MEDIA_TEMPERATURE].current_state) ||
						(SENSOR_CRITICAL == dimmDetails.sensors[SENSOR_CONTROLLER_TEMPERATURE].current_state))
				{
					operationalStatusList.push_back(OPSTATUS_PREDICTIVEFAILURE);
				}
				else
				{
					operationalStatusList.push_back(OPSTATUS_OK);
				}
			}
			else
			{
				operationalStatusList.push_back(OPSTATUS_UNKNOWN);
			}

			framework::Attribute attrOperationalStatus(operationalStatusList, false);
			pInstance->setAttribute(OPERATIONALSTATUS_KEY, attrOperationalStatus, attributes);
		}

		// HealthState
		if (containsAttribute(HEALTHSTATE_KEY, attributes))
		{
			std::string healthStateStr;
			NVM_UINT16 healthState = 0; // unknown
			if (NVM_SUCCESS == getDeviceDetailsReturnCode)
			{
				switch (dimmDetails.status.health)
				{
					case DEVICE_HEALTH_NORMAL:
						healthStateStr = "Healthy";
						healthState = HEALTHSTATE_OK;
						break;
					case DEVICE_HEALTH_NONCRITICAL:
						healthStateStr = "Degraded/Warning";
						healthState = HEALTHSTATE_DEGRATEDWARNING;
						break;
					case DEVICE_HEALTH_CRITICAL:
					case DEVICE_HEALTH_FATAL:
						healthStateStr = "Critical Failure";
						healthState = HEALTHSTATE_CRITICALFAILURE;
						break;
					default:
						break;
				}
			}

			framework::Attribute attrHealthState(healthState, healthStateStr, false);
			pInstance->setAttribute(HEALTHSTATE_KEY, attrHealthState, attributes);
		}

		if (containsAttribute(MEMORYCONTROLLERID_KEY, attributes))
		{
			std::string memoryControllerId = MemoryControllerFactory::generateUniqueMemoryControllerID(&dimmDiscovery);
			framework::Attribute attrMemoryControllerId(memoryControllerId, false);
			pInstance->setAttribute(MEMORYCONTROLLERID_KEY, attrMemoryControllerId, attributes);
		}
	}
	catch (framework::Exception) // clean up and re-throw
	{
		delete pInstance;
		throw;
	}
	return pInstance;
}

/*
 * Return an object path for each NVDIMM in the system
 */
wbem::framework::instance_names_t* wbem::memory::RawMemoryFactory::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		std::string hostName = wbem::server::getHostName();
		physical_asset::devices_t devices = physical_asset::NVDIMMFactory::getAllDevices();

		physical_asset::devices_t::const_iterator iter = devices.begin();

		// create an object path for each dimm
		for (; iter != devices.end(); iter++)
		{
			framework::attributes_t keys;

			// SystemCreationClassName = server::BASESERVER_CREATIONCLASSNAME
			framework::Attribute attrSysCCName(server::BASESERVER_CREATIONCLASSNAME, true);
			keys.insert(std::pair<std::string, framework::Attribute>(
					SYSTEMCREATIONCLASSNAME_KEY, attrSysCCName));

			// SystemName = (host) server name
			framework::Attribute attrSysName(hostName, true);
			keys.insert(std::pair<std::string, framework::Attribute>(
					SYSTEMNAME_KEY, attrSysName));

			// Creation Class Name = topology::RAWMEMORY_CREATIONCLASSNAME
			framework::Attribute attrCCName(RAWMEMORY_CREATIONCLASSNAME, true);
			keys.insert(std::pair<std::string, framework::Attribute>(
					CREATIONCLASSNAME_KEY, attrCCName));

			// DeviceID = DIMM UID
			NVM_UID uidStr;
			uid_copy((*iter).uid, uidStr);
			framework::Attribute attrDeviceID(uidStr, true);
			keys.insert(std::pair<std::string, framework::Attribute>(
					DEVICEID_KEY, attrDeviceID));

			// generate the ObjectPath for the instance
			framework::ObjectPath path(hostName, NVM_NAMESPACE,
					RAWMEMORY_CREATIONCLASSNAME, keys);
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
