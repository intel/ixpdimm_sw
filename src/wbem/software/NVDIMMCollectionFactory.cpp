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
 * This file contains the provider for the NVDIMMCollection instance
 * which represents the collection of all the NVM DIMMs in the host server.
 */

#include <LogEnterExit.h>
#include "NVDIMMCollectionFactory.h"
#include <libinvm-cim/Attribute.h>
#include <NvmStrings.h>


wbem::software::NVDIMMCollectionFactory::NVDIMMCollectionFactory()
    throw (wbem::framework::Exception)
{ }

wbem::software::NVDIMMCollectionFactory::~NVDIMMCollectionFactory()
{ }

void wbem::software::NVDIMMCollectionFactory::populateAttributeList(framework::attribute_names_t &attributes)
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
wbem::framework::Instance* wbem::software::NVDIMMCollectionFactory::getInstance(
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
		// ElementName - "DIMM Collection for" + host name
        if (containsAttribute(ELEMENTNAME_KEY, attributes))
        {
            std::string hostName = wbem::server::getHostName();
            framework::Attribute a(std::string("DIMM Collection for ") + hostName, false);
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
 * Return the object paths for the NVDIMMCollection class.
 */
wbem::framework::instance_names_t* wbem::software::NVDIMMCollectionFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
    LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

    framework::instance_names_t *pNames = new framework::instance_names_t();
    try
    {
		std::string hostName = wbem::server::getHostName();
        pNames->push_back(getObjectPath(hostName));
    }
    catch (framework::Exception &) // clean up and re-throw
    {
        delete pNames;
        throw;
    }
    return pNames;
}

wbem::framework::ObjectPath wbem::software::NVDIMMCollectionFactory::getObjectPath(
		const std::string& hostName)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

    framework::attributes_t keys;
	// InstanceID - "DIMMCollection" + host name
    keys[INSTANCEID_KEY] = framework::Attribute(std::string("DIMMCollection") + hostName, true);

    framework::ObjectPath path(hostName, NVM_NAMESPACE,
			NVDIMMCOLLECTION_CREATIONCLASSNAME, keys);

    return path;
}
