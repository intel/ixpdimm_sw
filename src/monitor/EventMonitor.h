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
		EventMonitor();

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
		static void acknowledgeEvent(const int eventCode, const NVM_UID deviceUid);


	private:
		/*
		 * Process "start of day" events - conditions to be detected on process start-up.
		 */
		void startOfDay();

		void checkDriver();

		/*
		 * Inspect the status for all devices and generate events.
		 */
		void processDeviceStartupStatus(DeviceMap &devices);

		void checkDeviceManageability(const std::string &uidStr, const deviceInfo &device);

		void checkConfigGoalStatus(const std::string &uidStr, const deviceInfo &device) const;

		void checkShutdownStatus(const std::string &uidStr, const deviceInfo &device) const;

		void checkSkuViolation(const std::string &uidStr, const deviceInfo &device) const;

		/*
		 * Helper to determine if the given dimm handle is a replacement of a missing dimm
		 * If it is, return the dimm uid, if not return empty string.
		 */
		std::string getReplacedDimmUid(const DeviceMap &devices, const NVM_UINT32 &handle);

		/*
		 * Helper function to look at previous topology state and detect newly-added/replaced DIMMS.
		 * @param[in] devices - current DIMM list as a hash map
		 * @param[out] replacedHandles - returns the list of handles whose previous DIMM has been replaced
		 */
		void processTopologyNewDimms(const DeviceMap &devices,
				std::vector<std::string> &replacedHandles);

		/*
		 * Helper function to look at previous topology state and detect DIMMs moved or removed.
		 * @param[in] devices - current DIMM list as a hash map
		 * @param[in] replacedHandles - returns the list of handles whose previous DIMM has been replaced
		 * @remark replacedHandles is to ensure we don't report a replaced DIMM as missing
		 */
		void processTopologyModifiedDimms(const DeviceMap &devices,
				const std::vector<std::string> &replacedHandles);

		/*
		 * Preserve the current topology state for future comparison.
		 * @param devices - current devices
		 */
		void saveCurrentTopologyState(const DeviceMap &devices);

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
		 * Build a map of uids to discovery information for all NVM-DIMM in the system
		 */
		void buildDeviceMap(monitor::DeviceMap& map, bool addStoredTopology = false);

		/*
		 * Device health to string helper
		 */
		std::string deviceHealthToStr(enum device_health health);

		/*
		 * Helper to store new fw error log event
		 */
		void storeFwErrorLogEvent(const NVM_UID &device_uid, const std::string &uidStr,
				const NVM_UINT32 errorCount);

		/*
		 * On start-up look for deleted namespaces and auto-acknowledge action required events
		 */
		void acknowledgeDeletedNamespaces();

		/*
		 * Helper to determine if a namespace has been deleted
		 */
		bool namespaceDeleted(const NVM_UID nsUid,
				const std::vector<std::string> &nsUids);

		/*
		 * On start-up log an event for mixed SKUs.
		 */
		void processMixedSkuSystem();

		// callback identifer for delete namespace events
		int m_nsMgmtCallbackId;
	};
}
#endif /* _MONITOR_EVENTMONITOR_H_ */
