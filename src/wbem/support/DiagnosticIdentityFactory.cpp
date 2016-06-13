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
 * This file contains the provider for the DiagnosticIdentity instances
 * which represent the version of a diagnostic test.
 */

#include <LogEnterExit.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include "NVDIMMDiagnosticFactory.h"
#include "DiagnosticIdentityFactory.h"
#include <server/BaseServerFactory.h>

wbem::support::DiagnosticIdentityFactory::DiagnosticIdentityFactory()
	throw (wbem::framework::Exception)
{
}

wbem::support::DiagnosticIdentityFactory::~DiagnosticIdentityFactory()
{
}

void wbem::support::DiagnosticIdentityFactory::populateAttributeList(
	framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(MAJORVERSION_KEY);
	attributes.push_back(MINORVERSION_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::support::DiagnosticIdentityFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		checkAttributes(attributes);

		// get the host server name
		const std::string& hostName = wbem::server::getHostName();

		// check InstanceID
		framework::Attribute idAttr = path.getKeyValue(INSTANCEID_KEY);
		std::string instanceId = idAttr.stringValue();
		if (instanceId.empty()) // no instance ID
		{
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
		}

		int hostnameLength = hostName.length();
		// verify host name at the start of the InstanceID string
		if (instanceId.compare(0u, hostnameLength, hostName) != 0) // not found - InstanceId invalid
		{
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
		}

		// verify test type from end of InstanceID to set ElementName
		std::string testType = instanceId.substr(hostnameLength, (int)(instanceId.length()) - hostnameLength);
		// verify test type
		if (!wbem::support::NVDIMMDiagnosticFactory::testTypeValid(testType))
		{
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
		}
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			// set ElementName =  "Diagnostic Identity" + test type
			framework::Attribute elementNameAttr(DIAGNOSTICIDENTITY_ELEMENTNAME + testType, false);
			pInstance->setAttribute(ELEMENTNAME_KEY, elementNameAttr, attributes);

		}
		if (containsAttribute(MAJORVERSION_KEY, attributes))
		{
			framework::Attribute majorAttr((NVM_UINT16)1, false);
			pInstance->setAttribute(MAJORVERSION_KEY, majorAttr, attributes);
		}
		if (containsAttribute(MINORVERSION_KEY, attributes))
		{
			framework::Attribute minorAttr((NVM_UINT16)0, false);
			pInstance->setAttribute(MINORVERSION_KEY, minorAttr, attributes);
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
 Return the object paths for the DiagnosticIdentity class
 There should be 5 possible instances, one for each type of diagnostic test.
 */
wbem::framework::instance_names_t* wbem::support::DiagnosticIdentityFactory::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::instance_names_t *pNames = new framework::instance_names_t();

	// get the host server name
	const std::string& hostName = wbem::server::getHostName();

	for (int i = 0; i < NVDIMMDIAGNOSTIC_NUMTESTTYPES; i++)
	{
		framework::attributes_t keys;

		// add test type key
		keys[INSTANCEID_KEY] =
				framework::Attribute(hostName + validTestTypes[i], true);

		framework::ObjectPath path(hostName, NVM_NAMESPACE,
				DIAGNOSTICIDENTITY_CREATIONCLASSNAME, keys);

		pNames->push_back(path);
	}

	return pNames;
}
