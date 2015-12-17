/*
 * Copyright (c) 2015, Intel Corporation
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
 * This file contains the provider for the NVDIMMFWVersion instances
 * which represents the version of the FW on an NVM DIMM.
 */

#include <LogEnterExit.h>
#include <nvm_management.h>
#include <string/revision.h>
#include <guid/guid.h>
#include <algorithm>
#include <server/BaseServerFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include "NVDIMMCollectionFactory.h"
#include "NVDIMMFWVersionFactory.h"
#include <framework_interface/NvmAssociationFactory.h>
#include <NvmStrings.h>

wbem::software::NVDIMMFWVersionFactory::NVDIMMFWVersionFactory()
throw (wbem::framework::Exception)
{ }

wbem::software::NVDIMMFWVersionFactory::~NVDIMMFWVersionFactory()
{ }

void wbem::software::NVDIMMFWVersionFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);


	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(MAJORVERSION_KEY);
	attributes.push_back(MINORVERSION_KEY);
	attributes.push_back(REVISIONNUMBER_KEY);
	attributes.push_back(BUILDNUMBER_KEY);
	attributes.push_back(VERSIONSTRING_KEY);
	attributes.push_back(CLASSIFICATIONS_KEY);
	attributes.push_back(SPECIFICATION_KEY);
	attributes.push_back(ISENTITY_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::software::NVDIMMFWVersionFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		checkPath(path);
		checkAttributes(attributes);

		std::string instanceId = path.getKeyValue(INSTANCEID_KEY).stringValue();

		COMMON_LOG_DEBUG_F("instanceID = %s", instanceId.c_str());

		// InstanceID has the form "NVDIMMFW xx.xx.xx.xxxx-x.x"
		// to get the fw version we lop off "NVDIMMFW " and then
		// take everything up to the "-"
		// to get the fwApiVersion we lop off everything up to and including
		// the dash

		// get rid of prefix
		instanceId.erase(0, NVDIMMFWVERSION_INSTANCEID_PREFIX.length());

		std::string fwVersion = instanceId.substr(0, instanceId.find("-"));
		std::string fwApiVersion = instanceId.substr(instanceId.find("-") + 1, instanceId.length());

		short unsigned int major, minor, hotfix, build;
		parse_main_revision(&major, &minor, &hotfix, &build, fwVersion.c_str(), NVM_VERSION_LEN);

		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			framework::Attribute a(std::string(
					"NVDIMM FW Version ") + fwVersion + std::string("-") + fwApiVersion, false);
			pInstance->setAttribute(ELEMENTNAME_KEY, a, attributes);
		}
		if (containsAttribute(MAJORVERSION_KEY, attributes))
		{

			framework::Attribute a(major, false);
			pInstance->setAttribute(MAJORVERSION_KEY, a, attributes);
		}
		if (containsAttribute(MINORVERSION_KEY, attributes))
		{
			framework::Attribute a(minor, false);
			pInstance->setAttribute(MINORVERSION_KEY, a, attributes);
		}
		if (containsAttribute(REVISIONNUMBER_KEY, attributes))
		{
			framework::Attribute a(hotfix, false);
			pInstance->setAttribute(REVISIONNUMBER_KEY, a, attributes);
		}
		if (containsAttribute(BUILDNUMBER_KEY, attributes))
		{
			framework::Attribute a(build, false);
			pInstance->setAttribute(BUILDNUMBER_KEY, a, attributes);
		}
		if (containsAttribute(VERSIONSTRING_KEY, attributes))
		{
			framework::Attribute a(fwVersion, false);
			pInstance->setAttribute(VERSIONSTRING_KEY, a, attributes);
		}
		if (containsAttribute(CLASSIFICATIONS_KEY, attributes))
		{
			framework::UINT16_LIST classifications;
			classifications.push_back(NVDIMMFWVERSION_CLASSIFICATIONS_FW);
			framework::Attribute a(classifications, false);
			pInstance->setAttribute(CLASSIFICATIONS_KEY, a, attributes);
		}
		if (containsAttribute(SPECIFICATION_KEY, attributes))
		{
			framework::Attribute a(fwApiVersion, false);
			pInstance->setAttribute(SPECIFICATION_KEY, a, attributes);
		}
		// IsEntity = true
		if (containsAttribute(ISENTITY_KEY, attributes))
		{
			framework::Attribute a(true, false);
			pInstance->setAttribute(ISENTITY_KEY, a, attributes);
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
 * Return the object paths for the NVDIMMFWVersion class.
 */
wbem::framework::instance_names_t* wbem::software::NVDIMMFWVersionFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		std::string hostServer = wbem::server::getHostName();

		physical_asset::devices_t devices = physical_asset::NVDIMMFactory::getAllDevices();

		for (size_t i = 0; i < devices.size(); i++)
		{
			framework::attributes_t keys;

			// InstanceID
			std::string fwVersion = std::string(devices[i].fw_revision);
			std::string fwApiVersion = std::string(devices[i].fw_api_version);
			keys[INSTANCEID_KEY] = framework::Attribute(
					NVDIMMFWVERSION_INSTANCEID_PREFIX + fwVersion + "-" + fwApiVersion, true);

			framework::ObjectPath path(hostServer, NVM_NAMESPACE,
					NVDIMMFWVERSION_CREATIONCLASSNAME, keys);

			if (std::find(pNames->begin(), pNames->end(), path) == pNames->end())
			{
				pNames->push_back(path);
			}
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		if (pNames != NULL)
		{
			delete pNames;
		}
		throw;
	}

	return pNames;
}

/*
 *  Determine if complex association.
 *   
 */
bool wbem::software::NVDIMMFWVersionFactory::isAssociated(
                const std::string &associationClass,
                framework::Instance* pAntInstance,
                framework::Instance* pDepInstance)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool result = false;

	if (associationClass == wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTSOFTWAREIDENTITY)
	{
		try
		{
			if ((pAntInstance->getClass() == wbem::software::NVDIMMFWVERSION_CREATIONCLASSNAME) &&
			    (pDepInstance->getClass() == wbem::software::NVDIMMCOLLECTION_CREATIONCLASSNAME))
			{
				physical_asset::devices_t devices = physical_asset::NVDIMMFactory::getAllDevices();
				if (devices.size() > 0)
				{
					std::string fwVersion = devices[0].fw_revision;
					result = true;
					for (size_t i = 1; i < devices.size(); i++)
					{
						std::string currentFwVersion = devices[i].fw_revision;
						if (fwVersion != currentFwVersion)
						{
							result = false;
							break;
						}
					}
				}
			}
			else if ((pAntInstance->getClass() == wbem::software::NVDIMMFWVERSION_CREATIONCLASSNAME) &&
				(pDepInstance->getClass() == wbem::physical_asset::NVDIMM_CREATIONCLASSNAME))
			{
				int rc = NVM_SUCCESS;
				NVM_GUID guid;
				struct device_discovery device;
				framework::Attribute attr;

				pAntInstance->getAttribute(wbem::VERSIONSTRING_KEY, attr);
				std::string fwVersion = attr.stringValue();

				pAntInstance->getAttribute(wbem::SPECIFICATION_KEY, attr);
				std::string fwApiVersion = attr.stringValue();

				pDepInstance->getAttribute(TAG_KEY, attr);
				std::string guidStr = attr.stringValue();

				str_to_guid(guidStr.c_str(), guid);
				if ((rc = nvm_get_device_discovery(guid, &device)) == NVM_SUCCESS)
				{
					std::string deviceFwVersion = device.fw_revision;
					std::string deviceFwApiVersion = device.fw_api_version;

					if (fwVersion == deviceFwVersion && fwApiVersion == deviceFwApiVersion)
					{
						result = true;
					}
					else
					{
						result = false;
					}
				}
				else
				{
					COMMON_LOG_WARN_F("nvm_get_device_discovery failed with %d", rc);
				}
			}
			else
			{
				COMMON_LOG_WARN("Incorrect antecedent and dependent class instances");
			}
		}
		catch (framework::Exception &)
		{
			COMMON_LOG_WARN_F("Cannot calculate if instances are an association "
					"based on association class: %s", associationClass.c_str());
		}
	}
	else
	{
		COMMON_LOG_WARN_F("This class has no associations of type: %s", associationClass.c_str());
	}
	return result;
}
