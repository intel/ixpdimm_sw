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

#include "Device.h"

logic::device::Device *logic::device::Device::clone()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return new Device(*this);
}
enum manageability_state logic::device::Device::getManageabilityState()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().manageability;
}
std::string logic::device::Device::getGuid()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return Helper::guidToString(getDiscovery().guid);
}
enum memory_type logic::device::Device::getMemoryType()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().memory_type;
}
bool logic::device::Device::isManageable()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getManageabilityState() == MANAGEMENT_VALIDCONFIG;
}
enum lock_state logic::device::Device::getLockState()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().lock_state;
}
NVM_UINT32 logic::device::Device::getDeviceHandle()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().device_handle.handle;
}
enum device_health logic::device::Device::getDeviceStatusHealth()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().status.health;
}
NVM_UINT32 logic::device::Device::getChannelPosition()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().device_handle.parts.mem_channel_dimm_num;
}
enum config_status logic::device::Device::getConfigStatus()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().status.config_status;
}
NVM_UINT32 logic::device::Device::getChannelId()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().device_handle.parts.mem_channel_id;
}
enum device_form_factor logic::device::Device::getFormFactor()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().form_factor;
}
NVM_UINT16 logic::device::Device::getPhysicalId()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().physical_id;
}
NVM_UINT16 logic::device::Device::getVendorId()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().vendor_id;
}
NVM_UINT16 logic::device::Device::getDeviceId()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().device_id;
}
NVM_UINT16 logic::device::Device::getRevisionId()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().revision_id;
}
NVM_UINT16 logic::device::Device::getSocketId()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().socket_id;
}
NVM_UINT16 logic::device::Device::getMemoryControllerId()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().memory_controller_id;
}
std::string logic::device::Device::getManufacturer()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string result;
	char manufacturerStr[NVM_MANUFACTURERSTR_LEN];
	if (lookup_jedec_jep106_manufacturer(
		getDiscovery().manufacturer, NVM_MANUFACTURER_LEN,
		manufacturerStr, NVM_MANUFACTURERSTR_LEN) == COMMON_SUCCESS)
	{
		result = std::string(manufacturerStr);
	}
	return result;
}
NVM_UINT16 logic::device::Device::getManufacturerId()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return MANUFACTURER_TO_UINT(getDiscovery().manufacturer);
}
std::string logic::device::Device::getSerialNumber()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	char serialNumStr[NVM_SERIALSTR_LEN];
	SERIAL_NUMBER_TO_STRING(getDiscovery().serial_number, serialNumStr);
	return std::string(serialNumStr);
}
std::string logic::device::Device::getModelNumber()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().model_number;
}
std::string logic::device::Device::getFwRevision()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().fw_revision;
}
std::string logic::device::Device::getFwApiVersion()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().fw_api_version;
}
fw_log_level logic::device::Device::getFwLogLevel()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	enum fw_log_level curr_log_level;
	int rc = m_api.getFwLogLevel(getDiscovery().guid, &curr_log_level);

	if (rc != NVM_SUCCESS)
	{
		curr_log_level = FW_LOG_LEVEL_UNKNOWN;
	}
	return curr_log_level;
}
NVM_UINT64 logic::device::Device::getRawCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().capacity;
}
NVM_UINT16 logic::device::Device::getInterfaceFormatCode()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().interface_format_code;
}
bool logic::device::Device::isPassphraseCapable()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().security_capabilities.passphrase_capable;
}
bool logic::device::Device::isUnlockDeviceCapable()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().security_capabilities.unlock_device_capable;
}
bool logic::device::Device::isEraseOverwriteCapable()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().security_capabilities.erase_overwrite_capable;
}
bool logic::device::Device::isEraseCryptoCapable()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().security_capabilities.erase_crypto_capable;
}
std::vector<NVM_UINT32> logic::device::Device::getSecurityCapabilities()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::vector<NVM_UINT32> result;
	if (isPassphraseCapable())
	{
		result.push_back(SECURITY_PASSPHRASE);
	}
	if (isUnlockDeviceCapable())
	{
		result.push_back(SECURITY_UNLOCK);
	}
	if (isEraseCryptoCapable())
	{
		result.push_back(SECURITY_ERASE);
	}

	return result;
}
bool logic::device::Device::isDieSparingCapable()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().device_capabilities.die_sparing_capable;
}
bool logic::device::Device::isAppDirectModeCapable()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().device_capabilities.app_direct_mode_capable;
}
bool logic::device::Device::isMemoryModeCapable()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().device_capabilities.memory_mode_capable;
}
bool logic::device::Device::isStorageModeCapable()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().device_capabilities.storage_mode_capable;
}
std::vector<NVM_UINT16> logic::device::Device::getMemoryCapabilities()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::vector<NVM_UINT16> result;

	if (isMemoryModeCapable())
	{
		result.push_back(MEMORY_CAPABILITY_MEMORYMODE);
	}
	if (isStorageModeCapable())
	{
		result.push_back(MEMORYTYPE_CAPABILITY_STORAGEMODE);
	}
	if (isAppDirectModeCapable())
	{
		result.push_back(MEMORYTYPE_CAPABILITY_APPDIRECTMODE);
	}
	return result;
}
NVM_UINT32 logic::device::Device::getSku()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDiscovery().dimm_sku;
}
NVM_UINT16 logic::device::Device::getHealthState()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	NVM_UINT16 healthState = isManageable() ? getDeviceStatusHealth()
											: DEVICE_HEALTH_UNMANAGEABLE;
	return healthState;
}
bool logic::device::Device::isNew()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().status.is_new;
}
bool logic::device::Device::getIsMissing()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().status.is_missing;
}
NVM_UINT8 logic::device::Device::getDieSparesUsed()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().status.die_spares_used;
}
std::vector<NVM_UINT16> logic::device::Device::getLastShutdownStatus()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::vector<NVM_UINT16> result;

	NVM_UINT8 lastShutdownState = getDetails().status.last_shutdown_status;
	if (lastShutdownState == SHUTDOWN_STATUS_UNKNOWN)
	{
		result.push_back(DEVICE_LAST_SHUTDOWN_STATUS_UKNOWN);
	}
	else // can't be "Unknown" and "Known"
	{
		if (lastShutdownState & SHUTDOWN_STATUS_CLEAN)
		{
			result.push_back(DEVICE_LAST_SHUTDOWN_STATUS_FW_FLUSH_COMPLETE);
		}
		if (lastShutdownState & SHUTDOWN_STATUS_PM_ADR)
		{
			result.push_back(DEVICE_LAST_SHUTDOWN_STATUS_PM_ADR_COMMAND);
		}
		if (lastShutdownState & SHUTDOWN_STATUS_PM_S3)
		{
			result.push_back(DEVICE_LAST_SHUTDOWN_STATUS_PM_S3);
		}
		if (lastShutdownState & SHUTDOWN_STATUS_PM_S5)
		{
			result.push_back(DEVICE_LAST_SHUTDOWN_STATUS_PM_S5);
		}
		if (lastShutdownState & SHUTDOWN_STATUS_DDRT_POWER_FAIL)
		{
			result.push_back(DEVICE_LAST_SHUTDOWN_STATUS_DDRT_POWER_FAIL);
		}
		if (lastShutdownState & SHUTDOWN_STATUS_PMIC_12V_POWER_FAIL)
		{
			result.push_back(DEVICE_LAST_SHUTDOWN_STATUS_PMIC_12V_POWER_FAIL);
		}
		if (lastShutdownState & SHUTDOWN_STATUS_WARM_RESET)
		{
			result.push_back(DEVICE_LAST_SHUTDOWN_STATUS_PM_WARM_RESET);
		}
		if (lastShutdownState & SHUTDOWN_STATUS_FORCED_THERMAL)
		{
			result.push_back(DEVICE_LAST_SHUTDOWN_STATUS_THERMAL_SHUTDOWN);
		}
	}

	return result;
}
NVM_UINT64 logic::device::Device::getLastShutdownTime()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().status.last_shutdown_time;
}
bool logic::device::Device::isMixedSku()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().status.mixed_sku;
}
bool logic::device::Device::isSkuViolation()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().status.sku_violation;
}
time_t logic::device::Device::getPerformanceTime()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().performance.time;
}
NVM_UINT64 logic::device::Device::getBytesRead()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().performance.bytes_read;
}
NVM_UINT64 logic::device::Device::getHostReads()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().performance.host_reads;
}
NVM_UINT64 logic::device::Device::getBytesWritten()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().performance.bytes_written;
}
NVM_UINT64 logic::device::Device::getHostWrites()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().performance.host_writes;
}
NVM_UINT64 logic::device::Device::getBlockReads()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().performance.block_reads;
}
NVM_UINT64 logic::device::Device::getBlockWrites()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().performance.block_writes;
}
NVM_UINT64 logic::device::Device::getTotalCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().capacities.capacity;
}
NVM_UINT64 logic::device::Device::getVolatileCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().capacities.volatile_capacity;
}
NVM_UINT64 logic::device::Device::getPersistentCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().capacities.persistent_capacity;
}
NVM_UINT64 logic::device::Device::getBlockCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().capacities.block_capacity;
}
NVM_UINT64 logic::device::Device::getUnconfiguredCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().capacities.unconfigured_capacity;
}
NVM_UINT64 logic::device::Device::getInaccessibleCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().capacities.inaccessible_capacity;
}
NVM_UINT64 logic::device::Device::getReservedCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().capacities.reserved_capacity;
}
NVM_UINT64 logic::device::Device::getDataWidth()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().data_width;
}
NVM_UINT64 logic::device::Device::getTotalWidth()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().total_width;
}
NVM_UINT64 logic::device::Device::getSpeed()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().speed;
}
bool logic::device::Device::getPowerManagementEnabled()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().power_management_enabled;
}
NVM_UINT8 logic::device::Device::getPowerLimit()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().power_limit;
}
NVM_UINT16 logic::device::Device::getPeakPowerBudget()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().peak_power_budget;
}
NVM_UINT16 logic::device::Device::getAvgPowerBudget()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().avg_power_budget;
}
bool logic::device::Device::getDieSparingEnabled()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().die_sparing_enabled;
}
std::string logic::device::Device::getPartNumber()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return std::string(getDetails().part_number);
}
NVM_UINT8 logic::device::Device::getDieSparingLevel()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().die_sparing_level;
}
std::string logic::device::Device::getDeviceLocator()
{
	return std::string(getDetails().device_locator);
}
std::string logic::device::Device::getBankLabel()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return std::string(getDetails().bank_label);
}
bool logic::device::Device::isFirstFastRefresh()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getDetails().settings.first_fast_refresh;
}
bool logic::device::Device::isActionRequired()
{
	return getEvents().size() > 0;
}
std::vector<std::string> logic::device::Device::getActionRequiredEvents()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getEvents();
}
const device_discovery &logic::device::Device::getDiscovery()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_discovery;
}
const device_details &logic::device::Device::getDetails()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (m_pDetails == NULL)
	{
		m_pDetails = (device_details *) malloc(sizeof(device_details));
		memset(m_pDetails, 0, sizeof(device_details));
		int rc = m_api.getDeviceDetails(m_discovery.guid, m_pDetails);

		if (rc != NVM_SUCCESS && rc != NVM_ERR_NOTMANAGEABLE)
		{
			throw LibraryException(rc);
		}
	}
	return *m_pDetails;
}
const std::vector<std::string> &logic::device::Device::getEvents()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (m_pActionRequiredEvents == NULL)
	{
		event_filter filter;
		memset(&filter, 0, sizeof(filter));
		filter.filter_mask = NVM_FILTER_ON_AR | NVM_FILTER_ON_GUID;
		filter.action_required = 1;
		memmove(filter.guid, getDiscovery().guid, sizeof(filter.guid));

		int count = m_api.getEventCount(&filter);
		event events[count];
		m_api.getEvents(&filter, events, count);
		m_pActionRequiredEvents = new std::vector<std::string>();

		for (int i = 0; i < count; i++)
		{
			std::stringstream eventMsg;
			eventMsg << "Event " << events[i].event_id;
			char msg[NVM_EVENT_MSG_LEN + (3 * NVM_EVENT_ARG_LEN)];
			s_snprintf(msg, (NVM_EVENT_MSG_LEN + (3 * NVM_EVENT_ARG_LEN)),
				events[i].message,
				events[i].args[0],
				events[i].args[1],
				events[i].args[2]);
			eventMsg << " - " << msg;
			m_pActionRequiredEvents->push_back(eventMsg.str());
		}
	}
	return *m_pActionRequiredEvents;
}