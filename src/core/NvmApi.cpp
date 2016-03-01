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

#include "NvmApi.h"
#include <guid/guid.h>
#include <LogEnterExit.h>
#include <os/os_adapter.h>
#include <core/exceptions/NoMemoryException.h>

#ifdef __WINDOWS__
#include <Windows.h>

HANDLE g_nvmapi_lock;
#else
pthread_mutex_t g_nvmapi_lock;
#endif

int apiMutexInit()
{
	int rc = NVM_SUCCESS;

	// initialize the api singleton lock
	// api singleton is per proces so no need to be cross-process safe
	// thus no name on the mutex
	if (!mutex_init((OS_MUTEX*)&g_nvmapi_lock, NULL))
	{
		rc = NVM_ERR_UNKNOWN;
	}
	return rc;
}

int apiMutexDelete()
{
	int rc = NVM_SUCCESS;

	// clean up the locks
	if (!mutex_delete((OS_MUTEX*)&g_nvmapi_lock, NULL))
	{
		rc = NVM_ERR_UNKNOWN;
	}
	return rc;
}

core::NvmApi *core::NvmApi::m_pSingleton = NULL;

core::NvmApi::NvmApi()
{
}

core::NvmApi::~NvmApi()
{
	m_pSingleton = NULL;
}

core::NvmApi* core::NvmApi::getApi()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	mutex_lock(&g_nvmapi_lock);

	if (!m_pSingleton)
	{
		m_pSingleton = new NvmApi();
		if (!m_pSingleton)
		{
			mutex_unlock(&g_nvmapi_lock);
			throw NoMemoryException();
		}
	}

	mutex_unlock(&g_nvmapi_lock);

	return m_pSingleton;
}

int core::NvmApi::getHost(struct host *pHost) const
{
	return nvm_get_host(pHost);
}

int core::NvmApi::getHostName(char *hostName, const NVM_SIZE hostNameLen) const
{
	return nvm_get_host_name(hostName, hostNameLen);
}

int core::NvmApi::getSwInventory(struct sw_inventory *pInventory)
{
	return nvm_get_sw_inventory(pInventory);
}

int core::NvmApi::getSocketCount()
{
	return nvm_get_socket_count();
}

int core::NvmApi::getSockets(struct socket *pSockets, const NVM_UINT16 count)
{
	return nvm_get_sockets(pSockets, count);
}

int core::NvmApi::getSocket(const NVM_UINT16 socketId, struct socket *pSocket)
{
	return nvm_get_socket(socketId, pSocket);
}

int core::NvmApi::getNvmCapabilities(struct nvm_capabilities *pCapabilities)
{
	return nvm_get_nvm_capabilities(pCapabilities);
}

int core::NvmApi::getNvmCapacities(struct device_capacities *pCapacities) const
{
	return nvm_get_nvm_capacities(pCapacities);
}

int core::NvmApi::getMemoryTopologyCount()
{
	return nvm_get_memory_topology_count();
}

int core::NvmApi::getMemoryTopology(struct memory_topology *pDevices, const NVM_UINT8 count)
{
	return nvm_get_memory_topology(pDevices, count);
}

int core::NvmApi::getDeviceCount() const
{
	return nvm_get_device_count();
}

int core::NvmApi::getDevices(struct device_discovery *pDevices, const NVM_UINT8 count) const
{
	return nvm_get_devices(pDevices, count);
}

int core::NvmApi::getDeviceDiscovery(NVM_GUID guid, struct device_discovery *pDevice)
{
	return nvm_get_device_discovery(guid, pDevice);
}

int core::NvmApi::getDeviceStatus(const NVM_GUID deviceGuid, struct device_status *pStatus)
{
	return nvm_get_device_status(deviceGuid, pStatus);
}

int core::NvmApi::getDeviceSettings(const NVM_GUID deviceGuid, struct device_settings *pSettings)
{
	return nvm_get_device_settings(deviceGuid, pSettings);
}

int core::NvmApi::modifyDeviceSettings(const NVM_GUID deviceGuid,
		const struct device_settings *pSettings)
{
	return nvm_modify_device_settings(deviceGuid, pSettings);
}

int core::NvmApi::getDeviceDetails(const NVM_GUID deviceGuid, struct device_details *pDetails)
{
	return nvm_get_device_details(deviceGuid, pDetails);
}

int core::NvmApi::getDevicePerformance(const NVM_GUID deviceGuid,
		struct device_performance *pPerformance)
{
	return nvm_get_device_performance(deviceGuid, pPerformance);
}

int core::NvmApi::updateDeviceFw(const NVM_GUID deviceGuid, const NVM_PATH path,
		const NVM_SIZE path_len, const NVM_BOOL activate, const NVM_BOOL force)
{
	return nvm_update_device_fw(deviceGuid, path, path_len, activate, force);
}

int core::NvmApi::examineDeviceFw(const NVM_GUID deviceGuid, const NVM_PATH path,
		const NVM_SIZE pathLen, NVM_VERSION imageVersion,
		const NVM_SIZE imageVersionSize)
{
	return nvm_examine_device_fw(deviceGuid, path, pathLen, imageVersion, imageVersionSize);
}

int core::NvmApi::setPassphrase(const NVM_GUID deviceGuid, const NVM_PASSPHRASE oldPassphrase,
		const NVM_SIZE oldPassphraseLen, const NVM_PASSPHRASE newPassphrase,
		const NVM_SIZE newPassphraseLen)
{
	return nvm_set_passphrase(deviceGuid, oldPassphrase, oldPassphraseLen,
			newPassphrase, newPassphraseLen);
}

int core::NvmApi::removePassphrase(const NVM_GUID deviceGuid, const NVM_PASSPHRASE passphrase,
		const NVM_SIZE passphraseLen)
{
	return nvm_remove_passphrase(deviceGuid, passphrase, passphraseLen);
}

int core::NvmApi::unlockDevice(const NVM_GUID deviceGuid, const NVM_PASSPHRASE passphrase,
		const NVM_SIZE passphraseLen)
{
	return nvm_unlock_device(deviceGuid, passphrase, passphraseLen);
}

int core::NvmApi::eraseDevice(const NVM_GUID deviceGuid, const erase_type type, const NVM_PASSPHRASE passphrase,
		const NVM_SIZE passphraseLen)
{
	return nvm_erase_device(deviceGuid, type, passphrase, passphraseLen);
}

int core::NvmApi::getJobCount()
{
	return nvm_get_job_count();
}

int core::NvmApi::getJobs(struct job *pJobs, const NVM_UINT32 count)
{
	return nvm_get_jobs(pJobs, count);
}

int core::NvmApi::getPoolCount()
{
	return nvm_get_pool_count();
}

int core::NvmApi::getPools(struct pool *pPools, const NVM_UINT8 count)
{
	return nvm_get_pools(pPools, count);
}

int core::NvmApi::getPool(NVM_GUID poolGuid, struct pool *pPool)
{
	return nvm_get_pool(poolGuid, pPool);
}

int core::NvmApi::getAvailablePersistentSizeRange(const NVM_GUID poolGuid,
		struct possible_namespace_ranges *pRange)
{
	return nvm_get_available_persistent_size_range(poolGuid, pRange);
}

int core::NvmApi::createConfigGoal(const NVM_GUID deviceGuid, struct config_goal *pGoal)
{
	return nvm_create_config_goal(deviceGuid, pGoal);
}

int core::NvmApi::getConfigGoal(const NVM_GUID deviceGuid, struct config_goal *pGoal)
{
	return nvm_get_config_goal(deviceGuid, pGoal);
}

int core::NvmApi::deleteConfigGoal(const NVM_GUID deviceGuid)
{
	return nvm_delete_config_goal(deviceGuid);
}

int core::NvmApi::dumpConfig(const NVM_GUID deviceGuid, const NVM_PATH file,
		const NVM_SIZE fileLen, const NVM_BOOL append)
{
	return nvm_dump_config(deviceGuid, file, fileLen, append);
}

int core::NvmApi::loadConfig(const NVM_GUID deviceGuid, const NVM_PATH file, const NVM_SIZE fileLen)
{
	return nvm_load_config(deviceGuid, file, fileLen);
}

int core::NvmApi::getNamespaceCount()
{
	return nvm_get_namespace_count();
}

int core::NvmApi::getDeviceNamespaceCount(const NVM_GUID deviceGuid,
		const enum namespace_type nsType)
{
	return nvm_get_device_namespace_count(deviceGuid, nsType);
}

int core::NvmApi::getNamespaces(struct namespace_discovery *pNamespaces, const NVM_UINT8 count)
{
	return nvm_get_namespaces(pNamespaces, count);
}

int core::NvmApi::getNamespaceDetails(const NVM_GUID namespaceGuid, struct namespace_details *pNamespace)
{
	return nvm_get_namespace_details(namespaceGuid, pNamespace);
}

int core::NvmApi::createNamespace(NVM_GUID *pNamespaceGuid, const NVM_GUID poolGuid,
		struct namespace_create_settings *pSettings, const struct interleave_format *pFormat,
		const NVM_BOOL allowAdjustment)
{
	return nvm_create_namespace(pNamespaceGuid, poolGuid, pSettings, pFormat, allowAdjustment);
}

int core::NvmApi::modifyNamespaceName(const NVM_GUID namespaceGuid, const NVM_NAMESPACE_NAME name)
{
	return nvm_modify_namespace_name(namespaceGuid, name);
}

int core::NvmApi::modifyNamespaceBlockCount(const NVM_GUID namespaceGuid, const NVM_UINT64 blockCount,
		NVM_BOOL allowAdjustment)
{
	return nvm_modify_namespace_block_count(namespaceGuid, blockCount, allowAdjustment);
}

int core::NvmApi::modifyNamespaceEnabled(const NVM_GUID namespaceGuid,
		const enum namespace_enable_state enabled)
{
	return nvm_modify_namespace_enabled(namespaceGuid, enabled);
}

int core::NvmApi::deleteNamespace(const NVM_GUID namespaceGuid)
{
	return nvm_delete_namespace(namespaceGuid);
}

int core::NvmApi::adjustCreateNamespaceBlockCount(const NVM_GUID poolGuid, struct namespace_create_settings *pSettings,
		const struct interleave_format *pFormat)
{
	return nvm_adjust_create_namespace_block_count(poolGuid, pSettings, pFormat);
}

int core::NvmApi::adjustModifyNamespaceBlockCount(const NVM_GUID namespaceGuid, NVM_UINT64 *pBlockCount)
{
	return nvm_adjust_modify_namespace_block_count(namespaceGuid, pBlockCount);
}

int core::NvmApi::getSensors(const NVM_GUID deviceGuid, struct sensor *pSensors, const NVM_UINT16 count)
{
	return nvm_get_sensors(deviceGuid, pSensors, count);
}

int core::NvmApi::getSensor(const NVM_GUID deviceGuid, const enum sensor_type type, struct sensor *pSensor)
{
	return nvm_get_sensor(deviceGuid, type, pSensor);
}

int core::NvmApi::setSensorSettings(const NVM_GUID deviceGuid, const enum sensor_type type,
		const struct sensor_settings *pSettings)
{
	return nvm_set_sensor_settings(deviceGuid, type, pSettings);
}

int core::NvmApi::addEventNotify(const enum event_type type, void (*pEventCallback) (struct event *pEvent))
{
	return nvm_add_event_notify(type, pEventCallback);
}

int core::NvmApi::removeEventNotify(const int callbackId)
{
	return nvm_remove_event_notify(callbackId);
}

int core::NvmApi::getEventCount(const struct event_filter *pFilter)
{
	return nvm_get_event_count(pFilter);
}

int core::NvmApi::getEvents(const struct event_filter *pFilter, struct event *pEvents, const NVM_UINT16 count)
{
	return nvm_get_events(pFilter, pEvents, count);
}

int core::NvmApi::purgeEvents(const struct event_filter *pFilter)
{
	return nvm_purge_events(pFilter);
}

int core::NvmApi::acknowledgeEvent(NVM_UINT32 eventId)
{
	return nvm_acknowledge_event(eventId);
}

int core::NvmApi::saveState(const char *name, const NVM_SIZE nameLen)
{
	return nvm_save_state(name, nameLen);
}

int core::NvmApi::purgeStateData()
{
	return nvm_purge_state_data();
}

int core::NvmApi::gatherSupport(const NVM_PATH supportFile, const NVM_SIZE supportFileLen)
{
	return nvm_gather_support(supportFile, supportFileLen);
}

int core::NvmApi::runDiagnostic(const NVM_GUID deviceGuid, const struct diagnostic *pDiagnostic,
		NVM_UINT32 *pResults)
{
	return nvm_run_diagnostic(deviceGuid, pDiagnostic, pResults);
}

int core::NvmApi::getFwLogLevel(const NVM_GUID deviceGuid, enum fw_log_level *pLogLevel)
{
	return nvm_get_fw_log_level(deviceGuid, pLogLevel);
}

int core::NvmApi::setFwLogLevel(const NVM_GUID deviceGuid, const enum fw_log_level logLevel)
{
	return nvm_set_fw_log_level(deviceGuid, logLevel);
}

int core::NvmApi::getDeviceFwInfo(const NVM_GUID device_guid, struct device_fw_info *p_fw_info)
{
	return nvm_get_device_fw_image_info(device_guid, p_fw_info);
}

int core::NvmApi::injectDeviceError(const NVM_GUID deviceGuid, const struct device_error *pError)
{
	return nvm_inject_device_error(deviceGuid, pError);
}

int core::NvmApi::clearInjectedDeviceError(const NVM_GUID deviceGuid, const struct device_error *pError)
{
	return nvm_clear_injected_device_error(deviceGuid, pError);
}

int core::NvmApi::addSimulator(const NVM_PATH simulator, const NVM_SIZE simulatorLen)
{
	return nvm_add_simulator(simulator, simulatorLen);
}

int core::NvmApi::removeSimulator()
{
	return nvm_remove_simulator();
}

int core::NvmApi::debugLoggingEnabled() const
{
	return nvm_debug_logging_enabled();
}

int core::NvmApi::toggleDebugLogging(const NVM_BOOL enabled)
{
	return nvm_toggle_debug_logging(enabled);
}

int core::NvmApi::getDebugLogCount()
{
	return nvm_get_debug_log_count();
}

int core::NvmApi::getDebugLogs(struct log *pLogs, const NVM_UINT32 count)
{
	return nvm_get_debug_logs(pLogs, count);
}

int core::NvmApi::purgeDebugLog()
{
	return nvm_purge_debug_log();
}
