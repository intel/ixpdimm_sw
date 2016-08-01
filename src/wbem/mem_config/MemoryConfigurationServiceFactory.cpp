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
 * This file contains the provider for the MemoryConfigurationService instances.
 * The MemoryConfigurationService supports changing how
 * NVM DIMMs are mapped into the system address space.
 */

#include <string.h>
#include <list>
#include <algorithm>
#include <file_ops/file_ops_adapter.h>
#include <nvm_management.h>
#include <utility.h>
#include <LogEnterExit.h>
#include <uid/uid.h>
#include <libinvm-cim/StringUtil.h>
#include <libinvm-cim/CimXml.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <libinvm-cim/ExceptionNotSupported.h>
#include <libinvm-cim/ObjectPathBuilder.h>
#include "MemoryConfigurationServiceFactory.h"
#include "MemoryConfigurationFactory.h"
#include "MemoryResourcesFactory.h"
#include <server/BaseServerFactory.h>
#include <memory/RawMemoryFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include <memory/SystemProcessorFactory.h>
#include <mem_config/PoolViewFactory.h>
#include <exception/NvmExceptionLibError.h>
#include <framework_interface/NvmAssociationFactory.h>
#include <framework_interface/NvmInstanceFactory.h>
#include <lib_interface/NvmApi.h>
#include <NvmStrings.h>
#include <core/memory_allocator/MemoryAllocator.h>
#include <core/memory_allocator/MemoryAllocationUtil.h>
#include <core/memory_allocator/RulePartialSocketConfigured.h>
#include <core/device/DeviceHelper.h>
#include <core/exceptions/NvmExceptionBadRequest.h>

wbem::mem_config::MemoryConfigurationServiceFactory::MemoryConfigurationServiceFactory()
	throw (wbem::framework::Exception) :
		wbem::framework_interface::NvmInstanceFactory(),
		m_DeleteConfigGoalProvider(nvm_delete_config_goal)
{
}

/*
 * Copy constructor
 */
wbem::mem_config::MemoryConfigurationServiceFactory::MemoryConfigurationServiceFactory(const MemoryConfigurationServiceFactory &config)
{
	m_DeleteConfigGoalProvider = config.m_DeleteConfigGoalProvider;
	m_cimNamespace = config.m_cimNamespace;
	m_pApi = config.m_pApi;
};

/*!
 * Assignment Operator
 */
wbem::mem_config::MemoryConfigurationServiceFactory& wbem::mem_config::MemoryConfigurationServiceFactory::operator=(const MemoryConfigurationServiceFactory &other)
{
	if (this != &other)
	{
		m_DeleteConfigGoalProvider = other.m_DeleteConfigGoalProvider;
		m_cimNamespace = other.m_cimNamespace;
		m_pApi = other.m_pApi;
	}
	return *this;
}

wbem::mem_config::MemoryConfigurationServiceFactory::~MemoryConfigurationServiceFactory()
{
}

void wbem::mem_config::MemoryConfigurationServiceFactory::populateAttributeList(
		framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(SYSTEMNAME_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(NAME_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::mem_config::MemoryConfigurationServiceFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);

		// validate system class name
		const framework::Attribute systemClassNameAttr = path.getKeyValue(SYSTEMCREATIONCLASSNAME_KEY);
		if (systemClassNameAttr.stringValue() != server::BASESERVER_CREATIONCLASSNAME)
		{
			throw framework::ExceptionBadParameter(systemClassNameAttr.stringValue().c_str());
		}

		// validate system name
		// get the actual host server name
		std::string hostName = wbem::server::getHostName();

		const framework::Attribute systemNameAttr = path.getKeyValue(SYSTEMNAME_KEY);
		if (systemNameAttr.stringValue() != hostName)
		{
			throw framework::ExceptionBadParameter(systemNameAttr.stringValue().c_str());
		}

		// validate creation class name
		const framework::Attribute classNameAttr = path.getKeyValue(CREATIONCLASSNAME_KEY);
		if (classNameAttr.stringValue() != MEMORYCONFIGURATIONSERVICE_CREATIONCLASSNAME)
		{
			throw framework::ExceptionBadParameter(classNameAttr.stringValue().c_str());
		}

		// validate name
		const framework::Attribute nameAttr = path.getKeyValue(NAME_KEY);
		if (nameAttr.stringValue() != (hostName + MEMORYCONFIGURATIONSERVICE_NAME))
		{
			throw framework::ExceptionBadParameter(nameAttr.stringValue().c_str());
		}

		// ElementName - hostname + " Memory Resource Configuration Service"
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			std::string elementName = hostName + MEMORYCONFIGURATIONSERVICE_ELEMENTNAME;
			framework::Attribute attr(elementName, false);
			pInstance->setAttribute(ELEMENTNAME_KEY, attr, attributes);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
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
 Return the object paths for the MemoryPoolConfigurationService class
 */
wbem::framework::instance_names_t* wbem::mem_config::MemoryConfigurationServiceFactory::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string hostName = wbem::server::getHostName();
	framework::instance_names_t *pNames = new framework::instance_names_t();

	// set creation class name to define
	framework::attributes_t keys;
	keys.insert(std::pair<std::string, framework::Attribute>(
			SYSTEMCREATIONCLASSNAME_KEY,
			framework::Attribute(MEMORYCONFIGURATIONSERVICE_SYSTEMCREATIONCLASSNAME, true)));

	// set system name to host name
	keys.insert(std::pair<std::string, framework::Attribute>(
			SYSTEMNAME_KEY, framework::Attribute(hostName, true)));

	// set creation class name to defined
	keys.insert(std::pair<std::string, framework::Attribute>(
			CREATIONCLASSNAME_KEY,
			framework::Attribute(MEMORYCONFIGURATIONSERVICE_CREATIONCLASSNAME, true)));

	// set name to host name + defined
	keys.insert(std::pair<std::string, framework::Attribute>(
			NAME_KEY, framework::Attribute(hostName + MEMORYCONFIGURATIONSERVICE_NAME, true)));

	framework::ObjectPath path(hostName, NVM_NAMESPACE,
			MEMORYCONFIGURATIONSERVICE_CREATIONCLASSNAME, keys);
	pNames->push_back(path);
	return pNames;
}

NVM_UINT16 wbem::mem_config::MemoryConfigurationServiceFactory::getSocketIdForSettingsString
	(std::string setting) throw(framework::Exception)
{
	framework::CimXml instance(setting);
	framework::attributes_t attrs = instance.getProperties();

	framework::attributes_t::iterator attrI = attrs.find(PARENT_KEY);
	if (attrI == attrs.end())
	{
		COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
				PARENT_KEY.c_str());
		throw framework::ExceptionBadParameter(PARENT_KEY.c_str());
	}
	return wbem::memory::SystemProcessorFactory::getSocketId(attrI->second.stringValue());
}

bool wbem::mem_config::MemoryConfigurationServiceFactory::getNewMemorySetting(std::string &setting)
{
	framework::CimXml instance(setting);
	framework::attributes_t attrs = instance.getProperties();

	framework::attributes_t::iterator newMemoryOnlyI = attrs.find(NEWMEMORYONLY_KEY);
	if (newMemoryOnlyI == attrs.end())
	{
		COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
				NEWMEMORYONLY_KEY.c_str());
		throw framework::ExceptionBadParameter(NEWMEMORYONLY_KEY.c_str());
	}
	return newMemoryOnlyI->second.boolValue();
}

wbem::framework::STR_LIST wbem::mem_config::MemoryConfigurationServiceFactory::getSettingsStringsForSocket
	(framework::STR_LIST &settingsStrings, NVM_UINT16 socketId)
	throw (framework::Exception)
{
	framework::STR_LIST socketSettings;

	for (size_t i = 0; i < settingsStrings.size(); i++)
	{
		if (getSocketIdForSettingsString(settingsStrings[i]) == socketId)
		{
			socketSettings.push_back(settingsStrings[i]);
		}
	}
	return socketSettings;
}

void wbem::mem_config::MemoryConfigurationServiceFactory::removeSettingsWithSocketId
	(framework::STR_LIST &settingsStrings, NVM_UINT16 socketId)
{
	framework::STR_LIST newSettingsStrings;

	for (size_t i = 0; i < settingsStrings.size(); i++)
	{
		if (getSocketIdForSettingsString(settingsStrings[i]) != socketId)
		{
			newSettingsStrings.push_back(settingsStrings[i]);
		}
	}
	settingsStrings = newSettingsStrings;
}

void wbem::mem_config::MemoryConfigurationServiceFactory::getDimmsForMemAllocSettings(
		framework::STR_LIST settings, std::vector<core::memory_allocator::Dimm> &dimms)
{
	// The list of settingsStrings will all be for the same socket and will all
	// have the same newMemoryOnly attribute so we need only look at the first
	// settings string
	bool newMemoryOnly = getNewMemorySetting(settings.front());
	NVM_UINT16 socketId = getSocketIdForSettingsString(settings.front());

	std::vector<struct device_discovery> devices;
	m_pApi->getManageableDimms(devices);
	for (size_t i = 0; i < devices.size(); i++)
	{
		if (devices[i].socket_id == socketId)
		{
			struct device_status status;
			memset(&status, 0, sizeof(device_status));
			int rc = NVM_SUCCESS;
			if ((rc = m_pApi->getDeviceStatus(devices[i].uid, &status)) != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR("Could not get device status");
				throw exception::NvmExceptionLibError(rc);
			}

			if (status.is_new || !newMemoryOnly)
			{
				dimms.push_back(core::memory_allocator::MemoryAllocationUtil::deviceDiscoveryToDimm(devices[i]));
			}
		}
	}
}

void wbem::mem_config::MemoryConfigurationServiceFactory::validateSettingsStrings(
		framework::STR_LIST settingsStrings) throw (framework::Exception)
{

	framework::STR_LIST::iterator iter = settingsStrings.begin();
	for (; iter != settingsStrings.end(); iter++)
	{

		framework::CimXml settingsInstance(*iter);

		if (settingsInstance.getClass() != MEMORYALLOCATIONSETTINGS_CREATIONCLASSNAME)
		{
			COMMON_LOG_ERROR_F("%s is not a valid settings class name", settingsInstance.getClass().c_str());
			throw framework::ExceptionBadParameter(settingsInstance.getClass().c_str());
		}

		framework::attributes_t attrs = settingsInstance.getProperties();

		framework::attributes_t::iterator attrI = attrs.find(ELEMENTNAME_KEY);
		if (attrI == attrs.end())
		{
			COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
					ELEMENTNAME_KEY.c_str());
			throw framework::ExceptionBadParameter(ELEMENTNAME_KEY.c_str());
		}

		attrI = attrs.find(PARENT_KEY);
		if (attrI == attrs.end())
		{
			COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
					PARENT_KEY.c_str());
			throw framework::ExceptionBadParameter(PARENT_KEY.c_str());
		}

		attrI = attrs.find(ALLOCATIONUNITS_KEY);
		if (attrI == attrs.end())
		{
			COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
					ALLOCATIONUNITS_KEY.c_str());
			throw framework::ExceptionBadParameter(ALLOCATIONUNITS_KEY.c_str());
		}

		attrI = attrs.find(RESERVATION_KEY);
		if (attrI == attrs.end())
		{
			COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
					RESERVATION_KEY.c_str());
			throw framework::ExceptionBadParameter(RESERVATION_KEY.c_str());
		}

		attrI = attrs.find(POOLID_KEY);
		if (attrI == attrs.end())
		{
			COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
					POOLID_KEY.c_str());
			throw framework::ExceptionBadParameter(POOLID_KEY.c_str());
		}

		attrI = attrs.find(RESOURCETYPE_KEY);
		if (attrI == attrs.end())
		{
			COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
					RESOURCETYPE_KEY.c_str());
			throw framework::ExceptionBadParameter(RESOURCETYPE_KEY.c_str());
		}

		attrI = attrs.find(CHANNELINTERLEAVESIZE_KEY);
		if (attrI == attrs.end())
		{
			COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
					CHANNELINTERLEAVESIZE_KEY.c_str());
			throw framework::ExceptionBadParameter(CHANNELINTERLEAVESIZE_KEY.c_str());
		}

		attrI = attrs.find(CHANNELCOUNT_KEY);
		if (attrI == attrs.end())
		{
			COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
					CHANNELCOUNT_KEY.c_str());
			throw framework::ExceptionBadParameter(CHANNELCOUNT_KEY.c_str());
		}

		attrI = attrs.find(CONTROLLERINTERLEAVESIZE_KEY);
		if (attrI == attrs.end())
		{
			COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
					CONTROLLERINTERLEAVESIZE_KEY.c_str());
			throw framework::ExceptionBadParameter(CONTROLLERINTERLEAVESIZE_KEY.c_str());
		}

		attrI = attrs.find(REPLICATION_KEY);
		if (attrI == attrs.end())
		{
			COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
					REPLICATION_KEY.c_str());
			throw framework::ExceptionBadParameter(REPLICATION_KEY.c_str());
		}

		attrI = attrs.find(NEWMEMORYONLY_KEY);
		if (attrI == attrs.end())
		{
			COMMON_LOG_ERROR_F("expected property %s wasn't in MemoryAllocationSettings XML",
					NEWMEMORYONLY_KEY.c_str());
			throw framework::ExceptionBadParameter(NEWMEMORYONLY_KEY.c_str());
		}
	}
}

wbem::framework::UINT32 wbem::mem_config::MemoryConfigurationServiceFactory::executeMethod(
		wbem::framework::UINT32 &wbemRc,
		const std::string method,
		wbem::framework::ObjectPath &object,
		wbem::framework::attributes_t &inParms,
		wbem::framework::attributes_t &outParms)
{
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;
	wbemRc = MEMORYCONFIGURATIONSERVICE_ERR_FAILED;

	COMMON_LOG_ENTRY_PARAMS("methodName: %s, number of in params: %d", method.c_str(), (int)(inParms.size()));

	try
	{
		std::string hostName = "";
		try
		{
			hostName = wbem::server::getHostName();
		}
		catch (wbem::framework::Exception &)
		{
			// can't get the host
			httpRc = framework::CIM_ERR_FAILED;
			throw;
		}
		// verify host
		if (!validatePath(object, hostName))
		{
			// invalid object path
			httpRc = framework::CIM_ERR_INVALID_PARAMETER;
		}
		else
		{
			if (method == MEMORYCONFIGURATIONSERVICE_ALLOCATEFROMPOOL)
			{
				httpRc = executeMethodAllocateFromPool(wbemRc, object, inParms, outParms);
			}
			else if (method == MEMORYCONFIGURATIONSERVICE_EXPORTTOURI)
			{
				httpRc = executeMethodExportUri(wbemRc, object, inParms, outParms);
			}
			else if (method == MEMORYCONFIGURATIONSERVICE_IMPORTFROMURI)
			{
				httpRc = executeMethodImportFromUri(wbemRc, object, inParms, outParms);
			}
			else if (method == MEMORYCONFIGURATIONSERVICE_REMOVEGOAL)
			{
				httpRc = executeMethodRemoveGoal(wbemRc, object, inParms, outParms);
			}
			else
			{
				httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
			}
		}
	}
	catch (core::NvmExceptionPartialResultsCouldNotBeUndone &)
	{
		wbemRc = MEMORYCONFIGURATIONSERVICE_ERR_DID_NOT_COMPLETE;
	}
	catch (core::NvmExceptionProvisionCapacityNotSupported &)
	{
		wbemRc = MEMORYCONFIGURATIONSERVICE_ERR_FAILED;
	}
	catch (core::NvmExceptionUnacceptableLayoutDeviation &)
	{
		wbemRc = MEMORYCONFIGURATIONSERVICE_ERR_FAILED;
	}
	catch (core::NvmExceptionBadRequest &)
	{
		wbemRc = MEMORYCONFIGURATIONSERVICE_ERR_INVALID_PARAMETER;
	}
	catch (framework::ExceptionBadParameter &)
	{
		wbemRc = MEMORYCONFIGURATIONSERVICE_ERR_INVALID_PARAMETER;
	}
	catch (exception::NvmExceptionLibError &e)
	{
		wbemRc = getReturnCodeFromLibException(e);
	}
	catch (core::LibraryException &e)
	{
		wbemRc = MEMORYCONFIGURATIONSERVICE_ERR_FAILED;
	}
	catch (framework::Exception &)
	{
		wbemRc = MEMORYCONFIGURATIONSERVICE_ERR_FAILED;
	}

	COMMON_LOG_EXIT_RETURN("httpRc: %u, wbemRc: %u", httpRc, wbemRc);
	return httpRc;
}

wbem::framework::UINT32
wbem::mem_config::MemoryConfigurationServiceFactory::getReturnCodeFromLibException(
		const exception::NvmExceptionLibError &e)
{
	wbem::framework::UINT32 rc = framework::MOF_ERR_SUCCESS;

	switch(e.getLibError())
	{
		case NVM_ERR_NOMEMORY:
			rc = MEMORYCONFIGURATIONSERVICE_ERR_INSUFFICIENT_RESOURCES;
			break;
		default:
			rc = MEMORYCONFIGURATIONSERVICE_ERR_FAILED;
			break;
	}

	return rc;
}

void wbem::mem_config::MemoryConfigurationServiceFactory::removeGoalFromDimms(std::vector<std::string> dimmUids)
{
	bool atLeastOneRequestSucceeded = false;
	std::vector<std::string>::iterator uidIter = dimmUids.begin();
	for (; uidIter!= dimmUids.end(); uidIter++)
	{
		NVM_UID uid;
		uid_copy((*uidIter).c_str(), uid);
		int rc = m_pApi->deleteConfigGoal(uid);
		if (rc != NVM_SUCCESS && rc != NVM_ERR_NOTFOUND)
		{
			COMMON_LOG_ERROR_F("deleting config goal failed with rc = %d", rc);
			if (atLeastOneRequestSucceeded)
			{
				throw core::NvmExceptionPartialResultsCouldNotBeUndone();
			}
			else
			{
				throw wbem::exception::NvmExceptionLibError(rc);
			}
		}
		if (rc == NVM_SUCCESS)
		{
			atLeastOneRequestSucceeded = true;
		}
	}
}

bool wbem::mem_config::MemoryConfigurationServiceFactory::socketIdIsValid(NVM_UINT16 socketId)
{
	bool isValid = true;
	struct socket socket;

	if (m_pApi->getSocket(socketId, &socket) != NVM_SUCCESS)
	{
		isValid = false;
	}

	return isValid;
}

void wbem::mem_config::MemoryConfigurationServiceFactory::validateSystemProcessorRef(std::string processorRef)
{
	framework::ObjectPathBuilder builder(processorRef);
	framework::ObjectPath path;
	builder.Build(&path);

	std::string className = path.getClass();
	if (className != wbem::memory::SYSTEMPROCESSOR_CREATIONCLASSNAME)
	{
		COMMON_LOG_ERROR("Not the valid creation class name.");
		throw framework::ExceptionBadParameter("Invalid system processor reference");
	}

	std::string hostName = m_pApi->getHostName();
	std::string host = path.getHost();
	if (hostName != host)
	{
		COMMON_LOG_ERROR("Not the valid system hostname");
		throw framework::ExceptionBadParameter("Invalid system processor reference.");
	}

	std::string systemCreationClassName = path.getKeyValue(wbem::SYSTEMCREATIONCLASSNAME_KEY).stringValue();
	if (systemCreationClassName != wbem::server::BASESERVER_CREATIONCLASSNAME)
	{
		COMMON_LOG_ERROR("Not the valid system creation class name");
		throw framework::ExceptionBadParameter("Invalid system processor reference.");
	}
}

NVM_UINT16 wbem::mem_config::MemoryConfigurationServiceFactory::getSocketIdForProcessorRef
		(std::string processorRef)
{
	framework::ObjectPathBuilder builder(processorRef);
	framework::ObjectPath path;
	builder.Build(&path);

	std::string deviceIdStr = path.getKeyValue(DEVICEID_KEY).stringValue();

	if (deviceIdStr.empty())
	{
		COMMON_LOG_ERROR("System processor reference does not contain a DeviceId");
		throw framework::ExceptionBadParameter("Invalid system processor reference");
	}

	std::string socketIdStr = framework::StringUtil::removeString(
			deviceIdStr, wbem::memory::SYSTEMPROCESSOR_DEVICEID_PREFIX);

	return (NVM_UINT16)framework::StringUtil::stringToUint64(socketIdStr);
}

bool wbem::mem_config::MemoryConfigurationServiceFactory::areNewMemoryOnlySettingsAllTheSame
		(framework::STR_LIST settingsStrings)
{
	bool allTheSame = true;

	if (settingsStrings.size() > 0)
	{
		bool firstNewMemoryOnlySetting = getNewMemorySetting(settingsStrings[0]);

		for (size_t i = 0; i < settingsStrings.size(); i++)
		{
			if (getNewMemorySetting(settingsStrings[i]) != firstNewMemoryOnlySetting)
			{
				allTheSame = false;
				break;
			}
		}
	}

	return allTheSame;
}

/*
 * Validates the instance path for the MemoryPoolConfigurationService.
 */
bool wbem::mem_config::MemoryConfigurationServiceFactory::validatePath(wbem::framework::ObjectPath &path, std::string hostName)
{
	bool valid = false;

	try
	{
		// Verify the path keys contain the expected values
		if ((hostName == path.getKeyValue(wbem::SYSTEMNAME_KEY).stringValue()) &&
			(wbem::server::BASESERVER_CREATIONCLASSNAME ==
				   path.getKeyValue(wbem::SYSTEMCREATIONCLASSNAME_KEY).stringValue()) &&
			(wbem::mem_config::MEMORYCONFIGURATIONSERVICE_CREATIONCLASSNAME ==
				path.getKeyValue(wbem::CREATIONCLASSNAME_KEY).stringValue()) &&
			(hostName + wbem::mem_config::MEMORYCONFIGURATIONSERVICE_NAME ==
				path.getKeyValue(wbem::NAME_KEY).stringValue()))
		{
			valid = true;
		}
	}
	catch (wbem::framework::Exception &)
	{
		valid = false;
	}

	return valid;
}


void wbem::mem_config::MemoryConfigurationServiceFactory::validatePool(std::string poolRef)
	throw (framework::Exception)
{
	// Turn the ref into an object path
	framework::ObjectPathBuilder pathBuilder(poolRef);
	framework::ObjectPath poolPath;
	if (pathBuilder.Build(&poolPath)) // successfully built the path
	{
		// get the instance
		MemoryResourcesFactory poolFactory;
		framework::attribute_names_t attributes; // all attributes
		framework::Instance *pInstance = poolFactory.getInstance(poolPath, attributes);
		if (!pInstance) // not found
		{
			COMMON_LOG_ERROR_F("'%s' is not a MemoryResources instance", poolRef.c_str());
			throw framework::ExceptionBadParameter(MEMORYCONFIGURATIONSERVICE_PARENTPOOL.c_str());
		}
	}
	else
	{
		COMMON_LOG_ERROR_F("parameter '%s' was not a valid object path: %s", MEMORYCONFIGURATIONSERVICE_PARENTPOOL.c_str(),
				poolRef.c_str());
		throw framework::ExceptionBadParameter(MEMORYCONFIGURATIONSERVICE_PARENTPOOL.c_str());
	}
}

/*
 * Parses CIM XML embedded instances, validates they are all MemoryAllocationSettings instances,
 * and harvests/validates their parameters.
 */
void wbem::mem_config::MemoryConfigurationServiceFactory::settingsStringsToRequestedExtents(
		const framework::STR_LIST &settingsStrings,
		NVM_UINT64 &memoryCapacity,
		std::vector<struct core::memory_allocator::AppDirectExtent> &appDirectCapacities)
	throw (wbem::framework::Exception)
{
	memoryCapacity  = 0;
	appDirectCapacities.clear();

	// If settings are empty, don't bother
	if (settingsStrings.empty())
	{
		COMMON_LOG_ERROR("Empty Settings list");
		throw framework::ExceptionBadParameter(MEMORYCONFIGURATIONSERVICE_SETTINGS.c_str());
	}

	// Parse the CIM XML strings
	for (std::vector<std::string>::const_iterator iter = settingsStrings.begin(); iter != settingsStrings.end(); iter++)
	{
		framework::CimXml settingsInstance(*iter);

		// Harvest and validate the attributes
		framework::attributes_t settingsAttrs = settingsInstance.getProperties();

		// Reservation (capacity of the region) - arrives in bytes
		NVM_UINT64 reservationGiB = 0;
		framework::attributes_t::iterator attrI = settingsAttrs.find(RESERVATION_KEY);
		if (attrI != settingsAttrs.end())
		{
			reservationGiB = (attrI->second.uint64Value() / BYTES_PER_GB);
		}

		// ResourceType
		NVM_UINT16 resourceType = 0;
		attrI = settingsAttrs.find(RESOURCETYPE_KEY);
		if (attrI != settingsAttrs.end())
		{
			resourceType = attrI->second.uintValue();
		}

		// ChannelInterleaveSize
		NVM_UINT16 channelInterleaveSize = 0;
		attrI = settingsAttrs.find(CHANNELINTERLEAVESIZE_KEY);
		if (attrI != settingsAttrs.end())
		{
			channelInterleaveSize = attrI->second.uintValue();
		}

		// ChannelCount
		NVM_UINT16 channelCount = 0;
		attrI = settingsAttrs.find(CHANNELCOUNT_KEY);
		if (attrI != settingsAttrs.end())
		{
			channelCount = attrI->second.uintValue();
		}

		// ControllerInterleaveSize
		NVM_UINT16 controllerInterleaveSize = 0;
		attrI = settingsAttrs.find(CONTROLLERINTERLEAVESIZE_KEY);
		if (attrI != settingsAttrs.end())
		{
			controllerInterleaveSize = attrI->second.uintValue();
		}

		// Replication
		NVM_UINT16 replication = 0;
		attrI = settingsAttrs.find(REPLICATION_KEY);
		if (attrI != settingsAttrs.end())
		{
			replication = attrI->second.uintValue();
		}

		if (resourceType == MEMORYALLOCATIONSETTINGS_RESOURCETYPE_MEMORY)
		{
			memoryCapacity += reservationGiB;
		}
		else if (resourceType == MEMORYALLOCATIONSETTINGS_RESOURCETYPE_NONVOLATILE)
		{
			struct core::memory_allocator::AppDirectExtent appDirectExtent;
			appDirectExtent.capacity = reservationGiB;
			appDirectExtent.channel = InterleaveSet::getInterleaveSizeFromExponent(channelInterleaveSize);
			appDirectExtent.imc = InterleaveSet::getInterleaveSizeFromExponent(controllerInterleaveSize);
			appDirectExtent.byOne = (channelCount == INTERLEAVE_WAYS_1);
			appDirectExtent.mirrored = (replication == MEMORYALLOCATIONSETTINGS_REPLICATION_LOCAL);
			appDirectCapacities.push_back(appDirectExtent);
		}
	}
}

// ------------------------------------------------------------------------------------------------
// EXTRINSIC METHODS
// Note: these extrinsic methods deviate from the CIM schema in that they throw exceptions instead
//		 of returning error codes.  The out parameters are returned instead.
// ------------------------------------------------------------------------------------------------

/*
 * Detect whether the DIMM has a config goal.
 */
bool wbem::mem_config::MemoryConfigurationServiceFactory::dimmHasGoal(const NVM_UID dimmUid)
{
	bool hasGoal = false;

	NVM_UID uidStr;
	uid_copy(dimmUid, uidStr);

	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();
	try
	{
		struct config_goal goal;
		pApi->getConfigGoalForDimm(uidStr, goal);

		// Goals that have been successfully applied are no longer goals
		// for the purposes of this view
		if (goal.status != CONFIG_GOAL_STATUS_SUCCESS)
		{
			hasGoal = true;
		}
	}
	catch (exception::NvmExceptionLibError &e)
	{
		int errorCode = e.getLibError();
		if (errorCode != NVM_ERR_NOTMANAGEABLE && errorCode != NVM_ERR_NOTFOUND)
		{
			COMMON_LOG_ERROR_F("Couldn't get config goal for DIMM %s, error %d",
			                   uidStr, errorCode);
			throw;
		}
	}

	return hasGoal;
}

/*
 * This method deletes a config goal from a dimm.
 */
wbem::framework::Instance* wbem::mem_config::MemoryConfigurationServiceFactory::deleteInstance(
					wbem::framework::ObjectPath &path) throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pInstance = NULL;

	framework::Attribute idAttr = path.getKeyValue(INSTANCEID_KEY);
	std::string instanceId = idAttr.stringValue();

	// We expect the the instanceId to be the device uid as a string + either a 'C' or 'G'.
	// So its length should be the same as NVM_MAX_UID_LEN because the 'C' or 'G' character
	// will take the place of the null terminator.
	if (!MemoryConfigurationFactory::isValidInstanceId(instanceId))
	{
		throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
	}

	try 
	{
		wbem::mem_config::MemoryConfigurationFactory provider;
		wbem::framework::attribute_names_t attributes;
		pInstance = provider.getInstance(path, attributes);
	}
	catch (wbem::framework::Exception &e)
	{
		COMMON_LOG_ERROR_F("Couldn't retrieve given instance for deleting. Error = %s.", e.what());
		throw;
	}
	
	std::string deviceUid = instanceId.substr(0, instanceId.length() - 1);

	NVM_UID uid;
	uid_copy(deviceUid.c_str(), uid);

	int rc = m_DeleteConfigGoalProvider(uid);
	if (rc != NVM_SUCCESS)
	{
		delete pInstance;
		throw exception::NvmExceptionLibError(rc);
	}
	return pInstance;
}

void wbem::mem_config::MemoryConfigurationServiceFactory::uriToPath(
		const std::string &uriParamName, const std::string &uri,
		std::string &path, bool check_exists) const
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// make sure the URI isn't empty
	if (uri.empty())
	{
		COMMON_LOG_ERROR("Uri is empty");
		throw framework::ExceptionBadParameter(uriParamName.c_str());
	}
	// convert it to an absolute path
	COMMON_PATH absPath;
	if (get_absolute_path(uri.c_str(), uri.length() + 1, absPath) != COMMON_SUCCESS)
	{
		COMMON_LOG_ERROR("Uri to absolute path failed");
		throw framework::ExceptionBadParameter(uriParamName.c_str());
	}
	// optionally check if it exists
	if (check_exists && !file_exists(absPath, COMMON_PATH_LEN))
	{
		COMMON_LOG_ERROR_F("Uri %s does not exist", absPath);
		throw framework::ExceptionBadParameter(uriParamName.c_str());
	}
	path = absPath;
}

/*!
 * Store the memory allocation settings
 * for all manageable NVM-DIMMs within a platform in the URI specified.
 */
void wbem::mem_config::MemoryConfigurationServiceFactory::exportSystemConfigToUri(
		const std::string &exportUri) throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string path;
	uriToPath(MEMORYCONFIGURATIONSERVICE_EXPORTURI, exportUri, path, false);

	exportSystemConfigToPath(path);
}

/*!
 * Store the memory allocation settings
 * for all manageable NVM-DIMMs within a platform in the filesystem path specified.
 */
void wbem::mem_config::MemoryConfigurationServiceFactory::exportSystemConfigToPath(
		const std::string &path) throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// iterate over all manageable dimms
	size_t unconfigured_dimm_count = 0;
	bool append = false;
	std::vector<std::string> dimms = physical_asset::NVDIMMFactory::getManageableDeviceUids();
	for (std::vector<std::string>::const_iterator iter = dimms.begin();
			iter != dimms.end(); iter++)
	{
		NVM_UID uid;
		uid_copy((*iter).c_str(), uid);
		int rc = nvm_dump_config(uid, path.c_str(), path.length() + 1, append);
		if (rc != NVM_SUCCESS)
		{
			if (rc == NVM_ERR_NOTFOUND)
			{
				unconfigured_dimm_count++;
			}
			else
			{
				throw exception::NvmExceptionLibError(rc);
			}
		}
		append = true;
	}

	// throw exception only if all dimms are unconfigured
	if (unconfigured_dimm_count == dimms.size())
	{
		throw exception::NvmExceptionLibError(NVM_ERR_NOTFOUND);
	}
}

/*!
 * Load configuration goal from the specified file URI onto the specified dimms
 */
void wbem::mem_config::MemoryConfigurationServiceFactory::importDimmConfigsFromURI(
		const std::string &importUri, std::vector<std::string> dimms)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string path;
	uriToPath(MEMORYCONFIGURATIONSERVICE_IMPORTURI, importUri, path, true);

	importDimmConfigsFromPath(path, dimms);
}

/*!
 * Load configuration goal from the specified file path onto the specified dimms
 */
void wbem::mem_config::MemoryConfigurationServiceFactory::importDimmConfigsFromPath(
		const std::string &path, std::vector<std::string> dimms)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	validateDimmList(dimms);

	// for each dimm, load the goal
	for (std::vector<std::string>::const_iterator dimmIter = dimms.begin();
			dimmIter != dimms.end(); dimmIter++)
	{
		NVM_UID uid;
		uid_copy((*dimmIter).c_str(), uid);
		int rc = m_pApi->loadConfig(uid, path.c_str(), path.length() + 1);
		// revert all only any failure
		if (rc != NVM_SUCCESS)
		{
			removeGoalFromDimms(dimms);
			throw exception::NvmExceptionLibError(rc);
		}
	}
}

void wbem::mem_config::MemoryConfigurationServiceFactory::validateDimmList(
		const std::vector<std::string> dimmUids)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	core::memory_allocator::MemoryAllocationRequest request;

	for (std::vector<std::string>::const_iterator uidIter = dimmUids.begin();
			uidIter != dimmUids.end(); uidIter++)
	{
		struct device_discovery discovery;
		memset(&discovery, 0, sizeof (discovery));
		m_pApi->getDeviceDiscoveryForDimm(*uidIter, discovery);

		request.dimms.push_back(core::memory_allocator::MemoryAllocationUtil::deviceDiscoveryToDimm(discovery));
	}

	std::vector<struct device_discovery> manageableDevices;
	m_pApi->getManageableDimms(manageableDevices);
	core::memory_allocator::RulePartialSocketConfigured dimmListValidationRule(manageableDevices, core::NvmLibrary::getNvmLibrary());
	dimmListValidationRule.verify(request);
}

std::vector<std::string>
	wbem::mem_config::MemoryConfigurationServiceFactory::getManageableDimmIDsForSocket(NVM_UINT32 socketId)
{
	std::vector<std::string> dimmIDs;

	wbem::physical_asset::devices_t devices = wbem::physical_asset::NVDIMMFactory::getManageableDevices();

	std::vector<struct device_discovery>::iterator iter = devices.begin();
	for (; iter != devices.end(); iter++)
	{
		if (((*iter).socket_id == socketId))
		{
			NVM_UID uidStr;
			uid_copy((*iter).uid, uidStr);
			dimmIDs.push_back(std::string(uidStr));
		}
	}

	return dimmIDs;
}

core::memory_allocator::MemoryAllocationRequest
wbem::mem_config::MemoryConfigurationServiceFactory::memAllocSettingsToRequest(
		const framework::STR_LIST &memoryAllocationSettings)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	core::memory_allocator::MemoryAllocationRequest request;

	getDimmsForMemAllocSettings(memoryAllocationSettings, request.dimms);
	settingsStringsToRequestedExtents(memoryAllocationSettings,
			request.memoryCapacity, request.appDirectExtents);
	request.reserveDimm = false;
	request.storageRemaining = true;

	return request;
}

wbem::framework::UINT32 wbem::mem_config::MemoryConfigurationServiceFactory::executeMethodAllocateFromPool(
	wbem::framework::UINT32 &wbemRc, wbem::framework::ObjectPath &object,
	wbem::framework::attributes_t &inParms, wbem::framework::attributes_t &outParms)
{
	// Validate Pool attribute
	std::string poolRef = inParms[MEMORYCONFIGURATIONSERVICE_PARENTPOOL].stringValue();

	// Generate representation of MemoryAllocationSettings instances from CIM XML strings
	const framework::Attribute &settings = inParms[MEMORYCONFIGURATIONSERVICE_SETTINGS];
	NVM_UINT32 httpRc = framework::MOF_ERR_SUCCESS;

	if (settings.getType() != framework::STR_LIST_T)
	{
		throw framework::ExceptionBadParameter(MEMORYCONFIGURATIONSERVICE_SETTINGS.c_str());
	}
	framework::STR_LIST settingsStrings = settings.strListValue();

	validateSettingsStrings(settingsStrings);
	validatePool(poolRef);

	// We'll take all the settings that go on a socket and use them to call AllocateFromPool
	while (!settingsStrings.empty())
	{
		framework::STR_LIST socketSettings;

		// grab the socketId of the first setting in the list
		NVM_UINT16 socketId = getSocketIdForSettingsString(settingsStrings.front());
		socketSettings = getSettingsStringsForSocket(settingsStrings, socketId);
		removeSettingsWithSocketId(settingsStrings, socketId);

		if (!areNewMemoryOnlySettingsAllTheSame(socketSettings))
		{
			COMMON_LOG_ERROR("Settings strings mix NewMemoryOnly and existing memory");
			throw framework::ExceptionNotSupported(__FILE__, (char *) __func__);
		}

		core::memory_allocator::MemoryAllocationRequest request = memAllocSettingsToRequest(socketSettings);
		core::memory_allocator::MemoryAllocator *pAllocator = NULL;
		try
		{
			core::memory_allocator::MemoryAllocator *pAllocator =
					core::memory_allocator::MemoryAllocator::getNewMemoryAllocator();
			core::memory_allocator::MemoryAllocationLayout layout = pAllocator->layout(request);
			pAllocator->allocate(layout);

			delete pAllocator;
		}
		catch (framework::Exception &)
		{
			if (pAllocator)
			{
				delete pAllocator;
			}
			throw;
		}
	}

	// If we got here, the configuration succeeded
	wbemRc = MEMORYCONFIGURATIONSERVICE_ERR_SUCCESSFULLY_STAGED_FOR_FUTURE;
	return httpRc;
}

wbem::framework::UINT32 wbem::mem_config::MemoryConfigurationServiceFactory::executeMethodExportUri(
	wbem::framework::UINT32 &wbemRc, wbem::framework::ObjectPath &object,
	wbem::framework::attributes_t &inParms, wbem::framework::attributes_t &outParms)
{
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;
	std::string exportFilePath = inParms[MEMORYCONFIGURATIONSERVICE_EXPORTURI].stringValue();
	if (exportFilePath.empty())
	{
		COMMON_LOG_ERROR("ExportToUri requires a URI");
		httpRc = framework::CIM_ERR_INVALID_PARAMETER;
	}
	else
	{
		// call ExportToUri
		exportSystemConfigToUri(exportFilePath);
		wbemRc = framework::MOF_ERR_SUCCESS;
	}
	return httpRc;
}

wbem::framework::UINT32 wbem::mem_config::MemoryConfigurationServiceFactory::executeMethodImportFromUri(
	wbem::framework::UINT32 &wbemRc, wbem::framework::ObjectPath &object,
	wbem::framework::attributes_t &inParms, wbem::framework::attributes_t &outParms)
{
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;

	// check for a URI
	std::string importFilePath = inParms[MEMORYCONFIGURATIONSERVICE_IMPORTURI].stringValue();

	if (importFilePath.empty())
	{
		COMMON_LOG_ERROR("ImportFromUri requires a URI");
		httpRc = framework::CIM_ERR_INVALID_PARAMETER;
		throw framework::ExceptionBadParameter(MEMORYCONFIGURATIONSERVICE_IMPORTURI.c_str());
	}

	// check for a target list
	framework::STR_LIST dimmRefs =
		inParms[MEMORYCONFIGURATIONSERVICE_TARGETS].strListValue();
	if (dimmRefs.empty())
	{
		COMMON_LOG_ERROR("ImportFromUri requires at least one DIMM");
		httpRc = framework::CIM_ERR_INVALID_PARAMETER;
		throw framework::ExceptionBadParameter(MEMORYCONFIGURATIONSERVICE_TARGETS.c_str());
	}

	// convert target object path strings to dimm uids
	framework::STR_LIST dimmUids;
	for (framework::STR_LIST::const_iterator iter = dimmRefs.begin();
		 iter!= dimmRefs.end(); iter++)
	{
		std::string uidStr;
		// Build the object path from the attribute
		framework::ObjectPathBuilder builder(*iter);
		framework::ObjectPath objPath;
		if (!builder.Build(&objPath))
		{
			COMMON_LOG_ERROR_F("Bad NVDIMM object path %s", (*iter).c_str());
			httpRc = framework::CIM_ERR_INVALID_PARAMETER;
			throw framework::ExceptionBadParameter(MEMORYCONFIGURATIONSERVICE_TARGETS.c_str());
		}
		try
		{
			// make sure the object path is for NVDIMM class
			if (objPath.getClass() != physical_asset::NVDIMM_CREATIONCLASSNAME)
			{
				COMMON_LOG_ERROR_F("Not an NVDIMM object path %s", (*iter).c_str());
				throw framework::ExceptionBadParameter(MEMORYCONFIGURATIONSERVICE_TARGETS.c_str());
			}

			// make sure it contains a UID and the UID is valid
			uidStr = objPath.getKeyValue(TAG_KEY).stringValue();
			if (!core::device::isUidValid(uidStr))
			{
				COMMON_LOG_ERROR_F("Bad NVDIMM object path %s", (*iter).c_str());
				throw framework::ExceptionBadParameter(MEMORYCONFIGURATIONSERVICE_TARGETS.c_str());
			}
			dimmUids.push_back(uidStr);
		}
		catch (framework::Exception &)
		{
			COMMON_LOG_ERROR_F("Invalid NVDIMM object path %s", (*iter).c_str());
			httpRc = framework::CIM_ERR_INVALID_PARAMETER;
			throw;
		}
	}
	importDimmConfigsFromURI(importFilePath, dimmUids);
	wbemRc = framework::MOF_ERR_SUCCESS;
	return httpRc;
}

wbem::framework::UINT32 wbem::mem_config::MemoryConfigurationServiceFactory::executeMethodRemoveGoal(
	wbem::framework::UINT32 &wbemRc, wbem::framework::ObjectPath &object,
	wbem::framework::attributes_t &inParms, wbem::framework::attributes_t &outParms)
{
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;
	wbem::framework::attributes_t::iterator iter = inParms.begin();

	std::string ref = inParms[MEMORYCONFIGURATIONSERVICE_SYSTEMPROCESSOR].stringValue();

	std::vector<std::string> dimmUids;
	if (ref.empty())
	{
		dimmUids = wbem::physical_asset::NVDIMMFactory::getManageableDeviceUids();
	}
	else
	{
		validateSystemProcessorRef(ref);
		NVM_UINT16 socketId = getSocketIdForProcessorRef(ref);

		if (socketIdIsValid(socketId))
		{
			dimmUids = getManageableDimmIDsForSocket(socketId);
		}
		else
		{
			COMMON_LOG_ERROR_F("SocketId %d is not valid", socketId);
			throw wbem::framework::ExceptionBadParameter("Invalid DeviceId");
		}
	}

	removeGoalFromDimms(dimmUids);

	wbemRc = framework::MOF_ERR_SUCCESS;
	return httpRc;
}
