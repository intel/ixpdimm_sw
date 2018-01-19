/*
 * Copyright (c) 2018, Intel Corporation
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
 * This file contains the definition of the performance monitoring class
 * of the NvmMonitor service which periodically polls and stores performance metrics
 * for each manageable NVM-DIMM in the system.
 */

#include "NvmMonitorBase.h"
#include "nvm_management.h"
#include <persistence/schema.h>
#include <string>
#include <map>
#include <vector>
#include <core/NvmLibrary.h>
#include <fis_types.h>

#ifndef _MONITOR_ACPIEVENTMONITOR_H_
#define _MONITOR_ACPIEVENTMONITOR_H_
#define ACPI_MONITOR_LOG_SRC "Ixpdimm-monitor ACPI monitor thread"
#define ACPI_MONITOR_INIT_MSG "ACPI MONITOR INIT\n"
#define ACPI_MONITOR_INIT_EXCEPTION_PRE_MSG "Issue during ACPI Monitor init, error: "
#define ACPI_MONITOR_GEN_EVENTS_EXCEPTION_PRE_MSG "Error getting FW error log entry, error: "
#define ACPI_MONITOR_GEN_NVM_EVENTS_EXCEPTION_PRE_MSG "Error generating Nvm events, error: "
#define ACPI_MONITOR_EXCEPTION_PRE_MSG "Couldn't get devices - error: "
#define ACPI_WAIT_FOR_API_TIMED_OUT_MSG	"Timed out waiting for an ACPI event\n"
#define ACPI_WAIT_FOR_API_UNKNOWN_MSG "Unknown error occured while waiting for an ACPI event\n"
#define ACPI_CREATE_CTX_GENERAL_ERROR_MSG "Error creating acpi context\n"
#define ACPI_SMART_HEALTH_EVENT_THERM_LOW_HEADER "ThermLow"
#define ACPI_SMART_HEALTH_EVENT_THERM_HIGH_HEADER "ThermHigh"
#define ACPI_SMART_HEALTH_EVENT_MEDIA_LOW_HEADER "MediaLow"
#define ACPI_SMART_HEALTH_EVENT_MEDIA_HIGH_HEADER "MediaHigh"
#define ACPI_SMART_HEALTH_EVENT_TS_HEADER "TS: 0x"
#define ACPI_SMART_HEALTH_EVENT_TEMP_HEADER " TEMP: "
#define ACPI_SMART_HEALTH_EVENT_SEQ_NUM_HEADER " SEQ NUM: "
#define ACPI_SMART_HEALTH_EVENT_DPA_HEADER " DPA: 0x"
#define ACPI_SMART_HEALTH_EVENT_PDA_HEADER " PDA: 0x"
#define ACPI_SMART_HEALTH_EVENT_RANGE_HEADER " RANGE: 0x"
#define ACPI_SMART_HEALTH_EVENT_ERROR_TYPE_HEADER " ERROR TYPE: 0x"
#define ACPI_SMART_HEALTH_EVENT_ERROR_FLAGS_HEADER " ERROR FLAGS: 0x"
#define ACPI_SMART_HEALTH_EVENT_TRANS_TYPE_HEADER " TRANS TYPE: 0x"
#define ACPI_SMART_HEALTH_EVENT_TEMP_CORE_STR "core temp : "
#define ACPI_SMART_HEALTH_EVENT_TEMP_MEDIA_STR "media temp : "
#define ACPI_SMART_HEALTH_EVENT_USER_ALARM_TRIP_STR "user alarm trip : "
#define ACPI_SMART_HEALTH_EVENT_TEMP_LOW_STR "low : "
#define ACPI_SMART_HEALTH_EVENT_TEMP_HIGH_STR "high : "
#define ACPI_SMART_HEALTH_EVENT_TEMP_CRITICAL_STR "critical : "
#define ACPI_SMART_HEALTH_EVENT_TEMP_NEG_STR "- : "
#define ACPI_SMART_HEALTH_EVENT_TEMP_RAW_STR "raw value: "
#define ACPI_WAIT_FOR_TIMEOUT_SEC	30

namespace monitor
{
	static const std::string MONITOR_NAME = "ACPI-EVENTS";

	class AcpiMonitor : public NvmMonitorBase
	{
		public:
			AcpiMonitor(core::NvmLibrary &lib = core::NvmLibrary::getNvmLibrary());
			virtual ~AcpiMonitor();
			virtual void monitor();
			virtual void init(SYSTEM_LOGGER logger);
			virtual void setAcpiInterface(MonitorAcpiInterface intf);
		private:
			core::NvmLibrary &m_lib;
			std::vector<device_details> last_dev_details;
			void processNvmEvents(const NVM_NFIT_DEVICE_HANDLE device_handle);
			int processNewEvents(NVM_UID uid,
								unsigned char log_type,
								unsigned char log_level,
								struct fw_error_log_sequence_numbers last_numbers,
								struct fw_error_log_sequence_numbers cur_numbers);
			void generateSystemEventEntry(NVM_UID uid, unsigned char log_type, unsigned char log_level, void * log_entry);
			std::string formatThermalSystemEventEntryDescription(NVM_UID uid, void * log_entry);
			std::string formatMediaSystemEventEntryDescription(NVM_UID uid, void * log_entry);
			void sendFwErrCntSystemEventEntry(const NVM_UID device_uid, const NVM_UINT64 error_count);
			void sendFwErrLogSystemEventEntry(const NVM_UID device_uid, std::string log_type_level, std::string log_details);
			std::string createThermStr(SMART_TEMP temp_data);
			std::string m_event_log_src;
			MonitorAcpiInterface m_mon_acpi_interface;
			void **acpi_contexts;
	};
}

#endif /* _MONITOR_ACPIEVENTMONITOR_H_ */
