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

#include "NvmMonitorBase.h"
#include "nvm_management.h"
#include <persistence/schema.h>
#include <string>
#include <map>
#include <vector>
#include <core/NvmLibrary.h>

#ifndef _MONITOR_EVENTMONITOR_H_
#define _MONITOR_EVENTMONITOR_H_

namespace monitor
{
	static std::string ERASURE_CODED = "erasure coded";
	static std::string CORRECTED = "corrected";
	static std::string UNCORRECTABLE = "uncorrectable";

	struct deviceInfo
	{
		bool discovered;
		struct device_discovery discovery;
		struct device_status status;
		bool stored;
		struct db_topology_state storedState;
	};

	//!< Map with a UID string key and deviceInfo Struct
	typedef std::map<std::string, struct deviceInfo> DeviceMap;

	/*!
	 * @brief Process to monitor conditions on the system and generate events for important
	 * changes.
	 */
	class EventMonitor : public NvmMonitorBase
	{
	public:
		/*!
		 * Constructor
		 */
		EventMonitor(core::NvmLibrary &lib = core::NvmLibrary::getNvmLibrary());

		virtual ~EventMonitor();

		/*!
		 * Startup and initialization logic.
		 */
		virtual void init();

		/*!
		 * Shutdown and cleanup logic.
		 */
		virtual void cleanup();

		/*!
		 * Core monitoring logic.
		 */
		virtual void monitor();

		/*
		 * Find and acknowledge an event of the specified code
		 */
		static void acknowledgeEventCodeForDevice(const int eventCode, const NVM_UID deviceUid);


	private:
		int m_nsMgmtCallbackId; // callback identifer for delete namespace events
		core::NvmLibrary &m_lib;

		/*
		 * Process "start of day" events - conditions to be detected on process start-up.
		 */
		void startOfDay();
		void runPlatformConfigDiagnostic();
		void checkDeviceStartUpStatus();

		DeviceMap getCurrentDeviceMapWithSavedTopology();
		DeviceMap getCurrentDeviceMap();
		void addCurrentDevicesToDeviceMap(DeviceMap& map);
		deviceInfo getTopologyInfoForDevice(const struct device_discovery &device);
		bool isSavedTopologyStateValid();
		void addSavedTopologyStateToDeviceMap(DeviceMap& map);
		std::vector<struct db_topology_state> getSavedTopologyState();

		void checkDeviceTopologyForChanges(const DeviceMap &devices);
		void processTopologyNewDimms(const DeviceMap &devices,
				std::vector<std::string> &replacedHandles);
		std::string getReplacedDimmUid(const DeviceMap &devices, const NVM_UINT32 &handle);
		void processTopologyModifiedDimms(const DeviceMap &devices,
				const std::vector<std::string> &replacedHandles);
		void saveCurrentTopologyState(const DeviceMap &devices);

		void checkConfigStatusForAllDevices(DeviceMap &devices);
		void checkConfigGoalStatus(const std::string &uidStr, const deviceInfo &device);
		config_goal_status getConfigGoalStatusForDevice(const std::string &uid);
		void createEventForConfigGoalAppliedOnDevice(const std::string &uid);
		void acknowledgePlatformConfigEventsForDevice(const std::string &uid);
		void acknowledgeEventTypeForDevice(const event_type type, const std::string &uid);

		/*
		 * Look for deleted namespaces and auto-acknowledge action required events
		 */
		void acknowledgeDeletedNamespaces();
		bool namespaceDeleted(const NVM_UID nsUid,
				const std::vector<std::string> &nsUids);

		/*
		 * Monitor dimm health status transitions
		 */
		void monitorDimmStatus(const std::string &uidStr,
				const struct device_discovery &discovery,
				struct db_dimm_state &storedState,
				bool &storedStateChanged,
				bool firstState);

		/*
		 * Monitor dimm sensors
		 */
		void monitorDimmSensors(const std::string &uidStr,
				const struct device_discovery &discovery,
				struct db_dimm_state &storedState,
				bool &storedStateChanged,
				bool firstState);

		/*
		 * Monitor Dimm Media Temperature
		 */
		void monitorDimmMediaTemperature(const std::string &uidStr,
				const struct device_discovery &discovery,
				struct db_dimm_state &storedState,
				bool &storedStateChanged,
				struct sensor &sensor);

		/*
		 * Monitor Dimm Controller Temperature
		 */
		void monitorDimmControllerTemperature(const std::string &uidStr,
				const struct device_discovery &discovery,
				struct db_dimm_state &storedState,
				bool &storedStateChanged,
				struct sensor &sensor);

		/*
		 * Monitor Dimm Spare Capacity
		 */
		void monitorDimmSpare(const std::string &uidStr,
				const struct device_discovery &discovery,
				struct db_dimm_state &storedState,
				bool &storedStateChanged,
				struct sensor &sensor);

		/*
		 * Monitor Dimm Wear Level
		 */
		void monitorDimmWearLevel(const std::string &uidStr,
				const struct device_discovery &discovery,
				struct db_dimm_state &storedState,
				bool &storedStateChanged,
				struct sensor &sensor);

		/*
		 * Monitor Dimm Error Counts
		 */
		void monitorDimmErrors(const std::string &uidStr,
				const struct device_discovery &discovery,
				NVM_UINT64 &stored,
				const NVM_UINT64 &current,
				const std::string &errorType,
				bool &storedStateChanged);

		/*
		 * Convert namespace health to string
		 */
		std::string namespaceHealthToStr(enum namespace_health health);

		/*
		 * Monitor Namespace health events
		 */
		void monitorNamespaces(PersistentStore *p_Store);

		/*
		 * Device health to string helper
		 */
		std::string deviceHealthToStr(enum device_health health);

		/*
		 * Helper to store new fw error log event
		 */
		void storeFwErrorLogEvent(const NVM_UID &device_uid, const std::string &uidStr,
				const NVM_UINT32 errorCount);
	};
}
#endif /* _MONITOR_EVENTMONITOR_H_ */
