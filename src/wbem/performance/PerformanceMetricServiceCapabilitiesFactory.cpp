/*
 * Copyright (c) 2016, Intel Corporation
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
 * This file contains the provider for the PerformanceMetricServiceCapabilities instances
 * which define an NVM DIMM performance metric service capabilities.
 */

#include <server/BaseServerFactory.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <performance/PerformanceMetricServiceCapabilitiesFactory.h>

wbem::performance::PerformanceMetricServiceCapabilitiesFactory
::PerformanceMetricServiceCapabilitiesFactory()
throw (wbem::framework::Exception)
{
}

wbem::performance::PerformanceMetricServiceCapabilitiesFactory
::~PerformanceMetricServiceCapabilitiesFactory()
{
}

void wbem::performance::PerformanceMetricServiceCapabilitiesFactory
	::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(SUPPORTEDMETHODS_KEY);
	attributes.push_back(CONTROLLEDMETRICS_KEY);
	attributes.push_back(METRICSCONTROLTYPES_KEY);
	attributes.push_back(CONTROLLABLEMANAGEDELEMENTS_KEY);
	attributes.push_back(MANAGEDELEMENTCONTROLTYPES_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::performance::PerformanceMetricServiceCapabilitiesFactory
::getInstance(framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pInstance = NULL;
	try
	{
		// Verify attributes
		checkAttributes(attributes);

		std::string hostName = wbem::server::getHostName();

		framework::Attribute instanceID = path.getKeyValue(INSTANCEID_KEY);
		if (instanceID.stringValue() != std::string(PERFORMANCEMETRICSERVICECAPABILITIES_INSTANCEID + hostName))
		{
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
		}

		pInstance = new framework::Instance(path);
		framework::Attribute elementAttr(std::string(PERFORMANCEMETRICSERVICECAPABILITIES_ELEMENTNAME + hostName), false);
		pInstance->setAttribute(wbem::ELEMENTNAME_KEY, elementAttr, attributes);

		framework::UINT16_LIST uintList;
		framework::Attribute uintListAttr(uintList, false);
		framework::STR_LIST strList;
		framework::Attribute strListAttr(strList, false);
		pInstance->setAttribute(wbem::SUPPORTEDMETHODS_KEY, uintListAttr, attributes);
		pInstance->setAttribute(wbem::CONTROLLEDMETRICS_KEY, strListAttr, attributes);
		pInstance->setAttribute(wbem::METRICSCONTROLTYPES_KEY, uintListAttr, attributes);
		pInstance->setAttribute(wbem::CONTROLLABLEMANAGEDELEMENTS_KEY, strListAttr, attributes);
		pInstance->setAttribute(wbem::MANAGEDELEMENTCONTROLTYPES_KEY, uintListAttr, attributes);
	}
	catch (framework::Exception) // clean up and re-throw
	{
		if (pInstance)
		{
			delete pInstance;
			pInstance = NULL;
		}
		throw;
	}

	return pInstance;
}

/*
 * Return the object paths for the PerformanceMetricDefinitionFactory class.
 */
wbem::framework::instance_names_t* wbem::performance::PerformanceMetricServiceCapabilitiesFactory
::getInstanceNames() throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		// get the host server name
		std::string hostName = wbem::server::getHostName();

		framework::attributes_t keys;
		keys[INSTANCEID_KEY] =
			framework::Attribute(PERFORMANCEMETRICSERVICECAPABILITIES_INSTANCEID + hostName, true);

		// create the object path
		framework::ObjectPath path(hostName, NVM_NAMESPACE,
				PERFORMANCEMETRICSERVICECAPABILITIES_CREATIONCLASSNAME, keys);

		if (pNames != NULL)
		{
			pNames->push_back(path);
		}
	}
	catch (framework::Exception) // clean up and re-throw
	{
		if (pNames != NULL)
		{
			delete pNames;
			pNames = NULL;
		}
		throw;
	}

	return pNames;
}
