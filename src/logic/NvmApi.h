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
#ifndef CR_MGMT_NVMAPI_H
#define CR_MGMT_NVMAPI_H

#include <nvm_management.h>

namespace logic
{
class NVM_API NvmApi
{
public:
	virtual ~NvmApi();

	/*
	 * Grab the NvmApi singleton.
	 * The return value is guaranteed to be non-NULL.
	 */
	static NvmApi *getApi();

	/*
	 * Retrieve basic information about the host server
	 */
	virtual int getHost(struct host *pHost);

	/*
	 * Retrieve the name of the host server
	 */
	virtual int getHostName(char *hostName, const NVM_SIZE hostNameLen) const;

	/*
	 * Retrieve a list of installed software
	 */
	virtual int getSwInventory(struct sw_inventory *pInventory);

	/*
	 * Retrieve the number of processors on the system
	 */
	virtual int getSocketCount();

	/*
	 * Retrieve information about each physical processor on the system
	 */
	virtual int getSockets(struct socket *pSockets, const NVM_UINT16 count);

	/*
	 * Retrieve information about the specified physical  processor
	 */
	virtual int getSocket(const NVM_UINT16 socketId, struct socket *pSocket);

	/*
	 * Retrieve supported NVM-DIMM capabilities in aggregate
	 */
	virtual int getNvmCapabilities(struct nvm_capabilities *pCapabilities);

	/*
	 * Retrieve the total NVM-DIMM resource allocation
	 */
	virtual int getNvmCapacities(struct device_capacities *pCapacities);

	/*
	 * Retrieve the number of memory devices installed on the system
	 */
	virtual int getMemoryTopologyCount();

	/*
	 * Retrieve topology information about all memory devices
	 */
	virtual int getMemoryTopology(struct memory_topology *pDevices, const NVM_UINT8 count);

	/*
	 * Retrieve the number of devices on the system
	 */
	virtual int getDeviceCount() const;

	/*
	 * Retrieve the information about a given device
	 */
	virtual int getDeviceDiscovery(NVM_GUID guid, struct device_discovery *pDevice);

	/*
	 * Retrieve discovery information about each NVM-DIMM
	 */
	virtual int getDevices(struct device_discovery *pDevices, const NVM_UINT8 count) const;

	/*
	 * Retrieve the current status of the specifies NVM-DIMM
	 */
	virtual int getDeviceStatus(const NVM_GUID deviceGuid, struct device_status *pStatus);

	/*
	 * Retrieve the optional configuration data from the specified NVM-DIMM
	 */
	virtual int getDeviceSettings(const NVM_GUID deviceGuid, struct device_settings *pSettings);

	/*
	 * Modify the optional configuration data on the specified NVM-DIMM
	 */
	virtual int modifyDeviceSettings(const NVM_GUID deviceGuid,
			const struct device_settings *pSettings);

	/*
	 * Retrieve detailed information about the specified NVM-DIMM
	 */
	virtual int getDeviceDetails(const NVM_GUID deviceGuid, struct device_details *pDetails);

	/*
	 * Retrieve a snapshot of the performance metrics for the specified NVM-DIMM
	 */
	virtual int getDevicePerformance(const NVM_GUID deviceGuid,
			struct device_performance *pPerformance);

	/*
	 * Update the firmware on the specified NVM-DIMM
	 */
	virtual int updateDeviceFw(const NVM_GUID deviceGuid, const NVM_PATH path,
			const NVM_SIZE path_len, const NVM_BOOL activate, const NVM_BOOL force);

	/*
	 * Examine a firmware image against a specified NVM-DIMM to determine if it is valid
	 */
	virtual int examineDeviceFw(const NVM_GUID deviceGuid, const NVM_PATH path,
			const NVM_SIZE pathLen, NVM_VERSION imageVersion, const NVM_SIZE imageVersionSize);

	/*
	 * Set or change the passphrase and enable security on the specified NVM-DIMM
	 */
	virtual int setPassphrase(const NVM_GUID deviceGuid, const NVM_PASSPHRASE oldPassphrase,
			const NVM_SIZE oldPassphraseLen, const NVM_PASSPHRASE newPassphrase,
			const NVM_SIZE newPassphraseLen);

	/*
	 * Disable security on the specified NVM-DIMM
	 */
	virtual int removePassphrase(const NVM_GUID deviceGuid, const NVM_PASSPHRASE passphrase,
			const NVM_SIZE passphraseLen);

	/*
	 * Unlock the specified NVM-DIMM
	 */
	virtual int unlockDevice(const NVM_GUID deviceGuid, const NVM_PASSPHRASE passphrase,
			const NVM_SIZE passphraseLen);

	/*
	 * Erase persistent data on the specified NVM-DIMM
	 */
	virtual int eraseDevice(const NVM_GUID deviceGuid, const erase_type type, const NVM_PASSPHRASE passphrase,
			const NVM_SIZE passphraseLen);

	/*
	 * Return the number of currently running jobs
	 */
	virtual int getJobCount();

	/*
	 * Retrieve job information about each NVM-DIMM
	 */
	virtual int getJobs(struct job *pJobs, const NVM_UINT32 count);

	/*
	 * Retrieve the number of pools
	 */
	virtual int getPoolCount();

	/*
	 * Retrieve a list of all pools in the system.
	 */
	virtual int getPools(struct pool *pPools, const NVM_UINT8 count);

	/*
	 * Retrieve information about a specified pool
	 */
	virtual int getPool(NVM_GUID poolGuid, struct pool *pPool);

	/*
	 * Retrieve the largest and smallest persistent and block namespaces that can
	 * be create on a given pool
	 */
	virtual int getAvailablePersistentSizeRange(const NVM_GUID poolGuid,
			struct possible_namespace_ranges *pRange);

	/*
	 * Modify how the NVM-DIMM capacity will be provisioned
	 */
	virtual int createConfigGoal(const NVM_GUID deviceGuid, struct config_goal *pGoal);

	/*
	 * Retrieve the configuration goal from the specified NVM-DIMM
	 */
	virtual int getConfigGoal(const NVM_GUID deviceGuid, struct config_goal *pGoal);

	/*
	 * Erase the config goal from the specified NVM-DIMM
	 */
	virtual int deleteConfigGoal(const NVM_GUID deviceGuid);

	/*
	 * Store the configuration goal of the specified NVM-DIMM to a given file
	 */
	virtual int dumpConfig(const NVM_GUID deviceGuid, const NVM_PATH file,
			const NVM_SIZE fileLen, const NVM_BOOL append);

	/*
	 * Load the configuration goal for a specified NVM-DIMM from a file
	 */
	virtual int loadConfig(const NVM_GUID deviceGuid, const NVM_PATH file, const NVM_SIZE fileLen);

	/*
	 * Retrieve the number of namespaces
	 */
	virtual int getNamespaceCount();

	/*
	 * Retrieve the number of namespaces allocated from the capacity of the specified device
	 */
	virtual int getDeviceNamespaceCount(const NVM_GUID deviceGuid,
			const enum namespace_type type);

	/*
	 * Retrieve discovery information about each namespace
	 */
	virtual int getNamespaces(struct namespace_discovery *pNamespaces, const NVM_UINT8 count);

	/*
	 * Retrieve detailed information about the specified namespace
	 */
	virtual int getNamespaceDetails(const NVM_GUID namespaceGuid, struct namespace_details *pNamespace);

	/*
	 * Create a namespace on the specified persistent memory pool
	 */
	virtual int createNamespace(NVM_GUID *p_namespace_guid, const NVM_GUID pool_guid,
			struct namespace_create_settings *p_settings,
			const struct interleave_format *p_format, const NVM_BOOL allow_adjustment);

	/*
	 * Change the friendly name of a specified namespace
	 */
	virtual int modifyNamespaceName(const NVM_GUID namespaceGuid, const NVM_NAMESPACE_NAME name);

	/*
	 * Change the block count of the specified namespace
	 */
	virtual int modifyNamespaceBlockCount(const NVM_GUID namespaceGuid, const NVM_UINT64 blockCount,
			NVM_BOOL allowAdjustment);

	/*
	 * Enable or disable the specified namespace
	 */
	virtual int modifyNamespaceEnabled(const NVM_GUID namespaceGuid,
			const enum namespace_enable_state enabled);

	/*
	 * Delete the specified namespace
	 */
	virtual int deleteNamespace(const NVM_GUID namespaceGuid);

	/*
	 * adjuste the block count to meet namespace creation alignment requirements
	 */
	virtual int adjustCreateNamespaceBlockCount(const NVM_GUID poolGuid,
			struct namespace_create_settings *pSettings, const struct interleave_format *pFormat);

	/*
	 * adjuste the block count to meet namespace modification alignment requirements
	 */
	virtual int adjustModifyNamespaceBlockCount(const NVM_GUID namespaceGuid, NVM_UINT64 *pBlockCount);

	/*
	 * Retrieve all the health sensors from the specified NVM-DIMM
	 */
	virtual int getSensors(const NVM_GUID deviceGuid, struct sensor *pSensors, const NVM_UINT16 count);

	/*
	 * Retrieve a specific health sensor from the specified NVM-DIMM
	 */
	virtual int getSensor(const NVM_GUID deviceGuid, const enum sensor_type type, struct sensor *pSensor);

	/*
	 * Change the critical threshold on the specified sensor
	 */
	virtual int setSensorSettings(const NVM_GUID deviceGuid, const enum sensor_type type,
			const struct sensor_settings *pSettings);

	/*
	 * Register a callback function to receive notifications
	 */
	virtual int addEventNotify(const enum event_type, void (*pEventCallback) (struct event *pEvent));

	/*
	 * Remove a callback subscription
	 */
	virtual int removeEventNotify(const int callbackId);

	/*
	 * Retrieve the number of stored events
	 */
	virtual int getEventCount(const struct event_filter *pFilter);

	/*
	 * Retrieve a list of stored events
	 */
	virtual int getEvents(const struct event_filter *pFilter, struct event *pEvents, const NVM_UINT16 count);

	/*
	 * Clear stored events
	 */
	virtual int purgeEvents(const struct event_filter *pFilter);

	/*
	 * Acknowledge an event
	 */
	virtual int acknowledgeEvent(NVM_UINT32 eventId);

	/*
	 * Create a snapshot of the system state
	 */
	virtual int saveState(const char *name, const NVM_SIZE nameLen);

	/*
	 * Clear any stored support data
	 */
	virtual int purgeStateData();

	/*
	 * Collect support data into a single file
	 */
	virtual int gatherSupport(const NVM_PATH supportFile, const NVM_SIZE supportFileLen);

	/*
	 * Run a diagnostic test on a specified NVM-DIMM
	 */
	virtual int runDiagnostic(const NVM_GUID deviceGuid, const struct diagnostic *pDiagnostic,
			NVM_UINT32 *pResults);

	/*
	 * Retrieve the current level of debug logging
	 */
	virtual int getFwLogLevel(const NVM_GUID deviceGuid, enum fw_log_level *pLogLevel);

	/*
	 * Modify the current level of debug logging
	 */
	virtual int setFwLogLevel(const NVM_GUID deviceGuid, const enum fw_log_level logLevel);

	/*
	 * Get device firmware info
	 */
	virtual int getDeviceFwInfo(const NVM_GUID device_guid, struct device_fw_info *p_fw_info);

	/*
	 * Inject an error into the specified NVM-DIMM
	 */
	virtual int injectDeviceError(const NVM_GUID deviceGuid, const struct device_error *pError);

	/*
	 * Clear the injected error
	 */
	virtual int clearInjectedDeviceError(const NVM_GUID deviceGuid, const struct device_error *pError);

	/*
	 * Load the simulator file
	 */
	virtual int addSimulator(const NVM_PATH simulator, const NVM_SIZE simulatorLen);

	/*
	 * Unload a simulator file
	 */
	virtual int removeSimulator();

	/*
	 * Determine if host software logging is enabled
	 */
	virtual int debugLoggingEnabled();

	/*
	 * Enable or disable host software logging
	 */
	virtual int toggleDebugLogging(const NVM_BOOL enabled);

	/*
	 * Retrieve the number of debug log entries
	 */
	virtual int getDebugLogCount();

	/*
	 * Retrieve a list of stored debug log entries
	 */
	virtual int getDebugLogs(struct log *pLogs, const NVM_UINT32 count);

	/*
	 * Clear all debug log entries
	 */
	virtual int purgeDebugLog();

protected:
	NvmApi();

	static NvmApi *m_pSingleton;
};

}

#endif //CR_MGMT_NVMAPI_H