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

#include <string.h>
#include <uid/uid.h>

#include <algorithm>
#include <nvm_management.h>
#include <LogEnterExit.h>
#include <libinvm-cim/Attribute.h>
#include <libinvm-cim/ExceptionNotSupported.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ExceptionBadAttribute.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <server/BaseServerFactory.h>
#include "NVDIMMFactory.h"
#include <utility.h>
#include <system/jedec_manufacturer.h>
#include <exception/NvmExceptionLibError.h>
#include <lib_interface/NvmApi.h>
#include <NvmStrings.h>
#include <framework_interface/FrameworkExtensions.h>
#include <persistence/config_settings.h>
#include <persistence/lib_persistence.h>
#include <core/exceptions/InvalidArgumentException.h>
#include <core/device/DeviceHelper.h>
#include <iomanip>

namespace wbem
{
namespace physical_asset
{
NVDIMMFactory::NVDIMMFactory(
		core::device::DeviceService &deviceService,
		core::system::SystemService &systemService) :
		NvmInstanceFactory(systemService),
		m_SetPassphrase(nvm_set_passphrase),
		m_RemovePassphrase(nvm_remove_passphrase),
		m_UnlockDevice(nvm_unlock_device),
		m_GetFwLogLevel(nvm_get_fw_log_level),
		m_injectDeviceError(nvm_inject_device_error),
		m_clearInjectedDeviceError(nvm_clear_injected_device_error),
		m_deviceService(deviceService),
		m_systemService(systemService)
{
}

NVDIMMFactory::~NVDIMMFactory()
{
}

void NVDIMMFactory::populateAttributeList(
		framework::attribute_names_t &attributes)
{
	// add key attributes
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(TAG_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(MANUFACTURER_KEY);
	attributes.push_back(MANUFACTURERID_KEY);
	attributes.push_back(MODEL_KEY);
	attributes.push_back(CAPACITY_KEY);
	attributes.push_back(VENDORID_KEY);
	attributes.push_back(DEVICEID_KEY);
	attributes.push_back(REVISIONID_KEY);
	attributes.push_back(SOCKETID_KEY);
	attributes.push_back(REMOVALCONDITIONS_KEY);
	attributes.push_back(MEMORYCONTROLLERID_KEY);
	attributes.push_back(MEMORYTYPE_KEY);
	attributes.push_back(SERIALNUMBER_KEY);
	attributes.push_back(SUBSYSTEMVENDORID_KEY);
	attributes.push_back(SUBSYSTEMDEVICEID_KEY);
	attributes.push_back(SUBSYSTEMREVISIONID_KEY);
	attributes.push_back(MANUFACTURINGINFOVALID_KEY);
	attributes.push_back(MANUFACTURINGLOCATION_KEY);
	attributes.push_back(MANUFACTURINGDATE_KEY);
	attributes.push_back(LOCKSTATE_KEY);
	attributes.push_back(MANAGEABILITYSTATE_KEY);
	attributes.push_back(PHYSICALID_KEY);
	attributes.push_back(FORMFACTOR_KEY);
	attributes.push_back(DATAWIDTH_KEY);
	attributes.push_back(TOTALWIDTH_KEY);
	attributes.push_back(SPEED_KEY);
	attributes.push_back(MEMORYCAPACITY_KEY);
	attributes.push_back(APP_DIRECT_CAPACITY_KEY);
	attributes.push_back(PARTNUMBER_KEY);
	attributes.push_back(BANKLABEL_KEY);
	attributes.push_back(HEALTHSTATE_KEY);
	attributes.push_back(COMMUNICATIONSTATUS_KEY);
	attributes.push_back(OPERATIONALSTATUS_KEY);
	attributes.push_back(ISNEW_KEY);
	attributes.push_back(POWERMANAGEMENTENABLED_KEY);
	attributes.push_back(POWERLIMIT_KEY);
	attributes.push_back(PEAKPOWERBUDGET_KEY);
	attributes.push_back(AVGPOWERBUDGET_KEY);
	attributes.push_back(DIESPARINGENABLED_KEY);
	attributes.push_back(DIESPARINGLEVEL_KEY);
	attributes.push_back(LASTSHUTDOWNSTATUS_KEY);
	attributes.push_back(DIESPARESUSED_KEY);
	attributes.push_back(FIRSTFASTREFRESH_KEY);
	attributes.push_back(CHANNEL_KEY);
	attributes.push_back(CHANNELPOS_KEY);
	attributes.push_back(CONFIGURATIONSTATUS_KEY);
	attributes.push_back(ARSSTATUS_KEY);
	attributes.push_back(SECURITYCAPABILITIES_KEY);
	attributes.push_back(LASTSHUTDOWNTIME_KEY);
	attributes.push_back(MEMORYTYPECAPABILITIES_KEY);
	attributes.push_back(DIESPARINGCAPABLE_KEY);
	attributes.push_back(FWLOGLEVEL_KEY);
	attributes.push_back(VIRALPOLICY_KEY);
	attributes.push_back(VIRALSTATE_KEY);
}

wbem::framework::Instance *NVDIMMFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);
		path.checkKey(CREATIONCLASSNAME_KEY, NVDIMM_CREATIONCLASSNAME);

		// extract the UID from the object path
		framework::Attribute uidAttr = path.getKeyValue(TAG_KEY);

		core::Result<core::device::Device> device =
				m_deviceService.getDevice(uidAttr.stringValue());

		toInstance(device.getValue(), *pInstance, attributes);
	}
	catch (core::LibraryException &e)
	{
		delete pInstance;
		pInstance = NULL;
		if (e.getErrorCode() == NVM_ERR_BADDEVICE)
		{
			throw framework::ExceptionBadParameter(TAG_KEY.c_str());
		}
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}
	catch (core::InvalidArgumentException &e)
	{
		delete pInstance;
		pInstance = NULL;
		throw framework::ExceptionBadParameter(e.getArgumentName().c_str());
	}

	return pInstance;

}

wbem::framework::instance_names_t *NVDIMMFactory::getInstanceNames()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		const std::vector<std::string> &devices = m_deviceService.getAllUids();

		if (devices.size() != 0)
		{
			for (size_t i = 0; i < devices.size(); i++)
			{
				framework::ObjectPath path;
				createPathFromUid(devices[i], path);
				pNames->push_back(path);
			}
		}
		else // nvm_get_device_count returned 0 DIMMs
		{
			// should never get here except in SIM
			COMMON_LOG_DEBUG("No Intel NVDIMMs found.");
		}
	}
	catch (core::LibraryException &e) // clean up and re-throw
	{
		delete pNames;
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}

	return pNames;
}

wbem::framework::UINT32 NVDIMMFactory::executeMethod(
		wbem::framework::UINT32 &wbemRc,
		const std::string method,
		wbem::framework::ObjectPath &object,
		wbem::framework::attributes_t &inParms,
		wbem::framework::attributes_t &outParms)
{
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;

	COMMON_LOG_ENTRY_PARAMS("methodName: %s, number of in params: %d", method.c_str(),
			(int) (inParms.size()));

	// tag is needed for all methods
	framework::Attribute tag = object.getKeyValue(TAG_KEY);
	std::string deviceUid = tag.stringValue();
	try
	{
		if (method == NVDIMM_SETPASSPHRASE)
		{
			// uint32 SetPassphrase(string NewPassphrase, string CurrentPassphrase);
			setPassphrase(deviceUid,
					inParms[NVDIMM_SETPASSPHRASE_NEWPASSPHRASE].stringValue(),
					inParms[NVDIMM_SETPASSPHRASE_CURRENTPASSPHRASE].stringValue());
		}
		else if (method == NVDIMM_REMOVEPASSPHRASE)
		{
			// uint32 RemovePassphrase(string CurrentPassphrase);
			removePassphrase(deviceUid,
					inParms[NVDIMM_SETPASSPHRASE_CURRENTPASSPHRASE].stringValue());
		}
		else if (method == NVDIMM_UNLOCK)
		{
			// uint32 Unlock(string CurrentPassphrase);
			unlock(deviceUid,
					inParms[NVDIMM_SETPASSPHRASE_CURRENTPASSPHRASE].stringValue());
		}
		else
		{
			httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
			COMMON_LOG_ERROR_F("methodName %s not supported", method.c_str());
		}
	}
	catch (framework::ExceptionBadParameter &)
	{
		wbemRc = NVDIMM_ERR_INVALID_PARAMETER;
	}
	catch (exception::NvmExceptionLibError &e)
	{
		wbemRc = getReturnCodeFromLibException(e);
	}
	catch (framework::ExceptionNoMemory &)
	{
		wbemRc = NVDIMM_ERR_FAILED;
	}
	catch (framework::ExceptionNotSupported &)
	{
		wbemRc = NVDIMM_ERR_NOT_SUPPORTED;
	}
	catch (framework::Exception &)
	{
		wbemRc = NVDIMM_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN("httpRc: %u, wbemRc: %u", httpRc, wbemRc);
	return httpRc;
}

wbem::framework::UINT32 NVDIMMFactory::getReturnCodeFromLibException
		(const exception::NvmExceptionLibError &e)
{
	wbem::framework::UINT32 rc = framework::MOF_ERR_SUCCESS;

	switch (e.getLibError())
	{
		case NVM_ERR_INVALIDPERMISSIONS:
		case NVM_ERR_BADSECURITYSTATE:
			rc = NVDIMM_ERR_NOT_ALLOWED;
			break;
		case NVM_ERR_INVALIDPASSPHRASE:
			rc = NVDIMM_ERR_INVALID_PARAMETER;
			break;
		case NVM_ERR_NOTSUPPORTED:
			rc = NVDIMM_ERR_NOT_SUPPORTED;
			break;
		default:
			rc = NVDIMM_ERR_FAILED;
			break;
	}

	return rc;
}

/*
 * set passphrase
 */
void NVDIMMFactory::setPassphrase(std::string deviceUid,
		std::string newPassphrase,
		std::string currentPassphrase)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (!core::device::isUidValid(deviceUid))
	{
		throw framework::ExceptionBadParameter("deviceUid");
	}
	// new passphrase is required
	if (newPassphrase.empty())
	{
		throw framework::ExceptionBadParameter(NVDIMM_SETPASSPHRASE_NEWPASSPHRASE.c_str());
	}
	NVM_UID uid;
	uid_copy(deviceUid.c_str(), uid);

	// current passphrase is not required.  Pass NULL if it is not provided
	const char *oldPassphrase = NULL;
	int oldPassphraseLen = 0;
	if (!currentPassphrase.empty())
	{
		oldPassphrase = currentPassphrase.c_str();
		oldPassphraseLen = currentPassphrase.length();
	}

	int rc = m_SetPassphrase(uid, oldPassphrase, oldPassphraseLen,
			newPassphrase.c_str(), newPassphrase.length());
	s_memset(&oldPassphrase, sizeof (oldPassphrase));
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

/*
 * remove passphrase
 */
void NVDIMMFactory::removePassphrase(
		std::string deviceUid, std::string currentPassphrase)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (currentPassphrase.empty())
	{
		throw framework::ExceptionBadParameter(NVDIMM_SETPASSPHRASE_CURRENTPASSPHRASE.c_str());
	}
	if (!core::device::isUidValid(deviceUid))
	{
		throw framework::ExceptionBadParameter("deviceUid");
	}

	NVM_UID uid;
	uid_copy(deviceUid.c_str(), uid);

	int rc = m_RemovePassphrase(uid, currentPassphrase.c_str(), currentPassphrase.length());

	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

void NVDIMMFactory::unlock(std::string deviceUid,
		std::string currentPassphrase)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (currentPassphrase.empty())
	{
		throw framework::ExceptionBadParameter(NVDIMM_SETPASSPHRASE_CURRENTPASSPHRASE.c_str());
	}
	if (!core::device::isUidValid(deviceUid))
	{
		throw framework::ExceptionBadParameter("deviceUid");
	}

	NVM_UID uid;
	uid_copy(deviceUid.c_str(), uid);

	int rc = m_UnlockDevice(uid, currentPassphrase.c_str(), currentPassphrase.length());

	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

std::vector<std::string>  wbem::physical_asset::NVDIMMFactory::getManageableDeviceUids()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::vector<std::string> result;

	wbem::physical_asset::devices_t devices = wbem::physical_asset::NVDIMMFactory::getManageableDevices();

	wbem::physical_asset::devices_t::const_iterator iter = devices.begin();
	for (; iter != devices.end(); iter++)
	{
		NVM_UID uidStr;
		uid_copy(iter->uid, uidStr);
		result.push_back(std::string(uidStr));
	}

	return result;
}

/*
 * Attempt to get the device discovery and check for manageability.
 * If can get it and discovery and is manageable return NVM_SUCCESS.
 * If doesn't exist, return NVM_ERR_BADDEVICE.
 * If is not manageable, return NVM_ERR_NOTMANAGEABLE
 */
int NVDIMMFactory::existsAndIsManageable(const std::string &dimmUid)
{
	struct device_discovery discover;
	NVM_UID uid;
	uid_copy(dimmUid.c_str(), uid);
	int rc = NVM_SUCCESS;

	if (NVM_SUCCESS != nvm_get_device_discovery(uid, &discover))
	{
		rc = NVM_ERR_BADDEVICE;
	}
	else if (discover.manageability != MANAGEMENT_VALIDCONFIG)
	{
		rc = NVM_ERR_NOTMANAGEABLE;
	}
	return rc;
}

wbem::framework::Instance *NVDIMMFactory::modifyInstance(
		framework::ObjectPath &path,
		framework::attributes_t &attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::Instance *pInstance = NULL;

	try
	{
		// get the device UID from the path
		framework::Attribute tagAttribute = path.getKeyValue(TAG_KEY);
		NVM_UID uid;
		uid_copy(tagAttribute.stringValue().c_str(), uid);


		framework::attribute_names_t attributeNames;
		pInstance = getInstance(path, attributeNames);

		framework::attribute_names_t modifyableAttributes;
		modifyableAttributes.push_back(FIRSTFASTREFRESH_KEY);
		modifyableAttributes.push_back(FWLOGLEVEL_KEY);
		modifyableAttributes.push_back(VIRALPOLICY_KEY);

		checkAttributesAreModifiable(pInstance, attributes, modifyableAttributes);

		if (pInstance)
		{
			if ((attributes.count(FIRSTFASTREFRESH_KEY)) ||
					(attributes.count(VIRALPOLICY_KEY)))
			{
				int rc = 0;
				struct device_settings settings;

				framework::Attribute refreshAttr = attributes[FIRSTFASTREFRESH_KEY];
				settings.first_fast_refresh = refreshAttr.boolValue() ? 1 : 0;
				framework::Attribute viralAttr = attributes[VIRALPOLICY_KEY];
				settings.viral_policy = viralAttr.boolValue() ? 1 : 0;

				if ((rc = nvm_modify_device_settings(uid, &settings)) == NVM_SUCCESS)
				{
					pInstance->setAttribute(FIRSTFASTREFRESH_KEY, refreshAttr);
					pInstance->setAttribute(VIRALPOLICY_KEY, viralAttr);
				}
				else
				{
					COMMON_LOG_ERROR_F("nvm_modify_device_settings failed with rc = %d", rc);
					throw exception::NvmExceptionLibError(rc);
				}
			}

			if (attributes.count(FWLOGLEVEL_KEY))
			{
				int rc = NVM_SUCCESS;
				framework::Attribute attr = attributes[FWLOGLEVEL_KEY];
				enum fw_log_level fwLogLevel = convertToLogLevelEnum(attr.uintValue());

				if ((rc = nvm_set_fw_log_level(uid, fwLogLevel)) == NVM_SUCCESS)
				{
					pInstance->setAttribute(FWLOGLEVEL_KEY, attr);
				}
				else
				{
					COMMON_LOG_ERROR_F("nvm_set_fw_log_level failed with rc = %d", rc);
					throw exception::NvmExceptionLibError(rc);
				}
			}
		}
	}
	catch (framework::Exception &)
	{
		if (pInstance) // clean up
		{
			delete pInstance;
		}
		throw;
	}

	return pInstance;
}

/*
 * Helper to convert an integer to a fw_log_level enum.
 */
enum fw_log_level NVDIMMFactory::convertToLogLevelEnum(NVM_UINT16 logLevel)
{
	enum fw_log_level level = FW_LOG_LEVEL_UNKNOWN;

	switch (logLevel)
	{
		case 0:
			level = FW_LOG_LEVEL_DISABLED;
			break;
		case 1:
			level = FW_LOG_LEVEL_ERROR;
			break;
		case 2:
			level = FW_LOG_LEVEL_WARN;
			break;
		case 3:
			level = FW_LOG_LEVEL_INFO;
			break;
		case 4:
			level = FW_LOG_LEVEL_DEBUG;
			break;
		default:
			level = FW_LOG_LEVEL_UNKNOWN;
			break;
	}

	return level;
}

void NVDIMMFactory::createPathFromUid(const std::string uid,
		wbem::framework::ObjectPath &path, std::string hostname)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (hostname.empty())
	{
		hostname = getHostName();
	}

	framework::attributes_t keys;

	keys[TAG_KEY] = framework::Attribute(uid, true);

	// CreationClassName = physical_asset::NVDIMM_CREATIONCLASSNAME
	framework::Attribute attrCCName(NVDIMM_CREATIONCLASSNAME, true);
	keys.insert(std::pair<std::string, framework::Attribute>(
			CREATIONCLASSNAME_KEY, attrCCName));

	// generate the ObjectPath for the instance
	path.setObjectPath(hostname, NVM_NAMESPACE, NVDIMM_CREATIONCLASSNAME, keys);
}


void NVDIMMFactory::createPathFromUid(const NVM_UID uid,
		framework::ObjectPath &path)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Tag = DIMM UID
	NVM_UID uidStr;
	uid_copy(uid, uidStr);

	createPathFromUid(std::string(uidStr), path);

}

devices_t NVDIMMFactory::getAllDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	devices_t devices;

	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();
	pApi->getDevices(devices);

	return devices;
}

devices_t NVDIMMFactory::getManageableDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	devices_t devices, manageableDevices;

	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();
	pApi->getManageableDimms(manageableDevices);

	return manageableDevices;
}


wbem::framework::UINT16_LIST NVDIMMFactory::deviceStatusToOpStatus(
		core::device::Device &device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::UINT16_LIST opStatus;

	// Apply device health
	if (device.getDeviceStatusHealth() == DEVICE_HEALTH_UNKNOWN)
	{
		opStatus.push_back(NVDIMM_OPSTATUS_UNKNOWN);
	}
	else if (device.getDeviceStatusHealth() == DEVICE_HEALTH_NORMAL)
	{
		opStatus.push_back(NVDIMM_OPSTATUS_OK);
	}
	else if (device.getDeviceStatusHealth() == DEVICE_HEALTH_NONCRITICAL)
	{
		opStatus.push_back(NVDIMM_OPSTATUS_DEGRADED);
	}
	else if (device.getDeviceStatusHealth() == DEVICE_HEALTH_CRITICAL)
	{
		opStatus.push_back(NVDIMM_OPSTATUS_ERROR);
	}
	else if (device.getDeviceStatusHealth() == DEVICE_HEALTH_FATAL)
	{
		opStatus.push_back(NVDIMM_OPSTATUS_NONRECOVERABLEERROR);
	}

	// Apply SKU statuses
	if (device.isMixedSku())
	{
		opStatus.push_back(NVDIMM_OPSTATUS_MIXEDSKU);
	}
	if (device.isSkuViolation())
	{
		opStatus.push_back(NVDIMM_OPSTATUS_SKUVIOLATION);
	}
	return opStatus;
}

void NVDIMMFactory::toInstance(core::device::Device &device,
		wbem::framework::Instance &instance, wbem::framework::attribute_names_t attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	ADD_ATTRIBUTE(instance, attributes, ELEMENTNAME_KEY, framework::STR,
			NVDIMM_ELEMENTNAME_PREFIX + device.getUid());
	ADD_ATTRIBUTE(instance, attributes, MANUFACTURER_KEY, framework::STR, device.getManufacturer());
	ADD_ATTRIBUTE(instance, attributes, MANUFACTURERID_KEY, framework::UINT16,
			device.getManufacturerId());
	ADD_ATTRIBUTE(instance, attributes, MODEL_KEY, framework::STR, device.getModelNumber());
	ADD_ATTRIBUTE(instance, attributes, CAPACITY_KEY, framework::UINT64, device.getRawCapacity());
	ADD_ATTRIBUTE(instance, attributes, VENDORID_KEY, framework::UINT32, device.getVendorId());
	ADD_ATTRIBUTE(instance, attributes, DEVICEID_KEY, framework::UINT16, device.getDeviceId());
	ADD_ATTRIBUTE(instance, attributes, REVISIONID_KEY, framework::UINT16, device.getRevisionId());
	ADD_ATTRIBUTE(instance, attributes, SOCKETID_KEY, framework::UINT16, device.getSocketId());
	ADD_ATTRIBUTE(instance, attributes, REMOVALCONDITIONS_KEY, framework::UINT16, NOT_APPLICABLE);
	ADD_ATTRIBUTE(instance, attributes, MEMORYCONTROLLERID_KEY, framework::UINT16,
			device.getMemoryControllerId());
	ADD_ATTRIBUTE(instance, attributes, MEMORYTYPE_KEY, framework::UINT16, device.getMemoryType());
	ADD_ATTRIBUTE(instance, attributes, SERIALNUMBER_KEY, framework::STR, device.getSerialNumber());
	ADD_ATTRIBUTE(instance, attributes, SUBSYSTEMVENDORID_KEY, framework::UINT32,
			device.getSubsystemVendor());
	ADD_ATTRIBUTE(instance, attributes, SUBSYSTEMDEVICEID_KEY, framework::UINT16,
			device.getSubsystemDevice());
	ADD_ATTRIBUTE(instance, attributes, SUBSYSTEMREVISIONID_KEY, framework::UINT16,
			device.getSubsystemRevision());
	ADD_ATTRIBUTE(instance, attributes, MANUFACTURINGINFOVALID_KEY, framework::BOOLEAN,
			device.isManufacturingInfoValid());
	ADD_ATTRIBUTE(instance, attributes, MANUFACTURINGLOCATION_KEY, framework::UINT8,
			device.getManufacturingLoc());
	ADD_ATTRIBUTE(instance, attributes, MANUFACTURINGDATE_KEY,framework::STR,
			core::device::Device::getFormattedManufacturingDate(device.getManufacturingDate()));
	ADD_ATTRIBUTE(instance, attributes, LOCKSTATE_KEY, framework::UINT32, device.getLockState());
	ADD_ATTRIBUTE(instance, attributes, MANAGEABILITYSTATE_KEY, framework::UINT32,
			device.getManageabilityState());
	ADD_ATTRIBUTE(instance, attributes, PHYSICALID_KEY, framework::UINT16, device.getPhysicalId());
	ADD_ATTRIBUTE(instance, attributes, FORMFACTOR_KEY, framework::UINT16,
			device.getFormFactor());
	ADD_ATTRIBUTE(instance, attributes, DATAWIDTH_KEY, framework::UINT16, device.getDataWidth());
	ADD_ATTRIBUTE(instance, attributes, TOTALWIDTH_KEY, framework::UINT16, device.getTotalWidth());
	ADD_ATTRIBUTE(instance, attributes, SPEED_KEY, framework::UINT32, device.getSpeed());
	ADD_ATTRIBUTE(instance, attributes, MEMORYCAPACITY_KEY, framework::UINT64,
			device.getMemoryCapacity());
	ADD_ATTRIBUTE(instance, attributes, APP_DIRECT_CAPACITY_KEY, framework::UINT64,
			device.getAppDirectCapacity());
	ADD_ATTRIBUTE(instance, attributes, PARTNUMBER_KEY, framework::STR, device.getPartNumber());
	ADD_ATTRIBUTE(instance, attributes, BANKLABEL_KEY, framework::STR, device.getBankLabel());
	ADD_ATTRIBUTE(instance, attributes, HEALTHSTATE_KEY, framework::UINT16,
			device.getHealthState());
	framework::UINT16 communicationStatus = (framework::UINT16) (device.getIsMissing()
																 ? NVDIMM_COMMUNICATION_NOCONTACT
																 : NVDIMM_COMMUNICATION_OK);
	ADD_ATTRIBUTE(instance, attributes, COMMUNICATIONSTATUS_KEY, framework::UINT16,
			communicationStatus);
	ADD_ATTRIBUTE(instance, attributes, OPERATIONALSTATUS_KEY, framework::UINT16_LIST,
			deviceStatusToOpStatus(device));
	ADD_ATTRIBUTE(instance, attributes, ISNEW_KEY, framework::BOOLEAN, device.isNew());
	ADD_ATTRIBUTE(instance, attributes, POWERMANAGEMENTENABLED_KEY, framework::BOOLEAN,
			device.isPowerManagementEnabled());
	ADD_ATTRIBUTE(instance, attributes, POWERLIMIT_KEY, framework::UINT8, device.getPowerLimit());
	ADD_ATTRIBUTE(instance, attributes, PEAKPOWERBUDGET_KEY, framework::UINT32,
			device.getPeakPowerBudget());
	ADD_ATTRIBUTE(instance, attributes, AVGPOWERBUDGET_KEY, framework::UINT32,
			device.getAvgPowerBudget());
	ADD_ATTRIBUTE(instance, attributes, DIESPARINGENABLED_KEY, framework::BOOLEAN,
			device.isDieSparingEnabled());
	ADD_ATTRIBUTE(instance, attributes, DIESPARINGLEVEL_KEY, framework::UINT16,
			device.getDieSparingLevel());
	ADD_ATTRIBUTE(instance, attributes, LASTSHUTDOWNSTATUS_KEY, framework::UINT16_LIST,
			device.getLastShutdownStatus());
	ADD_ATTRIBUTE(instance, attributes, DIESPARESUSED_KEY, framework::UINT8,
			device.getDieSparesUsed());
	ADD_ATTRIBUTE(instance, attributes, FIRSTFASTREFRESH_KEY, framework::BOOLEAN,
			device.isFirstFastRefresh());
	ADD_ATTRIBUTE(instance, attributes, CHANNEL_KEY, framework::UINT32, device.getChannelId());
	ADD_ATTRIBUTE(instance, attributes, CHANNELPOS_KEY, framework::UINT32,
			device.getChannelPosition());
	ADD_ATTRIBUTE(instance, attributes, CONFIGURATIONSTATUS_KEY, framework::UINT16,
			device.getConfigStatus());
	ADD_ATTRIBUTE(instance, attributes, ARSSTATUS_KEY, framework::UINT16,
			device.getArsStatus());
	ADD_ATTRIBUTE(instance, attributes, SECURITYCAPABILITIES_KEY, framework::UINT16_LIST,
			device.getSecurityCapabilities());
	ADD_DATETIME_ATTRIBUTE(instance, attributes, LASTSHUTDOWNTIME_KEY,
			device.getLastShutdownTime());
	ADD_ATTRIBUTE(instance, attributes, DIESPARINGCAPABLE_KEY, framework::BOOLEAN,
			device.isDieSparingCapable());
	ADD_ATTRIBUTE(instance, attributes, MEMORYTYPECAPABILITIES_KEY, framework::UINT16_LIST,
			device.getMemoryCapabilities());
	ADD_ATTRIBUTE(instance, attributes, FWLOGLEVEL_KEY, framework::UINT16,
			device.getFwLogLevel());
	ADD_ATTRIBUTE(instance, attributes, FWAPIVERSION_KEY, framework::STR, device.getFwApiVersion());
	ADD_ATTRIBUTE(instance, attributes, FWVERSION_KEY, framework::STR, device.getFwRevision());
	ADD_ATTRIBUTE(instance, attributes, UNCONFIGUREDCAPACITY_KEY, framework::UINT64,
			device.getUnconfiguredCapacity());
	ADD_ATTRIBUTE(instance, attributes, INACCESSIBLECAPACITY_KEY, framework::UINT64,
			device.getInaccessibleCapacity());
	ADD_ATTRIBUTE(instance, attributes, RESERVEDCAPACITY_KEY, framework::UINT64,
			device.getReservedCapacity());
	ADD_ATTRIBUTE(instance, attributes, DEVICELOCATOR_KEY, framework::STR,
			device.getDeviceLocator());
	ADD_ATTRIBUTE(instance, attributes, ACTIONREQUIRED_KEY, framework::BOOLEAN,
			device.isActionRequired());
	ADD_ATTRIBUTE(instance, attributes, ACTIONREQUIREDEVENTS_KEY, framework::STR_LIST,
			device.getActionRequiredEvents());
	ADD_ATTRIBUTE(instance, attributes, MEMORYMODESSUPPORTED_KEY, framework::STR,
			getMemoryModeString(device));
	ADD_ATTRIBUTE(instance, attributes, MIXEDSKU_KEY, framework::BOOLEAN, device.isMixedSku());
	ADD_ATTRIBUTE(instance, attributes, SKUVIOLATION_KEY, framework::BOOLEAN,
			device.isSkuViolation());
	ADD_ATTRIBUTE(instance, attributes, VIRALPOLICY_KEY, framework::BOOLEAN, device.isViralPolicyEnabled());
	ADD_ATTRIBUTE(instance, attributes, VIRALSTATE_KEY, framework::BOOLEAN, device.getCurrentViralState());
}

std::string NVDIMMFactory::getMemoryModeString(core::device::Device &device)
{
	std::map<NVM_UINT32, std::string> map;
	map[NVDIMM_MEMORYTYPECAPABILITIES_MEMORYMODE] = TR("Memory");
	map[NVDIMM_MEMORYTYPECAPABILITIES_STORAGEMODE] = TR("Storage");
	map[NVDIMM_MEMORYTYPECAPABILITIES_APPDIRECTMODE] = TR("AppDirect");

	std::stringstream result;
	const std::vector<NVM_UINT16> &capabilities = device.getMemoryCapabilities();
	for (size_t i = 0; i < capabilities.size(); i++)
	{
		if (i > 0)
		{
			result << ", ";
		}

		result << map[capabilities[i]];
	}
	return result.str();
}

wbem::framework::instances_t *NVDIMMFactory::getInstances(
		wbem::framework::attribute_names_t &attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	checkAttributes(attributes);

	try
	{
		core::device::DeviceCollection devices = m_deviceService.getAllDevices();

		framework::instances_t *pResult = new framework::instances_t();
		if (!pResult)
		{
			throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__, "pResult");
		}
		for (size_t i = 0; i < devices.size(); i++)
		{
			framework::ObjectPath path;
			createPathFromUid(devices[i].getUid(), path);
			framework::Instance instance(path);

			toInstance(devices[i], instance, attributes);

			pResult->push_back(instance);
		}
		return pResult;
	}
	catch (core::LibraryException &e)
	{
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}
	catch (core::InvalidArgumentException &e)
	{
		throw framework::ExceptionBadParameter(e.getArgumentName().c_str());
	}
}

void NVDIMMFactory::uidToHandle(const std::string &dimmUid,
		NVM_UINT32 &handle)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UID uid;
	handle = 0;

	uid_copy(dimmUid.c_str(), uid);
	struct device_discovery device;
	int rc;
	if ((rc = nvm_get_device_discovery(uid, &device)) == NVM_SUCCESS)
	{
		handle = device.device_handle.handle;
	}
	else
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

void NVDIMMFactory::injectPoisonError(const std::string &dimmUid,
		const NVM_UINT64 dpa)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct device_error error;
	memset(&error, 0, sizeof(error));
	error.type = ERROR_TYPE_POISON;
	error.dpa = dpa;
	injectError(dimmUid, &error);
}

void NVDIMMFactory::clearPoisonError(const std::string &dimmUid,
		const NVM_UINT64 dpa)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct device_error error;
	memset(&error, 0, sizeof(error));
	error.type = ERROR_TYPE_POISON;
	error.dpa = dpa;
	clearError(dimmUid, &error);
}

void NVDIMMFactory::clearTemperatureError(const std::string &dimmUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct device_error error;
	memset(&error, 0, sizeof(error));
	error.type = ERROR_TYPE_TEMPERATURE;
	clearError(dimmUid, &error);
}

void NVDIMMFactory::clearSoftwareTrigger(
		const std::string &dimmUid,
		const NVM_UINT16 error_type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct device_error error;
	memset(&error, 0, sizeof(error));
	error.type = (enum error_type)error_type;
	clearError(dimmUid, &error);
}

void NVDIMMFactory::injectTemperatureError(
		const std::string &dimmUid,
		const NVM_REAL32 temperature)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct device_error error;
	memset(&error, 0, sizeof(error));
	error.type = ERROR_TYPE_TEMPERATURE;
	error.temperature = nvm_encode_temperature(temperature);
	injectError(dimmUid, &error);
}

void NVDIMMFactory::injectSoftwareTrigger(
		const std::string &dimmUid,
		const NVM_UINT16 error_type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct device_error error;
	memset(&error, 0, sizeof(error));
	error.type = (enum error_type)error_type;
	injectError(dimmUid, &error);
}

void NVDIMMFactory::clearError(const std::string &dimmUid,
		struct device_error *p_error)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// check for valid dimm uid
	// note - .length() doesn't include NULL terminator
	if (!core::device::isUidValid(dimmUid))
	{
		COMMON_LOG_ERROR("Invalid dimm uid");
		throw wbem::framework::ExceptionBadParameter(wbem::DEVICEID_KEY.c_str());
	}

	int rc = m_clearInjectedDeviceError(dimmUid.c_str(), p_error);
	if (rc != NVM_SUCCESS)
	{
		throw wbem::exception::NvmExceptionLibError(rc);
	}
}

void NVDIMMFactory::injectError(const std::string &dimmUid,
		struct device_error *p_error)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// check for valid dimm uid
	// note - .length() doesn't include NULL terminator
	if (!core::device::isUidValid(dimmUid))
	{
		COMMON_LOG_ERROR("Invalid dimm uid");
		throw wbem::framework::ExceptionBadParameter(wbem::DEVICEID_KEY.c_str());
	}

	int rc = m_injectDeviceError(dimmUid.c_str(), p_error);
	if (rc != NVM_SUCCESS)
	{
		throw wbem::exception::NvmExceptionLibError(rc);
	}
}

std::string NVDIMMFactory::uidToDimmIdStr(const std::string &dimmUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::framework::Attribute attribute = uidToDimmIdAttribute(dimmUid);
	return attribute.asStr();
}

wbem::framework::Attribute NVDIMMFactory::uidToDimmIdAttribute(
		const std::string &dimmUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Attribute dimmIdAttr;
	// look up the database setting for the preferred DimmID output
	// default to using dimm handle
	bool useHandle = true;
	char value[CONFIG_VALUE_LEN];
	if (get_config_value(SQL_KEY_CLI_DIMM_ID, value) == COMMON_SUCCESS)
	{
		// switch to uid
		if (s_strncmpi("UID", value, strlen("UID")) == 0)
		{
			useHandle = false;
		}
	}

	// convert UID to handle
	if (useHandle)
	{
		NVM_UINT32 handle;
		NVDIMMFactory::uidToHandle(dimmUid.c_str(), handle);
		framework::Attribute attrHandle(handle, false);
		dimmIdAttr = attrHandle;
	}
		// use UID
	else
	{
		framework::Attribute attrUid(dimmUid, false);
		dimmIdAttr = attrUid;
	}
	return dimmIdAttr;
}
}
}

