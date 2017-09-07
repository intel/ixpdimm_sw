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

#include <LogEnterExit.h>
#include <cli/features/core/WbemToCli_utilities.h>
#include <libinvm-cli/CliFrameworkTypes.h>
#include <cli/features/core/CommandParts.h>
#include <persistence/config_settings.h>
#include <persistence/lib_persistence.h>
#include <cli/features/core/framework/CliHelper.h>
#include <iomanip>
#include <libinvm-cli/SyntaxErrorMissingValueResult.h>
#include "ShowDeviceCommand.h"
#include "ShowCommandPropertyUtilities.h"
#include "ShowCommandUtilities.h"
#include <common/string/s_str.h>

namespace cli
{
namespace nvmcli
{

static const std::string ZERO_FW_VERSION = "00.00.00.0000";
std::string ShowDeviceCommand::m_capacityUnits = "";

ShowDeviceCommand::ShowDeviceCommand(core::device::DeviceService &service)
	: m_service(service), m_pResult(NULL)
{
	m_props.addCustom("DimmID", ShowCommandUtilities::getDimmId).setIsRequired();
	m_props.addUint64("Capacity", &core::device::Device::getRawCapacity, convertCapacity).setIsDefault();
	m_props.addUint16("HealthState", &core::device::Device::getHealthState,
			&convertHealthState).setIsDefault();
	m_props.addBool("ActionRequired", &core::device::Device::isActionRequired).setIsDefault();
	m_props.addList("InterfaceFormatCode", &core::device::Device::getInterfaceFormatCodes,
			&convertInterfaceFormatCode);
	m_props.addOther("ManageabilityState", &core::device::Device::getManageabilityState,
			&convertManageabilityState);
	m_props.addUint16("PhysicalID", &core::device::Device::getPhysicalId);
	m_props.addUint32("DimmHandle", &core::device::Device::getDeviceHandle);
	m_props.addStr("DimmUID", &core::device::Device::getUid);
	m_props.addUint16("SocketID", &core::device::Device::getSocketId);
	m_props.addUint16("MemControllerID", &core::device::Device::getMemoryControllerId);
	m_props.addUint32("ChannelID", &core::device::Device::getChannelId);
	m_props.addUint32("ChannelPos", &core::device::Device::getChannelPosition);
	m_props.addOther("MemoryType", &core::device::Device::getMemoryType, &convertMemoryType);
	m_props.addUint16("VendorID", &core::device::Device::getVendorId, toHex);
	m_props.addUint16("DeviceID", &core::device::Device::getDeviceId, toHex);
	m_props.addUint16("RevisionID", &core::device::Device::getRevisionId, toHex);
	// New NFIT attributes here
	m_props.addStr("SerialNumber", &core::device::Device::getSerialNumber);
	m_props.addUint16("SubsystemVendorID", &core::device::Device::getSubsystemVendor, toHex);
	m_props.addUint16("SubsystemDeviceID", &core::device::Device::getSubsystemDevice, toHex);
	m_props.addUint16("SubsystemRevisionID", &core::device::Device::getSubsystemRevision, toHex);
	m_props.addBool("ManufacturingInfoValid", &core::device::Device::isManufacturingInfoValid);
	m_props.addCustom("ManufacturingLocation", &getManufacturingLoc);
	m_props.addCustom("ManufacturingDate", &getManufacturingDate);
	m_props.addStr("DeviceLocator", &core::device::Device::getDeviceLocator);
	m_props.addStr("BankLabel", &core::device::Device::getBankLabel);
	m_props.addUint64("DataWidth", &core::device::Device::getDataWidth, &convertWidth);
	m_props.addUint64("TotalWidth", &core::device::Device::getTotalWidth, &convertWidth);
	m_props.addUint64("Speed", &core::device::Device::getSpeed, &convertSpeed);
	m_props.addCustom("ActionRequiredEvents", getActionRequiredEvents);
	m_props.addOther("LockState", &core::device::Device::getLockState, &convertLockState).setIsDefault();
	m_props.addOther("FWVersion", &core::device::Device::getFwRevision, &convertFwVersion).setIsDefault();
	m_props.addStr("FWAPIVersion", &core::device::Device::getFwApiVersion);
	m_props.addStr("Manufacturer", &core::device::Device::getManufacturer);
	m_props.addUint16("ManufacturerID", &core::device::Device::getManufacturerId, toHex);
	m_props.addStr("PartNumber", &core::device::Device::getPartNumber);
	m_props.addBool("IsNew", &core::device::Device::isNew);
	m_props.addOther("FormFactor", &core::device::Device::getFormFactor, &convertFormFactor);
	m_props.addUint64("MemoryCapacity", &core::device::Device::getMemoryCapacityBytes,
		convertCapacity);
	m_props.addUint64("AppDirectCapacity", &core::device::Device::getAppDirectCapacityBytes,
		convertCapacity);
	m_props.addUint64("UnconfiguredCapacity", &core::device::Device::getUnconfiguredCapacityBytes,
		convertCapacity);
	m_props.addUint64("InaccessibleCapacity", &core::device::Device::getInaccessibleCapacityBytes,
		convertCapacity);
	m_props.addUint64("ReservedCapacity", &core::device::Device::getReservedCapacityBytes,
		convertCapacity);
	m_props.addOther("FWLogLevel", &core::device::Device::getFwLogLevel, &convertFwLogLevel);
	m_props.addBool("PowerManagementEnabled", &core::device::Device::isPowerManagementEnabled);
	m_props.addUint8("PowerLimit", &core::device::Device::getPowerLimit, &convertPowerLimit);
	m_props.addUint16("PeakPowerBudget", &core::device::Device::getPeakPowerBudget, &convertPowerBudget);
	m_props.addUint16("AvgPowerBudget", &core::device::Device::getAvgPowerBudget, &convertPowerBudget);
	m_props.addBool("DieSparingCapable", &core::device::Device::isDieSparingCapable);
	m_props.addBool("DieSparingEnabled", &core::device::Device::isDieSparingEnabled);
	m_props.addUint8("DieSparingLevel", &core::device::Device::getDieSparingLevel);
	m_props.addUint8("DieSparesAvailable", &core::device::Device::getDieSparesAvailable);
	m_props.addList("LastShutdownStatus", &core::device::Device::getLastShutdownStatus,
			&convertLastShutdownStatus);
	m_props.addUint64("LastShutdownTime", &core::device::Device::getLastShutdownTime, &convertToDate);
	m_props.addBool("FirstFastRefresh", &core::device::Device::isFirstFastRefresh);
	m_props.addList("ModesSupported", &core::device::Device::getMemoryCapabilities,
			&convertMemoryModes);
	m_props.addList("SecurityCapabilities", &core::device::Device::getSecurityCapabilities,
			&convertSecurityCapabilities);
	m_props.addOther("ConfigurationStatus", &core::device::Device::getConfigStatus,
			&convertConfigStatus);
	m_props.addOther("ARSStatus", &core::device::Device::getArsStatus, &convertArsStatus);
	m_props.addOther("SanitizeStatus", &core::device::Device::getSanitizeStatus, &convertSanitizeStatus);
	m_props.addBool("SKUViolation", &core::device::Device::isSkuViolation);
	m_props.addBool("ViralPolicy", &core::device::Device::isViralPolicyEnabled);
	m_props.addBool("ViralState", &core::device::Device::getCurrentViralState);
	m_props.addBool("AitDramEnabled", &core::device::Device::isAitDramEnabled);
	m_props.addList("BootStatus", &core::device::Device::getBootStatus, &convertBootStatus);
	m_props.addUint32("InjectedMediaErrors", &core::device::Device::getInjectedMediaErrors);
	m_props.addUint32("InjectedNonMediaErrors", &core::device::Device::getInjectedNonMediaErrors);
}

framework::ResultBase *ShowDeviceCommand::execute(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_parsedCommand = parsedCommand;
	m_dimmIds = framework::CliHelper::splitCommaSeperatedString(m_parsedCommand.targets[TARGET_DIMM.name]);
	m_socketIds = framework::CliHelper::splitCommaSeperatedString(m_parsedCommand.targets[TARGET_SOCKET.name]);
	m_displayOptions = framework::DisplayOptions(m_parsedCommand.options);
	m_unitsOption = framework::UnitsOption(m_parsedCommand.options);
	m_capacityUnits = m_unitsOption.getCapacityUnits();

	if (displayOptionsAreValid() &&
		unitsOptionIsValid())
	{
		try
		{
			m_devices = m_service.getAllDevices();

			if (dimmIdsAreValid())
			{
				filterDevicesOnDimmIds();

				if (socketIdsAreValid())
				{
					filterDevicesOnSocketIds();

					createResults();
				}
			}
		}
		catch (core::LibraryException &e)
		{
			int libRc = e.getErrorCode();
			if (libRc == NVM_ERR_NOMEMORY)
			{
				m_pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_OUTOFMEMORY, NOMEMORY_ERROR_STR);
			}
			else if (libRc == NVM_ERR_NOTSUPPORTED)
			{
				m_pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_NOTSUPPORTED, NOTSUPPORTED_ERROR_STR);
			}
			else
			{
				// return the library message
				m_pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN, e.what());
			}
		}
	}

	return m_pResult;
}

std::string ShowDeviceCommand::convertLockState(lock_state lockState)
{
	std::map<lock_state, std::string> map;
	map[LOCK_STATE_UNKNOWN] = TR("Unknown");
	map[LOCK_STATE_DISABLED] = TR("Disabled");
	map[LOCK_STATE_UNLOCKED] = TR("Unlocked");
	map[LOCK_STATE_LOCKED] = TR("Locked");
	map[LOCK_STATE_FROZEN] = TR("Frozen");
	map[LOCK_STATE_PASSPHRASE_LIMIT] = TR("Exceeded");
	map[LOCK_STATE_NOT_SUPPORTED] = TR("Not Supported");
	return map[lockState];
}

std::string ShowDeviceCommand::convertFwVersion(std::string fwRevision)
{
	if(fwRevision.compare(ZERO_FW_VERSION) == 0)
	{
		fwRevision = "N/A";
	}
	return fwRevision;
}

std::string ShowDeviceCommand::convertHealthState(NVM_UINT16 healthState)
{
	std::map<NVM_UINT64, std::string> map;
	map[DEVICE_HEALTH_UNKNOWN] = TR("Unknown");
	map[DEVICE_HEALTH_NORMAL] = TR("Healthy");
	map[DEVICE_HEALTH_NONCRITICAL] = TR("Minor Failure");
	map[DEVICE_HEALTH_CRITICAL] = TR("Critical Failure");
	map[DEVICE_HEALTH_FATAL] = TR("Non-recoverable error");
	map[DEVICE_HEALTH_UNMANAGEABLE] = TR("Unmanageable");
	return map[healthState];
}

std::string ShowDeviceCommand::convertManageabilityState(manageability_state state)
{
	std::map<NVM_UINT64, std::string> map;
	map[MANAGEMENT_VALIDCONFIG] = TR("Manageable");
	map[MANAGEMENT_INVALIDCONFIG] = TR("Unmanageable");
	map[MANAGEMENT_UNKNOWN] = TR("Unknown");
	return map[state];
}

std::string ShowDeviceCommand::convertMemoryType(memory_type type)
{
	std::map<NVM_UINT64, std::string> map;
	map[MEMORY_TYPE_UNKNOWN] = TR("Unknown");
	map[MEMORY_TYPE_DDR4] = TR("DDR4");
	map[MEMORY_TYPE_NVMDIMM] = TR("AEP DIMM");
	return map[type];
}

std::string ShowDeviceCommand::convertFormFactor(device_form_factor formFactor)
{
	std::map<NVM_UINT64, std::string> map;
	map[DEVICE_FORM_FACTOR_DIMM] = TR("DIMM");
	map[DEVICE_FORM_FACTOR_SODIMM] = TR("SODIMM");
	map[DEVICE_FORM_FACTOR_UNKNOWN] = TR("Unknown");

	return map[formFactor];
}

std::string ShowDeviceCommand::convertFwLogLevel(fw_log_level logLevel)
{
	std::map<NVM_UINT64, std::string> map;
	map[FW_LOG_LEVEL_DISABLED] = TR("Disabled");
	map[FW_LOG_LEVEL_ERROR] = TR("Error");
	map[FW_LOG_LEVEL_WARN] = TR("Warning");
	map[FW_LOG_LEVEL_INFO] = TR("Info");
	map[FW_LOG_LEVEL_DEBUG] = TR("Debug");
	map[FW_LOG_LEVEL_UNKNOWN] = TR("Unknown");

	return map[logLevel];
}

std::string ShowDeviceCommand::convertConfigStatus(config_status status)
{
	std::map<config_status, std::string> map;
	map[CONFIG_STATUS_NOT_CONFIGURED] = TR("Not Configured");
	map[CONFIG_STATUS_VALID] = TR("Valid");
	map[CONFIG_STATUS_ERR_CORRUPT] = TR("Failed - Bad configuration");
	map[CONFIG_STATUS_ERR_BROKEN_INTERLEAVE] = TR("Failed - Broken interleave");
	map[CONFIG_STATUS_ERR_REVERTED] = TR("Failed - Reverted");
	map[CONFIG_STATUS_ERR_NOT_SUPPORTED] = TR("Failed - Unsupported");
	map[CONFIG_STATUS_UNKNOWN] = TR("Unknown");
	return map[status];
}

std::string ShowDeviceCommand::convertArsStatus(device_ars_status status)
{
	std::map<device_ars_status, std::string> map;
	map[DEVICE_ARS_STATUS_UNKNOWN] = TR("Unknown");
	map[DEVICE_ARS_STATUS_NOTSTARTED] = TR("Not started");
	map[DEVICE_ARS_STATUS_INPROGRESS] = TR("In progress");
	map[DEVICE_ARS_STATUS_COMPLETE] = TR("Completed");
	return map[status];
}

std::string ShowDeviceCommand::convertSanitizeStatus(device_sanitize_status status)
{
	std::map<device_sanitize_status, std::string> map;
	map[DEVICE_SANITIZE_STATUS_UNKNOWN] = TR("Unknown");
	map[DEVICE_SANITIZE_STATUS_NOTSTARTED] = TR("Not started");
	map[DEVICE_SANITIZE_STATUS_INPROGRESS] = TR("In progress");
	map[DEVICE_SANITIZE_STATUS_COMPLETE] = TR("Completed");
	return map[status];
}

std::string ShowDeviceCommand::convertLastShutdownStatus(NVM_UINT16 status)
{
	std::map<NVM_UINT16, std::string> map;
	map[DEVICE_LAST_SHUTDOWN_STATUS_UKNOWN] = TR("Unknown");
	map[DEVICE_LAST_SHUTDOWN_STATUS_FW_FLUSH_COMPLETE] = TR("FW Flush Complete");
	map[DEVICE_LAST_SHUTDOWN_STATUS_PM_ADR_COMMAND] = TR("PM ADR Command");
	map[DEVICE_LAST_SHUTDOWN_STATUS_PM_S3] = TR("PM S3");
	map[DEVICE_LAST_SHUTDOWN_STATUS_PM_S5] = TR("PM S5");
	map[DEVICE_LAST_SHUTDOWN_STATUS_DDRT_POWER_FAIL] = TR("DDRT Power Fail Command");
	map[DEVICE_LAST_SHUTDOWN_STATUS_PMIC_12V_POWER_FAIL] = TR("PMIC 12V Power Fail");
	map[DEVICE_LAST_SHUTDOWN_STATUS_PM_WARM_RESET] = TR("PM Warm Reset");
	map[DEVICE_LAST_SHUTDOWN_STATUS_THERMAL_SHUTDOWN] = TR("Thermal Shutdown");
	return map[status];
}

std::string ShowDeviceCommand::convertBootStatus(NVM_UINT16 status)
{
	std::map<NVM_UINT16, std::string> map;
	map[DEVICE_BOOT_STATUS_UNKNOWN] = TR("Unknown");
	map[DEVICE_BOOT_STATUS_SUCCESS] = TR("Success");
	map[DEVICE_BOOT_STATUS_MEDIA_NOT_READY] = TR("Media Not Ready");
	map[DEVICE_BOOT_STATUS_MEDIA_ERROR] = TR("Media Error");
	map[DEVICE_BOOT_STATUS_MEDIA_DISABLED] = TR("Media Disabled");
	map[DEVICE_BOOT_STATUS_FW_ASSERT] = TR("FW Assert");
	return map[status];
}

std::string ShowDeviceCommand::convertToDate(NVM_UINT64 timeValue)
{
	time_t time = (time_t) (timeValue);
	std::string timeStr = ctime(&time);
	std::size_t nullpos = timeStr.find("\n");
	if (nullpos != std::string::npos)
	{
		timeStr.erase(nullpos, 1);
	}
	return timeStr;
}

std::string ShowDeviceCommand::convertMemoryModes(NVM_UINT16 mode)
{
	std::map<NVM_UINT16, std::string> map;
	map[MEMORY_CAPABILITY_MEMORYMODE] = TR("Memory Mode");
	map[MEMORYTYPE_CAPABILITY_APPDIRECTMODE] = TR("App Direct");
	return map[mode];
}

std::string ShowDeviceCommand::convertSecurityCapabilities(NVM_UINT16 capability)
{
	std::map<NVM_UINT16, std::string> map;
	map[SECURITY_PASSPHRASE] = TR("Encryption");
	map[SECURITY_ERASE] = TR("Erase");
	return map[capability];
}

std::string ShowDeviceCommand::convertCapacity(NVM_UINT64 value)
{
	return convertCapacityFormat(value, m_capacityUnits);
}

bool ShowDeviceCommand::dimmIdsAreValid()
{
	m_pResult = ShowCommandUtilities::getInvalidDimmIdResult(
			m_dimmIds, m_devices);

	return m_pResult == NULL;
}

void ShowDeviceCommand::filterDevicesOnDimmIds()
{
	ShowCommandUtilities::filterDevicesOnDimmIds(m_devices, m_dimmIds);
}

bool ShowDeviceCommand::socketIdsAreValid()
{
	m_pResult = ShowCommandUtilities::getInvalidSocketIdResult(m_socketIds, m_devices);

	return m_pResult == NULL;
}

void ShowDeviceCommand::filterDevicesOnSocketIds()
{
	ShowCommandUtilities::filterDevicesOnSocketIds(m_devices, m_socketIds);
}

void ShowDeviceCommand::createResults()
{
	framework::ObjectListResult *pList = new framework::ObjectListResult();
	pList->setRoot(ROOT);
	m_pResult = pList;

	for (size_t i = 0; i < m_devices.size(); i++)
	{
		framework::PropertyListResult value;
		for (size_t j = 0; j < m_props.size(); j++)
		{
			framework::IPropertyDefinition<core::device::Device> &p = m_props[j];
			if (isPropertyDisplayed(p))
			{
				value.insert(p.getName(), p.getValue(m_devices[i]));
			}
		}

		pList->insert(ROOT, value);
	}

	m_pResult->setOutputType(
		m_displayOptions.isDefault() ?
		framework::ResultBase::OUTPUT_TEXTTABLE :
		framework::ResultBase::OUTPUT_TEXT);
}

bool ShowDeviceCommand::displayOptionsAreValid()
{
	m_pResult = ShowCommandPropertyUtilities<core::device::Device>::getInvalidDisplayOptionResult(
			m_displayOptions, m_props);

	return m_pResult == NULL;
}

bool ShowDeviceCommand::unitsOptionIsValid()
{
	m_pResult = ShowCommandUtilities::getInvalidUnitsOptionResult(m_unitsOption);

	return m_pResult == NULL;
}

bool ShowDeviceCommand::isPropertyDisplayed(
	framework::IPropertyDefinition<core::device::Device> &p)
{
	return ShowCommandPropertyUtilities<core::device::Device>::isPropertyDisplayed(p, m_displayOptions);
}

std::string ShowDeviceCommand::getManufacturingDate(core::device::Device &device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string result;

	bool isValid =  device.isManufacturingInfoValid();

	if (isValid)
	{
		result = core::device::Device::getFormattedManufacturingDate(device.getManufacturingDate());
	}
	else
	{
		result = "N/A";
	}

	return result;

}

std::string ShowDeviceCommand::getManufacturingLoc(core::device::Device &device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream result;

	bool isValid =  device.isManufacturingInfoValid();

	if (isValid)
	{
		char location[HEX_STR_LEN];

		get_hex_string(device.getManufacturingLoc(), location, sizeof (location));

		result << location;
	}
	else
	{
		result << "N/A";

	}

	return result.str();
}

std::string ShowDeviceCommand::toHex(NVM_UINT16 value)
{
	char value_str[HEX_STR_LEN];

	get_hex_string(value, value_str, sizeof (value_str));

	return std::string(value_str);
}

std::string ShowDeviceCommand::convertInterfaceFormatCode(const NVM_UINT16 ifc)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream result;

	result << "0x" << std::hex << std::setw(4) << std::setfill('0') << ifc;
	result << " (";

	result << getJedecStringForInterfaceFormatCode(ifc);

	result << ")";

	return result.str();
}

std::string ShowDeviceCommand::getJedecStringForInterfaceFormatCode(const NVM_UINT16 ifc)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string result;
	if (ifc == FORMAT_BYTE_STANDARD)
	{
		result = TR("Non-Energy Backed Byte Addressable");
	}
	else if (ifc == FORMAT_BLOCK_STANDARD)
	{
		result = TR("Non-Energy Backed Block Addressable");
	}
	else
	{
		result = TR("Unknown");
	}

	return result;
}

std::string ShowDeviceCommand::getActionRequiredEvents(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string formattedEventList = "N/A";

	std::vector<event> events = device.getActionRequiredEvents();
	if (!events.empty())
	{
		formattedEventList = ShowCommandUtilities::getFormattedEventList(events);
	}

	return formattedEventList;
}

std::string ShowDeviceCommand::convertWidth(NVM_UINT64 width)
{
	std::stringstream widthWithUnits;
	widthWithUnits << width << " b";
	return widthWithUnits.str();
}

std::string ShowDeviceCommand::convertSpeed(NVM_UINT64 speed)
{
	std::stringstream speedWithUnits;
	speedWithUnits << speed << " MHz";
	return speedWithUnits.str();
}

std::string ShowDeviceCommand::convertPowerLimit(NVM_UINT8 powerLimit)
{
	std::stringstream powerLimitWithUnits;
	powerLimitWithUnits << unsigned(powerLimit) << " W"; //casting otherwise treated as ascii
	return powerLimitWithUnits.str();
}

std::string ShowDeviceCommand::convertPowerBudget(NVM_UINT16 powerBudget)
{
	std::stringstream powerBudgetWithUnits;
	powerBudgetWithUnits << powerBudget << " mW";
	return powerBudgetWithUnits.str();
}
}
}
