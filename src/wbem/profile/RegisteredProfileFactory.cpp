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
 * This file contains the provider for the RegisteredProfile instances.
 */

#include "RegisteredProfileFactory.h"
#include <server/BaseServerFactory.h>
#include <memory/VolatileMemoryFactory.h>
#include <memory/PersistentMemoryFactory.h>
#include <libinvm-cim/ExceptionBadAttribute.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <framework_interface/NvmAssociationFactory.h>
#include <LogEnterExit.h>
#include <algorithm>

wbem::profile::RegisteredProfileFactory::RegisteredProfileFactory()
{
	buildProfileInfoMap();
}

void wbem::profile::RegisteredProfileFactory::buildProfileInfoMap()
{
	// Set up the information for all the static instances

	// AdvertiseTypes should be the same for all instances
	framework::UINT16_LIST defaultAdvertiseTypes;
	defaultAdvertiseTypes.push_back(REGISTEREDPROFILE_ADVERTISETYPES_NOTADVERTISED);

	// Base Server
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_BASESERVER].registeredName = REGISTEREDPROFILE_REGISTEREDNAME_BASESERVER;
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_BASESERVER].version = REGISTEREDPROFILE_VERSION_BASESERVER;
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_BASESERVER].registeredOrg = REGISTEREDPROFILE_REGISTEREDORG_DMTF;
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_BASESERVER].otherRegisteredOrg = "";
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_BASESERVER].advertiseTypes = defaultAdvertiseTypes;

	// Software Inventory
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_SWINVENTORY].registeredName = REGISTEREDPROFILE_REGISTEREDNAME_SWINVENTORY;
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_SWINVENTORY].version = REGISTEREDPROFILE_VERSION_SWINVENTORY;
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_SWINVENTORY].registeredOrg = REGISTEREDPROFILE_REGISTEREDORG_DMTF;
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_SWINVENTORY].otherRegisteredOrg = "";
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_SWINVENTORY].advertiseTypes = defaultAdvertiseTypes;

	// Multi-type System Memory
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_MULTITYPESYSTEMMEMORY].registeredName =
			REGISTEREDPROFILE_REGISTEREDNAME_MULTITYPESYSTEMMEMORY;
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_MULTITYPESYSTEMMEMORY].version = REGISTEREDPROFILE_VERSION_MULTITYPESYSTEMMEMORY;
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_MULTITYPESYSTEMMEMORY].registeredOrg = REGISTEREDPROFILE_REGISTEREDORG_DMTF;
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_MULTITYPESYSTEMMEMORY].otherRegisteredOrg = "";
	m_profileInfoMap[REGISTEREDPROFILE_REGISTEREDNAME_MULTITYPESYSTEMMEMORY].advertiseTypes = defaultAdvertiseTypes;
}

wbem::profile::RegisteredProfileFactory::~RegisteredProfileFactory()
{
}

void wbem::profile::RegisteredProfileFactory::populateAttributeList(framework::attribute_names_t& attributes)
		throw (framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(REGISTEREDNAME_KEY);
	attributes.push_back(REGISTEREDVERSION_KEY);
	attributes.push_back(REGISTEREDORGANIZATION_KEY);
	attributes.push_back(OTHERREGISTEREDORGANIZATION_KEY);
	attributes.push_back(ADVERTISETYPES_KEY);
}

wbem::framework::Instance* wbem::profile::RegisteredProfileFactory::getInstance(framework::ObjectPath& path,
		framework::attribute_names_t& attributes) throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	checkAttributes(attributes);

	framework::Instance *pInstance = new framework::Instance(path);
	if (!pInstance)
	{
		throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__,
				"instance for RegisteredProfile");
	}

	try
	{
		buildInstanceFromProfileInfoMap(*pInstance, attributes);
	}
	catch (framework::Exception &)
	{
		delete pInstance;
		throw;
	}

	return pInstance;
}

void wbem::profile::RegisteredProfileFactory::buildInstanceFromProfileInfoMap(framework::Instance& instance,
		const framework::attribute_names_t& attributes) throw (framework::Exception)
{
	framework::Attribute instanceIdAttr;
	if (instance.getAttribute(INSTANCEID_KEY, instanceIdAttr) != framework::SUCCESS)
	{
		COMMON_LOG_ERROR_F("couldn't get key '%s'", INSTANCEID_KEY.c_str());
		throw framework::ExceptionBadAttribute(INSTANCEID_KEY.c_str());
	}

	// Make sure the InstanceID is in our map
	std::string instanceId = instanceIdAttr.stringValue(); // hostname + registeredname 
	std::string hostName = server::getHostName();

	if ((instanceId.size() >  hostName.size()) &&
		(instanceId.substr(0, hostName.size()) == hostName))
	{ // remove hostname from instanceid for comparision
		instanceId = instanceId.substr(hostName.length());
	}

	std::map<std::string, struct ProfileInfo>::iterator profileIter = m_profileInfoMap.find(instanceId);
	if (profileIter == m_profileInfoMap.end()) // not found!
	{
		COMMON_LOG_ERROR_F("value of key '%s' (%s) doesn't match a real instance",
				INSTANCEID_KEY.c_str(),
				instanceId.c_str());
		throw framework::ExceptionBadAttribute(INSTANCEID_KEY.c_str());
	}

	buildInstanceFromProfileInfo(instance, attributes, profileIter->second);
}

void wbem::profile::RegisteredProfileFactory::buildInstanceFromProfileInfo(framework::Instance& instance,
		const framework::attribute_names_t& attributes, const struct ProfileInfo& info)
{
	// RegisteredName - string
	if (containsAttribute(REGISTEREDNAME_KEY, attributes))
	{
		framework::Attribute attr(info.registeredName, false);
		instance.setAttribute(REGISTEREDNAME_KEY, attr);
	}

	// RegisteredVersion - string
	if (containsAttribute(REGISTEREDVERSION_KEY, attributes))
	{
		framework::Attribute attr(info.version, false);
		instance.setAttribute(REGISTEREDVERSION_KEY, attr);
	}

	// RegisteredOrganization - uint16
	if (containsAttribute(REGISTEREDORGANIZATION_KEY, attributes))
	{
		framework::Attribute attr(info.registeredOrg, false);
		instance.setAttribute(REGISTEREDORGANIZATION_KEY, attr);
	}

	// OtherRegisteredOrganization - string
	if (containsAttribute(OTHERREGISTEREDORGANIZATION_KEY, attributes))
	{
		framework::Attribute attr(info.otherRegisteredOrg, false);
		instance.setAttribute(OTHERREGISTEREDORGANIZATION_KEY, attr);
	}

	// AdvertiseTypes - uint16[]
	if (containsAttribute(ADVERTISETYPES_KEY, attributes))
	{
		framework::Attribute attr(info.advertiseTypes, false);
		instance.setAttribute(ADVERTISETYPES_KEY, attr);
	}
}

wbem::framework::instance_names_t* wbem::profile::RegisteredProfileFactory::getInstanceNames()
		throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::instance_names_t *pNames = new framework::instance_names_t();
	if (!pNames)
	{
		throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__,
				"instance names for RegisteredProfile");
	}

	try
	{
		std::string hostName = server::getHostName();

		// InstanceIDs are the map keys
		for (std::map<std::string, struct ProfileInfo>::const_iterator iter = m_profileInfoMap.begin();
				iter != m_profileInfoMap.end(); iter++)
		{
			const std::string &instanceId = hostName + iter->first;
			framework::attributes_t keys;
			keys[INSTANCEID_KEY] = framework::Attribute(instanceId, true);

			pNames->push_back(framework::ObjectPath(hostName, REGISTEREDPROFILE_NAMESPACE,
					REGISTEREDPROFILE_CREATIONCLASSNAME, keys));
		}
	}
	catch (framework::Exception &)
	{
		delete pNames;
		throw;
	}

	return pNames;
}

bool wbem::profile::RegisteredProfileFactory::isAssociated(const std::string& associationClass,
		framework::Instance* pAntInstance, framework::Instance* pDepInstance)
{
	bool result = true;

	// ElementConformsToProfile
	if (associationClass == wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTCONFORMSTOPROFILE)
	{
		result = isElementConformsToProfileAssociation(*pAntInstance, *pDepInstance);
	}

	return result;
}

bool wbem::profile::RegisteredProfileFactory::isElementConformsToProfileAssociation(
		const framework::Instance& antecedentInstance,
		const framework::Instance& dependentInstance)
	throw (framework::Exception)
{
	bool result = false;

	if (antecedentInstance.getClass() == REGISTEREDPROFILE_CREATIONCLASSNAME)
	{
		framework::Attribute profileInstanceIdAttr;
		if (antecedentInstance.getAttribute(INSTANCEID_KEY, profileInstanceIdAttr) != framework::SUCCESS)
		{
			COMMON_LOG_ERROR("couldn't get InstanceID for RegisteredProfile instance");
			throw framework::ExceptionBadAttribute(INSTANCEID_KEY.c_str());
		}

		std::string profileInstanceId = profileInstanceIdAttr.stringValue();
		std::string dependentClass = dependentInstance.getClass();

		std::string hostName = server::getHostName();
		std::string baseServerInstanceId = hostName + REGISTEREDPROFILE_REGISTEREDNAME_BASESERVER;
		std::string multiSystemMemoryInstanceId = hostName + REGISTEREDPROFILE_REGISTEREDNAME_MULTITYPESYSTEMMEMORY;
		// association - Base Server profile and Intel_BaseServer
		if (profileInstanceId.compare(baseServerInstanceId) == 0)
		{
			if (dependentClass == server::BASESERVER_CREATIONCLASSNAME)
			{
				result = true;
			}
		}
		// association - Multi-type System Memory profile and Intel_VolatileMemory/Intel_PersistentMemory
		else if (profileInstanceId.compare(multiSystemMemoryInstanceId) == 0)
		{
			if (dependentClass == memory::VOLATILEMEMORY_CREATIONCLASSNAME ||
				dependentClass == memory::PERSISTENTMEMORY_CREATIONCLASSNAME)
			{
				result = true;
			}
		}
	}

	return result;
}
