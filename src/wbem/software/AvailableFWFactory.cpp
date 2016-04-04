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
 * This file contains the provider for the AvailableFW instances.
 */

#include <LogEnterExit.h>
#include <server/BaseServerFactory.h>
#include "AvailableFWFactory.h"
#include <NvmStrings.h>


wbem::software::AvailableFWFactory::AvailableFWFactory()
    throw (wbem::framework::Exception)
{ }

wbem::software::AvailableFWFactory::~AvailableFWFactory()
{ }

void wbem::software::AvailableFWFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
    // add key attributes
    attributes.push_back(INSTANCEID_KEY);
    

    // add non-key attributes
    attributes.push_back(ELEMENTNAME_KEY);

    }

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::software::AvailableFWFactory::getInstance(
    framework::ObjectPath &path, framework::attribute_names_t &attributes)
    throw (wbem::framework::Exception)
{
    LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

    checkPath(path);
    checkAttributes(attributes);

    // create the instance, initialize with attributes from the path
    framework::Instance *pInstance = new framework::Instance(path);

    try
    {
        // Element Name
        if (containsAttribute(ELEMENTNAME_KEY, attributes))
        {
            std::string hostServer = wbem::server::getHostName();
            framework::Attribute a(std::string("DIMM FW Set for ")  + hostServer, false);
            pInstance->setAttribute(ELEMENTNAME_KEY, a, attributes);
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
 * Return the object paths for the AvailableFW class.
 */
wbem::framework::instance_names_t* wbem::software::AvailableFWFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
    LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

    framework::instance_names_t *pNames = new framework::instance_names_t();
    try
    {
    	std::string hostServer = wbem::server::getHostName();

        framework::attributes_t keys;
        keys[INSTANCEID_KEY] = framework::Attribute(AVAILABLEFW_INSTANCEID_PREFIX + hostServer, true);
    
	    framework::ObjectPath path(hostServer, NVM_NAMESPACE,
				AVAILABLEFW_CREATIONCLASSNAME, keys);
        pNames->push_back(path);
    }
    catch (framework::Exception &) // clean up and re-throw
    {
        delete pNames;
        throw;
    }
    return pNames;
}
