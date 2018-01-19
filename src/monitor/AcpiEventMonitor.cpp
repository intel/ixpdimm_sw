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
 * This file contains the implementation of the performance monitoring class
 * of the NvmMonitor service which periodically polls and stores performance metrics
 * for each manageable NVM-DIMM in the system.
 */

#include "AcpiEventMonitor.h"
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
#include <device_adapter.h>
#include <iostream>
#include <sstream>


monitor::AcpiMonitor::AcpiMonitor(core::NvmLibrary &lib) :
	NvmMonitorBase(MONITOR_NAME),
	m_lib(lib)
{
	m_event_log_src = std::string(ACPI_MONITOR_LOG_SRC);
	//registering real ACPI event APIs for now...
	//these can be overriden for unit testing purposes
	m_mon_acpi_interface.create_ctx = acpi_event_create_ctx;
	m_mon_acpi_interface.free_ctx = acpi_event_free_ctx;
	m_mon_acpi_interface.wait_for_event = acpi_wait_for_event;
	m_mon_acpi_interface.get_dimm_handle = acpi_event_ctx_get_dimm_handle;
	m_mon_acpi_interface.get_event_state = acpi_event_get_event_state;
	m_mon_acpi_interface.get_monitor_mask = acpi_event_get_monitor_mask;
	m_mon_acpi_interface.set_monitor_mask = acpi_event_set_monitor_mask;
	m_mon_acpi_interface.send_event = store_event_by_parts;
	//minimal delay in monitor execution
	m_intervalSeconds = 1;
}

monitor::AcpiMonitor::~AcpiMonitor()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

/*
* Called once on daemon startup.  Captures the health state
* of each dimm, which includes a snapshot of the error log.
* Error log entries found during init will not trigger future health events.
*
* @param[in] logger - logging interface.
*/
void monitor::AcpiMonitor::init(SYSTEM_LOGGER logger)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	try
	{
		m_logger = logger;
		std::vector<device_discovery> devList = m_lib.getDevices();
		unsigned int dev_cnt = devList.size();
		for (size_t i = 0; i < dev_cnt; i++)
		{
			struct device_details details = m_lib.getDeviceDetails(devList[i].uid);
			last_dev_details.push_back(details);
		}

		int rc;
		acpi_contexts = new void*[dev_cnt];
		for (size_t i = 0; i < dev_cnt; i++)
		{
			if (NVM_SUCCESS != (rc = m_mon_acpi_interface.create_ctx(last_dev_details[i].discovery.device_handle, &acpi_contexts[i])))
			{
				m_logger(SYSTEM_EVENT_TYPE_ERROR, m_event_log_src, ACPI_CREATE_CTX_GENERAL_ERROR_MSG);
				if (acpi_contexts)
					free(acpi_contexts);
				return;
			}
			else
			{
				m_mon_acpi_interface.set_monitor_mask(acpi_contexts[i], DIMM_ACPI_EVENT_SMART_HEALTH_MASK);
			}
		}
		m_logger(SYSTEM_EVENT_TYPE_INFO, m_event_log_src, ACPI_MONITOR_INIT_MSG);
	}
	catch (core::LibraryException &e)
	{
		std::stringstream err;
		err << ACPI_MONITOR_INIT_EXCEPTION_PRE_MSG << e.getErrorCode();
		m_logger(SYSTEM_EVENT_TYPE_ERROR, m_event_log_src, err.str());
	}
}

/*
* Override internal ACPI event monitoring interface.  Typical use is for
* unit testing.
*
* @param[in] intf - ACPI event monitoring interface
*/
void monitor::AcpiMonitor::setAcpiInterface(MonitorAcpiInterface intf)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	m_mon_acpi_interface = intf;
}

/*
* The main entry point for monitoring asynchronous smart health
* ACPI notifications.
*/
void monitor::AcpiMonitor::monitor()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	try
	{
		unsigned int dev_cnt = last_dev_details.size();
		enum acpi_get_event_result result;
		m_mon_acpi_interface.wait_for_event(acpi_contexts, dev_cnt, ACPI_WAIT_FOR_TIMEOUT_SEC, &result);
		switch (result)
		{
		case ACPI_EVENT_SIGNALLED_RESULT:
			for (unsigned int i = 0; i < dev_cnt; ++i)
			{
				enum acpi_event_state r;
				m_mon_acpi_interface.get_event_state(acpi_contexts[i], ACPI_SMART_HEALTH, &r);
				if (ACPI_EVENT_SIGNALLED == r)
				{
					NVM_NFIT_DEVICE_HANDLE dimm;
					m_mon_acpi_interface.get_dimm_handle(acpi_contexts[i], &dimm);
					//received an async ACPI event notification
					//now figure out what happened and generate system level events and messages
					processNvmEvents(dimm);
				}
			}
			break;
		case ACPI_EVENT_TIMED_OUT_RESULT:
			//no log msg, happens frequently
			break;
		case ACPI_EVENT_UNKNOWN_RESULT:
			m_logger(SYSTEM_EVENT_TYPE_INFO, m_event_log_src, ACPI_WAIT_FOR_API_UNKNOWN_MSG);
			break;
		}
	}
	catch (core::LibraryException &e)
	{
		std::stringstream err;
		err << ACPI_MONITOR_EXCEPTION_PRE_MSG << e.getErrorCode();
		m_logger(SYSTEM_EVENT_TYPE_ERROR, m_event_log_src, err.str());
		return;
	}
}

/*
* Start the process of dispositioning and generating system level events
* based on information gathered from the target device.
*
* @param[in] device_handle - target device in question
*/
void  monitor::AcpiMonitor::processNvmEvents(const NVM_NFIT_DEVICE_HANDLE device_handle)
{
	try
	{
		int total_events = 0;
		unsigned int dev_cnt = last_dev_details.size();
		for (size_t i = 0; i < dev_cnt; i++)
		{
			if (last_dev_details[i].discovery.device_handle.handle == device_handle.handle)
			{
				struct device_details details = m_lib.getDeviceDetails(last_dev_details[i].discovery.uid);
				total_events = processNewEvents(last_dev_details[i].discovery.uid,
					DEV_FW_ERR_LOG_THERMAL,
					DEV_FW_ERR_LOG_LOW,
					last_dev_details[i].status.therm_low,
					details.status.therm_low);

				total_events += processNewEvents(last_dev_details[i].discovery.uid,
					DEV_FW_ERR_LOG_THERMAL,
					DEV_FW_ERR_LOG_HIGH,
					last_dev_details[i].status.therm_high,
					details.status.therm_high);

				total_events += processNewEvents(last_dev_details[i].discovery.uid,
					DEV_FW_ERR_LOG_MEDIA,
					DEV_FW_ERR_LOG_LOW,
					last_dev_details[i].status.media_low,
					details.status.media_low);

				total_events += processNewEvents(last_dev_details[i].discovery.uid,
					DEV_FW_ERR_LOG_MEDIA,
					DEV_FW_ERR_LOG_HIGH,
					last_dev_details[i].status.media_high,
					details.status.media_high);

				sendFwErrCntSystemEventEntry(last_dev_details[i].discovery.uid, total_events);
				last_dev_details[i] = details;
				return;
			}
		}
	}
	catch (core::LibraryException &e)
	{
		std::stringstream err;
		err << ACPI_MONITOR_GEN_NVM_EVENTS_EXCEPTION_PRE_MSG << e.getErrorCode();
		m_logger(SYSTEM_EVENT_TYPE_ERROR, m_event_log_src, err.str());
	}
}

/*
* Generate a new system event for any new fw error log entries.
* A system event includes Windows event, or msg in syslog depending on
* underlying OS.  This will also add entries into the CR MGMT DB.
*
* @param[in] uid - dimm that generated the event
* @param[in] log_type - DEV_FW_ERR_LOG_MEDIA or DEV_FW_ERR_LOG_THERM
* @param[in] log_level - DEV_FW_ERR_LOG_LOW or DEV_FW_ERR_LOG_HIGH
* @param[in] last_numbers - the fw log sequence numbers obtained by either monitor init or
*							the last time an event of this type happened.
* @param[in] cur_numbers - the current fw log sequence numbers (see FIS for details)
*/
int monitor::AcpiMonitor::processNewEvents(NVM_UID uid,
	unsigned char log_type,
	unsigned char log_level,
	struct fw_error_log_sequence_numbers last_numbers,
	struct fw_error_log_sequence_numbers cur_numbers)
{
	unsigned char buffer[DEV_SMALL_PAYLOAD_SIZE];
	int new_log_cnt = 0;

	for (int index = cur_numbers.oldest; index <= cur_numbers.current; index++)
	{
		if (!(index >= last_numbers.oldest &&
			index <= last_numbers.current))
		{
			int rc;
			//get the log via the seq number and craft an appropriate event.
			if (NVM_SUCCESS == (rc = m_lib.getFwErrLogEntry(uid,
				index, log_level, log_type, buffer, DEV_SMALL_PAYLOAD_SIZE)))
			{
				generateSystemEventEntry(uid, log_type, log_level, (void *)buffer);
				new_log_cnt++;
			}
			else
			{
				std::stringstream err;
				err << ACPI_MONITOR_GEN_EVENTS_EXCEPTION_PRE_MSG << rc;
				m_logger(SYSTEM_EVENT_TYPE_ERROR, m_event_log_src, err.str());
			}
		}
	}
	return new_log_cnt;
}

/*
* Start the process of dispositioning and generating system level events
* based on information gathered from the target device.
*
* @param[in] uid - target device in question
* @param[in] log_type - DEV_FW_ERR_LOG_THERMAL or DEV_FW_ERR_LOG_MEDIA
* @param[in] log_level - DEV_FW_ERR_LOG_LOW or DEV_FW_ERR_LOG_HIGH
* @param[in] log_entry - raw log entry from the device FW.
*/
void monitor::AcpiMonitor::generateSystemEventEntry(NVM_UID uid, unsigned char log_type, unsigned char log_level, void * log_entry)
{
	std::string description;
	std::string header;

	if (DEV_FW_ERR_LOG_THERMAL == log_type && DEV_FW_ERR_LOG_LOW == log_level)
	{
		header = ACPI_SMART_HEALTH_EVENT_THERM_LOW_HEADER;
		description = formatThermalSystemEventEntryDescription(uid, log_entry);
	}
	else if (DEV_FW_ERR_LOG_THERMAL == log_type && DEV_FW_ERR_LOG_HIGH == log_level)
	{
		header = ACPI_SMART_HEALTH_EVENT_THERM_HIGH_HEADER;
		description = formatThermalSystemEventEntryDescription(uid, log_entry);
	}
	else if (DEV_FW_ERR_LOG_MEDIA == log_type && DEV_FW_ERR_LOG_LOW == log_level)
	{
		header = ACPI_SMART_HEALTH_EVENT_MEDIA_LOW_HEADER;
		description = formatMediaSystemEventEntryDescription(uid, log_entry);
	}
	else if (DEV_FW_ERR_LOG_MEDIA == log_type && DEV_FW_ERR_LOG_HIGH == log_level)
	{
		header = ACPI_SMART_HEALTH_EVENT_MEDIA_HIGH_HEADER;
		description = formatMediaSystemEventEntryDescription(uid, log_entry);
	}
	sendFwErrLogSystemEventEntry(uid, header, description);
}

/*
* Make a human readable description from the raw fw thermal log entry.
*
* @param[in] uid - target device in question
* @param[in] log_entry - raw log entry from the device FW.
*/
std::string monitor::AcpiMonitor::formatThermalSystemEventEntryDescription(NVM_UID uid, void * log_entry)
{
	struct pt_fw_thermal_log_entry *p_therm_log_entry = (struct pt_fw_thermal_log_entry *)log_entry;
	std::stringstream log_details;
	log_details	<< ACPI_SMART_HEALTH_EVENT_TS_HEADER << std::hex << p_therm_log_entry->system_timestamp
				<< ACPI_SMART_HEALTH_EVENT_TEMP_HEADER << createThermStr(p_therm_log_entry->host_reported_temp_data)
				<< ACPI_SMART_HEALTH_EVENT_SEQ_NUM_HEADER << std::dec << p_therm_log_entry->seq_num;
	return log_details.str();
}

/*
* Make a human readable description from the raw fw media log entry.
*
* @param[in] uid - target device in question
* @param[in] log_entry - raw log entry from the device FW.
*/
std::string monitor::AcpiMonitor::formatMediaSystemEventEntryDescription(NVM_UID uid, void * log_entry)
{
	struct pt_fw_media_log_entry *p_media_log_entry = (struct pt_fw_media_log_entry *)log_entry;
	std::stringstream log_details;
	log_details	<< ACPI_SMART_HEALTH_EVENT_TS_HEADER << std::hex << p_media_log_entry->system_timestamp
				<< ACPI_SMART_HEALTH_EVENT_DPA_HEADER << std::hex << p_media_log_entry->dpa
				<< ACPI_SMART_HEALTH_EVENT_PDA_HEADER << std::hex << p_media_log_entry->pda
				<< ACPI_SMART_HEALTH_EVENT_RANGE_HEADER << std::hex << (unsigned int)p_media_log_entry->range
				<< ACPI_SMART_HEALTH_EVENT_ERROR_TYPE_HEADER << std::hex << (unsigned int)p_media_log_entry->error_type
				<< ACPI_SMART_HEALTH_EVENT_ERROR_FLAGS_HEADER << std::hex << (unsigned int)p_media_log_entry->error_flags
				<< ACPI_SMART_HEALTH_EVENT_TRANS_TYPE_HEADER << std::hex << (unsigned int)p_media_log_entry->transaction_type
				<< ACPI_SMART_HEALTH_EVENT_SEQ_NUM_HEADER << std::dec << p_media_log_entry->seq_num;
	return log_details.str();
}

/*
* Send a system event that describes how many new FW error log entries were found.
* Should be visiable in Windows event viewer or Linux syslog depending on the running OS.
*
* @param[in] uid - target device in question
* @param[in] error_count - how many new events were found.
*/
void monitor::AcpiMonitor::sendFwErrCntSystemEventEntry(const NVM_UID device_uid,
	const NVM_UINT64 error_count)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream new_error_count;
	new_error_count << error_count;

	m_mon_acpi_interface.send_event(EVENT_TYPE_HEALTH,
		EVENT_SEVERITY_WARN,
		EVENT_CODE_HEALTH_NEW_FWERRORS_FOUND,
		device_uid,
		false,
		core::Helper::uidToString(device_uid).c_str(),
		new_error_count.str().c_str(),
		NULL,
		DIAGNOSTIC_RESULT_UNKNOWN);
}

/*
* Send a system event that describes a singular error log entry.
* Should be visiable in Windows event viewer or Linux syslog depending on the running OS.
*
* @param[in] uid - target device in question
*/
void monitor::AcpiMonitor::sendFwErrLogSystemEventEntry(const NVM_UID device_uid, std::string log_type_level, std::string log_details)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_mon_acpi_interface.send_event(EVENT_TYPE_HEALTH,
		EVENT_SEVERITY_CRITICAL,
		EVENT_CODE_HEALTH_SMART_HEALTH,
		device_uid,
		false,
		core::Helper::uidToString(device_uid).c_str(),
		log_type_level.c_str(),
		log_details.c_str(),
		DIAGNOSTIC_RESULT_UNKNOWN);
}

/*
*
* Helper func for creating a human readable event message that describes a smart temp
* event.
*
* @param[in] temp_data - smart temp data read from the dimm.
*/
std::string monitor::AcpiMonitor::createThermStr(SMART_TEMP temp_data)
{
	std::stringstream temp_details;
	if (TEMP_TYPE_CORE == temp_data.parts.type)
	{
		temp_details << ACPI_SMART_HEALTH_EVENT_TEMP_CORE_STR;
	}
	else if (TEMP_TYPE_MEDIA == temp_data.parts.type)
	{
		temp_details << ACPI_SMART_HEALTH_EVENT_TEMP_MEDIA_STR;
	}

	if (TEMP_USER_ALARM == temp_data.parts.reported)
	{
		temp_details << ACPI_SMART_HEALTH_EVENT_USER_ALARM_TRIP_STR;
	}
	else if (TEMP_LOW == temp_data.parts.reported)
	{
		temp_details << ACPI_SMART_HEALTH_EVENT_TEMP_LOW_STR;
	}
	else if (TEMP_HIGH == temp_data.parts.reported)
	{
		temp_details << ACPI_SMART_HEALTH_EVENT_TEMP_HIGH_STR;
	}
	else if (TEMP_CRIT == temp_data.parts.reported)
	{
		temp_details << ACPI_SMART_HEALTH_EVENT_TEMP_CRITICAL_STR;
	}

	temp_details << ACPI_SMART_HEALTH_EVENT_TEMP_RAW_STR;

	if (TEMP_NEGATIVE == temp_data.parts.sign)
	{
		temp_details << ACPI_SMART_HEALTH_EVENT_TEMP_NEG_STR;
	}

	temp_details << "0x" << std::hex << temp_data.parts.temp;
	return temp_details.str().c_str();
}