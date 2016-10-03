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
 * This file contains the provider for the PerformanceMetricService instances
 * which define an NVM DIMM performance metric service.
 */

#include <server/BaseServerFactory.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <persistence/config_settings.h>
#include <persistence/lib_persistence.h>
#include <framework_interface/FrameworkExtensions.h>
#include <support/NVDIMMEventLogFactory.h>
#include <performance/PerformanceMetricServiceFactory.h>

wbem::performance::PerformanceMetricServiceFactory
::PerformanceMetricServiceFactory()
throw (wbem::framework::Exception)
{
}

wbem::performance::PerformanceMetricServiceFactory
::~PerformanceMetricServiceFactory()
{
}

void wbem::performance::PerformanceMetricServiceFactory
	::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// add key attributes
	attributes.push_back(SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(SYSTEMNAME_KEY);
	attributes.push_back(NAME_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(ENABLEDSTATE_KEY);
	attributes.push_back(INTERVAL_KEY);
}

/*
 * Verify the object path is valid for PerformanceMetricService instance
 */
void wbem::performance::PerformanceMetricServiceFactory::validateObjectPath(framework::ObjectPath &path, std::string hostName)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Attribute instanceID = path.getKeyValue(SYSTEMCREATIONCLASSNAME_KEY);
	if (instanceID.stringValue() != server::BASESERVER_CREATIONCLASSNAME)
	{
		throw framework::ExceptionBadParameter(SYSTEMCREATIONCLASSNAME_KEY.c_str());
	}

	framework::Attribute creationClassName = path.getKeyValue(CREATIONCLASSNAME_KEY);
	if (creationClassName.stringValue() != PERFORMANCEMETRICSERVICE_CREATIONCLASSNAME)
	{
		throw framework::ExceptionBadParameter(CREATIONCLASSNAME_KEY.c_str());
	}

	framework::Attribute systemName = path.getKeyValue(SYSTEMNAME_KEY);
	if (systemName.stringValue() != hostName)
	{
		throw framework::ExceptionBadParameter(SYSTEMNAME_KEY.c_str());
	}

	framework::Attribute name = path.getKeyValue(NAME_KEY);
	if (name.stringValue() != PERFORMANCEMETRICSERVICE_NAME + hostName)
	{
		throw framework::ExceptionBadParameter(NAME_KEY.c_str());
	}
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::performance::PerformanceMetricServiceFactory
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
		validateObjectPath(path, hostName);

		pInstance = new framework::Instance(path);
		framework::Attribute elementNameAttr(PERFORMANCEMETRICSERVICE_NAME + hostName, false);
		pInstance->setAttribute(wbem::ELEMENTNAME_KEY, elementNameAttr, attributes);

		int isEnabled;
		framework::UINT16 enabled_state = wbem::support::NVDIMMEVENTLOG_ENABLEDSTATE_UNKNOWN;
		if (get_config_value_int(SQL_KEY_EVENT_MONITOR_ENABLED, &isEnabled) == COMMON_SUCCESS)
		{
			enabled_state =
				isEnabled ? wbem::support::NVDIMMEVENTLOG_ENABLEDSTATE_ENABLED : wbem::support::NVDIMMEVENTLOG_ENABLEDSTATE_DISABLED;
		}
		framework::Attribute enabledStateAttr(enabled_state, false);
		pInstance->setAttribute(wbem::ENABLEDSTATE_KEY, enabledStateAttr, attributes);

		int intervalMinutes = 0;
		get_config_value_int(SQL_KEY_PERFORMANCE_MONITOR_INTERVAL_MINUTES, &intervalMinutes);
		framework::Attribute IntervalAttr((wbem::framework::UINT16)intervalMinutes, false);
		pInstance->setAttribute(wbem::INTERVAL_KEY, IntervalAttr, attributes);
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
wbem::framework::instance_names_t* wbem::performance::PerformanceMetricServiceFactory
::getInstanceNames() throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		// get the host server name
		std::string hostName = wbem::server::getHostName();

		framework::attributes_t keys;
		keys[SYSTEMCREATIONCLASSNAME_KEY] = framework::Attribute(server::BASESERVER_CREATIONCLASSNAME, true);
		keys[CREATIONCLASSNAME_KEY] = framework::Attribute(PERFORMANCEMETRICSERVICE_CREATIONCLASSNAME, true);
		keys[SYSTEMNAME_KEY] = framework::Attribute(hostName, true);
		keys[NAME_KEY] = framework::Attribute(PERFORMANCEMETRICSERVICE_NAME + hostName, true);

		// create the object path
		framework::ObjectPath path(hostName, NVM_NAMESPACE, PERFORMANCEMETRICSERVICE_CREATIONCLASSNAME, keys);

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

wbem::framework::Instance* wbem::performance::PerformanceMetricServiceFactory::modifyInstance(
		framework::ObjectPath &path, framework::attributes_t &attributes)
throw(framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pInstance = NULL;
	try
	{
		framework::attribute_names_t modifiableAttributes;
		modifiableAttributes.push_back(ENABLEDSTATE_KEY);
		modifiableAttributes.push_back(INTERVAL_KEY);

		framework::attribute_names_t attributeNames;
		pInstance = getInstance(path, attributeNames);
		if (pInstance)
		{
			checkAttributesAreModifiable(pInstance, attributes, modifiableAttributes);

			framework::Attribute newEnabledAttribute;
			bool isEnabled = support::NVDIMMEventLogFactory::verifyEnabledState(attributes, pInstance,
					newEnabledAttribute);

			framework::Attribute newIntervalAttribute;
			NVM_UINT16 newInterval = support::NVDIMMEventLogFactory::verifyInterval(attributes, pInstance,
					newIntervalAttribute);

			support::NVDIMMEventLogFactory::updateConfigTable(SQL_KEY_EVENT_MONITOR_ENABLED, (NVM_UINT64)isEnabled);
			pInstance->setAttribute(ENABLEDSTATE_KEY, newEnabledAttribute);

			support::NVDIMMEventLogFactory::updateConfigTable(SQL_KEY_PERFORMANCE_MONITOR_INTERVAL_MINUTES, (NVM_UINT64)newInterval);
			pInstance->setAttribute(INTERVAL_KEY, newIntervalAttribute);
		}
	}
	catch (framework::Exception &)
	{
		if (pInstance)
		{
			delete pInstance;
		}
		throw;
	}

	return pInstance;
}
