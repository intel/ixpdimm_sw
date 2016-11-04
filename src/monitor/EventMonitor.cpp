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

	nvm_free_context();
}

void monitor::EventMonitor::runPlatformConfigDiagnostic()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	diagnostic platformConfigDiag;
	memset(&platformConfigDiag, 0, sizeof (platformConfigDiag));
	platformConfigDiag.test = DIAG_TYPE_PLATFORM_CONFIG;

	try
	{
		NVM_UINT32 results = 0;
		m_lib.runDiagnostic("", platformConfigDiag, results);
	}
	catch (core::LibraryException &e)
	{
		COMMON_LOG_ERROR_F("Platform config diagnostic returned error %d",
				e.getErrorCode());
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
			struct db_topology_state dbTopoState[topoStateCount];
			if (db_get_topology_states(pStore, dbTopoState, topoStateCount)
					== topoStateCount)
			{
				for (int i = 0; i < topoStateCount; i++)
				{
					topologyState.push_back(dbTopoState[i]);
				}
			}
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
							true);
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
							true);
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
					memmove(topoState.model_num, device.discovery.model_number, NVM_MODEL_LEN);

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
		memmove(filter.uid, uid, NVM_MAX_UID_LEN);
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
 * Helper to convert device health to translated string
 */
std::string monitor::EventMonitor::deviceHealthToStr(enum device_health health)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string healthStateStr =
			TR(get_string_for_device_health_status(health));
	return healthStateStr;
}

/*
 * Helper to store new fw error log event
 */
void monitor::EventMonitor::storeFwErrorLogEvent(const NVM_UID &device_uid,
		const std::string &uidStr, const NVM_UINT32 errorCount)
{
	std::stringstream newErrorCount;
	newErrorCount << errorCount;
	store_event_by_parts(EVENT_TYPE_HEALTH,
			EVENT_SEVERITY_WARN,
			EVENT_CODE_HEALTH_NEW_FWERRORS_FOUND,
			device_uid,
			false,
			uidStr.c_str(),
			newErrorCount.str().c_str(),
			NULL,
			DIAGNOSTIC_RESULT_UNKNOWN);
}

/*
 * Check for device health changes
 */
void monitor::EventMonitor::monitorDimmStatus(const std::string &uidStr,
		const struct device_discovery &discovery,
		struct db_dimm_state &storedState,
		bool &storedStateChanged,
		bool firstState)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	try
	{
		struct device_status status = m_lib.getDeviceStatus(uidStr);

		// first pass, just store current values
		// if there are errors that have not been seen in the fw error log
		// create an event for those errors
		if (firstState)
		{
			storedState.health_state = status.health;
			storedState.die_spares_used = status.die_spares_used;
			storedState.viral_state = status.viral_state;
			storedState.newest_error_log_timestamp = status.newest_error_log_timestamp;
			if (status.new_error_count > 0)
			{
				storeFwErrorLogEvent(discovery.uid, uidStr, status.new_error_count);
			}
		}
		// check for changes
		else
		{
			// check dimm health
			if (status.health != storedState.health_state)
			{
				// log transition event
				enum event_severity severity = EVENT_SEVERITY_INFO;
				bool actionRequired = false;
				if (status.health == DEVICE_HEALTH_NONCRITICAL)
				{
					severity = EVENT_SEVERITY_WARN;
				}
				else if (status.health == DEVICE_HEALTH_CRITICAL)
				{
					severity = EVENT_SEVERITY_CRITICAL;
					actionRequired = true;
				}
				else if (status.health >= DEVICE_HEALTH_FATAL)
				{
					severity = EVENT_SEVERITY_FATAL;
					actionRequired = true;
				}

				std::string oldState = deviceHealthToStr(
						(enum device_health)storedState.health_state);
				std::string newState = deviceHealthToStr(status.health);
				store_event_by_parts(
						EVENT_TYPE_HEALTH,
						severity,
						EVENT_CODE_HEALTH_HEALTH_STATE_CHANGED,
						discovery.uid,
						actionRequired,
						uidStr.c_str(),
						oldState.c_str(),
						newState.c_str(),
						DIAGNOSTIC_RESULT_UNKNOWN);

				storedState.health_state = status.health;
				storedStateChanged = true;
			}

			// check additional die consumed
			if (status.die_spares_used > storedState.die_spares_used)
			{
				std::stringstream numDieConsumed;
				numDieConsumed << status.die_spares_used;
				store_event_by_parts(EVENT_TYPE_HEALTH,
						EVENT_SEVERITY_WARN,
						EVENT_CODE_HEALTH_SPARE_DIE_CONSUMED,
						discovery.uid,
						false,
						uidStr.c_str(),
						numDieConsumed.str().c_str(),
						NULL,
						DIAGNOSTIC_RESULT_UNKNOWN);

				storedState.die_spares_used = status.die_spares_used;
				storedStateChanged = true;
			}

			if (status.viral_state != storedState.viral_state)
			{
				enum event_code_health code = EVENT_CODE_HEALTH_VIRAL_STATE;
				if (status.viral_state)
				{
					store_event_by_parts(EVENT_TYPE_HEALTH,
							EVENT_SEVERITY_CRITICAL,
							code,
							discovery.uid,
							true,
							uidStr.c_str(),
							NULL,
							NULL,
							DIAGNOSTIC_RESULT_UNKNOWN);
				}
				else
				{
					acknowledgeEventCodeForDevice(code, discovery.uid);
				}

				// update stored state
				storedState.viral_state = status.viral_state;
				storedStateChanged = true;
			}

			if (status.newest_error_log_timestamp > storedState.newest_error_log_timestamp)
			{
				storeFwErrorLogEvent(discovery.uid, uidStr, status.new_error_count);
				storedState.newest_error_log_timestamp = status.newest_error_log_timestamp;
				storedStateChanged = true;
			}
		}
	}
	catch (core::LibraryException &e)
	{
		COMMON_LOG_ERROR_F("Unable to get device status for dimm %s - error %d\n",
				uidStr.c_str(), e.getErrorCode());
	}
}

/*
 * Check for device media temperature changes
 */
void monitor::EventMonitor::monitorDimmMediaTemperature(const std::string &uidStr,
		const struct device_discovery &discovery,
		struct db_dimm_state &storedState,
		bool &storedStateChanged,
		struct sensor &sensor)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// temp has changed state
	if (sensor.current_state != storedState.mediatemperature_state)
	{
		// log an event
		enum event_code_health code = EVENT_CODE_HEALTH_MEDIA_TEMPERATURE_UNDER_THRESHOLD;
		enum event_severity severity = EVENT_SEVERITY_INFO;
		bool actionRequired = false;
		if (sensor.current_state == SENSOR_NONCRITICAL)
		{
			code = EVENT_CODE_HEALTH_MEDIA_TEMPERATURE_OVER_THRESHOLD;
			severity = EVENT_SEVERITY_WARN;
			actionRequired = true;
		}
		// auto-acknowledge any existing temperature over threshold events
		else if (sensor.current_state == SENSOR_NORMAL)
		{
			acknowledgeEventCodeForDevice(EVENT_CODE_HEALTH_MEDIA_TEMPERATURE_OVER_THRESHOLD, discovery.uid);
		}

		std::stringstream threshold, temperature;
		temperature << nvm_decode_temperature(sensor.reading);
		threshold << nvm_decode_temperature(sensor.settings.upper_critical_threshold);
		store_event_by_parts(EVENT_TYPE_HEALTH,
				severity,
				code,
				discovery.uid,
				actionRequired,
				uidStr.c_str(),
				temperature.str().c_str(),
				threshold.str().c_str(),
				DIAGNOSTIC_RESULT_UNKNOWN);

		// update stored state
		storedStateChanged = true;
		storedState.mediatemperature_state = sensor.current_state;
	}
}

/*
 * Check for device media temperature changes
 */
void monitor::EventMonitor::monitorDimmControllerTemperature(const std::string &uidStr,
		const struct device_discovery &discovery,
		struct db_dimm_state &storedState,
		bool &storedStateChanged,
		struct sensor &sensor)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// temp has changed state
	if (sensor.current_state != storedState.controllertemperature_state)
	{
		// log an event
		enum event_code_health code = EVENT_CODE_HEALTH_CONTROLLER_TEMPERATURE_UNDER_THRESHOLD;
		enum event_severity severity = EVENT_SEVERITY_INFO;
		bool actionRequired = false;
		if (sensor.current_state == SENSOR_NONCRITICAL)
		{
			code = EVENT_CODE_HEALTH_CONTROLLER_TEMPERATURE_OVER_THRESHOLD;
			severity = EVENT_SEVERITY_WARN;
			actionRequired = true;
		}
		// auto-acknowledge any existing temperature over threshold events
		else if (sensor.current_state == SENSOR_NORMAL)
		{
			acknowledgeEventCodeForDevice(EVENT_CODE_HEALTH_CONTROLLER_TEMPERATURE_OVER_THRESHOLD, discovery.uid);
		}

		std::stringstream threshold, temperature;
		temperature << nvm_decode_temperature(sensor.reading);
		threshold << nvm_decode_temperature(sensor.settings.upper_critical_threshold);
		store_event_by_parts(EVENT_TYPE_HEALTH,
				severity,
				code,
				discovery.uid,
				actionRequired,
				uidStr.c_str(),
				temperature.str().c_str(),
				threshold.str().c_str(),
				DIAGNOSTIC_RESULT_UNKNOWN);

		// update stored state
		storedStateChanged = true;
		storedState.controllertemperature_state = sensor.current_state;
	}
}

/*
 * Check for device spare capacity changes
 */
void monitor::EventMonitor::monitorDimmSpare(const std::string &uidStr,
		const struct device_discovery &discovery,
		struct db_dimm_state &storedState,
		bool &storedStateChanged,
		struct sensor &sensor)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// spare capacity has changed state
	if (sensor.current_state != storedState.spare_capacity_state)
	{
		if (sensor.current_state == SENSOR_NONCRITICAL)
		{
			// log an event
			std::stringstream percentUsed, threshold;
			percentUsed << sensor.reading;
			threshold << sensor.settings.lower_critical_threshold;

			store_event_by_parts(EVENT_TYPE_HEALTH,
					EVENT_SEVERITY_WARN,
					EVENT_CODE_HEALTH_LOW_SPARE_CAPACITY,
					discovery.uid,
					true,
					uidStr.c_str(),
					percentUsed.str().c_str(),
					threshold.str().c_str(),
					DIAGNOSTIC_RESULT_UNKNOWN);
		}
		// auto-acknowledge any existing spare below threshold events
		else if (sensor.current_state == SENSOR_NORMAL)
		{
			acknowledgeEventCodeForDevice(EVENT_CODE_HEALTH_LOW_SPARE_CAPACITY, discovery.uid);
		}

		// update stored state
		storedStateChanged = true;
		storedState.spare_capacity_state = sensor.current_state;
	}
}

/*
 * Check for device wear level changes changes
 */
void monitor::EventMonitor::monitorDimmWearLevel(const std::string &uidStr,
		const struct device_discovery &discovery,
		struct db_dimm_state &storedState,
		bool &storedStateChanged,
		struct sensor &sensor)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// spare capacity has changed state
	if (sensor.current_state != storedState.wearlevel_state)
	{
		if (sensor.current_state == SENSOR_CRITICAL)
		{
			// log an event
			std::stringstream wearLevel, wearLevelThreshold;
			wearLevel << sensor.reading;
			int percent_used_threshold = 0;
			get_config_value_int(SQL_KEY_PERCENT_USED_THRESHOLD, &percent_used_threshold);
			wearLevelThreshold << percent_used_threshold;

			store_event_by_parts(EVENT_TYPE_HEALTH,
					EVENT_SEVERITY_WARN,
					EVENT_CODE_HEALTH_HIGH_WEARLEVEL,
					discovery.uid,
					true,
					uidStr.c_str(),
					wearLevel.str().c_str(),
					wearLevelThreshold.str().c_str(),
					DIAGNOSTIC_RESULT_UNKNOWN);
		}
		// NOTE: wear level will never go from critical to normal so
		// no need to auto-acknowledge these events on monitor.

		// update stored state
		storedStateChanged = true;
		storedState.wearlevel_state = sensor.current_state;
	}
}

/*
 * Check for dimm errors
 */
void monitor::EventMonitor::monitorDimmErrors(const std::string &uidStr,
		const struct device_discovery &discovery,
		NVM_UINT64 &stored,
		const NVM_UINT64 &current,
		const std::string &errorType,
		bool &storedStateChanged)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// new errors
	if (current > stored)
	{
		// log an event
		std::stringstream errors;
		errors << current;

		store_event_by_parts(EVENT_TYPE_HEALTH,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_HEALTH_NEW_MEDIAERRORS_FOUND,
				discovery.uid,
				false,
				uidStr.c_str(),
				errorType.c_str(),
				errors.str().c_str(),
				DIAGNOSTIC_RESULT_UNKNOWN);

		// update stored state
		storedStateChanged = true;
		stored = current;
	}
}


/*
 * Check for device sensor changes
 */
void monitor::EventMonitor::monitorDimmSensors(const std::string &uidStr,
		const struct device_discovery &discovery,
		struct db_dimm_state &storedState,
		bool &storedStateChanged,
		bool firstState)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	try
	{
		std::vector<struct sensor> sensors = m_lib.getSensors(uidStr);

		// first pass, just store current values
		if (firstState)
		{
			storedState.mediaerrors_corrected =
					sensors[SENSOR_MEDIAERRORS_CORRECTED].reading;
			storedState.mediaerrors_erasurecoded =
					sensors[SENSOR_MEDIAERRORS_ERASURECODED].reading;
			storedState.mediaerrors_uncorrectable =
					sensors[SENSOR_MEDIAERRORS_UNCORRECTABLE].reading;
			storedState.wearlevel_state =
					sensors[SENSOR_WEARLEVEL].current_state;
			storedState.mediatemperature_state =
					sensors[SENSOR_MEDIA_TEMPERATURE].current_state;
			storedState.controllertemperature_state =
					sensors[SENSOR_CONTROLLER_TEMPERATURE].current_state;
			storedState.spare_capacity_state =
					sensors[SENSOR_SPARECAPACITY].current_state;

		}
		// check for changes
		else
		{
			// monitor media temperature
			monitorDimmMediaTemperature(uidStr, discovery, storedState,
					storedStateChanged, sensors[SENSOR_MEDIA_TEMPERATURE]);

			// monitor controller temperature
			monitorDimmControllerTemperature(uidStr, discovery, storedState,
					storedStateChanged, sensors[SENSOR_CONTROLLER_TEMPERATURE]);

			// monitor spare capacity
			monitorDimmSpare(uidStr, discovery, storedState,
					storedStateChanged, sensors[SENSOR_SPARECAPACITY]);

			// monitor wear level
			monitorDimmWearLevel(uidStr, discovery, storedState,
					storedStateChanged, sensors[SENSOR_WEARLEVEL]);

			// monitor errors - uncorrectable
			monitorDimmErrors(uidStr, discovery, storedState.mediaerrors_uncorrectable,
					sensors[SENSOR_MEDIAERRORS_UNCORRECTABLE].reading,
					UNCORRECTABLE, storedStateChanged);

			// monitor errors - corrected
			monitorDimmErrors(uidStr, discovery, storedState.mediaerrors_corrected,
					sensors[SENSOR_MEDIAERRORS_CORRECTED].reading,
					CORRECTED, storedStateChanged);

			// monitor errors - erasure coded
			monitorDimmErrors(uidStr, discovery, storedState.mediaerrors_erasurecoded,
					sensors[SENSOR_MEDIAERRORS_ERASURECODED].reading,
					ERASURE_CODED, storedStateChanged);
		}
	}
	catch (core::LibraryException &e)
	{
		COMMON_LOG_ERROR_F("Unable to get sensors for dimm %s - error %d\n",
				uidStr.c_str(), e.getErrorCode());
	}
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
	PersistentStore *pStore = get_lib_store();
	if (pStore)
	{
		LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

		// clean up any context
		nvm_create_context();

		DeviceMap devMap = getCurrentDeviceMap();
		for (DeviceMap::const_iterator devIter = devMap.begin();
				devIter != devMap.end(); devIter++)
		{
			const std::string &uidStr = devIter->first;
			const struct device_discovery &discovery = devIter->second.discovery;

			// ignore unmanageable dimms
			if (discovery.manageability == MANAGEMENT_VALIDCONFIG)
			{
				bool storedStateChanged = false;
				bool firstState = false;

				// get stored device state
				struct db_dimm_state storedState;
				memset(&storedState, 0, sizeof (storedState));
				if (db_get_dimm_state_by_device_handle(pStore,
						discovery.device_handle.handle, &storedState) != DB_SUCCESS)
				{
					// initial state, just store current
					firstState = true;
					COMMON_LOG_INFO_F("Failed to retrieve the stored health state of dimm %u",
							discovery.device_handle.handle);
					storedState.device_handle = discovery.device_handle.handle;
					storedStateChanged = true;
				}

				// check for dimm health state transition
				monitorDimmStatus(uidStr, discovery, storedState,
						storedStateChanged, firstState);

				// check for dimm sensor transitions
				monitorDimmSensors(uidStr, discovery, storedState,
						storedStateChanged, firstState);

				// update stored dimm state
				if (storedStateChanged)
				{
					// clear existing dimm state
					if (!firstState)
					{
						db_delete_dimm_state_by_device_handle(pStore,
								discovery.device_handle.handle);
					}
					// add current state
					if (db_add_dimm_state(pStore, &storedState) != DB_SUCCESS)
					{
						COMMON_LOG_ERROR_F("Failed to store the health state of dimm %u",
								discovery.device_handle.handle);
					}
				}
			}
		}

		// Monitor namespace health transitions
		monitorNamespaces(pStore);

		// clean up
		devMap.clear();
		nvm_free_context();
	}

}
