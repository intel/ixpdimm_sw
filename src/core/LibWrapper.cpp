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

#include "LibWrapper.h"
#include <uid/uid.h>
#include <LogEnterExit.h>

namespace core
{

LibWrapper &LibWrapper::getLibWrapper()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	static LibWrapper *result = new LibWrapper();
	return *result;
}

LibWrapper::LibWrapper()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

LibWrapper::~LibWrapper()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

int LibWrapper::getHost(struct host *pHost) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_host(pHost);
}

int LibWrapper::getHostName(char *hostName, const NVM_SIZE hostNameLen) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_host_name(hostName, hostNameLen);
}

int LibWrapper::getSwInventory(struct sw_inventory *pInventory) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_sw_inventory(pInventory);
}

int LibWrapper::getSocketCount() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_socket_count();
}

int LibWrapper::getSockets(struct socket *pSockets, const NVM_UINT16 count) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_sockets(pSockets, count);
}

int LibWrapper::getSocket(const NVM_UINT16 socketId, struct socket *pSocket) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_socket(socketId, pSocket);
}

int LibWrapper::getNvmCapabilities(struct nvm_capabilities *pCapabilities) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_nvm_capabilities(pCapabilities);
}

int LibWrapper::getNvmCapacities(struct device_capacities *pCapacities) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_nvm_capacities(pCapacities);
}

int LibWrapper::getMemoryTopologyCount() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_memory_topology_count();
}

int LibWrapper::getMemoryTopology(struct memory_topology *pDevices, const NVM_UINT8 count) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_memory_topology(pDevices, count);
}

int LibWrapper::getDeviceCount() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_device_count();
}

int LibWrapper::getDevices(struct device_discovery *pDevices, const NVM_UINT8 count) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_devices(pDevices, count);
}

int LibWrapper::getDeviceDiscovery(NVM_GUID guid, struct device_discovery *pDevice) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_device_discovery(guid, pDevice);
}

int LibWrapper::getDeviceStatus(const NVM_GUID deviceGuid, struct device_status *pStatus) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_device_status(deviceGuid, pStatus);
}

int LibWrapper::getDeviceSettings(const NVM_GUID deviceGuid,
	struct device_settings *pSettings) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_device_settings(deviceGuid, pSettings);
}

int LibWrapper::modifyDeviceSettings(const NVM_GUID deviceGuid,
	const struct device_settings *pSettings) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_modify_device_settings(deviceGuid, pSettings);
}

int LibWrapper::getDeviceDetails(const NVM_GUID deviceGuid, struct device_details *pDetails) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_device_details(deviceGuid, pDetails);
}

int LibWrapper::getDevicePerformance(const NVM_GUID deviceGuid,
	struct device_performance *pPerformance) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_device_performance(deviceGuid, pPerformance);
}

int LibWrapper::updateDeviceFw(const NVM_GUID deviceGuid, const NVM_PATH path,
	const NVM_SIZE path_len, const NVM_BOOL activate, const NVM_BOOL force) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_update_device_fw(deviceGuid, path, path_len, activate, force);
}

int LibWrapper::examineDeviceFw(const NVM_GUID deviceGuid, const NVM_PATH path,
	const NVM_SIZE pathLen, NVM_VERSION imageVersion,
	const NVM_SIZE imageVersionSize) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_examine_device_fw(deviceGuid, path, pathLen, imageVersion, imageVersionSize);
}

int LibWrapper::setPassphrase(const NVM_GUID deviceGuid, const NVM_PASSPHRASE oldPassphrase,
	const NVM_SIZE oldPassphraseLen, const NVM_PASSPHRASE newPassphrase,
	const NVM_SIZE newPassphraseLen) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_set_passphrase(deviceGuid, oldPassphrase, oldPassphraseLen,
		newPassphrase, newPassphraseLen);
}

int LibWrapper::removePassphrase(const NVM_GUID deviceGuid, const NVM_PASSPHRASE passphrase,
	const NVM_SIZE passphraseLen) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_remove_passphrase(deviceGuid, passphrase, passphraseLen);
}

int LibWrapper::unlockDevice(const NVM_GUID deviceGuid, const NVM_PASSPHRASE passphrase,
	const NVM_SIZE passphraseLen) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_unlock_device(deviceGuid, passphrase, passphraseLen);
}

int LibWrapper::eraseDevice(const NVM_GUID deviceGuid,
	const NVM_PASSPHRASE passphrase,
	const NVM_SIZE passphraseLen) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_erase_device(deviceGuid, passphrase, passphraseLen);
}

int LibWrapper::getJobCount() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_job_count();
}

int LibWrapper::getJobs(struct job *pJobs, const NVM_UINT32 count) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_jobs(pJobs, count);
}

int LibWrapper::getPoolCount() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_pool_count();
}

int LibWrapper::getPools(struct pool *pPools, const NVM_UINT8 count) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_pools(pPools, count);
}

int LibWrapper::getPool(NVM_GUID poolGuid, struct pool *pPool) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_pool(poolGuid, pPool);
}

int LibWrapper::getAvailablePersistentSizeRange(const NVM_GUID poolGuid,
	struct possible_namespace_ranges *pRange) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_available_persistent_size_range(poolGuid, pRange);
}

int LibWrapper::createConfigGoal(const NVM_GUID deviceGuid, struct config_goal *pGoal) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_create_config_goal(deviceGuid, pGoal);
}

int LibWrapper::getConfigGoal(const NVM_GUID deviceGuid, struct config_goal *pGoal) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_config_goal(deviceGuid, pGoal);
}

int LibWrapper::deleteConfigGoal(const NVM_GUID deviceGuid) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_delete_config_goal(deviceGuid);
}

int LibWrapper::dumpConfig(const NVM_GUID deviceGuid, const NVM_PATH file,
	const NVM_SIZE fileLen, const NVM_BOOL append) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_dump_config(deviceGuid, file, fileLen, append);
}

int LibWrapper::loadConfig(const NVM_GUID deviceGuid, const NVM_PATH file,
	const NVM_SIZE fileLen) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_load_config(deviceGuid, file, fileLen);
}

int LibWrapper::getNamespaceCount() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_namespace_count();
}

int LibWrapper::getDeviceNamespaceCount(const NVM_GUID deviceGuid,
	const enum namespace_type nsType) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_device_namespace_count(deviceGuid, nsType);
}

int LibWrapper::getNamespaces(struct namespace_discovery *pNamespaces, const NVM_UINT8 count) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_namespaces(pNamespaces, count);
}

int LibWrapper::getNamespaceDetails(const NVM_GUID namespaceGuid,
	struct namespace_details *pNamespace) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_namespace_details(namespaceGuid, pNamespace);
}

int LibWrapper::createNamespace(NVM_GUID *pNamespaceGuid, const NVM_GUID poolGuid,
	struct namespace_create_settings *pSettings, const struct interleave_format *pFormat,
	const NVM_BOOL allowAdjustment) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_create_namespace(pNamespaceGuid, poolGuid, pSettings, pFormat, allowAdjustment);
}

int LibWrapper::modifyNamespaceName(const NVM_GUID namespaceGuid,
	const NVM_NAMESPACE_NAME name) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_modify_namespace_name(namespaceGuid, name);
}

int LibWrapper::modifyNamespaceBlockCount(const NVM_GUID namespaceGuid,
	const NVM_UINT64 blockCount,
	NVM_BOOL allowAdjustment) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_modify_namespace_block_count(namespaceGuid, blockCount, allowAdjustment);
}

int LibWrapper::modifyNamespaceEnabled(const NVM_GUID namespaceGuid,
	const enum namespace_enable_state enabled) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_modify_namespace_enabled(namespaceGuid, enabled);
}

int LibWrapper::deleteNamespace(const NVM_GUID namespaceGuid) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_delete_namespace(namespaceGuid);
}

int LibWrapper::adjustCreateNamespaceBlockCount(const NVM_GUID poolGuid,
	struct namespace_create_settings *pSettings,
	const struct interleave_format *pFormat) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_adjust_create_namespace_block_count(poolGuid, pSettings, pFormat);
}

int LibWrapper::adjustModifyNamespaceBlockCount(const NVM_GUID namespaceGuid,
	NVM_UINT64 *pBlockCount) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_adjust_modify_namespace_block_count(namespaceGuid, pBlockCount);
}

int LibWrapper::getSensors(const NVM_GUID deviceGuid, struct sensor *pSensors,
	const NVM_UINT16 count) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_sensors(deviceGuid, pSensors, count);
}

int LibWrapper::getSensor(const NVM_GUID deviceGuid, const enum sensor_type type,
	struct sensor *pSensor) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_sensor(deviceGuid, type, pSensor);
}

int LibWrapper::setSensorSettings(const NVM_GUID deviceGuid, const enum sensor_type type,
	const struct sensor_settings *pSettings) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_set_sensor_settings(deviceGuid, type, pSettings);
}

int LibWrapper::addEventNotify(const enum event_type type,
	void (*pEventCallback)(struct event *)) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_add_event_notify(type, pEventCallback);
}

int LibWrapper::removeEventNotify(const int callbackId) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_remove_event_notify(callbackId);
}

int LibWrapper::getEventCount(const struct event_filter *pFilter) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_event_count(pFilter);
}

int LibWrapper::getEvents(const struct event_filter *pFilter, struct event *pEvents,
	const NVM_UINT16 count) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_events(pFilter, pEvents, count);
}

int LibWrapper::purgeEvents(const struct event_filter *pFilter) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_purge_events(pFilter);
}

int LibWrapper::acknowledgeEvent(NVM_UINT32 eventId) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_acknowledge_event(eventId);
}

int LibWrapper::saveState(const char *name, const NVM_SIZE nameLen) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_save_state(name, nameLen);
}

int LibWrapper::purgeStateData() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_purge_state_data();
}

int LibWrapper::gatherSupport(const NVM_PATH supportFile, const NVM_SIZE supportFileLen) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_gather_support(supportFile, supportFileLen);
}

int LibWrapper::runDiagnostic(const NVM_GUID deviceGuid, const struct diagnostic *pDiagnostic,
	NVM_UINT32 *pResults) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_run_diagnostic(deviceGuid, pDiagnostic, pResults);
}

int LibWrapper::getFwLogLevel(const NVM_GUID deviceGuid, enum fw_log_level *pLogLevel) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_fw_log_level(deviceGuid, pLogLevel);
}

int LibWrapper::setFwLogLevel(const NVM_GUID deviceGuid, const enum fw_log_level logLevel) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_set_fw_log_level(deviceGuid, logLevel);
}

int LibWrapper::getDeviceFwInfo(const NVM_GUID device_guid, struct device_fw_info *p_fw_info) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_device_fw_image_info(device_guid, p_fw_info);
}

int LibWrapper::injectDeviceError(const NVM_GUID deviceGuid,
	const struct device_error *pError) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_inject_device_error(deviceGuid, pError);
}

int LibWrapper::clearInjectedDeviceError(const NVM_GUID deviceGuid,
	const struct device_error *pError) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_clear_injected_device_error(deviceGuid, pError);
}

int LibWrapper::addSimulator(const NVM_PATH simulator, const NVM_SIZE simulatorLen) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_add_simulator(simulator, simulatorLen);
}

int LibWrapper::removeSimulator() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_remove_simulator();
}

int LibWrapper::debugLoggingEnabled() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_debug_logging_enabled();
}

int LibWrapper::toggleDebugLogging(const NVM_BOOL enabled) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_toggle_debug_logging(enabled);
}

int LibWrapper::getDebugLogCount() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_debug_log_count();
}

int LibWrapper::getDebugLogs(struct log *pLogs, const NVM_UINT32 count) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_get_debug_logs(pLogs, count);
}

int LibWrapper::purgeDebugLog() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	return nvm_purge_debug_log();
}

}
