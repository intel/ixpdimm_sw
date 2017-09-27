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
 * This file contains the provider for the OpaqueSupportData instance.
 */

#include <nvm_management.h>
#include <LogEnterExit.h>

#include "OpaqueSupportDataFactory.h"
#include <server/BaseServerFactory.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>

wbem::support::OpaqueSupportDataFactory::OpaqueSupportDataFactory()
{
}

wbem::support::OpaqueSupportDataFactory::~OpaqueSupportDataFactory()
{
}

void wbem::support::OpaqueSupportDataFactory::populateAttributeList(
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	attributes.push_back(DEVICEID_KEY);
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(SYSTEMNAME_KEY);
}



/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::support::OpaqueSupportDataFactory::getInstance(
		framework::ObjectPath &path,
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pInstance = new framework::Instance(path);

	checkPath(path);
	checkAttributes(attributes);

	return pInstance;
}

/*
 * Return the object paths for the OpaqueSupportDataclass.
 */
wbem::framework::instance_names_t* wbem::support::OpaqueSupportDataFactory::getInstanceNames()
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::instance_names_t *pNames = new framework::instance_names_t();

	// get the host server name
	std::string hostName;
	try
	{
		hostName = wbem::server::getHostName();
	}
	catch (exception::NvmExceptionLibError &)
	{
		// instance always exists and is updated as necessary, so we must have one for no host
		hostName = OPAQUESUPPORTDATA_UNKNOWN_ELEMENTNAME;
	}

	framework::attributes_t keys;
	keys[SYSTEMCREATIONCLASSNAME_KEY] =
			framework::Attribute(OPAQUESUPPORTDATA_SYSTEMCREATIONCLASSNAME, true);

	keys[SYSTEMNAME_KEY] =
			framework::Attribute(hostName, true);

	keys[CREATIONCLASSNAME_KEY] =
			framework::Attribute(OPAQUESUPPORTDATA_CREATIONCLASSNAME, true);

	keys[DEVICEID_KEY] =
			framework::Attribute(hostName, true);

	keys[ELEMENTNAME_KEY] =
			framework::Attribute(OPAQUESUPPORTDATA_ELEMENTNAME + hostName, true);

	framework::ObjectPath path(hostName, NVM_NAMESPACE,
			OPAQUESUPPORTDATA_CREATIONCLASSNAME, keys);

	pNames->push_back(path);

	return pNames;
}
