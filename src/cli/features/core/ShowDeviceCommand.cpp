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
#include "ShowDeviceCommand.h"

namespace cli
{
namespace nvmcli
{

ShowDeviceCommand::ShowDeviceCommand(core::device::DeviceService &service)
	: m_service(service), m_pResult(NULL)
{
	m_props.addCustom("DimmID", getDimmId).setIsRequired();
	m_props.addUint64("Capacity", &core::device::Device::getRawCapacity, &convertCapacity).setIsDefault();
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
	m_props.addUint16("RevisionID", &core::device::Device::getRevisionId);
	// New NFIT attirbutes here
	m_props.addStr("SerialNumber", &core::device::Device::getSerialNumber);
	m_props.addUint16("SubsystemVendorID", &core::device::Device::getSubsystemVendor, toHex);
	m_props.addUint16("SubsystemDeviceID", &core::device::Device::getSubsystemDevice, toHex);
	m_props.addUint16("SubsystemRevisionID", &core::device::Device::getSubsystemRevision, toHex);
	m_props.addBool("ManufacturingInfoValid", &core::device::Device::isManufacturingInfoValid);
	m_props.addCustom("ManufacturingLocation", &getManufacturingLoc);
	m_props.addCustom("ManufacturingDate", &getManufacturingDate);
	m_props.addStr("PartNumber", &core::device::Device::getPartNumber);
	m_props.addStr("DeviceLocator", &core::device::Device::getDeviceLocator);
	m_props.addStr("BankLabel", &core::device::Device::getBankLabel);
	m_props.addUint64("DataWidth", &core::device::Device::getDataWidth);
	m_props.addUint64("TotalWidth", &core::device::Device::getTotalWidth);
	m_props.addUint64("Speed", &core::device::Device::getSpeed);
	m_props.addList("ActionRequiredEvents", &core::device::Device::getActionRequiredEvents);
	m_props.addOther("LockState", &core::device::Device::getLockState, &convertLockState).setIsDefault();
	m_props.addStr("FWVersion", &core::device::Device::getFwRevision).setIsDefault();
	m_props.addStr("FWAPIVersion", &core::device::Device::getFwApiVersion);
	m_props.addStr("Manufacturer", &core::device::Device::getManufacturer);
	m_props.addUint16("ManufacturerID", &core::device::Device::getManufacturerId);
	m_props.addStr("Model", &core::device::Device::getModelNumber);
	m_props.addBool("IsNew", &core::device::Device::isNew);
	m_props.addOther("FormFactor", &core::device::Device::getFormFactor, &convertFormFactor);
	m_props.addUint64("MemoryCapacity", &core::device::Device::getMemoryCapacity,
			convertCapacity);
	m_props.addUint64("AppDirectCapacity", &core::device::Device::getAppDirectCapacity,
			convertCapacity);
	m_props.addUint64("UnconfiguredCapacity", &core::device::Device::getUnconfiguredCapacity,
			convertCapacity);
	m_props.addUint64("InaccessibleCapacity", &core::device::Device::getInaccessibleCapacity,
			convertCapacity);
	m_props.addUint64("ReservedCapacity", &core::device::Device::getReservedCapacity,
			convertCapacity);
	m_props.addOther("FWLogLevel", &core::device::Device::getFwLogLevel, &convertFwLogLevel);
	m_props.addBool("PowerManagementEnabled", &core::device::Device::isPowerManagementEnabled);
	m_props.addUint8("PowerLimit", &core::device::Device::getPowerLimit);
	m_props.addUint16("PeakPowerBudget", &core::device::Device::getPeakPowerBudget);
	m_props.addUint16("AvgPowerBudget", &core::device::Device::getAvgPowerBudget);
	m_props.addBool("DieSparingCapable", &core::device::Device::isDieSparingCapable);
	m_props.addBool("DieSparingEnabled", &core::device::Device::isDieSparingEnabled);
	m_props.addUint8("DieSparingLevel", &core::device::Device::getDieSparingLevel);
	m_props.addUint8("DieSparesUsed", &core::device::Device::getDieSparesUsed);
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
	m_props.addBool("SKUViolation", &core::device::Device::isSkuViolation);
	m_props.addBool("ViralPolicy", &core::device::Device::isViralPolicyEnabled);
	m_props.addBool("ViralState", &core::device::Device::getCurrentViralState);
}


framework::ResultBase *ShowDeviceCommand::execute(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_parsedCommand = parsedCommand;
	m_dimmIds = framework::CliHelper::splitCommaSeperatedString(m_parsedCommand.targets[TARGET_DIMM.name]);
	m_socketIds = framework::CliHelper::splitCommaSeperatedString(m_parsedCommand.targets[TARGET_SOCKET.name]);
	m_displayOptions = framework::DisplayOptions(m_parsedCommand.options);

	if (displayOptionsAreValid())
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
	map[MEMORY_TYPE_NVMDIMM] = TR("NVM-DIMM");
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
	map[CONFIG_STATUS_NOT_CONFIGURED] = TR("Not configured");
	map[CONFIG_STATUS_VALID] = TR("Valid");
	map[CONFIG_STATUS_ERR_CORRUPT] = TR("Failed - Bad configuration");
	map[CONFIG_STATUS_ERR_BROKEN_INTERLEAVE] = TR("Failed - Broken interleave");
	map[CONFIG_STATUS_ERR_REVERTED] = TR("Failed - Reverted");
	map[CONFIG_STATUS_ERR_NOT_SUPPORTED] = TR("Failed - Unsupported");
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
	map[MEMORYTYPE_CAPABILITY_STORAGEMODE] = TR("Storage Mode");
	map[MEMORYTYPE_CAPABILITY_APPDIRECTMODE] = TR("App Direct");
	return map[mode];
}

std::string ShowDeviceCommand::convertSecurityCapabilities(NVM_UINT16 capability)
{
	std::map<NVM_UINT32, std::string> map;
	map[SECURITY_PASSPHRASE] = ENCRYPTION_STR;
	map[SECURITY_ERASE] = ERASE_STR;
	return map[capability];
}

std::string ShowDeviceCommand::convertCapacity(NVM_UINT64 value)
{
	return convertCapacityFormat(value);
}


bool ShowDeviceCommand::dimmIdsAreValid()
{
	std::string badDimmId = getFirstBadDimmId(m_devices);
	if (!badDimmId.empty())
	{
		m_pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
			getInvalidDimmIdErrorString(badDimmId));
	}

	return m_pResult == NULL;
}

void ShowDeviceCommand::filterDevicesOnDimmIds()
{
	if (m_dimmIds.size() > 0)
	{
		for (size_t i = m_devices.size(); i > 0; i--)
		{
			core::device::Device &device = m_devices[i - 1];

			std::string deviceHandle = uint64ToString(device.getDeviceHandle());

			if (!m_dimmIds.contains(device.getUid()) && !m_dimmIds.contains(deviceHandle))
			{
				m_devices.removeAt(i - 1);
			}
		}
	}

}
bool ShowDeviceCommand::socketIdsAreValid()
{
	std::string badSocketId = getFirstBadSocketId(m_devices);
	if (!badSocketId.empty())
	{
		m_pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_TARGET,
			TARGET_SOCKET.name, badSocketId);
	}

	return m_pResult == NULL;
}
void ShowDeviceCommand::filterDevicesOnSocketIds()
{
	if (m_socketIds.size() > 0)
	{
		for (size_t i = m_devices.size(); i > 0; i--)
		{
			core::device::Device &device = m_devices[i - 1];

			std::string socketId = uint64ToString(device.getSocketId());

			if (!m_socketIds.contains(socketId))
			{
				m_devices.removeAt(i - 1);
			}
		}
	}
}

std::string ShowDeviceCommand::getFirstBadSocketId(core::device::DeviceCollection &devices) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string badsocketId = "";
	for (size_t i = 0; i < m_socketIds.size() && badsocketId.empty(); i++)
	{
		bool socketIdFound = false;
		for (size_t j = 0; j < devices.size() && !socketIdFound; j++)
		{
			if (m_socketIds[i] == uint64ToString(devices[j].getSocketId()))
			{
				socketIdFound = true;
			}
		}
		if (!socketIdFound)
		{
			badsocketId = m_socketIds[i];
		}
	}
	return badsocketId;
}

std::string ShowDeviceCommand::getFirstBadDimmId(core::device::DeviceCollection &devices) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string badDimmId = "";

	for (size_t i = 0; i < m_dimmIds.size() && badDimmId.empty(); i++)
	{
		bool dimmIdFound = false;
		for (size_t j = 0; j < devices.size() && !dimmIdFound; j++)
		{
			if (framework::stringsIEqual(m_dimmIds[i], devices[j].getUid()) ||
				m_dimmIds[i] == uint64ToString(devices[j].getDeviceHandle()))
			{
				dimmIdFound = true;
			}
		}
		if (!dimmIdFound)
		{
			badDimmId = m_dimmIds[i];
		}
	}
	return badDimmId;
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
	std::string invalidDisplay;
	const std::vector<std::string> &display = m_displayOptions.getDisplay();
	for (size_t i = 0; i < display.size() && invalidDisplay.empty(); i++)
	{
		if (!m_props.contains(display[i]))
		{
			invalidDisplay = display[i];
		}
	}

	if (!invalidDisplay.empty())
	{
		m_pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_OPTION,
			framework::OPTION_DISPLAY.name, invalidDisplay);
	}
	return m_pResult == NULL;
}

bool ShowDeviceCommand::isPropertyDisplayed(
	framework::IPropertyDefinition<core::device::Device> &p)
{
	return p.isRequired() ||
		   m_displayOptions.isAll() ||
		   (p.isDefault() && m_displayOptions.isDefault()) ||
		   m_displayOptions.contains(p.getName());
}
std::string ShowDeviceCommand::getDimmId(core::device::Device &device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream result;
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

	if (useHandle)
	{
		result << device.getDeviceHandle();
	}
	else
	{
		result << device.getUid();
	}
	return result.str();
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
		result << std::hex << (unsigned int)device.getManufacturingLoc();
	}
	else
	{
		result << "N/A";

	}

	return result.str();
}

std::string ShowDeviceCommand::toHex(NVM_UINT16 value)
{
	std::stringstream result;
	result << std::hex << value;

	return result.str();
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

}
}
