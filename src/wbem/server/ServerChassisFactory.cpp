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
 * This file contains the provider for the ServerChassis instances.
 */

#include <LogEnterExit.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include "ServerChassisFactory.h"

#include <NvmStrings.h>
#include <core/exceptions/LibraryException.h>
#include <exception/NvmExceptionLibError.h>
#include "BaseServerFactory.h"

wbem::server::ServerChassisFactory::ServerChassisFactory()
throw (wbem::framework::Exception)
{ }

wbem::server::ServerChassisFactory::~ServerChassisFactory()
{ }

void wbem::server::ServerChassisFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(TAG_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::server::ServerChassisFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		checkAttributes(attributes);

		// validate tag key is for the correct host
		framework::Attribute tagAttr;
		if (pInstance->getAttribute(TAG_KEY, tagAttr) != framework::SUCCESS)
		{
			throw framework::ExceptionBadParameter(TAG_KEY.c_str());
		}
		if (tagAttr.stringValue().compare(server::getHostName()) != 0)
		{
			throw framework::ExceptionBadParameter(TAG_KEY.c_str());
		}

		// validate creation class name is correct
		framework::Attribute ccNameAttr;
		if (pInstance->getAttribute(CREATIONCLASSNAME_KEY, ccNameAttr) != framework::SUCCESS)
		{
			throw framework::ExceptionBadParameter(CREATIONCLASSNAME_KEY.c_str());
		}
		if (ccNameAttr.stringValue().compare(server::SERVERCHASSIS_CREATIONCLASSNAME) != 0)
		{
			throw framework::ExceptionBadParameter(CREATIONCLASSNAME_KEY.c_str());
		}

		// no additional attributes in this class beyond the keys
	}
	catch (framework::Exception &)
	{
		delete pInstance;
		throw;
	}

	return pInstance;
}

/*
 * Return the object paths for the ServerChassis class.
 */
wbem::framework::instance_names_t* wbem::server::ServerChassisFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		std::string hostName = getHostName();
		framework::attributes_t keys;
		keys[TAG_KEY] = framework::Attribute(hostName, true);
		keys[CREATIONCLASSNAME_KEY] =
				framework::Attribute(SERVERCHASSIS_CREATIONCLASSNAME, true);

		framework::ObjectPath path(hostName, NVM_NAMESPACE,
				std::string(SERVERCHASSIS_CREATIONCLASSNAME), keys);
		pNames->push_back(path);
	}
	catch (core::LibraryException &e)
	{
		delete pNames;
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}
	catch (framework::Exception &)
	{
		delete pNames;
		throw;
	}
	return pNames;
}
