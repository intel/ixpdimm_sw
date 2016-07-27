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
 * This file contains the provider for the MemoryController instances that
 * model the memory controller associated with a set of NVM DIMMs.
 */

#include <algorithm>
#include <LogEnterExit.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <server/BaseServerFactory.h>
#include "MemoryControllerFactory.h"
#include "SystemProcessorFactory.h"
#include <framework_interface/NvmAssociationFactory.h>
#include <libinvm-cim/StringUtil.h>
#include <sstream>
#include <exception/NvmExceptionLibError.h>


wbem::memory::MemoryControllerFactory::MemoryControllerFactory()
	throw (framework::Exception)
{
}

wbem::memory::MemoryControllerFactory::~MemoryControllerFactory()
{
}

void wbem::memory::MemoryControllerFactory::populateAttributeList(
	framework::attribute_names_t &attributes)
	throw (framework::Exception)
{
	// add key attributes
	attributes.push_back(SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(SYSTEMNAME_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(DEVICEID_KEY);

	// add non-key attributes
	attributes.push_back(PROTOCOLSUPPORTED_KEY);
	attributes.push_back(PROCESSORAFFINITY_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::memory::MemoryControllerFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);

		path.checkKey(CREATIONCLASSNAME_KEY, MEMORYCONTROLLER_CREATIONCLASSNAME);
		path.checkKey(SYSTEMCREATIONCLASSNAME_KEY, server::BASESERVER_CREATIONCLASSNAME);
		path.checkKey(SYSTEMNAME_KEY, server::getHostName());

		// extract the memory controller id from the object path
		framework::Attribute devIdAttr = path.getKeyValue(DEVICEID_KEY);
		COMMON_LOG_DEBUG_F("DeviceID: %s", devIdAttr.asStr().c_str());

		int rc = nvm_get_device_count();
		if (rc < NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(rc);
		}
		else if (rc == 0)
		{
			throw framework::Exception(
					"Could not find any NVDIMMs connected to Memory Controller");
		}

		// get the device_discovery information for all of the dimms
		struct device_discovery dimms[rc];
		if ((rc = nvm_get_devices(dimms, rc)) < NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(rc);
		}
		else if (rc == 0)
		{
			throw framework::Exception(
					"Could not find any NVDIMMs connected to Memory Controller");
		}

		// initialize indicator
		int instance_found = 0;
		// find the set of unique memory controller ids used across all DIMMs
		for (int i = 0; i < rc; i++)
		{
			// compare the current DIMM's mem controller to the one that we are searching for
			instance_found = (devIdAttr.stringValue().compare(
					generateUniqueMemoryControllerID(&(dimms[i]))) == 0);
			if (instance_found)
			{
				// if found, update pInstance
				MemoryControllerFactory::addNonKeyAttributesToInstance(
						pInstance, &attributes, &(dimms[i]));

				// break the loop since we found what we were looking for
				break;
			}
		}

		// handle failures
		if (!instance_found)
		{
			COMMON_LOG_ERROR_F("Device ID Not Found: %s", devIdAttr.stringValue().c_str());
			throw framework::ExceptionBadParameter(DEVICEID_KEY.c_str());
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
 * Return an object path for each MemoryController in the system
 */
wbem::framework::instance_names_t* wbem::memory::MemoryControllerFactory::getInstanceNames()
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		// use our helper function to take care of business
		int rc = getInstancesHelperLoop(pNames, NULL, NULL);
		if (rc < NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(rc);
		}
	}
	catch (framework::Exception) // clean up and re-throw
	{
		delete pNames;
		throw;
	}

	return pNames;
}

/*
 * Overload of standard CIM method to retrieve a list of instances in this factory.
 */
wbem::framework::instances_t* wbem::memory::MemoryControllerFactory::getInstances(
	framework::attribute_names_t &attributes)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instances_t *pInstList = new framework::instances_t();
	try
	{
		checkAttributes(attributes);

		// We could start by running getInstanceNames(..), however that would be incredibly
		// inefficient because the subsequent call(s) to getInstance(..) would be doing the same work
		// N times.  Instead, we pass attributes and pInstList to the helper function, and let it take
		// care of the rest.
		int rc = getInstancesHelperLoop(NULL, pInstList, &attributes);
		if (rc < NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(rc);
		}
	}
	catch (framework::Exception) // clean up and re-throw
	{
		delete pInstList;
		throw;
	}

	return pInstList;
}

/*
 * ******************************************************************************
 * Helper Functions
 * ******************************************************************************
 */

/*
 * Helper function to generate a unique memory controller ID from its core components
 */
std::string wbem::memory::MemoryControllerFactory::generateUniqueMemoryControllerID(
		struct device_discovery *pDiscovery)
{
	// note that the Memory Controller ID is only unique w.r.t a given processor ID
	// which means that to guarantee uniqueness, we must incorporate the processor
	// socket ID into the this key attribute
	size_t unique_id_str_len = 100;
	char unique_memory_controller_id_cstr[unique_id_str_len];
	snprintf(unique_memory_controller_id_cstr, unique_id_str_len, "CPU %u Memory Controller ID %u",
			pDiscovery->socket_id, pDiscovery->memory_controller_id);

	return std::string(unique_memory_controller_id_cstr);
}

/*
 * Helper function that builds a unique object path for an instance defined by the input arguments
 */
wbem::framework::ObjectPath wbem::memory::MemoryControllerFactory::getInstanceObjectPath(
		std::string &hostServerName, std::string &memory_controller_id)
{
	framework::attributes_t keys;

	// SystemCreationClassName = server::BASESERVER_CREATIONCLASSNAME
	framework::Attribute attrSysCCName(server::BASESERVER_CREATIONCLASSNAME, true);
	keys.insert(std::pair<std::string, framework::Attribute>(
			SYSTEMCREATIONCLASSNAME_KEY, attrSysCCName));

	// SystemName = (host) server name
	framework::Attribute attrSysName(hostServerName, true);
	keys.insert(std::pair<std::string, framework::Attribute>(
			SYSTEMNAME_KEY, attrSysName));

	// Creation Class Name = topology::MEMORYCONTROLLER_CREATIONCLASSNAME
	framework::Attribute attrCCName(MEMORYCONTROLLER_CREATIONCLASSNAME, true);
	keys.insert(std::pair<std::string, framework::Attribute>(
			CREATIONCLASSNAME_KEY, attrCCName));

	// DeviceID = Unique Memory Controller ID
	framework::Attribute attrDeviceID(memory_controller_id, true);
	keys.insert(std::pair<std::string, framework::Attribute>(
			DEVICEID_KEY, attrDeviceID));

	// generate the ObjectPath for the instance
	return framework::ObjectPath(hostServerName, NVM_NAMESPACE,
			MEMORYCONTROLLER_CREATIONCLASSNAME, keys);
}

/*
 * Helper function that adds the non-key attributes to an already existing instance
 */
void wbem::memory::MemoryControllerFactory::addNonKeyAttributesToInstance(
		framework::Instance *pInstance,
		framework::attribute_names_t *pAttrNames,
		struct device_discovery *pDiscovery)
{
	// Affinity = processor socket id (same as proximity domain)
	if (containsAttribute(PROCESSORAFFINITY_KEY, *pAttrNames))
	{
		std::stringstream socketIdStr;
		socketIdStr << pDiscovery->socket_id;

		framework::Attribute attrSocketID(socketIdStr.str(), false);
		pInstance->setAttribute(PROCESSORAFFINITY_KEY, attrSocketID, *pAttrNames);
	}
	// ProtocolSupported = "DDR4"
	if (containsAttribute(PROTOCOLSUPPORTED_KEY, *pAttrNames))
	{
		framework::Attribute attrProtocolSupported((NVM_UINT16)50, (std::string)"DDR4", false);
		pInstance->setAttribute(PROTOCOLSUPPORTED_KEY, attrProtocolSupported, *pAttrNames);
	}
}

/*
 * This is a helper function that allows us to gather and iterate through all of the
 * dimm instances in such a way as to enable us to gather the data for the memory controllers.
 * this function is used to help getInstances(..), getInstanceNames(..), and getInstance(..)
 * which all require iterating through the entire list due to the nature of this class
 */
int wbem::memory::MemoryControllerFactory::getInstancesHelperLoop(
	framework::instance_names_t *pNames,
	framework::instances_t *pInstList,
	framework::attribute_names_t *pAttrNames)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc = NVM_ERR_UNKNOWN;

	std::string hostServerName = wbem::server::getHostName();

	// only bother to continue if there are devices to associate with a memory controller
	if ((rc = nvm_get_device_count()) > 0)
	{
		// get the device_discovery information for all of the dimms
		struct device_discovery dimms[rc];
		if ((rc = nvm_get_devices(dimms, rc)) > 0)
		{
			// store the number of dimms recovered
			int numDimms = rc;

			// init a container to aid a simple find routine
			std::vector<std::string> unique_mem_crtlr_ids;

			// find the set of unique memory controller ids used across all DIMMs
			for (int i = 0; i < numDimms; i++)
			{
				// generate the unique memory controller id for this particular DIMM
				std::string cur_mem_crtlr_id = generateUniqueMemoryControllerID(&(dimms[i]));

				// if it is not in the list, add it.  Otherwise, skip
				if (unique_mem_crtlr_ids.end() ==
						find(unique_mem_crtlr_ids.begin(),
								unique_mem_crtlr_ids.end(), cur_mem_crtlr_id))
				{
					// build the object path for the new instance
					framework::ObjectPath newObjectPath = getInstanceObjectPath(
							hostServerName, cur_mem_crtlr_id);

					// support getInstances(..)
					if (pInstList != NULL)
					{
						// create a new instance for the current object path
						framework::Instance *pNewCurrentInstance =
								new framework::Instance(newObjectPath);

						// add the non-key attributes to complete the instance
						addNonKeyAttributesToInstance(pNewCurrentInstance, pAttrNames,
								&(dimms[i]));

						// add the instance to the instance list
						pInstList->push_back(*pNewCurrentInstance);
						delete pNewCurrentInstance;
					}

					// support getInstanceNames(..)
					if (pNames != NULL)
					{
						pNames->push_back(newObjectPath);
					}

					// add to our quick-search list
					unique_mem_crtlr_ids.push_back(cur_mem_crtlr_id);
				}
			}
		}
	}
	else // nvm_get_device_count returned 0 DIMMs
	{
		COMMON_LOG_DEBUG("No Intel NVDIMMs found.");
	}

	return rc;
}

/*
 * determine if instances are associated based on the ConcreteDependency association
 */
bool wbem::memory::MemoryControllerFactory::isAssociated(const std::string &associationClass,
		framework::Instance* pAntInstance,
		framework::Instance* pDepInstance)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool result = false;

	if (associationClass == framework_interface::ASSOCIATION_CLASS_CONCRETEDEPENDENCY)
	{
		framework::Attribute antAttribute;
		framework::Attribute depAttribute;
		if (pAntInstance->getAttribute(PROCESSORAFFINITY_KEY, antAttribute) == framework::SUCCESS &&
				pDepInstance->getAttribute(DEVICEID_KEY, depAttribute) == framework::SUCCESS)
		{
			std::string depString = framework::StringUtil::removeString(depAttribute.asStr(), SYSTEMPROCESSOR_DEVICEID_PREFIX);
			std::string antString = antAttribute.asStr();

			int depSocketId = framework::StringUtil::stringToUint64(depString);
			int antSocketId = framework::StringUtil::stringToUint64(antString);

			result = (antSocketId == depSocketId);
		}
	}
	else
	{
		COMMON_LOG_WARN_F("Cannot calculate if instances are an association "
				"based on association class: %s", associationClass.c_str());
	}

	return result;


}
