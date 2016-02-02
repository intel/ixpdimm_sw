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
 * This file contains the provider for the NVDIMMView instances which
 * is an internal only NVM DIMM view used by the CLI.
 */

#include <exception/NvmExceptionLibError.h>
#include <intel_cim_framework/ExceptionBadParameter.h>
#include <physical_asset/NVDIMMViewFactory.h>
#include <LogEnterExit.h>
#include <logic/exceptions/LibraryException.h>
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include <logic/exceptions/InvalidArgumentException.h>
#include <intel_cim_framework/ExceptionNoMemory.h>

#include "NVDIMMFactory.h"
#include "framework_interface/FrameworkExtensions.h"

void wbem::physical_asset::NVDIMMViewFactory::populateAttributeList(
	framework::attribute_names_t &attributes)
throw(wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(DIMMGUID_KEY);

	// add non-key NVDIMM attributes
	attributes.push_back(DIMMID_KEY);
	attributes.push_back(DIMMHANDLE_KEY);
	attributes.push_back(PHYSICALID_KEY);
	attributes.push_back(MANUFACTURER_KEY);
	attributes.push_back(MANUFACTURERID_KEY);
	attributes.push_back(MODEL_KEY);
	attributes.push_back(CAPACITY_KEY);
	attributes.push_back(VENDORID_KEY);
	attributes.push_back(DEVICEID_KEY);
	attributes.push_back(REVISIONID_KEY);
	attributes.push_back(SOCKETID_KEY);
	attributes.push_back(MEMORYTYPE_KEY);
	attributes.push_back(SERIALNUMBER_KEY);
	attributes.push_back(LOCKSTATE_KEY);
	attributes.push_back(MANAGEABILITYSTATE_KEY);
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
	attributes.push_back(DIESPARINGCAPABLE_KEY);
	attributes.push_back(MEMORYTYPECAPABILITIES_KEY);
	attributes.push_back(FWLOGLEVEL_KEY);

	// Add View Attributes
	attributes.push_back(FWLOGLEVEL_KEY);
	attributes.push_back(FWAPIVERSION_KEY);
	attributes.push_back(FWVERSION_KEY);
	attributes.push_back(UNCONFIGUREDCAPACITY_KEY);
	attributes.push_back(INACCESSIBLECAPACITY_KEY);
	attributes.push_back(RESERVEDCAPACITY_KEY);
	attributes.push_back(INTERFACEFORMATCODE_KEY);
	attributes.push_back(DEVICELOCATOR_KEY);
	attributes.push_back(ACTIONREQUIRED_KEY);
	attributes.push_back(ACTIONREQUIREDEVENTS_KEY);
	attributes.push_back(MEMORYMODESSUPPORTED_KEY);
	attributes.push_back(MIXEDSKU_KEY);
	attributes.push_back(SKUVIOLATION_KEY);
	attributes.push_back(MEMCONTROLLERID_KEY);
}

wbem::framework::instances_t *wbem::physical_asset::NVDIMMViewFactory::getInstances(
	wbem::framework::attribute_names_t &attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	checkAttributes(attributes);

	try
	{
		logic::device::DeviceCollection devices = m_deviceService.getAllDevices();

		framework::instances_t *pResult = new framework::instances_t();
		if (!pResult)
		{
			throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__, "pResult");
		}
		for (size_t i = 0; i < devices.size(); i++)
		{
			framework::ObjectPath path = createPath(devices[i].getGuid());
			framework::Instance instance(path);

			toInstance(devices[i], instance, attributes);

			pResult->push_back(instance);
		}
		return pResult;
	}
	catch(logic::LibraryException &e)
	{
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}
	catch(logic::InvalidArgumentException &e)
	{
		throw framework::ExceptionBadParameter(e.getArgumentName().c_str());
	}
}

wbem::framework::Instance *wbem::physical_asset::NVDIMMViewFactory::getInstance(
	wbem::framework::ObjectPath &path, wbem::framework::attribute_names_t &attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	checkAttributes(attributes);
	framework::Instance *pResult = new framework::Instance(path);
	if (!pResult)
	{
		throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__, "pResult");
	}

	framework::Attribute guidAttr = path.getKeyValue(DIMMGUID_KEY);

	try
	{
		logic::device::Device *pNvdimm = m_deviceService.getDevice(guidAttr.stringValue());

		toInstance(*pNvdimm, *pResult, attributes);
		delete pNvdimm;
	}
	catch(logic::LibraryException &e)
	{
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}
	catch(logic::InvalidArgumentException &e)
	{
		throw framework::ExceptionBadParameter(e.getArgumentName().c_str());
	}

	return pResult;
}

wbem::framework::instance_names_t *wbem::physical_asset::NVDIMMViewFactory::getInstanceNames()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames;
	try
	{
		const std::vector<std::string> &guids = m_deviceService.getAllGuids();
		pNames = new framework::instance_names_t();
		if (!pNames)
		{
			throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__, "pNames");
		}
		for (size_t i = 0; i < guids.size(); i++)
		{
			pNames->push_back(createPath(guids[i]));
		}
	}
	catch (logic::LibraryException &e)
	{
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}

	return pNames;
}

wbem::framework::ObjectPath wbem::physical_asset::NVDIMMViewFactory::createPath(const std::string &guid)
{
	framework::attributes_t keys;
	ADD_KEY_ATTRIBUTE(keys, DIMMGUID_KEY, framework::STR, guid);
	framework::ObjectPath path(getHostName(), NVM_NAMESPACE, NVDIMMVIEW_CREATIONCLASSNAME, keys);

	return path;
}

std::string wbem::physical_asset::NVDIMMViewFactory::getHostName()
{
	if (m_hostName.empty())
	{
		m_hostName = m_systemService.getHostName();
	}
	return m_hostName;
}


void wbem::physical_asset::NVDIMMViewFactory::toInstance(logic::device::Device &nvdimm,
	wbem::framework::Instance &instance, wbem::framework::attribute_names_t attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// enum to string look ups
	std::map<memory_type, std::string> memoryMap;
	memoryMap[MEMORY_TYPE_UNKNOWN] = "Unknown";
	memoryMap[MEMORY_TYPE_DDR4] = "DDR4";
	memoryMap[MEMORY_TYPE_NVMDIMM] = "NVM-DIMM";

	std::map<lock_state, std::string> lockStateMap;
	lockStateMap[LOCK_STATE_UNKNOWN] = "Unknown";
	lockStateMap[LOCK_STATE_DISABLED] = "Disabled";
	lockStateMap[LOCK_STATE_UNLOCKED] = "Unlocked";
	lockStateMap[LOCK_STATE_LOCKED] = "Locked";
	lockStateMap[LOCK_STATE_FROZEN] = "Frozen";
	lockStateMap[LOCK_STATE_PASSPHRASE_LIMIT] = "Exceeded";
	lockStateMap[LOCK_STATE_NOT_SUPPORTED] = "Not Supported";

	std::map<manageability_state, std::string> manageabilityMap;
	manageabilityMap[MANAGEMENT_VALIDCONFIG] = "Manageable";
	manageabilityMap[MANAGEMENT_INVALIDCONFIG] = "Unmanageable";
	manageabilityMap[MANAGEMENT_UNKNOWN] = "Unknown";

	std::map<device_form_factor, std::string> formFactorMap; 
	formFactorMap[DEVICE_FORM_FACTOR_DIMM] = "DIMM";
	formFactorMap[DEVICE_FORM_FACTOR_SODIMM] = "SODIMM";
	formFactorMap[DEVICE_FORM_FACTOR_UNKNOWN] = "Unknown";

	NVM_UINT16 DEVICE_HEALTH_UNMANAGEABLE = 65534; //!< Additional health state for unmanageable dimms
	std::map<framework::UINT16, std::string> healthStateMap;
	healthStateMap[DEVICE_HEALTH_UNKNOWN] = "Unknown";
	healthStateMap[DEVICE_HEALTH_NORMAL] = "OK";
	healthStateMap[DEVICE_HEALTH_NONCRITICAL] = "Minor Failure";
	healthStateMap[DEVICE_HEALTH_CRITICAL] = "Critical Failure";
	healthStateMap[DEVICE_HEALTH_FATAL] = "Non-recoverable error";
	healthStateMap[DEVICE_HEALTH_UNMANAGEABLE] = "Unmanageable";

	std::map<config_status, std::string> configStatusMap;
	configStatusMap[CONFIG_STATUS_NOT_CONFIGURED] = "Not configured";
	configStatusMap[CONFIG_STATUS_VALID] = "Valid";
	configStatusMap[CONFIG_STATUS_ERR_CORRUPT] = "Failed - Bad configuration";
	configStatusMap[CONFIG_STATUS_ERR_BROKEN_INTERLEAVE] = "Failed - Broken interleave";
	configStatusMap[CONFIG_STATUS_ERR_REVERTED] = "Failed - Reverted";
	configStatusMap[CONFIG_STATUS_ERR_NOT_SUPPORTED] = "Failed - Unsupported";

	std::map<fw_log_level, std::string> fwLogLevelMap;
	fwLogLevelMap[FW_LOG_LEVEL_DISABLED] = "Disabled";
	fwLogLevelMap[FW_LOG_LEVEL_ERROR] = "Error";
	fwLogLevelMap[FW_LOG_LEVEL_WARN] = "Warning";
	fwLogLevelMap[FW_LOG_LEVEL_INFO] = "Info";
	fwLogLevelMap[FW_LOG_LEVEL_DEBUG] = "Debug";
	fwLogLevelMap[FW_LOG_LEVEL_UNKNOWN] = "Unknown";

	std::map<NVM_UINT16, std::string> communicationStatusMap;
	communicationStatusMap[NVDIMM_COMMUNICATION_NOCONTACT] = "No Contact";
	communicationStatusMap[NVDIMM_COMMUNICATION_OK] = "Communication OK";

	ADD_ATTRIBUTE(instance, attributes, DIMMID_KEY, framework::STR, getDimmId(nvdimm));
	ADD_ATTRIBUTE(instance, attributes, DIMMHANDLE_KEY, framework::UINT32, nvdimm.getDeviceHandle());
	ADD_ATTRIBUTE(instance, attributes, PHYSICALID_KEY, framework::UINT16, nvdimm.getPhysicalId());
	ADD_ATTRIBUTE(instance, attributes, MANUFACTURER_KEY, framework::STR, nvdimm.getManufacturer());
	ADD_ATTRIBUTE(instance, attributes, MANUFACTURERID_KEY, framework::UINT16, nvdimm.getManufacturerId());
	ADD_ATTRIBUTE(instance, attributes, MODEL_KEY, framework::STR, nvdimm.getModelNumber());
	ADD_ATTRIBUTE(instance, attributes, CAPACITY_KEY, framework::UINT64, nvdimm.getRawCapacity());
	ADD_ATTRIBUTE(instance, attributes, VENDORID_KEY, framework::UINT16, nvdimm.getVendorId());
	ADD_ATTRIBUTE(instance, attributes, DEVICEID_KEY, framework::UINT16, nvdimm.getDeviceId());
	ADD_ATTRIBUTE(instance, attributes, REVISIONID_KEY, framework::UINT16, nvdimm.getRevisionId());
	ADD_ATTRIBUTE(instance, attributes, SOCKETID_KEY, framework::UINT16, nvdimm.getSocketId());
	ADD_ENUM_ATTRIBUTE(instance, attributes, MEMORYTYPE_KEY, framework::UINT16,  nvdimm.getMemoryType(), memoryMap);
	ADD_ATTRIBUTE(instance, attributes, SERIALNUMBER_KEY, framework::STR, nvdimm.getSerialNumber());
	ADD_ENUM_ATTRIBUTE(instance, attributes, LOCKSTATE_KEY, framework::UINT16, nvdimm.getLockState(), lockStateMap);
	ADD_ENUM_ATTRIBUTE(instance, attributes, MANAGEABILITYSTATE_KEY, framework::UINT16, nvdimm.getManageabilityState(), manageabilityMap);
	ADD_ENUM_ATTRIBUTE(instance, attributes, FORMFACTOR_KEY, framework::UINT16, nvdimm.getFormFactor(), formFactorMap);
	ADD_ATTRIBUTE(instance, attributes, DATAWIDTH_KEY, framework::UINT64, nvdimm.getDataWidth());
	ADD_ATTRIBUTE(instance, attributes, TOTALWIDTH_KEY, framework::UINT64, nvdimm.getTotalWidth());
	ADD_ATTRIBUTE(instance, attributes, SPEED_KEY, framework::UINT64, nvdimm.getSpeed());
	ADD_ATTRIBUTE(instance, attributes, VOLATILECAPACITY_KEY, framework::UINT64, nvdimm.getVolatileCapacity());
	ADD_ATTRIBUTE(instance, attributes, PERSISTENTCAPACITY_KEY, framework::UINT64, nvdimm.getPersistentCapacity());
	ADD_ATTRIBUTE(instance, attributes, PARTNUMBER_KEY, framework::STR, nvdimm.getPartNumber());
	ADD_ATTRIBUTE(instance, attributes, BANKLABEL_KEY, framework::STR, nvdimm.getBankLabel());
	ADD_ENUM_ATTRIBUTE(instance, attributes, HEALTHSTATE_KEY, framework::UINT16, nvdimm.getHealthState(), healthStateMap);
	framework::UINT16 communicationStatus = (framework::UINT16)(nvdimm.getIsMissing() ? NVDIMM_COMMUNICATION_NOCONTACT : NVDIMM_COMMUNICATION_OK);
	ADD_ENUM_ATTRIBUTE(instance, attributes, COMMUNICATIONSTATUS_KEY, framework::UINT16, communicationStatus, communicationStatusMap);
	ADD_ATTRIBUTE(instance, attributes, ISNEW_KEY, framework::BOOLEAN, nvdimm.isNew());
	ADD_ATTRIBUTE(instance, attributes, POWERMANAGEMENTENABLED_KEY, framework::BOOLEAN, nvdimm.getPowerManagementEnabled());
	ADD_ATTRIBUTE(instance, attributes, POWERLIMIT_KEY, framework::UINT8, nvdimm.getPowerLimit());
	ADD_ATTRIBUTE(instance, attributes, PEAKPOWERBUDGET_KEY, framework::UINT16, nvdimm.getPeakPowerBudget());
	ADD_ATTRIBUTE(instance, attributes, AVGPOWERBUDGET_KEY, framework::UINT16, nvdimm.getAvgPowerBudget());
	ADD_ATTRIBUTE(instance, attributes, DIESPARINGENABLED_KEY, framework::BOOLEAN, nvdimm.getDieSparingEnabled());
	ADD_ATTRIBUTE(instance, attributes, DIESPARINGLEVEL_KEY, framework::UINT8, nvdimm.getDieSparingLevel());
	ADD_ATTRIBUTE(instance, attributes, LASTSHUTDOWNSTATUS_KEY, framework::UINT16_LIST, nvdimm.getLastShutdownStatus());
	ADD_ATTRIBUTE(instance, attributes, DIESPARESUSED_KEY, framework::UINT8, nvdimm.getDieSparesUsed());
	ADD_ATTRIBUTE(instance, attributes, FIRSTFASTREFRESH_KEY, framework::BOOLEAN, nvdimm.isFirstFastRefresh());
	ADD_ATTRIBUTE(instance, attributes, CHANNEL_KEY, framework::UINT32, nvdimm.getChannelId());
	ADD_ATTRIBUTE(instance, attributes, CHANNELPOS_KEY, framework::UINT32, nvdimm.getChannelPosition());
	ADD_ENUM_ATTRIBUTE(instance, attributes, CONFIGURATIONSTATUS_KEY, framework::UINT16, nvdimm.getConfigStatus(), configStatusMap);
	ADD_ATTRIBUTE(instance, attributes, SECURITYCAPABILITIES_KEY, framework::UINT32_LIST, nvdimm.getSecurityCapabilities());
	ADD_DATETIME_ATTRIBUTE(instance, attributes, LASTSHUTDOWNTIME_KEY, nvdimm.getLastShutdownTime());
	ADD_ATTRIBUTE(instance, attributes, DIESPARINGCAPABLE_KEY, framework::BOOLEAN, nvdimm.isDieSparingCapable());
	ADD_ATTRIBUTE(instance, attributes, MEMORYTYPECAPABILITIES_KEY, framework::UINT16_LIST, nvdimm.getMemoryCapabilities());
	ADD_ENUM_ATTRIBUTE(instance, attributes, FWLOGLEVEL_KEY, framework::UINT16, nvdimm.getFwLogLevel(), fwLogLevelMap);
	ADD_ATTRIBUTE(instance, attributes, FWAPIVERSION_KEY, framework::STR, nvdimm.getFwApiVersion());
	ADD_ATTRIBUTE(instance, attributes, FWVERSION_KEY, framework::STR, nvdimm.getFwRevision());
	ADD_ATTRIBUTE(instance, attributes, UNCONFIGUREDCAPACITY_KEY, framework::UINT64, nvdimm.getUnconfiguredCapacity());
	ADD_ATTRIBUTE(instance, attributes, INACCESSIBLECAPACITY_KEY, framework::UINT64, nvdimm.getInaccessibleCapacity());
	ADD_ATTRIBUTE(instance, attributes, RESERVEDCAPACITY_KEY, framework::UINT64, nvdimm.getReservedCapacity());
	ADD_ATTRIBUTE(instance, attributes, INTERFACEFORMATCODE_KEY, framework::UINT16, nvdimm.getInterfaceFormatCode());
	ADD_ATTRIBUTE(instance, attributes, DEVICELOCATOR_KEY, framework::STR, nvdimm.getDeviceLocator());
	ADD_ATTRIBUTE(instance, attributes, ACTIONREQUIRED_KEY, framework::BOOLEAN, nvdimm.isActionRequired());
	ADD_ATTRIBUTE(instance, attributes, ACTIONREQUIREDEVENTS_KEY, framework::STR_LIST, nvdimm.getActionRequiredEvents());
	ADD_ATTRIBUTE(instance, attributes, MEMORYMODESSUPPORTED_KEY, framework::STR, getMemoryModeString(nvdimm));
	ADD_ATTRIBUTE(instance, attributes, MIXEDSKU_KEY, framework::BOOLEAN, nvdimm.isMixedSku());
	ADD_ATTRIBUTE(instance, attributes, SKUVIOLATION_KEY, framework::BOOLEAN, nvdimm.isSkuViolation());
	ADD_ATTRIBUTE(instance, attributes, MEMCONTROLLERID_KEY, framework::UINT16, nvdimm.getMemoryControllerId());
}

std::string wbem::physical_asset::NVDIMMViewFactory::getMemoryModeString(
	logic::device::Device &nvdimm)
{
	std::map<NVM_UINT32, std::string> map;
	map[NVDIMM_MEMORYTYPECAPABILITIES_MEMORYMODE] = TR("2LM");
	map[NVDIMM_MEMORYTYPECAPABILITIES_STORAGEMODE] = TR("Storage");
	map[NVDIMM_MEMORYTYPECAPABILITIES_APPDIRECTMODE] = TR("AppDirect");

	std::stringstream result;
	const std::vector<NVM_UINT16> &capabilities = nvdimm.getMemoryCapabilities();
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

std::string  wbem::physical_asset::NVDIMMViewFactory::getDimmId(logic::device::Device &nvdimm)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream result;
	// look up the database setting for the preferred DimmID output
	// default to using dimm handle
	bool useHandle = true;
	char value[CONFIG_VALUE_LEN];
	if (get_config_value(SQL_KEY_CLI_DIMM_ID, value) == COMMON_SUCCESS)
	{
		// switch to guid
		if (s_strncmpi("GUID", value, strlen("GUID")) == 0)
		{
			useHandle = false;
		}
	}

	// convert GUID to handle
	if (useHandle)
	{
		result << nvdimm.getDeviceHandle();
	}
		// use GUID
	else
	{
		result << nvdimm.getGuid();
	}
	return result.str();
}
