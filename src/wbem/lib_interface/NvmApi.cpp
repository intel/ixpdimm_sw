/*
 * Copyright (c) 2015 2016, Intel Corporation
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
 * This file implements a C++ interface for retrieving data from the NVM library.
 */

#include "NvmApi.h"

#include <libinvm-cim/ExceptionNoMemory.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <uid/uid.h>
#include <LogEnterExit.h>
#include <os/os_adapter.h>
#include <exception/NvmExceptionLibError.h>

namespace wbem
{
namespace lib_interface
{

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

NvmApi *NvmApi::m_pSingleton = NULL;

NvmApi::NvmApi()
{
}

NvmApi::~NvmApi()
{
	m_pSingleton = NULL;
}

NvmApi* NvmApi::getApi()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	if (!m_pSingleton)
	{
		if(mutex_lock(&g_nvmapi_lock))
		{
			m_pSingleton = new NvmApi();
			if (!m_pSingleton)
			{
				mutex_unlock(&g_nvmapi_lock);
				// Never return NULL
				throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__,
						"couldn't allocate NvmApi singleton");
			}
			mutex_unlock(&g_nvmapi_lock);
		}
	}
	return m_pSingleton;
}

int NvmApi::getHost(struct host *pHost)
{
	return nvm_get_host(pHost);
}

int NvmApi::getHostName(char *hostName, const NVM_SIZE hostNameLen) const
{
	return nvm_get_host_name(hostName, hostNameLen);
}

int NvmApi::getSwInventory(struct sw_inventory *pInventory)
{
	return nvm_get_sw_inventory(pInventory);
}

int NvmApi::getSocketCount()
{
	return nvm_get_socket_count();
}

int NvmApi::getSockets(struct socket *pSockets, const NVM_UINT16 count)
{
	return nvm_get_sockets(pSockets, count);
}

int NvmApi::getSocket(const NVM_UINT16 socketId, struct socket *pSocket)
{
	return nvm_get_socket(socketId, pSocket);
}

int NvmApi::getNvmCapabilities(struct nvm_capabilities *pCapabilities)
{
	return nvm_get_nvm_capabilities(pCapabilities);
}

int NvmApi::getNvmCapacities(struct device_capacities *pCapacities)
{
	return nvm_get_nvm_capacities(pCapacities);
}

int NvmApi::getMemoryTopologyCount()
{
	return nvm_get_memory_topology_count();
}

int NvmApi::getMemoryTopology(struct memory_topology *pDevices, const NVM_UINT8 count)
{
	return nvm_get_memory_topology(pDevices, count);
}

int NvmApi::getDeviceCount() const
{
	return nvm_get_device_count();
}

int NvmApi::getDevices(struct device_discovery *pDevices, const NVM_UINT8 count) const
{
	return nvm_get_devices(pDevices, count);
}

int NvmApi::getDeviceDiscovery(NVM_UID uid, struct device_discovery *pDevice)
{
	return nvm_get_device_discovery(uid, pDevice);
}

int NvmApi::getDeviceStatus(const NVM_UID deviceUid, struct device_status *pStatus)
{
	return nvm_get_device_status(deviceUid, pStatus);
}

int NvmApi::getDeviceSettings(const NVM_UID deviceUid, struct device_settings *pSettings)
{
	return nvm_get_device_settings(deviceUid, pSettings);
}

int NvmApi::modifyDeviceSettings(const NVM_UID deviceUid,
				const struct device_settings *pSettings)
{
	return nvm_modify_device_settings(deviceUid, pSettings);
}

int NvmApi::getDeviceDetails(const NVM_UID deviceUid, struct device_details *pDetails)
{
	return nvm_get_device_details(deviceUid, pDetails);
}

int NvmApi::getDevicePerformance(const NVM_UID deviceUid,
				struct device_performance *pPerformance)
{
	return nvm_get_device_performance(deviceUid, pPerformance);
}

int NvmApi::getDeviceFwImageInfo(const NVM_UID deviceUid, struct device_fw_info *pFwInfo)
{
	return nvm_get_device_fw_image_info(deviceUid, pFwInfo);
}

int NvmApi::updateDeviceFw(const NVM_UID deviceUid, const NVM_PATH path,
				const NVM_SIZE path_len, const NVM_BOOL activate, const NVM_BOOL force)
{
	return nvm_update_device_fw(deviceUid, path, path_len, activate, force);
}

int NvmApi::examineDeviceFw(const NVM_UID deviceUid, const NVM_PATH path,
				const NVM_SIZE pathLen, NVM_VERSION imageVersion,
				const NVM_SIZE imageVersionSize)
{
	return nvm_examine_device_fw(deviceUid, path, pathLen, imageVersion, imageVersionSize);
}

int NvmApi::setPassphrase(const NVM_UID deviceUid, const NVM_PASSPHRASE oldPassphrase,
				const NVM_SIZE oldPassphraseLen, const NVM_PASSPHRASE newPassphrase,
				const NVM_SIZE newPassphraseLen)
{
	return nvm_set_passphrase(deviceUid, oldPassphrase, oldPassphraseLen,
			newPassphrase, newPassphraseLen);
}

int NvmApi::removePassphrase(const NVM_UID deviceUid, const NVM_PASSPHRASE passphrase,
				const NVM_SIZE passphraseLen)
{
	return nvm_remove_passphrase(deviceUid, passphrase, passphraseLen);
}

int NvmApi::unlockDevice(const NVM_UID deviceUid, const NVM_PASSPHRASE passphrase,
				const NVM_SIZE passphraseLen)
{
	return nvm_unlock_device(deviceUid, passphrase, passphraseLen);
}

int NvmApi::eraseDevice(const NVM_UID deviceUid, const NVM_PASSPHRASE passphrase,
				const NVM_SIZE passphraseLen)
{
	return nvm_erase_device(deviceUid, passphrase, passphraseLen);
}

int NvmApi::getJobCount()
{
	return nvm_get_job_count();
}

int NvmApi::getJobs(struct job *pJobs, const NVM_UINT32 count)
{
	return nvm_get_jobs(pJobs, count);
}

int NvmApi::getPoolCount()
{
	return nvm_get_pool_count();
}

int NvmApi::getPools(struct pool *pPools, const NVM_UINT8 count)
{
	return nvm_get_pools(pPools, count);
}

int NvmApi::getPool(NVM_UID poolUid, struct pool *pPool)
{
	return nvm_get_pool(poolUid, pPool);
}

int NvmApi::getAvailablePersistentSizeRange(const NVM_UID poolUid,
				struct possible_namespace_ranges *pRange)
{
	return nvm_get_available_persistent_size_range(poolUid, pRange);
}

int NvmApi::createConfigGoal(const NVM_UID deviceUid, struct config_goal *pGoal)
{
	return nvm_create_config_goal(deviceUid, pGoal);
}

int NvmApi::getConfigGoal(const NVM_UID deviceUid, struct config_goal *pGoal)
{
	return nvm_get_config_goal(deviceUid, pGoal);
}

int NvmApi::deleteConfigGoal(const NVM_UID deviceUid)
{
	return nvm_delete_config_goal(deviceUid);
}

int NvmApi::dumpConfig(const NVM_UID deviceUid, const NVM_PATH file,
				const NVM_SIZE fileLen, const NVM_BOOL append)
{
	return nvm_dump_config(deviceUid, file, fileLen, append);
}

int NvmApi::loadConfig(const NVM_UID deviceUid, const NVM_PATH file, const NVM_SIZE fileLen)
{
	return nvm_load_config(deviceUid, file, fileLen);
}

int NvmApi::getNamespaceCount()
{
	return nvm_get_namespace_count();
}

int NvmApi::getDeviceNamespaceCount(const NVM_UID deviceUid,
		const enum namespace_type nsType)
{
	return nvm_get_device_namespace_count(deviceUid, nsType);
}

int NvmApi::getNamespaces(struct namespace_discovery *pNamespaces, const NVM_UINT8 count)
{
	return nvm_get_namespaces(pNamespaces, count);
}

int NvmApi::getNamespaceDetails(const NVM_UID namespaceUid, struct namespace_details *pNamespace)
{
	return nvm_get_namespace_details(namespaceUid, pNamespace);
}

int NvmApi::createNamespace(NVM_UID *pNamespaceUid, const NVM_UID poolUid,
				struct namespace_create_settings *pSettings, const struct interleave_format *pFormat,
				const NVM_BOOL allowAdjustment)
{
	return nvm_create_namespace(pNamespaceUid, poolUid, pSettings, pFormat, allowAdjustment);
}

int NvmApi::modifyNamespaceName(const NVM_UID namespaceUid, const NVM_NAMESPACE_NAME name)
{
	return nvm_modify_namespace_name(namespaceUid, name);
}

int NvmApi::modifyNamespaceBlockCount(const NVM_UID namespaceUid, const NVM_UINT64 blockCount,
				NVM_BOOL allowAdjustment)
{
	return nvm_modify_namespace_block_count(namespaceUid, blockCount, allowAdjustment);
}

int NvmApi::modifyNamespaceEnabled(const NVM_UID namespaceUid,
				const enum namespace_enable_state enabled)
{
	return nvm_modify_namespace_enabled(namespaceUid, enabled);
}

int NvmApi::deleteNamespace(const NVM_UID namespaceUid)
{
	return nvm_delete_namespace(namespaceUid);
}

int NvmApi::adjustCreateNamespaceBlockCount(const NVM_UID poolUid, struct namespace_create_settings *pSettings,
											const struct interleave_format *pFormat)
{
	return nvm_adjust_create_namespace_block_count(poolUid, pSettings, pFormat);
}

int NvmApi::adjustModifyNamespaceBlockCount(const NVM_UID namespaceUid, NVM_UINT64 *pBlockCount)
{
	return nvm_adjust_modify_namespace_block_count(namespaceUid, pBlockCount);
}

int NvmApi::getSensors(const NVM_UID deviceUid, struct sensor *pSensors, const NVM_UINT16 count)
{
	return nvm_get_sensors(deviceUid, pSensors, count);
}

int NvmApi::getSensor(const NVM_UID deviceUid, const enum sensor_type type, struct sensor *pSensor)
{
	return nvm_get_sensor(deviceUid, type, pSensor);
}

int NvmApi::setSensorSettings(const NVM_UID deviceUid, const enum sensor_type type,
							const struct sensor_settings *pSettings)
{
	return nvm_set_sensor_settings(deviceUid, type, pSettings);
}

int NvmApi::addEventNotify(const enum event_type type, void (*pEventCallback) (struct event *pEvent))
{
	return nvm_add_event_notify(type, pEventCallback);
}

int NvmApi::removeEventNotify(const int callbackId)
{
	return nvm_remove_event_notify(callbackId);
}

int NvmApi::getEventCount(const struct event_filter *pFilter)
{
	return nvm_get_event_count(pFilter);
}

int NvmApi::getEvents(const struct event_filter *pFilter, struct event *pEvents, const NVM_UINT16 count)
{
	return nvm_get_events(pFilter, pEvents, count);
}

int NvmApi::purgeEvents(const struct event_filter *pFilter)
{
	return nvm_purge_events(pFilter);
}

int NvmApi::acknowledgeEvent(NVM_UINT32 eventId)
{
	return nvm_acknowledge_event(eventId);
}

int NvmApi::saveState(const char *name, const NVM_SIZE nameLen)
{
	return nvm_save_state(name, nameLen);
}

int NvmApi::purgeStateData()
{
	return nvm_purge_state_data();
}

int NvmApi::gatherSupport(const NVM_PATH supportFile, const NVM_SIZE supportFileLen)
{
	return nvm_gather_support(supportFile, supportFileLen);
}

int NvmApi::runDiagnostic(const NVM_UID deviceUid, const struct diagnostic *pDiagnostic,
				NVM_UINT32 *pResults)
{
	return nvm_run_diagnostic(deviceUid, pDiagnostic, pResults);
}

int NvmApi::getFwLogLevel(const NVM_UID deviceUid, enum fw_log_level *pLogLevel)
{
	return nvm_get_fw_log_level(deviceUid, pLogLevel);
}

int NvmApi::setFwLogLevel(const NVM_UID deviceUid, const enum fw_log_level logLevel)
{
	return nvm_set_fw_log_level(deviceUid, logLevel);
}

int NvmApi::injectDeviceError(const NVM_UID deviceUid, const struct device_error *pError)
{
	return nvm_inject_device_error(deviceUid, pError);
}

int NvmApi::clearInjectedDeviceError(const NVM_UID deviceUid, const struct device_error *pError)
{
	return nvm_clear_injected_device_error(deviceUid, pError);
}

int NvmApi::addSimulator(const NVM_PATH simulator, const NVM_SIZE simulatorLen)
{
	return nvm_add_simulator(simulator, simulatorLen);
}

int NvmApi::removeSimulator()
{
	return nvm_remove_simulator();
}

int NvmApi::debugLoggingEnabled()
{
	return nvm_debug_logging_enabled();
}

int NvmApi::toggleDebugLogging(const NVM_BOOL enabled)
{
	return nvm_toggle_debug_logging(enabled);
}

int NvmApi::getDebugLogCount()
{
	return nvm_get_debug_log_count();
}

int NvmApi::getDebugLogs(struct log *pLogs, const NVM_UINT32 count)
{
	return nvm_get_debug_logs(pLogs, count);
}

int NvmApi::purgeDebugLog()
{
	return nvm_purge_debug_log();
}

void NvmApi::stringToNvmUid(const std::string& uidStr, NVM_UID uid)
{
	// note: uidStr.size() doesn't include the null terminator so the '>=' comparator leaves
	// a char for that.
	if (uidStr.size() >= NVM_MAX_UID_LEN)
	{
		throw framework::ExceptionBadParameter("uidStr");
	}

	uid_copy(uidStr.c_str(), uid);
}

std::string NvmApi::getHostName() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	char host_name[NVM_COMPUTERNAME_LEN];
	int rc = NVM_SUCCESS;
	if ((rc = getHostName(host_name, NVM_COMPUTERNAME_LEN)) != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
	return host_name;
}

void NvmApi::getDevices(std::vector<struct device_discovery>& devices) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	int rc = getDeviceCount();
	if (rc < 0)
	{
		throw exception::NvmExceptionLibError(rc);
	}
	else if (rc > 0) // don't bother if there's nothing to fetch
	{
		int count = rc;
		struct device_discovery apiDevices[count];
		memset(apiDevices, 0, sizeof (apiDevices));
		rc = getDevices(apiDevices, count);
		if (rc < 0)
		{
			throw exception::NvmExceptionLibError(rc);
		}

		devices.clear();
		for (int i = 0; i < count; i++)
		{
			devices.push_back(apiDevices[i]);
		}
	}
}

void NvmApi::getMemoryTopology(std::vector<struct memory_topology>& memoryTopology) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	int rc = nvm_get_memory_topology_count();
	if (rc < 0)
	{
		throw exception::NvmExceptionLibError(rc);
	}
	else if (rc > 0) // don't bother if there's nothing to fetch
	{
		int count = rc;
		struct memory_topology apiMemTopology[count];
		memset(apiMemTopology, 0, sizeof (apiMemTopology));
		rc = nvm_get_memory_topology(apiMemTopology, count);
		if (rc < 0)
		{
			throw exception::NvmExceptionLibError(rc);
		}

		memoryTopology.clear();
		for (int i = 0; i < count; i++)
		{
			memoryTopology.push_back(apiMemTopology[i]);
		}
	}
}

void NvmApi::getPools(std::vector<struct pool>& pools) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	int rc = nvm_get_pool_count();
	if (rc < 0)
	{
		throw exception::NvmExceptionLibError(rc);
	}
	else if (rc > 0)
	{
		int count  = rc;
		pools.reserve(count);
		// Note: due to the size of struct pool and limitations
		// with OpenPegasus, pools has to be allocated from the heap
		struct pool *pPoolArray = (struct pool *)calloc(rc, sizeof (struct pool));
		if (pPoolArray)
		{
			rc = nvm_get_pools(pPoolArray, count);
			if (rc < 0)
			{
				free (pPoolArray);
				throw exception::NvmExceptionLibError(rc);
			}

			pools.clear();
			pools.reserve(count);
			for (int i = 0; i < count; i++)
			{
				pools.push_back(pPoolArray[i]);
			}
			free(pPoolArray);
		}
		else
		{
			throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__,
					"Allocating the pool array failed");
		}
	}
}

void NvmApi::getDeviceDiscoveryForDimm(const std::string& dimmUid,
		struct device_discovery& device) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	NVM_UID uid;
	stringToNvmUid(dimmUid, uid);

	int rc = nvm_get_device_discovery(uid, &device);
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

void NvmApi::getConfigGoalForDimm(const std::string &dimmUid, struct config_goal &goal) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	NVM_UID uid;
	stringToNvmUid(dimmUid, uid);

	int rc = nvm_get_config_goal(uid, &goal);
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

void NvmApi::getManageableDimms(std::vector<struct device_discovery>& manageableDevices) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<struct device_discovery> devices;
	getDevices(devices);

	std::vector<struct device_discovery>::const_iterator iter = devices.begin();
	for (; iter != devices.end(); iter++)
	{
		if ((*iter).manageability == MANAGEMENT_VALIDCONFIG)
		{
			manageableDevices.push_back(*iter);
		}
	}
}

} /* namespace framework */
} /* namespace wbem */
