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

#include <lib/nvm_management.h>

namespace core
{
class NVM_API LibWrapper
{
public:
	virtual ~LibWrapper();

	static LibWrapper &getLibWrapper();

	virtual int getError(const enum return_code code, NVM_ERROR_DESCRIPTION description,
		const NVM_SIZE descriptionLen) const;

	virtual int getHost(struct host *pHost) const;

	virtual int getHostName(char *hostName, const NVM_SIZE hostNameLen) const;

	virtual int getSwInventory(struct sw_inventory *pInventory) const;

	virtual int getSocketCount() const;

	virtual int getSockets(struct socket *pSockets, const NVM_UINT16 count) const;

	virtual int getSocket(const NVM_UINT16 socketId, struct socket *pSocket) const;

	virtual int getNvmCapabilities(struct nvm_capabilities *pCapabilities) const;

	virtual int getNvmCapacities(struct device_capacities *pCapacities) const;

	virtual int getMemoryTopologyCount() const;

	virtual int getMemoryTopology(struct memory_topology *pDevices, const NVM_UINT8 count) const;

	virtual int getDeviceCount() const;

	virtual int getDeviceDiscovery(NVM_UID uid, struct device_discovery *pDevice) const;

	virtual int getDevices(struct device_discovery *pDevices, const NVM_UINT8 count) const;

	virtual int getDeviceStatus(const NVM_UID deviceUid, struct device_status *pStatus) const;

	virtual int getDeviceSettings(const NVM_UID deviceUid,
		struct device_settings *pSettings) const;

	virtual int modifyDeviceSettings(const NVM_UID deviceUid,
		const struct device_settings *pSettings) const;

	virtual int getDeviceDetails(const NVM_UID deviceUid, struct device_details *pDetails) const;

	virtual int getDevicePerformance(const NVM_UID deviceUid,
		struct device_performance *pPerformance) const;

	virtual int updateDeviceFw(const NVM_UID deviceUid, const NVM_PATH path,
		const NVM_SIZE path_len, const NVM_BOOL activate, const NVM_BOOL force) const;

	virtual int examineDeviceFw(const NVM_UID deviceUid, const NVM_PATH path,
		const NVM_SIZE pathLen, NVM_VERSION imageVersion, const NVM_SIZE imageVersionSize) const;

	virtual int setPassphrase(const NVM_UID deviceUid, const NVM_PASSPHRASE oldPassphrase,
		const NVM_SIZE oldPassphraseLen, const NVM_PASSPHRASE newPassphrase,
		const NVM_SIZE newPassphraseLen) const;

	virtual int removePassphrase(const NVM_UID deviceUid, const NVM_PASSPHRASE passphrase,
		const NVM_SIZE passphraseLen) const;

	virtual int unlockDevice(const NVM_UID deviceUid, const NVM_PASSPHRASE passphrase,
		const NVM_SIZE passphraseLen) const;

	virtual int eraseDevice(const NVM_UID deviceUid,
		const NVM_PASSPHRASE passphrase,
		const NVM_SIZE passphraseLen) const;

	virtual int getJobCount() const;

	virtual int getJobs(struct job *pJobs, const NVM_UINT32 count) const;

	virtual int getPoolCount() const;

	virtual int getPools(struct pool *pPools, const NVM_UINT8 count) const;

	virtual int getPool(NVM_UID poolUid, struct pool *pPool) const;

	virtual int getAvailablePersistentSizeRange(const NVM_UID poolUid,
		struct possible_namespace_ranges *pRange) const;

	virtual int createConfigGoal(const NVM_UID deviceUid, struct config_goal *pGoal) const;

	virtual int getConfigGoal(const NVM_UID deviceUid, struct config_goal *pGoal) const;

	virtual int deleteConfigGoal(const NVM_UID deviceUid) const;

	virtual int dumpConfig(const NVM_UID deviceUid, const NVM_PATH file,
		const NVM_SIZE fileLen, const NVM_BOOL append) const;

	virtual int loadConfig(const NVM_UID deviceUid, const NVM_PATH file,
		const NVM_SIZE fileLen) const;

	virtual int getNamespaceCount() const;

	virtual int getDeviceNamespaceCount(const NVM_UID deviceUid,
		const enum namespace_type type) const;

	virtual int getNamespaces(struct namespace_discovery *pNamespaces, const NVM_UINT8 count) const;

	virtual int getNamespaceDetails(const NVM_UID namespaceUid,
		struct namespace_details *pNamespace) const;

	virtual int createNamespace(NVM_UID *p_namespace_uid, const NVM_UID pool_uid,
		struct namespace_create_settings *p_settings,
		const struct interleave_format *p_format, const NVM_BOOL allow_adjustment) const;

	virtual int modifyNamespaceName(const NVM_UID namespaceUid,
		const NVM_NAMESPACE_NAME name) const;

	virtual int modifyNamespaceBlockCount(const NVM_UID namespaceUid, const NVM_UINT64 blockCount,
		NVM_BOOL allowAdjustment) const;

	virtual int modifyNamespaceEnabled(const NVM_UID namespaceUid,
		const enum namespace_enable_state enabled) const;

	virtual int deleteNamespace(const NVM_UID namespaceUid) const;

	virtual int adjustCreateNamespaceBlockCount(const NVM_UID poolUid,
		struct namespace_create_settings *pSettings, const struct interleave_format *pFormat) const;

	virtual int adjustModifyNamespaceBlockCount(const NVM_UID namespaceUid,
		NVM_UINT64 *pBlockCount) const;

	virtual int getSensors(const NVM_UID deviceUid, struct sensor *pSensors,
		const NVM_UINT16 count) const;

	virtual int getSensor(const NVM_UID deviceUid, const enum sensor_type type,
		struct sensor *pSensor) const;

	virtual int setSensorSettings(const NVM_UID deviceUid, const enum sensor_type type,
		const struct sensor_settings *pSettings) const;

	virtual int addEventNotify(const enum event_type type,
		void (*pEventCallback)(struct event *pEvent)) const;

	virtual int removeEventNotify(const int callbackId) const;

	virtual int getEventCount(const struct event_filter *pFilter) const;

	virtual int getEvents(const struct event_filter *pFilter, struct event *pEvents,
		const NVM_UINT16 count) const;

	virtual int purgeEvents(const struct event_filter *pFilter) const;

	virtual int acknowledgeEvent(NVM_UINT32 eventId) const;

	virtual int saveState(const char *name, const NVM_SIZE nameLen) const;

	virtual int purgeStateData() const;

	virtual int gatherSupport(const NVM_PATH supportFile, const NVM_SIZE supportFileLen) const;

	virtual int runDiagnostic(const NVM_UID deviceUid, const struct diagnostic *pDiagnostic,
		NVM_UINT32 *pResults) const;

	virtual int getFwLogLevel(const NVM_UID deviceUid, enum fw_log_level *pLogLevel) const;

	virtual int setFwLogLevel(const NVM_UID deviceUid, const enum fw_log_level logLevel) const;

	virtual int getDeviceFwInfo(const NVM_UID device_uid, struct device_fw_info *p_fw_info) const;

	virtual int injectDeviceError(const NVM_UID deviceUid,
		const struct device_error *pError) const;

	virtual int clearInjectedDeviceError(const NVM_UID deviceUid,
		const struct device_error *pError) const;

	virtual int addSimulator(const NVM_PATH simulator, const NVM_SIZE simulatorLen) const;

	virtual int removeSimulator() const;

	virtual int debugLoggingEnabled() const;

	virtual int toggleDebugLogging(const NVM_BOOL enabled) const;

	virtual int getDebugLogCount() const;

	virtual int getDebugLogs(struct log *pLogs, const NVM_UINT32 count) const;

	virtual int purgeDebugLog() const;

protected:
	LibWrapper();
};

}

#endif //CR_MGMT_NVMAPI_H
