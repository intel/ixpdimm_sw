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
#ifndef CR_MGMT_NVMLIBRARY_H
#define CR_MGMT_NVMLIBRARY_H

#include "LibWrapper.h"

#include <string>
#include <vector>

namespace core
{
class NVM_API NvmLibrary
{

public:
	NvmLibrary(const LibWrapper &lib = LibWrapper::getLibWrapper());
	NvmLibrary(const NvmLibrary &other);
	virtual ~NvmLibrary();

	NvmLibrary &operator=(const NvmLibrary &other);
	static NvmLibrary &getNvmLibrary();

	virtual std::string getErrorMessage(const int errorCode);
	virtual struct host getHost();
	virtual std::string getHostName();
	virtual struct sw_inventory getSwInventory();
	virtual int getSocketCount();
	virtual std::vector<struct socket> getSockets();
	virtual struct socket getSocket(const NVM_UINT16 socketId);
	virtual struct nvm_capabilities getNvmCapabilities();
	virtual struct device_capacities getNvmCapacities();
	virtual int getMemoryTopologyCount();
	virtual std::vector<struct memory_topology> getMemoryTopology();
	virtual int getDeviceCount();
	virtual struct device_discovery getDeviceDiscovery(const std::string &uid);
	virtual std::vector<struct device_discovery> getDevices();
	virtual struct device_status getDeviceStatus(const std::string &deviceUid);
	virtual struct device_settings getDeviceSettings(const std::string &deviceUid);
	virtual void modifyDeviceSettings(const std::string &deviceUid,
		const struct device_settings &settings);
	virtual struct device_details getDeviceDetails(const std::string &deviceUid);
	virtual struct device_performance getDevicePerformance(const std::string &deviceUid);
	virtual void updateDeviceFw(const std::string &deviceUid, const std::string path,
		const bool activate, const bool force);
	virtual void examineDeviceFw(const std::string &deviceUid, const std::string path,
		std::string imageVersion);
	virtual void setPassphrase(const std::string &deviceUid, const std::string oldPassphrase,
		const std::string newPassphrase);
	virtual void removePassphrase(const std::string &deviceUid, const std::string passphrase);
	virtual void unlockDevice(const std::string &deviceUid, const std::string passphrase);
	virtual void eraseDevice(const std::string &deviceUid,
		const std::string passphrase);
	virtual int getJobCount();
	virtual std::vector<struct job> getJobs();
	virtual int getPoolCount();
	virtual std::vector<struct pool> getPools();
	virtual struct pool *getPool(const std::string &poolUid);
	virtual struct possible_namespace_ranges getAvailablePersistentSizeRange(const std::string &poolUid);
	virtual void createConfigGoal(const std::string &deviceUid, struct config_goal &pGoal);
	virtual struct config_goal getConfigGoal(const std::string &deviceUid);
	virtual void deleteConfigGoal(const std::string &deviceUid);
	virtual void dumpConfig(const std::string &deviceUid, const std::string file,
		const bool append);
	virtual void loadConfig(const std::string &deviceUid, const std::string file);
	virtual int getNamespaceCount();
	virtual int getDeviceNamespaceCount(const std::string &deviceUid,
		const enum namespace_type type);
	virtual std::vector<struct namespace_discovery> getNamespaces();
	virtual struct namespace_details getNamespaceDetails(const std::string &namespaceUid);
	virtual std::string createNamespace(const std::string &pool_uid,
		struct namespace_create_settings &p_settings, const struct interleave_format &p_format,
		const bool allow_adjustment);
	virtual void modifyNamespaceName(const std::string &namespaceUid,
		const std::string &name);
	virtual int modifyNamespaceBlockCount(const std::string &namespaceUid,
		const NVM_UINT64 blockCount, bool allowAdjustment);
	virtual void modifyNamespaceEnabled(const std::string &namespaceUid,
		const enum namespace_enable_state enabled);
	virtual void deleteNamespace(const std::string &namespaceUid);
	virtual void adjustCreateNamespaceBlockCount(const std::string &poolUid,
		struct namespace_create_settings &pSettings, const struct interleave_format &pFormat);
	virtual void adjustModifyNamespaceBlockCount(const std::string &namespaceUid, NVM_UINT64 &pBlockCount);
	virtual std::vector<struct sensor> getSensors(const std::string &deviceUid);
	virtual struct sensor getSensor(const std::string &deviceUid, const enum sensor_type type);
	virtual void setSensorSettings(const std::string &deviceUid, const enum sensor_type type,
		const struct sensor_settings &pSettings);
	virtual void addEventNotify(const enum event_type type,
		void (*pEventCallback)(struct event *pEvent));
	virtual void removeEventNotify(const int callbackId);
	virtual int getEventCount(const struct event_filter &pFilter);
	virtual std::vector<struct event> getEvents(const struct event_filter &pFilter);
	virtual void purgeEvents(const struct event_filter &pFilter);
	virtual void acknowledgeEvent(NVM_UINT32 eventId);
	virtual void saveState(const std::string name);
	virtual void purgeStateData();
	virtual void gatherSupport(const std::string supportFile);
	virtual void runDiagnostic(const std::string &deviceUid, const struct diagnostic &pDiagnostic, NVM_UINT32 &pResults);
	virtual enum fw_log_level getFwLogLevel(const std::string &deviceUid);
	virtual void setFwLogLevel(const std::string &deviceUid, const enum fw_log_level logLevel);
	virtual struct device_fw_info getDeviceFwInfo(const std::string &device_uid);
	virtual void injectDeviceError(const std::string &deviceUid,
		const struct device_error &pError);
	virtual void clearInjectedDeviceError(const std::string &deviceUid,
		const struct device_error &pError);
	virtual void addSimulator(const std::string simulator);
	virtual void removeSimulator();
	virtual bool isDebugLoggingEnabled();
	virtual void toggleDebugLogging(const bool enabled);
	virtual int getDebugLogCount();
	virtual std::vector<struct log> getDebugLogs();
	virtual void purgeDebugLog();

private:
	const LibWrapper &m_lib;
};
}

#endif //CR_MGMT_NVMLIBRARY_H
