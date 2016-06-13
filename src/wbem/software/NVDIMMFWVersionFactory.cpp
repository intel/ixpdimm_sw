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
 * This file contains the provider for the NVDIMMFWVersion instances
 * which represents the version of the FW on an NVM DIMM.
 */

#include <LogEnterExit.h>
#include <nvm_management.h>
#include <string/revision.h>
#include <uid/uid.h>
#include <algorithm>
#include <server/BaseServerFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include "ElementSoftwareIdentityFactory.h"
#include "NVDIMMCollectionFactory.h"
#include "NVDIMMFWVersionFactory.h"
#include <exception/NvmExceptionLibError.h>
#include <framework_interface/NvmAssociationFactory.h>
#include <NvmStrings.h>
#include <lib_interface/NvmApi.h>
#include <sstream>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <exception/NvmExceptionLibError.h>

wbem::software::NVDIMMFWVersionFactory::NVDIMMFWVersionFactory()
throw (wbem::framework::Exception)
{
}

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
	attributes.push_back(FWTYPE_KEY);
	attributes.push_back(COMMITID_KEY);
	attributes.push_back(BUILDCONFIGURATION_KEY);
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

		std::string fwVersion, fwApiVersion, commitId, build_configuration;
		NVM_UINT16 fwType = DEVICE_FW_TYPE_UNKNOWN;
		parseInstanceId(instanceId, fwVersion, fwApiVersion, fwType, commitId, build_configuration);

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

		if (containsAttribute(FWTYPE_KEY, attributes))
		{
			framework::Attribute a(fwType, translateFwType((enum device_fw_type)fwType), false);
			pInstance->setAttribute(FWTYPE_KEY, a, attributes);
		}

		if (containsAttribute(COMMITID_KEY, attributes))
		{
			framework::Attribute a(commitId, false);
			pInstance->setAttribute(COMMITID_KEY, a, attributes);
		}
		if (containsAttribute(BUILDCONFIGURATION_KEY, attributes))
		{
			framework::Attribute a(build_configuration, false);
			pInstance->setAttribute(BUILDCONFIGURATION_KEY, a, attributes);
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
		lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();
		std::string hostServer = pApi->getHostName();
		std::vector<struct device_discovery> devices;
		pApi->getDevices(devices);

		for (size_t i = 0; i < devices.size(); i++)
		{
			addFirmwareInstanceNamesForDevice(*pNames, hostServer, devices[i]);
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

void wbem::software::NVDIMMFWVersionFactory::addFirmwareInstanceNamesForDevice(
		framework::instance_names_t& instanceNames, const std::string& hostName,
		const struct device_discovery& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct device_fw_info fw_info;
	memset(&fw_info, 0, sizeof (struct device_fw_info));
	int rc = lib_interface::NvmApi::getApi()->getDeviceFwImageInfo(device.uid, &fw_info);
	if (rc == NVM_ERR_NOTMANAGEABLE)
	{
		// Unmanageable DIMMs can coexist with us, we just can't get
		// FW image details.
	}
	else if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}

	// Use the core classes to access static methods
	core::device::Device deviceWrapper(core::NvmLibrary::getNvmLibrary(), device);
	core::device::DeviceFirmwareInfo fwInfoWrapper(deviceWrapper.getUid(), fw_info);
	addFirmwareInstanceNamesForDeviceFromFwInfo(instanceNames,
			hostName, deviceWrapper, fwInfoWrapper);
}

void wbem::software::NVDIMMFWVersionFactory::addFirmwareInstanceNamesForDeviceFromFwInfo(
		framework::instance_names_t& instanceNames,
		const std::string& hostName,
		core::device::Device& device,
		const core::device::DeviceFirmwareInfo& fwInfo)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::ObjectPath activeFwPath = getActiveFirmwareInstanceName(hostName, device, fwInfo);
	if (std::find(instanceNames.begin(), instanceNames.end(), activeFwPath) == instanceNames.end())
	{
		instanceNames.push_back(activeFwPath);
	}

	if (fwInfo.isStagedPending())
	{
		framework::ObjectPath stagedFwPath = getStagedFirmwareInstanceName(hostName, device, fwInfo);
		if (std::find(instanceNames.begin(), instanceNames.end(), stagedFwPath) == instanceNames.end())
		{
			instanceNames.push_back(stagedFwPath);
		}
	}
}

std::string wbem::software::NVDIMMFWVersionFactory::getInstanceId(const std::string& fwVersion,
		const std::string& fwApiVersion, const enum device_fw_type fwType,
		const std::string& commitId, const std::string &build_configuration)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream instanceId;
	instanceId << NVDIMMFWVERSION_INSTANCEID_PREFIX <<
			fwVersion << NVMDIMMFWVERSION_DELIMITER <<
			fwApiVersion << NVMDIMMFWVERSION_DELIMITER <<
			fwType;
	if (!commitId.empty())
	{
		instanceId << NVMDIMMFWVERSION_DELIMITER << commitId;
	}
	if (!build_configuration.empty())
	{
		instanceId << NVMDIMMFWVERSION_DELIMITER << build_configuration;
	}

	return instanceId.str();
}

wbem::framework::ObjectPath wbem::software::NVDIMMFWVersionFactory::getInstanceName(
		const std::string& hostName, const std::string instanceId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::attributes_t keys;
	keys[INSTANCEID_KEY] = framework::Attribute(instanceId, true);
	return framework::ObjectPath(hostName, NVM_NAMESPACE, NVDIMMFWVERSION_CREATIONCLASSNAME, keys);
}

wbem::framework::ObjectPath wbem::software::NVDIMMFWVersionFactory::getActiveFirmwareInstanceName(
		const std::string &hostName,
		core::device::Device& device,
		const core::device::DeviceFirmwareInfo& fwInfo)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string instanceIDStr = getInstanceId(device.getFwRevision(),
			device.getFwApiVersion(),
			fwInfo.getActiveType(),
			fwInfo.getActiveCommitId(),
			fwInfo.getActiveBuildConfiguration());

	return getInstanceName(hostName, instanceIDStr);
}

wbem::framework::ObjectPath wbem::software::NVDIMMFWVersionFactory::getStagedFirmwareInstanceName(
		const std::string &hostName,
		core::device::Device& device,
		const core::device::DeviceFirmwareInfo& fwInfo)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string instanceIDStr = getInstanceId(fwInfo.getStagedRevision(),
			device.getFwApiVersion(),
			fwInfo.getStagedType());

	return getInstanceName(hostName, instanceIDStr);
}

std::string wbem::software::NVDIMMFWVersionFactory::translateFwType(
	const NVM_UINT16 fw_type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string fw_type_str = NVMDIMMFWVERSION_FWTYPE_UNKNOWN_STR;
	switch (fw_type)
	{
		case NVMDIMMFWVERSION_FWTYPE_PRODUCTION:
			fw_type_str = NVMDIMMFWVERSION_FWTYPE_PRODUCTION_STR;
			break;
		case NVMDIMMFWVERSION_FWTYPE_DFX:
			fw_type_str = NVMDIMMFWVERSION_FWTYPE_DFX_STR;
			break;
		case NVMDIMMFWVERSION_FWTYPE_DEBUG:
			fw_type_str = NVMDIMMFWVERSION_FWTYPE_DEBUG_STR;
			break;
		default:
			break;
	}

	return fw_type_str;
}

/*
 * Parse instanceId string into FW version, FW API version, FW type and commit ID
 */
void wbem::software::NVDIMMFWVersionFactory::parseInstanceId(std::string instanceId,
		std::string &fwVersion, std::string &fwApiVersion, NVM_UINT16 &fwType, std::string &commitId, std::string &build_configuration)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	/* InstanceID has the form NVDIMMFW fw_rev-fw_api-fw_type-commitid-build_configuration
	 "NVDIMMFW xx.xx.xx.xxxx-x.x-x-<commid_id>-<build_configuration>"
	 */

	instanceId.erase(0, NVDIMMFWVERSION_INSTANCEID_PREFIX.length());

	fwVersion = instanceId.substr(0, instanceId.find(NVMDIMMFWVERSION_DELIMITER));

	instanceId.erase(0, instanceId.find(NVMDIMMFWVERSION_DELIMITER) + NVMDIMMFWVERSION_DELIMITER.length());
	fwApiVersion = instanceId.substr(0, instanceId.find(NVMDIMMFWVERSION_DELIMITER));

	instanceId.erase(0, instanceId.find(NVMDIMMFWVERSION_DELIMITER) + NVMDIMMFWVERSION_DELIMITER.length());
	std::string fwtypeStr = instanceId.substr(0, instanceId.find(NVMDIMMFWVERSION_DELIMITER));
	fwType = (NVM_UINT16)atoi(fwtypeStr.c_str());

	size_t commitIdPos = instanceId.find(NVMDIMMFWVERSION_DELIMITER);
	if (commitIdPos == std::string::npos)
	{
		commitId = "";
	}
	else
	{
		instanceId.erase(0, instanceId.find(NVMDIMMFWVERSION_DELIMITER) + NVMDIMMFWVERSION_DELIMITER.length());
		commitId = instanceId.substr(0, instanceId.find(NVMDIMMFWVERSION_DELIMITER));
	}

	size_t buildConfigPos = instanceId.find(NVMDIMMFWVERSION_DELIMITER);
	if (buildConfigPos == std::string::npos)
	{
		build_configuration = "";
	}
	else
	{
		instanceId.erase(0, instanceId.find(NVMDIMMFWVERSION_DELIMITER) + NVMDIMMFWVERSION_DELIMITER.length());
		build_configuration = instanceId.substr(0, instanceId.length());
	}
}
