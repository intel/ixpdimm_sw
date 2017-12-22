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
 * This file contains the implementation of the event monitoring class
 * of the NvmMonitor service which periodically detects and stores interesting events.
 */

#include <string.h>
#include <uid/uid.h>
#include <persistence/config_settings.h>
#include <algorithm>
#include "EventMonitor.h"
#include <persistence/event.h>
#include <string/s_str.h>
#include <LogEnterExit.h>
#include <cr_i18n.h>
#include <utility.h>
#include <nvm_context.h>
#include <core/exceptions/LibraryException.h>
#include <core/Helper.h>

/*
 * Macro to log a "platform config invalid" event.
 * We detect this in a few different places.
 */
#define	LOG_PLATFORM_CONFIG_INVALID_EVENT(uid, uidStr) \
	store_event_by_parts(EVENT_TYPE_CONFIG, \
			EVENT_SEVERITY_CRITICAL, \
			EVENT_CODE_CONFIG_DATA_INVALID, \
			uid, \
			true, \
			uidStr, \
			NULL, \
			NULL, \
			(enum diagnostic_result)DIAGNOSTIC_RESULT_UNKNOWN)

/*
 * Macro to simplify event logging for new DIMMs
 */
#define	LOG_NEW_DIMM_EVENT(code, uid, arg1, arg2, actionRequired) \
		store_event_by_parts(EVENT_TYPE_CONFIG, \
				EVENT_SEVERITY_INFO, \
				code, \
				uid, \
				actionRequired, \
				arg1, \
				arg2, \
				NULL, \
				DIAGNOSTIC_RESULT_UNKNOWN)

monitor::EventMonitor::EventMonitor(core::NvmLibrary &lib) :
	NvmMonitorBase("EVENT"),
	m_nsMgmtCallbackId(-1),
	m_lib(lib)
{
}

monitor::EventMonitor::~EventMonitor()
{
}

void monitor::EventMonitor::init()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NvmMonitorBase::init();
	startOfDay();

	log_gather();
}

void monitor::EventMonitor::cleanup()
{
	NvmMonitorBase::cleanup();
}

/*
 * Called when monitor starts, monitors issues that generally happen on reboot.
 */
void monitor::EventMonitor::startOfDay()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	nvm_create_context();

	checkDeviceStartUpStatus();
	runPlatformConfigDiagnostic();

	// auto-acknowledge action required events for namespaces
	// that no longer exist
	acknowledgeDeletedNamespaces();

	nvm_free_context(1);
}

void monitor::EventMonitor::runPlatformConfigDiagnostic()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	runDiagnostic(DIAG_TYPE_PLATFORM_CONFIG);

}

void monitor::EventMonitor::runDiagnostic(const diagnostic_test diagType, const std::string& uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	diagnostic diag;
	memset(&diag, 0, sizeof (diag));
	diag.test = diagType;

	try
	{
		NVM_UINT32 results = 0;
		m_lib.runDiagnostic(uid, diag, results);
	}
	catch (core::LibraryException &e)
	{
		COMMON_LOG_ERROR_F("Diagnostic type %d returned error %d",
				diagType, e.getErrorCode());
	}
}

void monitor::EventMonitor::checkDeviceStartUpStatus()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	DeviceMap devMap = getCurrentDeviceMapWithSavedTopology();

	checkDeviceTopologyForChanges(devMap);
	checkConfigStatusForAllDevices(devMap);

	saveCurrentTopologyState(devMap);
}

monitor::DeviceMap monitor::EventMonitor::getCurrentDeviceMapWithSavedTopology()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	DeviceMap map = getCurrentDeviceMap();
	if (isSavedTopologyStateValid())
	{
		addSavedTopologyStateToDeviceMap(map);
	}

	return map;
}

monitor::DeviceMap monitor::EventMonitor::getCurrentDeviceMap()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	DeviceMap map;
	addCurrentDevicesToDeviceMap(map);
	return map;
}

void monitor::EventMonitor::addCurrentDevicesToDeviceMap(monitor::DeviceMap& map)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	try
	{
		std::vector<device_discovery> devList = m_lib.getDevices();

		for (size_t i = 0; i < devList.size(); i++)
		{
			std::string uid = core::Helper::uidToString(devList[i].uid);
			map[uid] = getTopologyInfoForDevice(devList[i]);
		}
	}
	catch (core::LibraryException &e)
	{
		COMMON_LOG_ERROR_F("Couldn't get devices - error: %d", e.getErrorCode());
	}
}

monitor::deviceInfo monitor::EventMonitor::getTopologyInfoForDevice(const struct device_discovery& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct deviceInfo devInfo;
	memset(&devInfo, 0, sizeof (deviceInfo));
	devInfo.discovered = true;
	devInfo.discovery = device;

	if (device.manageability == MANAGEMENT_VALIDCONFIG)
	{
		std::string uid = core::Helper::uidToString(device.uid);
		try
		{
			devInfo.status = m_lib.getDeviceStatus(uid);
		}
		catch (core::LibraryException &e)
		{
			COMMON_LOG_ERROR_F("Couldn't get status for dimm %s, error = %d",
					uid.c_str(), e.getErrorCode());
		}
	}

	return devInfo;
}

bool monitor::EventMonitor::isSavedTopologyStateValid()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	int valid = 0;
	get_config_value_int(SQL_KEY_TOPOLOGY_STATE_VALID, &valid);

	return (bool)valid;
}

void monitor::EventMonitor::addSavedTopologyStateToDeviceMap(monitor::DeviceMap& map)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<struct db_topology_state> topologyState = getSavedTopologyState();

	for (size_t i = 0; i < topologyState.size(); i++)
	{
		std::string uidStr = topologyState[i].uid;
		if (map.find(uidStr) == map.end()) // doesn't exist - missing dimm
		{
			struct deviceInfo devInfo;
			memset(&devInfo, 0, sizeof (deviceInfo));
			devInfo.discovered = false;
			devInfo.stored = true;
			devInfo.storedState = topologyState[i];
			map[uidStr] = devInfo;
		}
		else
		{
			map[uidStr].stored = true;
			map[uidStr].storedState = topologyState[i];
		}
	}
}

std::vector<struct db_topology_state> monitor::EventMonitor::getSavedTopologyState()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<struct db_topology_state> topologyState;
	PersistentStore *pStore = get_lib_store();
	if (pStore && isSavedTopologyStateValid())
	{
		int topoStateCount = 0;
		if (db_get_topology_state_count(pStore, &topoStateCount) == DB_SUCCESS &&
				topoStateCount > 0)
		{
			// Populate the previous topology state map
			struct db_topology_state *dbTopoState = new db_topology_state[topoStateCount];
			if (db_get_topology_states(pStore, dbTopoState, topoStateCount)
					== topoStateCount)
			{
				for (int i = 0; i < topoStateCount; i++)
				{
					topologyState.push_back(dbTopoState[i]);
				}
			}
            delete dbTopoState;
		}
	}

	return topologyState;
}

void monitor::EventMonitor::checkDeviceTopologyForChanges(const DeviceMap &devMap)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Detect topology changes if topo is saved
	if (isSavedTopologyStateValid())
	{
		std::vector<std::string> replacedUids;
		processTopologyNewDimms(devMap, replacedUids);
		processTopologyModifiedDimms(devMap, replacedUids);
	}
}

void monitor::EventMonitor::processTopologyNewDimms(const DeviceMap &devices,
		std::vector<std::string> &replacedUids)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (DeviceMap::const_iterator iter = devices.begin();
			iter != devices.end(); iter++)
	{
		const std::string &uidStr = iter->first;
		const struct deviceInfo &device = iter->second;

		// device is new if it's discovered, but not stored
		if (device.discovered && !device.stored)
		{
			std::string replacedUid =
					getReplacedDimmUid(devices, device.discovery.device_handle.handle);

			// new dimm
			if (replacedUid.empty())
			{
				if (device.status.is_new)
				{
					// user needs to configure the DIMM
					LOG_NEW_DIMM_EVENT(EVENT_CODE_CONFIG_TOPOLOGY_ADDED_NEW_DEVICE,
							device.discovery.uid,
							uidStr.c_str(),
							NULL,
							false); // no Action Required
				}
				else
				{
					// configured DIMM found
					LOG_NEW_DIMM_EVENT(EVENT_CODE_CONFIG_TOPOLOGY_ADDED_CONFIGURED_DEVICE,
							device.discovery.uid,
							uidStr.c_str(),
							NULL,
							false);
				}
			}
			// replaced dimm
			else
			{
				replacedUids.push_back(replacedUid);
				if (device.status.is_new)
				{
					// user needs to configure the DIMM
					LOG_NEW_DIMM_EVENT(EVENT_CODE_CONFIG_TOPOLOGY_REPLACED_NEW_DEVICE,
							device.discovery.uid,
							replacedUid.c_str(),
							uidStr.c_str(),
							false);  // no Action Required
				}
				else
				{
					// configured DIMM found
					LOG_NEW_DIMM_EVENT(EVENT_CODE_CONFIG_TOPOLOGY_REPLACED_CONFIGURED_DEVICE,
							device.discovery.uid,
							replacedUid.c_str(),
							uidStr.c_str(),
							false);
				}

				// acknowledge any action required events on the replaced dimm
				NVM_UID uid;
				uid_copy(replacedUid.c_str(), uid);
				acknowledgeEventCodeForDevice(-1, uid);
			}
		}
	}
}

std::string monitor::EventMonitor::getReplacedDimmUid(
		const DeviceMap &devices, const NVM_UINT32 &handle)
{
	std::string replacedUid = "";

	// find a stored device with the same handle that is no longer discovered
	// this would indicated a new dimm replaced an old dimm
	for (DeviceMap::const_iterator iter = devices.begin();
				iter != devices.end(); iter++)
	{
		const struct deviceInfo &device = iter->second;
		if (!device.discovered &&
			device.stored &&
			device.storedState.device_handle == handle)
		{
			replacedUid = device.storedState.uid;
			break;
		}
	}
	return replacedUid;
}

void monitor::EventMonitor::processTopologyModifiedDimms(const DeviceMap &devices,
		const std::vector<std::string> &replacedUids)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (DeviceMap::const_iterator iter = devices.begin();
				iter != devices.end(); iter++)
	{
		const std::string &uidStr = iter->first;
		const struct deviceInfo &device = iter->second;

		// device has moved (handle has changed)
		if (device.discovered && device.stored &&
				(device.discovery.device_handle.handle !=
				device.storedState.device_handle))
		{
			store_event_by_parts(EVENT_TYPE_CONFIG,
					EVENT_SEVERITY_INFO,
					EVENT_CODE_CONFIG_TOPOLOGY_MOVED_DEVICE,
					device.discovery.uid,
					false,
					uidStr.c_str(),
					NULL,
					NULL,
					DIAGNOSTIC_RESULT_UNKNOWN);
		}
		// old device not discovered
		else if (device.stored && !device.discovered)
		{
			// Make sure it's not in the replaced UIDs list
			// if so it's already been covered by replacement events
			if (std::find(replacedUids.begin(), replacedUids.end(),
					uidStr) == replacedUids.end())
			{
				NVM_UID uid;
				uid_copy(uidStr.c_str(), uid);
				store_event_by_parts(EVENT_TYPE_CONFIG,
							EVENT_SEVERITY_CRITICAL,
							EVENT_CODE_CONFIG_TOPOLOGY_MISSING_DEVICE,
							uid,
							false,
							uidStr.c_str(),
							NULL,
							NULL,
							DIAGNOSTIC_RESULT_UNKNOWN);

				// since the dimm is missing,
				// automatically acknowledge any action required events for this dimm
				acknowledgeEventCodeForDevice(-1, uid);
			}
		}
	}
}

void monitor::EventMonitor::saveCurrentTopologyState(const DeviceMap &devices)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool saved = true;

	PersistentStore *pStore = get_lib_store();
	if (pStore)
	{
		// Only keep the latest topology
		if (db_delete_all_topology_states(pStore) != DB_SUCCESS)
		{
			COMMON_LOG_ERROR("couldn't delete old topology_state");
			saved = false;
		}
		else
		{
			// Preserve topology state in config DB
			for (DeviceMap::const_iterator iter = devices.begin();
					iter != devices.end(); iter++)
			{
				const std::string &uidStr = iter->first;
				const struct deviceInfo &device = iter->second;

				// only store current devices
				if (device.discovered)
				{
					struct db_topology_state topoState;
					memset(&topoState, 0, sizeof(topoState));
					s_strcpy(topoState.uid, uidStr.c_str(), NVM_MAX_UID_LEN);
					topoState.device_handle = device.discovery.device_handle.handle;
					topoState.manufacturer = MANUFACTURER_TO_UINT(device.discovery.manufacturer);
					topoState.serial_num = SERIAL_NUMBER_TO_UINT(device.discovery.serial_number);
					memmove(topoState.part_num, device.discovery.part_number, NVM_PART_NUM_LEN);

					topoState.current_config_status = device.status.config_status;
					if (device.discovery.manageability == MANAGEMENT_VALIDCONFIG)
					{
						topoState.config_goal_status = getConfigGoalStatusForDevice(uidStr);
					}
					else
					{
						topoState.config_goal_status = CONFIG_GOAL_STATUS_UNKNOWN;
					}

					if (db_add_topology_state(pStore, &topoState) != DB_SUCCESS)
					{
						COMMON_LOG_ERROR_F("couldn't add topology_state for DIMM %s",
								topoState.uid);
						saved = false;
						break;
					}
				}
			}
		}

		// everything succeeded
		if (saved)
		{
			add_config_value(SQL_KEY_TOPOLOGY_STATE_VALID, "1");
		}
		else
		{
			add_config_value(SQL_KEY_TOPOLOGY_STATE_VALID, "0");
		}
	}
}

void monitor::EventMonitor::checkConfigStatusForAllDevices(monitor::DeviceMap &devMap)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (monitor::DeviceMap::iterator devIter = devMap.begin(); devIter != devMap.end(); devIter++)
	{
		const std::string &uidStr = devIter->first;
		struct deviceInfo &device = devIter->second;

		if (device.discovered && device.discovery.manageability == MANAGEMENT_VALIDCONFIG)
		{
			checkConfigGoalStatus(uidStr, device);
		}
	}
}

void monitor::EventMonitor::checkConfigGoalStatus(const std::string &uidStr, const deviceInfo &device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	enum config_goal_status configGoalStatus = getConfigGoalStatusForDevice(uidStr);
	// ensure this is a new event
	if (!device.stored ||
		configGoalStatus != device.storedState.config_goal_status)
	{
		if (configGoalStatus == CONFIG_GOAL_STATUS_SUCCESS)
		{
			createEventForConfigGoalAppliedOnDevice(uidStr);

			// Action Required config events for this DIMM are resolved
			acknowledgePlatformConfigEventsForDevice(uidStr);
		}
	}
}

config_goal_status monitor::EventMonitor::getConfigGoalStatusForDevice(const std::string& uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	config_goal_status status = CONFIG_GOAL_STATUS_UNKNOWN;
	try
	{
		struct config_goal goal = m_lib.getConfigGoal(uid);
		status = goal.status;
	}
	catch (core::LibraryException &e)
	{
		status = CONFIG_GOAL_STATUS_UNKNOWN;

		if (e.getErrorCode() == NVM_ERR_NOTFOUND)
		{
			COMMON_LOG_DEBUG_F("No goal for DIMM %s", uid.c_str());
		}
		else
		{
			COMMON_LOG_ERROR_F("Error fetching config goal for DIMM %s: %d",
					uid.c_str(),
					e.getErrorCode());
		}
	}

	return status;
}

void monitor::EventMonitor::createEventForConfigGoalAppliedOnDevice(const std::string& uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	store_event_by_parts(EVENT_TYPE_CONFIG,
						 EVENT_SEVERITY_INFO,
						 EVENT_CODE_CONFIG_GOAL_APPLIED,
						 uid.c_str(),
						 false,
						 uid.c_str(),
						 NULL,
						 NULL,
						 DIAGNOSTIC_RESULT_UNKNOWN);
}

void monitor::EventMonitor::acknowledgePlatformConfigEventsForDevice(const std::string& uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	acknowledgeEventTypeForDevice(EVENT_TYPE_CONFIG, uid);
	acknowledgeEventTypeForDevice(EVENT_TYPE_DIAG_PLATFORM_CONFIG, uid);
}

void monitor::EventMonitor::acknowledgeEventTypeForDevice(const event_type type,
		const std::string& uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct event_filter filter;
	memset(&filter, 0, sizeof (filter));
	filter.filter_mask = NVM_FILTER_ON_UID | NVM_FILTER_ON_TYPE;
	s_strcpy(filter.uid, uid.c_str(), NVM_MAX_UID_LEN);
	filter.type = type;
	acknowledge_events(&filter);
}

void monitor::EventMonitor::acknowledgeEventCodeForDevice(const int eventCode, const NVM_UID uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// find event
	struct event_filter filter;
	memset(&filter, 0, sizeof (filter));
	filter.filter_mask = NVM_FILTER_ON_AR;
	filter.action_required = true;

	// add event code filter
	if (eventCode >= 0)
	{
		filter.filter_mask = NVM_FILTER_ON_CODE;
		filter.code = eventCode;
	}

	// add device uid filter
	if (uid)
	{
		filter.filter_mask |= NVM_FILTER_ON_UID;
		uid_copy(uid, filter.uid);
	}

	acknowledge_events(&filter);
}

void monitor::EventMonitor::acknowledgeDeletedNamespaces()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	try
	{
		// find action required events for all namespaces
		struct event_filter filter;
		memset(&filter, 0, sizeof(filter));
		filter.filter_mask = NVM_FILTER_ON_AR | NVM_FILTER_ON_CODE;
		filter.action_required = true;
		filter.code = EVENT_CODE_HEALTH_NAMESPACE_HEALTH_STATE_CHANGED;
		std::vector<event> events = m_lib.getEvents(filter);
		if (events.size() > 0)
		{
			std::vector<std::string> nsUids;
			bool ackAll = false;
			// get namespace list

			std::vector<namespace_discovery> namespaces = m_lib.getNamespaces();
			if (namespaces.size() == 0)
			{
				ackAll = true; // no namespaces, acknowledge them all
			}
			else // at least one namespace
			{
				for (size_t i = 0; i < namespaces.size(); i++)
				{
					nsUids.push_back(core::Helper::uidToString(namespaces[i].namespace_uid));
				}
			}

			// don't auto-acknowledge on failure to get namespaces
			if (namespaces.size() > 0 || ackAll)
			{
				for (size_t i = 0; i < events.size(); i++)
				{
					if (ackAll || namespaceDeleted(events[i].uid, nsUids))
					{
						acknowledgeEventCodeForDevice(EVENT_CODE_HEALTH_NAMESPACE_HEALTH_STATE_CHANGED,
								events[i].uid);
					}
				}
			}
		}
	}
	catch (core::LibraryException &e)
	{
		COMMON_LOG_ERROR_F("Unable to acknowledge events for namespaces - error %d", e.getErrorCode());
	}
}

bool monitor::EventMonitor::namespaceDeleted(const NVM_UID nsUid,
		const std::vector<std::string> &nsUids)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool deleted = false;

	// look for the uid in the list
	std::string uidStr = core::Helper::uidToString(nsUid);
	if (std::find(nsUids.begin(), nsUids.end(), uidStr) == nsUids.end())
	{
		// if not found, then deleted
		deleted = true;
	}

	return deleted;
}

/*
 * Helper to convert device health to string
 */
std::string monitor::EventMonitor::namespaceHealthToStr(enum namespace_health health)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string healthStr;
	switch (health)
	{
		case NAMESPACE_HEALTH_NORMAL:
			healthStr = TR("Healthy");
			break;
		case NAMESPACE_HEALTH_NONCRITICAL:
			healthStr = TR("Degraded/Warning");
			break;
		case NAMESPACE_HEALTH_CRITICAL:
			healthStr = TR("Critical Error");
			break;
		case NAMESPACE_HEALTH_BROKENMIRROR:
			healthStr = TR("Broken Mirror");
			break;
		default:
			healthStr = TR("Unknown");
			break;
	}
	return healthStr;
}

/*
 * Check for namespace health transitions
 */
void monitor::EventMonitor::monitorNamespaces(PersistentStore *pStore)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (pStore)
	{
		try
		{
			std::vector<namespace_discovery> namespaces = m_lib.getNamespaces();

			// for each namespace
			for (size_t i = 0; i < namespaces.size(); i++)
			{
				std::string uidStr = core::Helper::uidToString(namespaces[i].namespace_uid);
				try
				{
					struct namespace_details details = m_lib.getNamespaceDetails(uidStr);

					// get the stored state for this namespace
					bool storedStateChanged = false;

					struct db_namespace_state storedState;
					memset(&storedState, 0, sizeof (storedState));
					if (db_get_namespace_state_by_namespace_uid(pStore,
							uidStr.c_str(), &storedState) != DB_SUCCESS)
					{
						// initial state, just store current state
						s_strcpy(storedState.namespace_uid, uidStr.c_str(),
								NAMESPACE_STATE_NAMESPACE_UID_LEN);
						storedState.health_state = details.health;
						storedStateChanged = true;
					}
					// log health transition event
					else if (details.health != storedState.health_state)
					{
						enum event_severity severity = EVENT_SEVERITY_INFO;
						bool actionRequired = false;
						// namespace is failed
						if (details.health == NAMESPACE_HEALTH_CRITICAL ||
							details.health == NAMESPACE_HEALTH_BROKENMIRROR)
						{
							severity = EVENT_SEVERITY_CRITICAL;
							actionRequired = true;
						}
						// namespace is not failed
						else
						{
							// auto-acknowledge any old namespace health failed events
							acknowledgeEventCodeForDevice(EVENT_CODE_HEALTH_NAMESPACE_HEALTH_STATE_CHANGED,
									namespaces[i].namespace_uid);
						}

						std::string oldState = namespaceHealthToStr(
								(enum namespace_health)storedState.health_state);
						std::string newState = namespaceHealthToStr(details.health);
						store_event_by_parts(
								EVENT_TYPE_HEALTH,
								severity,
								EVENT_CODE_HEALTH_NAMESPACE_HEALTH_STATE_CHANGED,
								namespaces[i].namespace_uid,
								actionRequired,
								uidStr.c_str(),
								oldState.c_str(),
								newState.c_str(),
								DIAGNOSTIC_RESULT_UNKNOWN);

						storedStateChanged = true;
						storedState.health_state = details.health;
						if (db_delete_namespace_state_by_namespace_uid(pStore,
								uidStr.c_str()) != DB_SUCCESS)
						{
							COMMON_LOG_ERROR_F(
								"Failed to clean up the stored health state for namespace %s",
								uidStr.c_str());
						}
					}

					if (storedStateChanged)
					{
						if (db_add_namespace_state(pStore, &storedState) != DB_SUCCESS)
						{
							COMMON_LOG_ERROR_F(
								"Failed to update the stored health state for namespace %s",
								uidStr.c_str());
						}
					}
				} // end nvm_get_namespace_details
				catch (core::LibraryException &e)
				{
					COMMON_LOG_ERROR_F("Unable to get details for namespace %s - error %d",
							uidStr.c_str(), e.getErrorCode());
				}
			}
		}
		catch (core::LibraryException &e)
		{
			COMMON_LOG_ERROR_F("Unable to get namespaces - error %d", e.getErrorCode());
		}
	} // end get peristent store
}

void monitor::EventMonitor::monitor()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	nvm_free_context(1);
	nvm_create_context();

	monitorDevices();

	PersistentStore *pStore = get_lib_store();
	if (pStore)
	{
		// Monitor namespace health transitions
		monitorNamespaces(pStore);
	}

	nvm_free_context(1);
	log_gather();
}

void monitor::EventMonitor::monitorDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	DeviceMap devices = getCurrentDeviceMap();
	for (DeviceMap::const_iterator dev = devices.begin(); dev != devices.end(); dev++)
	{
		runQuickHealthDiagnosticForDevice(dev->first);
		monitorChangesForDevice(dev->second);
	}
}

void monitor::EventMonitor::runQuickHealthDiagnosticForDevice(const std::string& uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	runDiagnostic(DIAG_TYPE_QUICK, uid);
}

void monitor::EventMonitor::monitorChangesForDevice(const deviceInfo& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (device.discovery.manageability == MANAGEMENT_VALIDCONFIG)
	{
		struct db_dimm_state updatedDeviceState;
		memset(&updatedDeviceState, 0, sizeof (updatedDeviceState));

		try
		{
			updatedDeviceState = getSavedStateForDevice(device);

			processSensorStateChangesForDevice(device, updatedDeviceState);
			processHealthChangesForDevice(device, updatedDeviceState);
			processSanitizeChangesForDevice(device, updatedDeviceState);
		}
		catch (NoDeviceSavedState &)
		{
			initializeDimmState(updatedDeviceState, device);
		}

		saveStateForDevice(updatedDeviceState);
	}
}

struct db_dimm_state monitor::EventMonitor::getSavedStateForDevice(const deviceInfo& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct db_dimm_state savedState;
	memset(&savedState, 0, sizeof (savedState));

	if (db_get_dimm_state_by_device_handle(get_lib_store(),
			device.discovery.device_handle.handle,
			&savedState) != DB_SUCCESS)
	{
		throw NoDeviceSavedState();
	}

	return savedState;
}

void monitor::EventMonitor::saveStateForDevice(struct db_dimm_state& newState)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (db_delete_dimm_state_by_device_handle(get_lib_store(), newState.device_handle)
			!= DB_SUCCESS)
	{
		COMMON_LOG_INFO_F("Unable to delete old dimm_state for %d",
				newState.device_handle);
	}

	if (db_add_dimm_state(get_lib_store(), &newState) != DB_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Unable to save dimm_state for %d",
				newState.device_handle);
	}
}

void monitor::EventMonitor::processSensorStateChangesForDevice(const deviceInfo& device,
		struct db_dimm_state &dimmState)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<sensor> sensors = getSensorsForDevice(device);

	detectFwErrorSensorChanges(sensors, device.discovery.uid, dimmState);

	updateStateForFwErrorSensors(dimmState, sensors);
}

/*
 * Returns an empty list if there was an error getting the sensors
 */
std::vector<sensor> monitor::EventMonitor::getSensorsForDevice(const deviceInfo& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<sensor> sensors;
	std::string uid = core::Helper::uidToString(device.discovery.uid);
	try
	{
		sensors = m_lib.getSensors(uid);
	}
	catch (core::LibraryException &e)
	{
		COMMON_LOG_ERROR_F("Unable to get sensors for device %s, rc = %d",
				uid.c_str(), e.getErrorCode());
	}

	return sensors;
}

bool monitor::EventMonitor::sensorReadingHasIncreased(const std::vector<sensor>& sensors,
		const sensor_type sensorType, const NVM_UINT64 oldReading)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (sensorsIncludeType(sensors, sensorType) &&
			(sensors[sensorType].reading > oldReading));
}

bool monitor::EventMonitor::sensorsIncludeType(const std::vector<sensor>& sensors,
		const sensor_type type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (sensors.size() > type);
}

void monitor::EventMonitor::detectFwErrorSensorChanges(const std::vector<sensor>& sensors,
		const NVM_UID deviceUid, const struct db_dimm_state& savedState)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (sensorReadingHasIncreased(sensors, SENSOR_FWERRORLOGCOUNT, savedState.fw_log_errors))
	{
		NVM_UINT64 newErrors = sensors[SENSOR_FWERRORLOGCOUNT].reading - savedState.fw_log_errors;
		createFwErrorLogEvent(deviceUid, newErrors);
	}
}

void monitor::EventMonitor::createFwErrorLogEvent(const NVM_UID deviceUid,
		const NVM_UINT64 errorCount)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream newErrorCount;
	newErrorCount << errorCount;

	store_event_by_parts(EVENT_TYPE_HEALTH,
			EVENT_SEVERITY_WARN,
			EVENT_CODE_HEALTH_NEW_FWERRORS_FOUND,
			deviceUid,
			false,
			core::Helper::uidToString(deviceUid).c_str(),
			newErrorCount.str().c_str(),
			NULL,
			DIAGNOSTIC_RESULT_UNKNOWN);
}

void monitor::EventMonitor::updateStateForFwErrorSensors(struct db_dimm_state& dimmState,
		const std::vector<sensor>& sensors)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	dimmState.fw_log_errors = getLatestSensorReading(sensors, SENSOR_FWERRORLOGCOUNT,
			dimmState.fw_log_errors);
}

void monitor::EventMonitor::initializeDimmState(struct db_dimm_state& dimmState, const deviceInfo& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	dimmState.device_handle = device.discovery.device_handle.handle;
	dimmState.health_state = device.status.health;
	dimmState.sanitize_status = device.status.sanitize_status;

	initializeSensorStateForDevice(dimmState, device);
}

void monitor::EventMonitor::initializeSensorStateForDevice(struct db_dimm_state& dimmState,
		const deviceInfo& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<sensor> sensors = getSensorsForDevice(device);

	dimmState.fw_log_errors = getLatestSensorReading(sensors, SENSOR_FWERRORLOGCOUNT, 0);
}

NVM_UINT64 monitor::EventMonitor::getLatestSensorReading(const std::vector<sensor>& sensors,
		const sensor_type sensorType, const NVM_UINT64 oldReading)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 reading = oldReading;
	if (sensorsIncludeType(sensors, sensorType))
	{
		reading = sensors[sensorType].reading;
	}

	return reading;
}

void monitor::EventMonitor::processHealthChangesForDevice(const deviceInfo& device,
		struct db_dimm_state& dimmState)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	device_health oldHealth = (device_health)dimmState.health_state;
	device_health newHealth = device.status.health;

	if (newHealth != oldHealth)
	{
		acknowledgePastHealthChangesForDevice(device);

		createDeviceHealthEvent(device.discovery.uid, oldHealth, newHealth);
		dimmState.health_state = newHealth;
	}
}

void monitor::EventMonitor::acknowledgePastHealthChangesForDevice(const deviceInfo& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	acknowledgeEventCodeForDevice(EVENT_CODE_HEALTH_HEALTH_STATE_CHANGED,
			device.discovery.uid);
}

void monitor::EventMonitor::createDeviceHealthEvent(const NVM_UID uid,
		const device_health oldHealth, const device_health newHealth)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string oldHealthStr = deviceHealthToStr(oldHealth);
	std::string newHealthStr = deviceHealthToStr(newHealth);

	store_event_by_parts(EVENT_TYPE_HEALTH,
			getEventSeverityForDeviceHealth(newHealth),
			EVENT_CODE_HEALTH_HEALTH_STATE_CHANGED,
			uid,
			isActionRequiredForDeviceHealth(newHealth),
			core::Helper::uidToString(uid).c_str(),
			oldHealthStr.c_str(),
			newHealthStr.c_str(),
			DIAGNOSTIC_RESULT_UNKNOWN);
}

void monitor::EventMonitor::processSanitizeChangesForDevice(const deviceInfo& device,
		struct db_dimm_state& dimmState)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	device_sanitize_status oldSanitizeStatus = (device_sanitize_status)dimmState.sanitize_status;
	device_sanitize_status newSanitizeStatus = device.status.sanitize_status;
	if ((newSanitizeStatus == DEVICE_SANITIZE_STATUS_INPROGRESS || newSanitizeStatus == DEVICE_SANITIZE_STATUS_COMPLETE) &&
		(newSanitizeStatus != oldSanitizeStatus))
	{
		createSanitizeOperationEvent(device.discovery.uid, oldSanitizeStatus, newSanitizeStatus);
		dimmState.sanitize_status = newSanitizeStatus;
	}
}

void monitor::EventMonitor::createSanitizeOperationEvent(const NVM_UID uid,
	const device_sanitize_status oldSanitizeStatus, const device_sanitize_status newSanitizeStatus)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (newSanitizeStatus == DEVICE_SANITIZE_STATUS_INPROGRESS)
	{
		store_event_by_parts(EVENT_TYPE_HEALTH,
				EVENT_SEVERITY_CRITICAL,
				EVENT_CODE_HEALTH_SANITIZE_INPROGRESS,
				uid,
				true,
				core::Helper::uidToString(uid).c_str(),
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_UNKNOWN);
	}
	else if (newSanitizeStatus == DEVICE_SANITIZE_STATUS_COMPLETE)
	{
		acknowledgeEventCodeForDevice(EVENT_CODE_HEALTH_SANITIZE_INPROGRESS, uid);

		store_event_by_parts(EVENT_TYPE_HEALTH,
				EVENT_SEVERITY_CRITICAL,
				EVENT_CODE_HEALTH_SANITIZE_COMPLETE,
				uid,
				true,
				core::Helper::uidToString(uid).c_str(),
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_UNKNOWN);
	}
}

bool monitor::EventMonitor::isActionRequiredForDeviceHealth(enum device_health health)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (health == DEVICE_HEALTH_CRITICAL) || (health == DEVICE_HEALTH_FATAL);
}

event_severity monitor::EventMonitor::getEventSeverityForDeviceHealth(enum device_health health)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	event_severity severity = EVENT_SEVERITY_INFO;
	switch (health)
	{
	case DEVICE_HEALTH_FATAL:
		severity = EVENT_SEVERITY_FATAL;
		break;
	case DEVICE_HEALTH_CRITICAL:
		severity = EVENT_SEVERITY_CRITICAL;
		break;
	case DEVICE_HEALTH_NORMAL:
		severity = EVENT_SEVERITY_INFO;
		break;
	case DEVICE_HEALTH_NONCRITICAL:
	case DEVICE_HEALTH_UNKNOWN:
	default:
		severity = EVENT_SEVERITY_WARN;
		break;
	}

	return severity;
}

std::string monitor::EventMonitor::deviceHealthToStr(enum device_health health)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string healthStateStr =
			TR(get_string_for_device_health_status(health));
	return healthStateStr;
}
