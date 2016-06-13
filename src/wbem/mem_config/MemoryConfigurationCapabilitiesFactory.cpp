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
 * This file contains the provider for the MemoryConfigurationCapabilities instances.
 */


#include <LogEnterExit.h>

#include <libinvm-cim/Types.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include "MemoryConfigurationCapabilitiesFactory.h"
#include <server/BaseServerFactory.h>
#include <NvmStrings.h>

wbem::mem_config::MemoryConfigurationCapabilitiesFactory::MemoryConfigurationCapabilitiesFactory()
{
}

wbem::mem_config::MemoryConfigurationCapabilitiesFactory::~MemoryConfigurationCapabilitiesFactory()
{
}

void wbem::mem_config::MemoryConfigurationCapabilitiesFactory::populateAttributeList(
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(SUPPORTEDSYNCHRONOUSOPERATIONS_KEY);
	attributes.push_back(SUPPORTEDASYNCHRONOUSOPERATIONS_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::mem_config::MemoryConfigurationCapabilitiesFactory::getInstance(
		framework::ObjectPath &path,
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Verify ObjectPath ...
	std::string hostName = wbem::server::getHostName();

	// Verify InstanceID
	framework::Attribute attribute = path.getKeyValue(INSTANCEID_KEY);
	if (attribute.stringValue() != (hostName + MEMORYCONFIGURATIONCAPABILITIES_INSTANCEID))
	{
		throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
	}

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		checkAttributes(attributes);

		// ElementName - hostname + " NVM Configuration Capabilities"
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			framework::Attribute a((hostName + MEMORYCONFIGURATIONCAPABILITIES_ELEMENTNAME), false);
			pInstance->setAttribute(ELEMENTNAME_KEY, a, attributes);
		}

		// SynchronousMethodsSupported
		if (containsAttribute(SUPPORTEDSYNCHRONOUSOPERATIONS_KEY, attributes))
		{
			// empty list
			framework::UINT16_LIST synchmethods;
			framework::Attribute a(synchmethods, false);
			pInstance->setAttribute(SUPPORTEDSYNCHRONOUSOPERATIONS_KEY, a, attributes);
		}

		// AsynchronousMethodsSupported - none
		if (containsAttribute(SUPPORTEDASYNCHRONOUSOPERATIONS_KEY, attributes))
		{
			framework::UINT16_LIST asynchmethods;
			asynchmethods.push_back(METHOD_ALLOCATEFROMPOOL);
			framework::Attribute a(asynchmethods, false);
			pInstance->setAttribute(SUPPORTEDASYNCHRONOUSOPERATIONS_KEY, a, attributes);
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
 * Return the object paths for the MemoryConfigurationCapabilities Class.
 */
wbem::framework::instance_names_t* wbem::mem_config::MemoryConfigurationCapabilitiesFactory::getInstanceNames()
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// get the host server name
	std::string hostName = wbem::server::getHostName();
	framework::instance_names_t *pNames = new framework::instance_names_t();
	framework::attributes_t keys;

	std::string instance_id = hostName + MEMORYCONFIGURATIONCAPABILITIES_INSTANCEID;
	keys[INSTANCEID_KEY] = framework::Attribute(instance_id, true);

	// generate the ObjectPath for the instance (only one per server)
	framework::ObjectPath path(hostName, NVM_NAMESPACE,
			MEMORYCONFIGURATIONCAPABILITIES_CREATIONCLASSNAME, keys);
	pNames->push_back(path);

	return pNames;
}
