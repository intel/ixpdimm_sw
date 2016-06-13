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
 * This file contains the provider for the SystemProcessor instancees
 * that model the physical processor/socket associated with a set of NVM DIMMs.
 */

#include <string/s_str.h>
#include <nvm_management.h>

#include <LogEnterExit.h>

#include <libinvm-cim/ExceptionBadParameter.h>
#include <server/BaseServerFactory.h>
#include "SystemProcessorFactory.h"
#include <exception/NvmExceptionLibError.h>


wbem::memory::SystemProcessorFactory::SystemProcessorFactory()
	throw (wbem::framework::Exception)
{
}

wbem::memory::SystemProcessorFactory::~SystemProcessorFactory()
{
}

void wbem::memory::SystemProcessorFactory::populateAttributeList(
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
	attributes.push_back(FAMILY_KEY);
	attributes.push_back(OTHERFAMILYDESCRIPTION_KEY);
	attributes.push_back(STEPPING_KEY);
	attributes.push_back(NUMBEROFLOGICALPROCESSORS_KEY);
	attributes.push_back(TYPE_KEY);
	attributes.push_back(MODEL_KEY);
	attributes.push_back(MANUFACTURER_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::memory::SystemProcessorFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);

		// need to extract the NUMA node_id from the DeviceID key
		// Note that the expected format of the nodeIdAttribute is "CPU 00001"
		framework::Attribute nodeIdAttr = path.getKeyValue(DEVICEID_KEY);
		if (nodeIdAttr.stringValue().length() <= SYSTEMPROCESSOR_DEVICEID_PREFIX.length())
		{
			throw framework::ExceptionBadParameter(DEVICEID_KEY.c_str());
		}
		// loosely verify that we have received the correct prefix
		else if (s_strncmpi(nodeIdAttr.stringValue().c_str(), SYSTEMPROCESSOR_DEVICEID_PREFIX.c_str(),
				SYSTEMPROCESSOR_DEVICEID_PREFIX.length()) != 0)
		{
			throw framework::ExceptionBadParameter(DEVICEID_KEY.c_str());
		}
		// verify that the node_id value is a number
		else if (!isdigit(nodeIdAttr.stringValue().c_str()[SYSTEMPROCESSOR_DEVICEID_PREFIX.length()]))
		{
			throw framework::ExceptionBadParameter(DEVICEID_KEY.c_str());
		}
		// verify object path system creation classname
		else if (path.getKeyValue(SYSTEMCREATIONCLASSNAME_KEY).asStr() != server::BASESERVER_CREATIONCLASSNAME)
		{
			throw framework::ExceptionBadParameter(SYSTEMCREATIONCLASSNAME_KEY.c_str());
		}
		// verify object path system name
		else if (path.getKeyValue(SYSTEMNAME_KEY).asStr() != server::getHostName())
		{
			throw framework::ExceptionBadParameter(SYSTEMNAME_KEY.c_str());
		}
		// verify object path creation classname
		else if(path.getKeyValue(CREATIONCLASSNAME_KEY).asStr() != SYSTEMPROCESSOR_CREATIONCLASSNAME)
		{
			throw framework::ExceptionBadParameter(CREATIONCLASSNAME_KEY.c_str());
		}
		// get the node_id
		else
		{
			unsigned short node_id = 0;
			s_strtous(nodeIdAttr.stringValue().c_str(), nodeIdAttr.stringValue().length(),
					NULL, &node_id);

			struct socket node;
			int rc = nvm_get_socket(node_id, &node);
			if (rc != NVM_SUCCESS)
			{
				// couldn't retrieve struct for given node
				throw exception::NvmExceptionLibError(rc);
			}

			// init a buffer for string conversions
			size_t temp_cstr_len = 32;
			char temp_cstr[temp_cstr_len];

			// ElementName = brand (use brand index here instead of brand string)
			if (containsAttribute(ELEMENTNAME_KEY, attributes))
			{
				snprintf(temp_cstr, temp_cstr_len, "%hhu", node.brand);
				framework::Attribute elementNameAttr(std::string(temp_cstr), false);
				pInstance->setAttribute(ELEMENTNAME_KEY, elementNameAttr, attributes);
			}

			// Family = 1 (other)
			if (containsAttribute(FAMILY_KEY, attributes))
			{
				framework::Attribute familyAttr((NVM_UINT16)1, false);
				pInstance->setAttribute(FAMILY_KEY, familyAttr, attributes);
			}

			// OtherFamilyDescription = CPUID family
			if (containsAttribute(OTHERFAMILYDESCRIPTION_KEY, attributes))
			{
				snprintf(temp_cstr, temp_cstr_len, "%hhu", node.family);
				framework::Attribute otherFamilyDescAttr(std::string(temp_cstr), false);
				pInstance->setAttribute(OTHERFAMILYDESCRIPTION_KEY, otherFamilyDescAttr,
						attributes);
			}

			// Stepping
			if (containsAttribute(STEPPING_KEY, attributes))
			{
				snprintf(temp_cstr, temp_cstr_len, "%hhu", node.stepping);
				framework::Attribute steppingAttr(std::string(temp_cstr), false);
				pInstance->setAttribute(STEPPING_KEY, steppingAttr, attributes);
			}

			// NumberOfLogicalProcessors
			if (containsAttribute(NUMBEROFLOGICALPROCESSORS_KEY, attributes))
			{
				framework::Attribute lpAttr(node.logical_processor_count, false);
				pInstance->setAttribute(NUMBEROFLOGICALPROCESSORS_KEY, lpAttr,
						attributes);
			}

			// Type
			if (containsAttribute(TYPE_KEY, attributes))
			{
				framework::Attribute typeAttr((NVM_UINT16)node.type, false);
				pInstance->setAttribute(TYPE_KEY, typeAttr, attributes);
			}

			// Model
			if (containsAttribute(MODEL_KEY, attributes))
			{
				framework::Attribute modelAttr((NVM_UINT16)node.model, false);
				pInstance->setAttribute(MODEL_KEY, modelAttr, attributes);
			}

			// Manufacturer
			if (containsAttribute(MANUFACTURER_KEY, attributes))
			{
				framework::Attribute mfrAttr(std::string(node.manufacturer), false);
				pInstance->setAttribute(MANUFACTURER_KEY, mfrAttr, attributes);
			}
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
 * Return an object path for each SystemProcessor in the system
 */
wbem::framework::instance_names_t* wbem::memory::SystemProcessorFactory::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{

		// get NUMA node count
		int nodeCount = nvm_get_socket_count();
		if (nodeCount < NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(nodeCount);
		}

		// get the list of NUMA nodes
		if (nodeCount > 0)
		{
			// get the host server name
			std::string hostName = wbem::server::getHostName();

			// get the device_discovery information for all of the dimms
			struct socket nodes[nodeCount];
			nodeCount = nvm_get_sockets(nodes, (NVM_UINT16)nodeCount);
			if (nodeCount < NVM_SUCCESS)
			{
				throw exception::NvmExceptionLibError(nodeCount);
			}

			// find the set of unique proximity domains used across all DIMMs
			for (int i = 0; i < nodeCount; i++)
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

				// CreationClassName = topology::SYSTEMPROCESSOR_CREATIONCLASSNAME
				framework::Attribute attrCCName(SYSTEMPROCESSOR_CREATIONCLASSNAME, true);
				keys.insert(std::pair<std::string, framework::Attribute>(
						CREATIONCLASSNAME_KEY, attrCCName));

				// DeviceID = SYSTEMPROCESSOR_DEVICEID_PREFIX + NUMA node number
				std::string device_id_str = getDeviceId(nodes[i].id);
				framework::Attribute attrDeviceID(device_id_str, true);
				keys.insert(std::pair<std::string, framework::Attribute>(
						DEVICEID_KEY, attrDeviceID));

				// generate the ObjectPath for the instance
				framework::ObjectPath path(hostName, NVM_NAMESPACE,
						SYSTEMPROCESSOR_CREATIONCLASSNAME, keys);
				pNames->push_back(path);
			}
		}
		else // nvm_get_numa_node_count returned 0 NUMA nodes
		{
			// should never get here except in SIM
			COMMON_LOG_DEBUG("No System Processors found.");
		}
	}
	catch (framework::Exception &) // cleanup and re-throw
	{
		delete pNames;
		throw;
	}

	return pNames;
}

std::string wbem::memory::SystemProcessorFactory::getDeviceId(const NVM_UINT16 socketId)
{
	// DeviceID = SYSTEMPROCESSOR_DEVICEID_PREFIX + NUMA node number
	// 2^16 = 65536 -> 5 chars max, +1 for null terminator
	size_t numa_node_log_10_complexity = 6;
	char node_id_cstr[numa_node_log_10_complexity];
	snprintf(node_id_cstr, numa_node_log_10_complexity, "%05hu", socketId);
	std::string device_id_str = SYSTEMPROCESSOR_DEVICEID_PREFIX
			+ std::string(node_id_cstr);
	return device_id_str;
}

NVM_UINT16 wbem::memory::SystemProcessorFactory::getSocketId(std::string deviceId)
{
	NVM_UINT16 id;
	const char *pEnd = NULL;

	if (s_strtous(deviceId.c_str(), (NVM_UINT64)deviceId.size(), (const char **)&pEnd, &id) != deviceId.size())
	{
		COMMON_LOG_ERROR("Could not get socket number from deviceId.");
		throw framework::ExceptionBadParameter("DeviceID");
	}
	return id;
}
