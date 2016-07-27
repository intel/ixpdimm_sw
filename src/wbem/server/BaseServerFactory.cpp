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
 * This file contains the provider for the BaseServer instances which models
 * the server containing the NVM DIMMs.
 */

#include <nvm_management.h>

#include <LogEnterExit.h>
#include <persistence/config_settings.h>
#include <persistence/lib_persistence.h>

#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include "BaseServerFactory.h"

#include <exception/NvmExceptionLibError.h>
#include <core/exceptions/NoMemoryException.h>
#include <lib_interface/NvmApi.h>
#include <NvmStrings.h>

#include "framework_interface/FrameworkExtensions.h"

wbem::server::BaseServerFactory::BaseServerFactory()
	throw (wbem::framework::Exception)
{
}

wbem::server::BaseServerFactory::~BaseServerFactory()
{
}

void wbem::server::BaseServerFactory::populateAttributeList(
	framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(NAME_KEY);

	// add non-key attributes
	attributes.push_back(OSNAME_KEY);
	attributes.push_back(OSVERSION_KEY);
	attributes.push_back(LOGLEVEL_KEY);
	attributes.push_back(DEDICATED_KEY);
	attributes.push_back(OPERATIONALSTATUS_KEY);
}

void wbem::server::BaseServerFactory::validateObjectPath(framework::ObjectPath &path)
	throw (wbem::framework::Exception)
{
	// Inspect key attributes from object path
	// Note there's only one instance of VolatileMemory per system

	// CreationClassName - BaseServer
	framework::Attribute className = path.getKeyValue(CREATIONCLASSNAME_KEY);
	if (className.stringValue() != BASESERVER_CREATIONCLASSNAME)
	{
		throw framework::ExceptionBadParameter(CREATIONCLASSNAME_KEY.c_str());
	}

	// Name - host name
	framework::Attribute sysName = path.getKeyValue(NAME_KEY);
	std::string hostName = wbem::server::getHostName();
	if (sysName.stringValue() != hostName)
	{
		throw framework::ExceptionBadParameter(NAME_KEY.c_str());
	}
}

void wbem::server::BaseServerFactory::toInstance(core::system::SystemInfo &hostInfo,
		wbem::framework::Instance &instance, wbem::framework::attribute_names_t attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	ADD_ATTRIBUTE(instance, attributes, OSNAME_KEY, framework::STR, hostInfo.getOsName());

	ADD_ATTRIBUTE(instance, attributes, OSVERSION_KEY, framework::STR, hostInfo.getOsVersion());

	ADD_ATTRIBUTE(instance, attributes, LOGLEVEL_KEY, framework::UINT16, hostInfo.getLogLevel());

	framework::UINT16_LIST dedicatedValue;
	dedicatedValue.push_back(1u); // "unknown"
	ADD_ATTRIBUTE(instance, attributes, DEDICATED_KEY, framework::UINT16_LIST, dedicatedValue);

	wbem::framework::UINT16_LIST hostOpStatusList =
			hostToOpStatus(hostInfo.getMixedSku(), hostInfo.getSkuViolation());
	ADD_ATTRIBUTE(instance, attributes, OPERATIONALSTATUS_KEY,
			framework::UINT16_LIST, hostOpStatusList);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::server::BaseServerFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pResult = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);
		validateObjectPath(path);

		core::system::SystemService &service = core::system::SystemService::getService();
		core::Result<core::system::SystemInfo> s = service.getHostInfo();

		toInstance(s.getValue(), *pResult, attributes);
	}
	catch (framework::Exception &)
	{
		delete pResult;
		throw;
	}
	catch (core::NoMemoryException)
	{
		delete pResult;
		throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__, "Could not allocate memory");
	}

	return pResult;
}

/*
 * Return the object paths for the BaseServer class.  Should only
 * be one server.
 */
wbem::framework::instance_names_t* wbem::server::BaseServerFactory::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		// Note that unlike several other WBEM classes, this function cannot succeed with zero
		// instances.  That is to say, either the only instance was found, or there was an error,
		// and that error needs to be carried back to the caller.

		// get the host server info so that we have the host name
		std::string hostName = wbem::server::getHostName();

		framework::attributes_t keys;

		// CreationClassName = server::BASESERVER_CREATIONCLASSNAME
		framework::Attribute attrCCName(BASESERVER_CREATIONCLASSNAME, true);
		keys.insert(std::pair<std::string, framework::Attribute>(
				CREATIONCLASSNAME_KEY, attrCCName));

		// Name = (host) server name
		framework::Attribute attrName(std::string(hostName), true);
		keys.insert(std::pair<std::string, framework::Attribute>(
				NAME_KEY, attrName));

		// generate the ObjectPath for the instance (there should only be one)
		framework::ObjectPath path(hostName, NVM_NAMESPACE,
				BASESERVER_CREATIONCLASSNAME, keys);
		pNames->push_back(path);
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}

	return pNames;
}

/*
 * Add a default simulator to be loaded when the nvm library is loaded
 */
void wbem::server::BaseServerFactory::addDefaultSimulator(std::string fileName)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// add the key to make this simulator the default
	if (add_config_value(SQL_KEY_DEFAULT_SIMULATOR, fileName.c_str()) != COMMON_SUCCESS)
	{
		throw framework::Exception("Failed to add the default simulator to the config database.");
	}
}

/*
 *  Remove the default simulator setting
 */
void wbem::server::BaseServerFactory::removeDefaultSimulator()
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// remove the configuration setting for the default simulator
	if (rm_config_value(SQL_KEY_DEFAULT_SIMULATOR) != COMMON_SUCCESS)
	{
		throw framework::Exception("Failed to remove the default simulator from the config database.");
	}
}

/*
 * Change the debug logging level of the management software
 */
void wbem::server::BaseServerFactory::setDebugLogging(int logSetting)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	int rc = nvm_toggle_debug_logging(logSetting);
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

/*
 * Retrieve just the host server name
 */
const std::string wbem::server::getHostName()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();
	return pApi->getHostName();
}


/*
 * Helper function to convert the host information into operational status
 */
wbem::framework::UINT16_LIST wbem::server::BaseServerFactory::hostToOpStatus(bool mixedSku, bool skuViolation)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::UINT16_LIST opStatus;

	// OK - this is always set as we don't have a "health" state for the server
	opStatus.push_back(BASESERVER_OPSTATUS_OK);

	if (mixedSku)
	{
		opStatus.push_back(BASESERVER_OPSTATUS_MIXEDSKU);
	}

	if (skuViolation)
	{
		opStatus.push_back(BASESERVER_OPSTATUS_SKUVIOLATION);
	}

	return opStatus;
}
