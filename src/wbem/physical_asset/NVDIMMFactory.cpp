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
 * This file contains the provider for the NVDIMM instances which
 * model the physical aspects of an NVM DIMM.
 */

#include <string.h>
#include <guid/guid.h>

#include <algorithm>
#include <nvm_management.h>
#include <LogEnterExit.h>
#include <intel_cim_framework/Attribute.h>
#include <intel_cim_framework/ExceptionNotSupported.h>
#include <intel_cim_framework/ExceptionBadParameter.h>
#include <intel_cim_framework/ExceptionBadAttribute.h>
#include <intel_cim_framework/ExceptionNoMemory.h>
#include <server/BaseServerFactory.h>
#include "NVDIMMFactory.h"
#include <utility.h>
#include <system/jedec_manufacturer.h>
#include <exception/NvmExceptionLibError.h>
#include <lib_interface/NvmApi.h>
#include <NvmStrings.h>

wbem::physical_asset::NVDIMMFactory::NVDIMMFactory()
	throw (wbem::framework::Exception)
	: m_SetPassphrase(nvm_set_passphrase), m_RemovePassphrase(nvm_remove_passphrase),
	  m_UnlockDevice(nvm_unlock_device)
{
}

wbem::physical_asset::NVDIMMFactory::~NVDIMMFactory()
{
}

void wbem::physical_asset::NVDIMMFactory::populateAttributeList(
	framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
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
	attributes.push_back(MEMORYCONTROLLERID_KEY);
	attributes.push_back(MEMORYTYPE_KEY);
	attributes.push_back(SERIALNUMBER_KEY);
	attributes.push_back(LOCKSTATE_KEY);
	attributes.push_back(MANAGEABILITYSTATE_KEY);
	attributes.push_back(PHYSICALID_KEY);
	attributes.push_back(FORMFACTOR_KEY);
	attributes.push_back(DATAWIDTH_KEY);
	attributes.push_back(TOTALWIDTH_KEY);
	attributes.push_back(SPEED_KEY);
	attributes.push_back(VOLATILECAPACITY_KEY);
	attributes.push_back(PERSISTENTCAPACITY_KEY);
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
	attributes.push_back(SECURITYCAPABILITIES_KEY);
	attributes.push_back(LASTSHUTDOWNTIME_KEY);
	attributes.push_back(MEMORYTYPECAPABILITIES_KEY);
	attributes.push_back(DIESPARINGCAPABLE_KEY);
	attributes.push_back(FWLOGLEVEL_KEY);
}
/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::physical_asset::NVDIMMFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		checkAttributes(attributes);
		path.checkKey(CREATIONCLASSNAME_KEY, NVDIMM_CREATIONCLASSNAME);

		// extract the GUID from the object path
		framework::Attribute guidAttr = path.getKeyValue(TAG_KEY);

		// -1 because the length of the string does not include the null terminator
		if (guidAttr.stringValue().length() != NVM_GUIDSTR_LEN - 1)
		{
			throw framework::ExceptionBadParameter(guidAttr.stringValue().c_str());
		}

		NVM_GUID dimmGuid;
		str_to_guid((char*)guidAttr.stringValue().c_str(), dimmGuid);

		// get dimm info
		struct device_details dimm;
		memset(&dimm, 0, sizeof(dimm));
		int rc = nvm_get_device_details(dimmGuid, &dimm);
		// try to get device discovery if device is not manageable
		if (rc == NVM_ERR_NOTMANAGEABLE)
		{
			int tmpRc = nvm_get_device_discovery(dimmGuid, &dimm.discovery);
			if (tmpRc != NVM_SUCCESS)
			{
				// couldn't retrieve any dimm info, punt
				throw exception::NvmExceptionLibError(rc);
			}
		}
		// any other error besides not manageable, punt
		else if (rc != NVM_SUCCESS)
		{
			// couldn't retrieve the dimm info for some other reason than not being manageable
			throw exception::NvmExceptionLibError(rc);
		}

		// Element Name = NVDIMM_ELEMENTNAME_prefix + GUID
		// We do this here because fillInNVDIMMInstance is also used by the NVDIMM view class
		// and it does not need ELEMEMTNAME
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			NVM_GUID_STR guidStr;
			guid_to_str(dimm.discovery.guid, guidStr);
			framework::Attribute attrElementName(
					NVDIMM_ELEMENTNAME_prefix + std::string(guidStr), false);
			pInstance->setAttribute(ELEMENTNAME_KEY, attrElementName, attributes);
		}
		// Operational Status
		if (containsAttribute(OPERATIONALSTATUS_KEY, attributes))
		{
			framework::Attribute attrOpStatus(deviceStatusToOpStatus(&dimm.status), false);
			pInstance->setAttribute(OPERATIONALSTATUS_KEY, attrOpStatus, attributes);
		}


		fillInNVDIMMInstance(dimm, attributes, pInstance);
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		if (pInstance != NULL)
		{
			delete pInstance;
		}
		throw;
	}

	return pInstance;

}
/*
 * Retrieve a specific instance given an object path
 */
void wbem::physical_asset::NVDIMMFactory::fillInNVDIMMInstance(
	struct device_details &dimm, framework::attribute_names_t &attributes,
	framework::Instance *pInstance)throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Manufacturer - try to look up manufacturer name based on JEDEC JEP-106 ID
	if (containsAttribute(MANUFACTURER_KEY, attributes))
	{
		// lookup manufacturer name
		std::string manufacturerName = "";
		char manufacturerStr[NVM_MANUFACTURERSTR_LEN];
		if (lookup_jedec_jep106_manufacturer(
				dimm.discovery.manufacturer, NVM_MANUFACTURER_LEN,
				manufacturerStr, NVM_MANUFACTURERSTR_LEN) == COMMON_SUCCESS)
		{
			manufacturerName = manufacturerStr;
		}
		framework::Attribute attrManufacturer(manufacturerName, false);
		pInstance->setAttribute(MANUFACTURER_KEY, attrManufacturer, attributes);
	}
	// ManufacturerID - convert Manufacturer ID bytes to a number
	if (containsAttribute(MANUFACTURERID_KEY, attributes))
	{
		NVM_UINT16 manufacturerID = MANUFACTURER_TO_UINT(dimm.discovery.manufacturer);
		framework::Attribute attrManufacturerID(manufacturerID, false);
		pInstance->setAttribute(MANUFACTURERID_KEY, attrManufacturerID, attributes);
	}
	// Model Number
	if (containsAttribute(MODEL_KEY, attributes))
	{
		framework::Attribute attrModel(dimm.discovery.model_number, false);
		pInstance->setAttribute(MODEL_KEY, attrModel, attributes);
	}
	// Capacity
	if (containsAttribute(CAPACITY_KEY, attributes))
	{
		framework::Attribute attrCapacity(dimm.discovery.capacity, false);
		pInstance->setAttribute(CAPACITY_KEY, attrCapacity, attributes);
	}
	// Vendor ID
	if (containsAttribute(VENDORID_KEY, attributes))
	{
		framework::Attribute attrVendorID((NVM_UINT32)dimm.discovery.vendor_id, false);
		pInstance->setAttribute(VENDORID_KEY, attrVendorID, attributes);
	}
	// Device ID
	if (containsAttribute(DEVICEID_KEY, attributes))
	{
		framework::Attribute attrDeviceID(dimm.discovery.device_id, false);
		pInstance->setAttribute(DEVICEID_KEY, attrDeviceID, attributes);
	}
	// Revision ID
	if (containsAttribute(REVISIONID_KEY, attributes))
	{
		framework::Attribute attrRevisionID(dimm.discovery.revision_id, false);
		pInstance->setAttribute(REVISIONID_KEY, attrRevisionID, attributes);
	}
	// Socket ID
	if (containsAttribute(SOCKETID_KEY, attributes))
	{
		framework::Attribute attrProxDomain(dimm.discovery.socket_id, false);
		pInstance->setAttribute(SOCKETID_KEY, attrProxDomain, attributes);
	}
	// Memory Controller ID
	if (containsAttribute(MEMORYCONTROLLERID_KEY, attributes))
	{
		framework::Attribute attrMemCtrl(dimm.discovery.memory_controller_id, false);
		pInstance->setAttribute(MEMORYCONTROLLERID_KEY, attrMemCtrl, attributes);
	}
	// Memory Type
	if (containsAttribute(MEMORYTYPE_KEY, attributes))
	{
		std::string typeStr = memoryTypetoStr(dimm.discovery.memory_type);
		framework::Attribute attrType((NVM_UINT16)dimm.discovery.memory_type, typeStr, false);
		pInstance->setAttribute(MEMORYTYPE_KEY, attrType, attributes);
	}
	// Serial Number - Convert serial number to a string
	if (containsAttribute(SERIALNUMBER_KEY, attributes))
	{
		char serialNumStr[NVM_SERIALSTR_LEN];
		SERIAL_NUMBER_TO_STRING(dimm.discovery.serial_number, serialNumStr);
		framework::Attribute attrSerialNo(serialNumStr, false);
		pInstance->setAttribute(SERIALNUMBER_KEY, attrSerialNo, attributes);
	}
	// Lock State
	if (containsAttribute(LOCKSTATE_KEY, attributes))
	{
		std::string lockStateStr;
		switch (dimm.discovery.lock_state)
		{
			case LOCK_STATE_UNKNOWN:
				lockStateStr = "Unknown";
				break;
			case LOCK_STATE_DISABLED:
				lockStateStr = "Disabled";
				break;
			case LOCK_STATE_UNLOCKED:
				lockStateStr = "Unlocked";
				break;
			case LOCK_STATE_LOCKED:
				lockStateStr = "Locked";
				break;
			case LOCK_STATE_FROZEN:
				lockStateStr = "Frozen";
				break;
			case LOCK_STATE_PASSPHRASE_LIMIT:
				lockStateStr = "Exceeded";
				break;
			case LOCK_STATE_NOT_SUPPORTED:
				lockStateStr = "Not Supported";
				break;
		}
		framework::Attribute attrLockState((NVM_UINT32)dimm.discovery.lock_state, lockStateStr, false);
		pInstance->setAttribute(LOCKSTATE_KEY, attrLockState, attributes);
	}
	// Manageability State
	if (containsAttribute(MANAGEABILITYSTATE_KEY, attributes))
	{
		std::string manageabilityStrValue;
		switch (dimm.discovery.manageability)
		{
			case MANAGEMENT_VALIDCONFIG:
				manageabilityStrValue = "Manageable";
				break;
			case MANAGEMENT_INVALIDCONFIG:
				manageabilityStrValue = "Unmanageable";
				break;
			default:
				manageabilityStrValue = "Unknown";
				break;
		}
		framework::Attribute attrMgmtState((NVM_UINT32)dimm.discovery.manageability, manageabilityStrValue, false);
		pInstance->setAttribute(MANAGEABILITYSTATE_KEY, attrMgmtState, attributes);
	}
	// Physical ID
	if (containsAttribute(PHYSICALID_KEY, attributes))
	{
		framework::Attribute attrPid(dimm.discovery.physical_id, false);
		pInstance->setAttribute(PHYSICALID_KEY, attrPid, attributes);
	}
	// Health State
	if (containsAttribute(HEALTHSTATE_KEY, attributes))
	{
		std::string healthStateStr;
		if (dimm.discovery.manageability != MANAGEMENT_VALIDCONFIG)
		{
			healthStateStr = "Unmanageable";
			framework::Attribute attrHealth(DEVICE_HEALTH_UNMANAGEABLE, healthStateStr, false);
			pInstance->setAttribute(HEALTHSTATE_KEY, attrHealth, attributes);
		}
		else
		{
			healthStateStr = get_string_for_device_health_status(dimm.status.health);
			framework::Attribute attrHealth((NVM_UINT16)dimm.status.health, healthStateStr, false);
			pInstance->setAttribute(HEALTHSTATE_KEY, attrHealth, attributes);
		}
	}
	// Form Factor
	if (containsAttribute(FORMFACTOR_KEY, attributes))
	{
		std::string formFactorStr;
		switch (dimm.form_factor)
		{
			case DEVICE_FORM_FACTOR_DIMM:
				formFactorStr = "DIMM";
				break;
			case DEVICE_FORM_FACTOR_SODIMM:
				formFactorStr = "SODIMM";
				break;
			default:
				formFactorStr = "Unknown";
				break;
		}
		framework::Attribute attrFormFactor((NVM_UINT16)dimm.form_factor, formFactorStr, false);
		pInstance->setAttribute(FORMFACTOR_KEY, attrFormFactor, attributes);
	}
	// Data Width
	if (containsAttribute(DATAWIDTH_KEY, attributes))
	{
		framework::Attribute attrDataWidth((NVM_UINT16)dimm.data_width, false);
		pInstance->setAttribute(DATAWIDTH_KEY, attrDataWidth, attributes);
	}
	// Total Width
	if (containsAttribute(TOTALWIDTH_KEY, attributes))
	{
		framework::Attribute attrTotalWidth((NVM_UINT16)dimm.total_width, false);
		pInstance->setAttribute(TOTALWIDTH_KEY, attrTotalWidth, attributes);
	}
	// Speed
	if (containsAttribute(SPEED_KEY, attributes))
	{
		framework::Attribute attrSpeed((NVM_UINT32)dimm.speed, false);
		pInstance->setAttribute(SPEED_KEY, attrSpeed, attributes);
	}
	// Volatile Capacity
	if (containsAttribute(VOLATILECAPACITY_KEY, attributes))
	{
		framework::Attribute attrVolCap(dimm.capacities.volatile_capacity, false);
		pInstance->setAttribute(VOLATILECAPACITY_KEY, attrVolCap, attributes);
	}
	// Persistent Capacity
	if (containsAttribute(PERSISTENTCAPACITY_KEY, attributes))
	{
		framework::Attribute attrPmCap(dimm.capacities.persistent_capacity, false);
		pInstance->setAttribute(PERSISTENTCAPACITY_KEY, attrPmCap, attributes);
	}
	// Part Number
	if (containsAttribute(PARTNUMBER_KEY, attributes))
	{
		framework::Attribute attrPartNumber(dimm.part_number, false);
		pInstance->setAttribute(PARTNUMBER_KEY, attrPartNumber, attributes);
	}
	// Bank Label
	if (containsAttribute(BANKLABEL_KEY, attributes))
	{
		framework::Attribute attrBankLabel(dimm.bank_label, false);
		pInstance->setAttribute(BANKLABEL_KEY, attrBankLabel, attributes);
	}
	// Communication Status
	if (containsAttribute(COMMUNICATIONSTATUS_KEY, attributes))
	{
		NVM_UINT16 commStatus = NVDIMM_COMMUNICATION_OK;
		std::string commStatusStr = "Communication OK";
		if (dimm.status.is_missing)
		{
			commStatus = NVDIMM_COMMUNICATION_NOCONTACT;
			commStatusStr = "No Contact";
		}
		framework::Attribute attrCommStatus(commStatus, commStatusStr, false);
		pInstance->setAttribute(COMMUNICATIONSTATUS_KEY, attrCommStatus, attributes);
	}
	// Is New
	if (containsAttribute(ISNEW_KEY, attributes))
	{
		framework::Attribute attrIsNew((bool)dimm.status.is_new, false);
		pInstance->setAttribute(ISNEW_KEY, attrIsNew, attributes);
	}
	// PowerManagementEnabled
	if (containsAttribute(POWERMANAGEMENTENABLED_KEY, attributes))
	{
		framework::Attribute attrPowerManagement((bool)dimm.power_management_enabled, false);
		pInstance->setAttribute(POWERMANAGEMENTENABLED_KEY, attrPowerManagement, attributes);
	}
	// PowerLimit
	if (containsAttribute(POWERLIMIT_KEY, attributes))
	{
		framework::Attribute attrPowerLimit(dimm.power_limit, false);
		pInstance->setAttribute(POWERLIMIT_KEY, attrPowerLimit, attributes);
	}
	// PeakPowerBudget
	if (containsAttribute(PEAKPOWERBUDGET_KEY, attributes))
	{
		framework::Attribute attrPeakPowerBudget((NVM_UINT32)dimm.peak_power_budget, false);
		pInstance->setAttribute(PEAKPOWERBUDGET_KEY, attrPeakPowerBudget, attributes);
	}
	// AvgPowerBudget
	if (containsAttribute(AVGPOWERBUDGET_KEY, attributes))
	{
		framework::Attribute attrAvgPowerBudget((NVM_UINT32)dimm.avg_power_budget, false);
		pInstance->setAttribute(AVGPOWERBUDGET_KEY, attrAvgPowerBudget, attributes);
	}
	// DieSparingEnabled
	if (containsAttribute(DIESPARINGENABLED_KEY, attributes))
	{
		framework::Attribute attrAvgPowerBudget((bool)dimm.die_sparing_enabled, false);
		pInstance->setAttribute(DIESPARINGENABLED_KEY, attrAvgPowerBudget, attributes);
	}
	// DieSparingLevel
	if (containsAttribute(DIESPARINGLEVEL_KEY, attributes))
	{
		framework::Attribute attrAvgPowerBudget((NVM_UINT16)dimm.die_sparing_level, false);
		pInstance->setAttribute(DIESPARINGLEVEL_KEY, attrAvgPowerBudget, attributes);
	}
	// LastShutdownStatus
	if (containsAttribute(LASTSHUTDOWNSTATUS_KEY, attributes))
	{
		wbem::framework::UINT16_LIST shutdownStatuses;
		constructLastShutDownStatuses(dimm.status.last_shutdown_status, shutdownStatuses);

		framework::Attribute attrShutdownState(shutdownStatuses, false);
		pInstance->setAttribute(LASTSHUTDOWNSTATUS_KEY, attrShutdownState, attributes);
	}
	// Die Spares Used
	if (containsAttribute(DIESPARESUSED_KEY, attributes))
	{
		framework::Attribute attrDieSparesUsed((NVM_UINT8)dimm.status.die_spares_used, false);
		pInstance->setAttribute(DIESPARESUSED_KEY, attrDieSparesUsed, attributes);
	}
	// First fast refresh
	if (containsAttribute(FIRSTFASTREFRESH_KEY, attributes))
	{
		framework::Attribute attrFirstFastRefresh((bool)dimm.settings.first_fast_refresh, false);
		pInstance->setAttribute(FIRSTFASTREFRESH_KEY, attrFirstFastRefresh, attributes);
	}
	// Channel
	if (containsAttribute(CHANNEL_KEY, attributes))
	{
		framework::Attribute attrChannel((NVM_UINT32)dimm.discovery.device_handle.parts.mem_channel_id, false);
		pInstance->setAttribute(CHANNEL_KEY, attrChannel, attributes);
	}
	// Channel position
	if (containsAttribute(CHANNELPOS_KEY, attributes))
	{
		framework::Attribute attrChannelPos((NVM_UINT32)dimm.discovery.device_handle.parts.mem_channel_dimm_num, false);
		pInstance->setAttribute(CHANNELPOS_KEY, attrChannelPos, attributes);
	}
	// Config Status
	if (containsAttribute(CONFIGURATIONSTATUS_KEY, attributes))
	{
		std::string configStatusStr = deviceConfigStatusToStr(dimm.status.config_status);
		framework::Attribute attrConfigStatus((NVM_UINT16)dimm.status.config_status, configStatusStr, false);
		pInstance->setAttribute(CONFIGURATIONSTATUS_KEY, attrConfigStatus, attributes);
	}
	
	// Security Capabilities Status
	if (containsAttribute(SECURITYCAPABILITIES_KEY, attributes))
	{
		framework::UINT16_LIST securityCapabilities;
		if (dimm.discovery.security_capabilities.passphrase_capable)
		{
			securityCapabilities.push_back(SECURITY_PASSPHRASE);
		}

		if (dimm.discovery.security_capabilities.unlock_device_capable)
		{
			securityCapabilities.push_back(SECURITY_UNLOCK);
		}

		if (dimm.discovery.security_capabilities.erase_crypto_capable)
		{
			securityCapabilities.push_back(SECURITY_ERASE);
		}

		framework::Attribute a(securityCapabilities, false);
		pInstance->setAttribute(SECURITYCAPABILITIES_KEY, a, attributes);
	}

	// LastShutdownTime
	if (containsAttribute(LASTSHUTDOWNTIME_KEY, attributes))
	{
		framework::Attribute timetAttr((NVM_UINT64) (dimm.status.last_shutdown_time),
			wbem::framework::DATETIME_SUBTYPE_DATETIME, false);
		pInstance->setAttribute(LASTSHUTDOWNTIME_KEY, timetAttr, attributes);
	}

	// MemoryTypeCapabilities
	if (containsAttribute(MEMORYTYPECAPABILITIES_KEY, attributes))
	{
		framework::UINT16_LIST memCapabilitiesList;
		buildMemoryTypeCapabilitiesFromDeviceCapabilities(dimm.discovery.device_capabilities,
				memCapabilitiesList);

		framework::Attribute memCapabilitiesAttr(memCapabilitiesList, false);
		pInstance->setAttribute(MEMORYTYPECAPABILITIES_KEY, memCapabilitiesAttr);
	}

	// DieSparingCapable
	if (containsAttribute(DIESPARINGCAPABLE_KEY, attributes))
	{
		framework::Attribute dieSparingAttr(
				(bool)dimm.discovery.device_capabilities.die_sparing_capable,
				false);
		pInstance->setAttribute(DIESPARINGCAPABLE_KEY, dieSparingAttr);
	}

	// FwLogLevel
	if (containsAttribute(FWLOGLEVEL_KEY, attributes))
	{
		enum fw_log_level fwLogLevel = FW_LOG_LEVEL_UNKNOWN;
		if (dimm.discovery.manageability == MANAGEMENT_VALIDCONFIG)
		{
			int rc = NVM_SUCCESS;
			if ((rc = nvm_get_fw_log_level(dimm.discovery.guid, &fwLogLevel)) != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR_F("nvm_get_fw_log_level failed with rc = %d", rc);
				throw exception::NvmExceptionLibError(rc);
			}
		}

		framework::Attribute fwLogLevelAttr((NVM_UINT16)fwLogLevel, false);
		pInstance->setAttribute(FWLOGLEVEL_KEY, fwLogLevelAttr);
	}
}

void wbem::physical_asset::NVDIMMFactory::constructLastShutDownStatuses(NVM_UINT8 lastShutdownState,
	wbem::framework::UINT16_LIST &shutdownStatuses)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (lastShutdownState == SHUTDOWN_STATUS_UNKNOWN)
	{
		shutdownStatuses.push_back(DEVICE_LAST_SHUTDOWN_STATUS_UKNOWN);
	}
	else // can't be "Unknown" and "Known"
	{
		if (lastShutdownState & SHUTDOWN_STATUS_CLEAN)
		{
			shutdownStatuses.push_back(DEVICE_LAST_SHUTDOWN_STATUS_FW_FLUSH_COMPLETE);
		}

		if (lastShutdownState & SHUTDOWN_STATUS_PM_ADR)
		{
			shutdownStatuses.push_back(DEVICE_LAST_SHUTDOWN_STATUS_PM_ADR_COMMAND);
		}

		if (lastShutdownState & SHUTDOWN_STATUS_PM_S3)
		{
			shutdownStatuses.push_back(DEVICE_LAST_SHUTDOWN_STATUS_PM_S3);
		}

		if (lastShutdownState & SHUTDOWN_STATUS_PM_S5)
		{
			shutdownStatuses.push_back(DEVICE_LAST_SHUTDOWN_STATUS_PM_S5);
		}

		if (lastShutdownState & SHUTDOWN_STATUS_DDRT_POWER_FAIL)
		{
			shutdownStatuses.push_back(DEVICE_LAST_SHUTDOWN_STATUS_DDRT_POWER_FAIL);
		}

		if (lastShutdownState & SHUTDOWN_STATUS_PMIC_12V_POWER_FAIL)
		{
			shutdownStatuses.push_back(DEVICE_LAST_SHUTDOWN_STATUS_PMIC_12V_POWER_FAIL);
		}

		if (lastShutdownState & SHUTDOWN_STATUS_WARM_RESET)
		{
			shutdownStatuses.push_back(DEVICE_LAST_SHUTDOWN_STATUS_PM_WARM_RESET);
		}

		if (lastShutdownState & SHUTDOWN_STATUS_FORCED_THERMAL)
		{
			shutdownStatuses.push_back(DEVICE_LAST_SHUTDOWN_STATUS_THERMAL_SHUTDOWN);
		}
	}
}

void wbem::physical_asset::NVDIMMFactory::buildMemoryTypeCapabilitiesFromDeviceCapabilities(
		const struct device_capabilities& capabilities, framework::UINT16_LIST &memoryTypeCapabilities)
{
	if (capabilities.memory_mode_capable)
	{
		memoryTypeCapabilities.push_back(NVDIMM_MEMORYTYPECAPABILITIES_MEMORYMODE);
	}

	if (capabilities.storage_mode_capable)
	{
		memoryTypeCapabilities.push_back(NVDIMM_MEMORYTYPECAPABILITIES_STORAGEMODE);
	}

	if (capabilities.app_direct_mode_capable)
	{
		memoryTypeCapabilities.push_back(NVDIMM_MEMORYTYPECAPABILITIES_APPDIRECTMODE);
	}
}

/*
 * Return an object path for each NVDIMM in the system
 */
wbem::framework::instance_names_t* wbem::physical_asset::NVDIMMFactory::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		wbem::physical_asset::devices_t devices = NVDIMMFactory::getAllDevices();

		if (devices.size() != 0)
		{
			// create an object path for each dimm
			wbem::physical_asset::devices_t::const_iterator iter = devices.begin();
			for (; iter != devices.end(); iter++)
			{
				framework::ObjectPath path;
				createPathFromGuid(iter->guid, path);
				pNames->push_back(path);
			}
		}
		else // nvm_get_device_count returned 0 DIMMs
		{
			// should never get here except in SIM
			COMMON_LOG_DEBUG("No Intel NVDIMMs found.");
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}

	return pNames;
}

wbem::framework::UINT32 wbem::physical_asset::NVDIMMFactory::executeMethod(
		wbem::framework::UINT32 &wbemRc,
		const std::string method,
		wbem::framework::ObjectPath &object,
		wbem::framework::attributes_t &inParms,
		wbem::framework::attributes_t &outParms)
{
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;

	COMMON_LOG_ENTRY_PARAMS("methodName: %s, number of in params: %d", method.c_str(), (int)(inParms.size()));

	// tag is needed for all methods
	framework::Attribute tag = object.getKeyValue(TAG_KEY);
	std::string deviceGuid = tag.stringValue();
	try
	{
		if (method == NVDIMM_SETPASSPHRASE)
		{
			// uint32 SetPassphrase(string NewPassphrase, string CurrentPassphrase);
			setPassphrase(deviceGuid,
					inParms[NVDIMM_SETPASSPHRASE_NEWPASSPHRASE].stringValue(),
					inParms[NVDIMM_SETPASSPHRASE_CURRENTPASSPHRASE].stringValue());
		}
		else if (method == NVDIMM_REMOVEPASSPHRASE)
		{
			// uint32 RemovePassphrase(string CurrentPassphrase);
			removePassphrase(deviceGuid,
					inParms[NVDIMM_SETPASSPHRASE_CURRENTPASSPHRASE].stringValue());
		}
		else if (method == NVDIMM_UNLOCK)
		{
			// uint32 Unlock(string CurrentPassphrase);
			unlock(deviceGuid,
					inParms[NVDIMM_SETPASSPHRASE_CURRENTPASSPHRASE].stringValue());
		}
		else
		{
			httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
			COMMON_LOG_ERROR_F("methodName %s not supported", method.c_str());
		}
	}
	catch(framework::ExceptionBadParameter &)
	{
		wbemRc = framework::MOF_ERR_INVALIDPARAMETER;
	}
	catch(exception::NvmExceptionLibError &)
	{
		wbemRc = framework::MOF_ERR_FAILED;
	}
	catch(framework::ExceptionNoMemory &)
	{
		wbemRc = framework::MOF_ERR_FAILED;
	}
	catch(framework::ExceptionNotSupported &)
	{
		wbemRc = framework::MOF_ERR_NOTSUPPORTED;
	}
	catch(framework::Exception &)
	{
		wbemRc = framework::MOF_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN("httpRc: %u, wbemRc: %u", httpRc, wbemRc);
	return httpRc;
}

/*
 * set passphrase
 */
void wbem::physical_asset::NVDIMMFactory::setPassphrase(std::string deviceGuid, std::string newPassphrase,
		std::string currentPassphrase)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (deviceGuid.empty() || (deviceGuid.size() != NVM_GUIDSTR_LEN - 1))
	{
		throw framework::ExceptionBadParameter("deviceGuid");
	}
	// new passphrase is required
	if (newPassphrase.empty())
	{
		throw framework::ExceptionBadParameter(NVDIMM_SETPASSPHRASE_NEWPASSPHRASE.c_str());
	}
	NVM_GUID guid;
	str_to_guid(deviceGuid.c_str(), guid);

	// current passphrase is not required.  Pass NULL if it is not provided
	const char *oldPassphrase = NULL;
	int oldPassphraseLen = 0;
	if(!currentPassphrase.empty())
	{
		oldPassphrase = currentPassphrase.c_str();
		oldPassphraseLen = currentPassphrase.length();
	}

	int rc = m_SetPassphrase(guid, oldPassphrase, oldPassphraseLen,
			newPassphrase.c_str(), newPassphrase.length());
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

/*
 * remove passphrase
 */
void wbem::physical_asset::NVDIMMFactory::removePassphrase(
		std::string deviceGuid, std::string currentPassphrase)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (currentPassphrase.empty())
	{
		throw framework::ExceptionBadParameter(NVDIMM_SETPASSPHRASE_CURRENTPASSPHRASE.c_str());
	}
	if (deviceGuid.empty() || (deviceGuid.size() != NVM_GUIDSTR_LEN - 1))
	{
		throw framework::ExceptionBadParameter("deviceGuid");
	}

	NVM_GUID guid;
	str_to_guid(deviceGuid.c_str(), guid);

	int rc = m_RemovePassphrase(guid, currentPassphrase.c_str(), currentPassphrase.length());

	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

void wbem::physical_asset::NVDIMMFactory::unlock(std::string deviceGuid,
		std::string currentPassphrase)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (currentPassphrase.empty())
	{
		throw framework::ExceptionBadParameter(NVDIMM_SETPASSPHRASE_CURRENTPASSPHRASE.c_str());
	}
	if (deviceGuid.empty() || (deviceGuid.size() != NVM_GUIDSTR_LEN - 1))
	{
		throw framework::ExceptionBadParameter("deviceGuid");
	}

	NVM_GUID guid;
	str_to_guid(deviceGuid.c_str(), guid);

	int rc = m_UnlockDevice(guid, currentPassphrase.c_str(), currentPassphrase.length());

	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

/*
 * enumerate all devices found from API and return only manageable ones
 */
std::vector<std::string>  wbem::physical_asset::NVDIMMFactory::getManageableDeviceGuids()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::vector<std::string> result;

	wbem::physical_asset::devices_t devices = wbem::physical_asset::NVDIMMFactory::getManageableDevices();

	wbem::physical_asset::devices_t::const_iterator iter = devices.begin();
	for (; iter != devices.end(); iter++)
	{
		NVM_GUID_STR guidStr;
		guid_to_str(iter->guid, guidStr);
		result.push_back(std::string(guidStr));
	}

	return result;
}

/*
 * Attempt to get the device discovery and check for manageability.
 * If can get it and discovery and is manageable return NVM_SUCCESS.
 * If doesn't exist, return NVM_ERR_BADDEVICE.
 * If is not manageable, return NVM_ERR_NOTMANAGEABLE
 */
int wbem::physical_asset::NVDIMMFactory::existsAndIsManageable(const std::string &dimmGuid)
{
	struct device_discovery discover;
	NVM_GUID guid;
	str_to_guid(dimmGuid.c_str(), guid);
	int rc = NVM_SUCCESS;

	if (NVM_SUCCESS != nvm_get_device_discovery(guid, &discover))
	{
		rc = NVM_ERR_BADDEVICE;
	}
	else if (discover.manageability != MANAGEMENT_VALIDCONFIG)
	{
		rc = NVM_ERR_NOTMANAGEABLE;
	}
	return rc;
}

wbem::framework::Instance* wbem::physical_asset::NVDIMMFactory::modifyInstance(framework::ObjectPath &path,
		framework::attributes_t &attributes) throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::Instance *pInstance = NULL;

	try
	{
		// get the device GUID from the path
		framework::Attribute tagAttribute = path.getKeyValue(TAG_KEY);
		NVM_GUID guid;
		str_to_guid(tagAttribute.stringValue().c_str(), guid);


		framework::attribute_names_t attributeNames;
		pInstance = getInstance(path, attributeNames);

		framework::attribute_names_t modifyableAttributes;
		modifyableAttributes.push_back(FIRSTFASTREFRESH_KEY);
		modifyableAttributes.push_back(FWLOGLEVEL_KEY);

		checkAttributesAreModifiable(pInstance, attributes, modifyableAttributes);

		if (pInstance)
		{
			if (attributes.count(FIRSTFASTREFRESH_KEY))
			{
				int rc = 0;
				struct device_settings settings;
				framework::Attribute attr = attributes[FIRSTFASTREFRESH_KEY];

				settings.first_fast_refresh = attr.boolValue() ? 1 : 0;

				if ((rc = nvm_modify_device_settings(guid, &settings)) == NVM_SUCCESS)
				{
					pInstance->setAttribute(FIRSTFASTREFRESH_KEY, attr);
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

				if ((rc = nvm_set_fw_log_level(guid, fwLogLevel)) == NVM_SUCCESS)
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
enum fw_log_level wbem::physical_asset::NVDIMMFactory::convertToLogLevelEnum(NVM_UINT16 logLevel)
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

/*
 * Helper to convert device config status to a string
 */
std::string wbem::physical_asset::NVDIMMFactory::deviceConfigStatusToStr(int configStatus)
{
	std::string configStatusStr;
	switch (configStatus)
	{
		case CONFIG_STATUS_NOT_CONFIGURED:
			configStatusStr = "Not configured";
			break;

		case CONFIG_STATUS_VALID:
			configStatusStr = "Valid";
			break;

		case CONFIG_STATUS_ERR_CORRUPT:
			configStatusStr = "Failed - Bad configuration";
			break;

		case CONFIG_STATUS_ERR_BROKEN_INTERLEAVE:
			configStatusStr = "Failed - Broken interleave";
			break;

		case CONFIG_STATUS_ERR_REVERTED:
			configStatusStr = "Failed - Reverted";
			break;

		case CONFIG_STATUS_ERR_NOT_SUPPORTED:
			configStatusStr = "Failed - Unsupported";
			break;

		default:
			COMMON_LOG_ERROR_F("config status %d does not match any valid status", configStatus);
			configStatusStr = "Unknown";
			break;
	}
	return configStatusStr;
}

void wbem::physical_asset::NVDIMMFactory::createPathFromGuid(const std::string guid,
		wbem::framework::ObjectPath &path)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::attributes_t keys;

	keys[TAG_KEY] = framework::Attribute (guid, true);

	// CreationClassName = physical_asset::NVDIMM_CREATIONCLASSNAME
	framework::Attribute attrCCName(NVDIMM_CREATIONCLASSNAME, true);
	keys.insert(std::pair<std::string, framework::Attribute>(
			CREATIONCLASSNAME_KEY, attrCCName));

	// generate the ObjectPath for the instance
	path.setObjectPath(server::getHostName(), NVM_NAMESPACE,
			NVDIMM_CREATIONCLASSNAME, keys);
}


void wbem::physical_asset::NVDIMMFactory::createPathFromGuid(const NVM_GUID guid, framework::ObjectPath &path)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);


	// Tag = DIMM GUID
	NVM_GUID_STR guidStr;
	guid_to_str(guid, guidStr);

	createPathFromGuid(std::string(guidStr), path);

}

wbem::physical_asset::devices_t wbem::physical_asset::NVDIMMFactory::getAllDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	devices_t devices;

	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();
	pApi->getDevices(devices);

	return devices;
}

wbem::physical_asset::devices_t wbem::physical_asset::NVDIMMFactory::getManageableDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	devices_t devices, manageableDevices;

	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();
	pApi->getManageableDimms(manageableDevices);

	return manageableDevices;
}

std::string wbem::physical_asset::NVDIMMFactory::memoryTypetoStr(enum memory_type memoryType)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string typeStr;
	switch (memoryType)
	{
		case MEMORY_TYPE_UNKNOWN:
			typeStr = "Unknown";
			break;
		case MEMORY_TYPE_DDR4:
			typeStr = "DDR4";
			break;
		case MEMORY_TYPE_NVMDIMM:
			typeStr = "NVM-DIMM";
			break;
	}
	return typeStr;
}

wbem::framework::UINT16_LIST wbem::physical_asset::NVDIMMFactory::deviceStatusToOpStatus(
		const struct device_status *p_status)
{
	framework::UINT16_LIST opStatus;
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

    // Apply device health
	if (p_status->health == DEVICE_HEALTH_UNKNOWN)
	{
		opStatus.push_back(NVDIMM_OPSTATUS_UNKNOWN);
	}
	else if (p_status->health == DEVICE_HEALTH_NORMAL)
	{
		opStatus.push_back(NVDIMM_OPSTATUS_OK);
	}
	else if (p_status->health == DEVICE_HEALTH_NONCRITICAL)
	{
		opStatus.push_back(NVDIMM_OPSTATUS_DEGRADED);
	}
	else if (p_status->health == DEVICE_HEALTH_CRITICAL)
	{
		opStatus.push_back(NVDIMM_OPSTATUS_ERROR);
	}
	else if (p_status->health == DEVICE_HEALTH_FATAL)
	{
		opStatus.push_back(NVDIMM_OPSTATUS_NONRECOVERABLEERROR);
	}

	// Apply SKU statuses
	if (p_status->mixed_sku)
	{
		opStatus.push_back(NVDIMM_OPSTATUS_MIXEDSKU);
	}
	if (p_status->sku_violation)
	{
		opStatus.push_back(NVDIMM_OPSTATUS_SKUVIOLATION);
	}
	return opStatus;
}

bool wbem::physical_asset::NVDIMMFactory::attributeHasChanged(framework::Attribute oldAttr, framework::Attribute newAttr)
{
	return oldAttr != newAttr;
}

bool wbem::physical_asset::NVDIMMFactory::attributeIsModifiable(
		framework::attribute_names_t modifiableAttributes, std::string attributeThatWasModified)
{
	return (std::find(modifiableAttributes.begin(), modifiableAttributes.end(), attributeThatWasModified)
								!= modifiableAttributes.end());
}

void wbem::physical_asset::NVDIMMFactory::checkAttributesAreModifiable(
		wbem::framework::Instance *pInstance, wbem::framework::attributes_t &attributesToModify,
		wbem::framework::attribute_names_t modifiableAttributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::attributes_t::iterator iter = attributesToModify.begin();
	for (; iter != attributesToModify.end(); iter++)
	{
		std::string attributeName = iter->first;

		framework::Attribute instanceAttr;
		if (pInstance->getAttribute(attributeName, instanceAttr) == NVM_SUCCESS)
		{
			if ((attributeHasChanged(instanceAttr, iter->second)) && !attributeIsModifiable(modifiableAttributes, iter->first))
			{
				COMMON_LOG_ERROR_F("Cannot modify attribute %s", attributeName.c_str());
				throw framework::ExceptionBadAttribute(iter->first.c_str());
			}
		}
	}
}
