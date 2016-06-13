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
 * This file contains the CIM provider for the ErasureCapabilities class.
 */

#include <LogEnterExit.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include "ErasureCapabilitiesFactory.h"
#include <server/BaseServerFactory.h>
#include <NvmStrings.h>

wbem::erasure::ErasureCapabilitiesFactory::ErasureCapabilitiesFactory()
throw (wbem::framework::Exception)
{ }

wbem::erasure::ErasureCapabilitiesFactory::~ErasureCapabilitiesFactory()
{ }

void wbem::erasure::ErasureCapabilitiesFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(ERASUREMETHODS_KEY);
	attributes.push_back(DEFAULTERASUREMETHOD_KEY);
	attributes.push_back(CANERASEONRETURNTOSTORAGEPOOL_KEY);
	attributes.push_back(ELEMENTTYPESSUPPORTED_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::erasure::ErasureCapabilitiesFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// verify object path
	std::string hostName = wbem::server::getHostName();

	std::string instanceId = path.getKeyValue(INSTANCEID_KEY).stringValue();
	if (instanceId != hostName)
	{
		throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
	}
	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);

		// ElementName - "Erasure Capabilities"
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			framework::Attribute a(ERASURECAPABILITIES_ELEMENTNAME, false);
			pInstance->setAttribute(ELEMENTNAME_KEY, a, attributes);
		}

		// ErasureMethods - "Crypto Erase"
		if (containsAttribute(ERASUREMETHODS_KEY, attributes))
		{
			framework::STR_LIST methods;
			methods.push_back(ERASURECAPABILITIES_ERASUREMETHOD_CRYPTO_ERASE);

			framework::Attribute a(methods, false);
			pInstance->setAttribute(ERASUREMETHODS_KEY, a, attributes);
		}

		// DefaultErasureMethod - "Secure Erase"
		if (containsAttribute(DEFAULTERASUREMETHOD_KEY, attributes))
		{
			framework::Attribute a(ERASURECAPABILITIES_ERASUREMETHOD_CRYPTO_ERASE, false);
			pInstance->setAttribute(DEFAULTERASUREMETHOD_KEY, a, attributes);
		}

		// CanEraseOnReturnToStoragePool - false
		if (containsAttribute(CANERASEONRETURNTOSTORAGEPOOL_KEY, attributes))
		{
			framework::Attribute a(false, false);
			pInstance->setAttribute(CANERASEONRETURNTOSTORAGEPOOL_KEY, a, attributes);
		}

		// ElementTypesSupported - 3 - Storage Extent
		if (containsAttribute(ELEMENTTYPESSUPPORTED_KEY, attributes))
		{
			framework::UINT16_LIST types;
			types.push_back(ERASURECAPABILITIES_ELEMENTTYPESSUPPORTED_EXTENT);
			framework::Attribute a(types, false);
			pInstance->setAttribute(ELEMENTTYPESSUPPORTED_KEY, a, attributes);
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
 * Return the object paths for the ErasureCapabilities class.
 */
wbem::framework::instance_names_t* wbem::erasure::ErasureCapabilitiesFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	try
	{
		std::string hostName = wbem::server::getHostName();

		framework::attributes_t keys;
		// InstanceID - Host name
		keys[INSTANCEID_KEY] = framework::Attribute(std::string(hostName), true);

		framework::ObjectPath path(hostName, NVM_NAMESPACE,
				ERASURECAPABILITIES_CREATIONCLASSNAME, keys);

		framework::instance_names_t *pNames = new framework::instance_names_t();
		pNames->push_back(path);
		return pNames;
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		throw;
	}
}
