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
 * This file contains a set of filters to use on event logs.
 */

#ifndef EVENTLOGFILTER_H_
#define EVENTLOGFILTER_H_

#include <libinvm-cim/Exception.h>
#include <libinvm-cim/Types.h>
#include <ctime>
#include <common_types.h>
#include <nvm_types.h>

namespace wbem
{
namespace support
{

/*
 * Event Severity Values - happen to be the same as the enum event_severity in the library
 */
const COMMON_UINT32 EVENT_SEVERITY_INFO = 2; //!< Informational event.
const COMMON_UINT32 EVENT_SEVERITY_WARN = 3; //!< Warning or degraded.
const COMMON_UINT32 EVENT_SEVERITY_CRITICAL = 6; //!< Critical.
const COMMON_UINT32 EVENT_SEVERITY_FATAL = 7; //!< Fatal or nonrecoverable.

/*
 * Event "Category" or "type" - happen to be the same values as enum event_type in the lib
*/
const COMMON_UINT32 EVENT_TYPE_ALL = 0;	//!< Subscribe or filter on all event types
const COMMON_UINT32 EVENT_TYPE_CONFIG = 1; //!< Platform config errors on device
const COMMON_UINT32 EVENT_TYPE_HEALTH = 2; //!< Device health event.
const COMMON_UINT32 EVENT_TYPE_MGMT	= 3; //!< Management software generated event.
const COMMON_UINT32 EVENT_TYPE_DIAG = 4; //!< Subscribe or filter on all diagnostic event types
const COMMON_UINT32 EVENT_TYPE_DIAG_QUICK = 5; //!< Quick diagnostic test event.
const COMMON_UINT32 EVENT_TYPE_DIAG_PLATFORM_CONFIG = 6; //!< Platform config diagnostic test event.
const COMMON_UINT32 EVENT_TYPE_DIAG_PM_META = 7; //!< PM metadata diagnostic test event.
const COMMON_UINT32 EVENT_TYPE_DIAG_SECURITY = 8; //!< Security diagnostic test event.
const COMMON_UINT32 EVENT_TYPE_DIAG_FW_CONSISTENCY = 9; //!< FW consistency diagnostic test event.


class NVM_API EventLogFilter
{
	public:

		/*!
		 * Constructor
		 */
		EventLogFilter();

		/*!
		 * Destructor
		 */
		virtual ~EventLogFilter();

		/*!
		 * Set the event ID filter.
		 */
		void setEventId(const framework::UINT32 &eventId);

		/*!
		 * Is the event ID filter set?
		 * @return true if it is set, false otherwise
		 */
		bool hasEventId() { return m_eventIdValueSet; }

		/*!
		 * Fetch the event ID filter value.
		 * @throw Exception if not set
		 */
		framework::UINT32 getEventId() throw (framework::Exception);

		/*!
		 * Set the event severity filter.
		 */
		void setSeverity(const framework::UINT32 &severity);

		/*!
		 * Is the event severity filter set?
		 * @return true if it is set, false otherwise
		 */
		bool hasSeverity() { return m_severityValueSet; }

		/*!
		 * Fetch the event severity filter value.
		 * @throw Exception if not set
		 */
		framework::UINT32 getSeverity() throw (framework::Exception);

		/*!
		 * Set the event type filter.
		 */
		void setType(const framework::UINT32 &type);

		/*!
		 * Is the event type filter set?
		 * @return true if it is set, false otherwise
		 */
		bool hasType() { return m_typeValueSet; }

		/*!
		 * Fetch the event type filter value.
		 * @throw Exception if not set
		 */
		framework::UINT32 getType() throw (framework::Exception);

		/*!
		 * Set the event code filter.
		 */
		void setCode(const framework::UINT16 &code);

		/*!
		 * Is the event code filter set?
		 * @return true if it is set, false otherwise
		 */
		bool hasCode() { return m_codeValueSet; }

		/*!
		 * Fetch the event code filter value.
		 * @throw Exception if not set
		 */
		framework::UINT16 getCode() throw (framework::Exception);

		/*!
		 * Set the UID filter.
		 */
		void setUid(const std::string &uid);

		/*!
		 * Is the UID filter set?
		 * @return true if UID is set, false otherwise
		 */
		bool hasUid() { return m_uidValueSet; }

		/*!
		 * Fetch the UID filter value.
		 * @throw Exception if not set
		 */
		std::string getUid() throw (framework::Exception);

		/*!
		 * Set the action required filter.
		 */
		void setActionRequired(const bool &actionRequired);

		/*!
		 * Is the action required filter set?
		 * @return true if it is set, false otherwise
		 */
		bool hasActionRequired() { return m_actionReqValueSet; }

		/*!
		 * Fetch the action required filter value.
		 * @throw Exception if not set
		 */
		bool getActionRequired() throw (framework::Exception);

		/*!
		 * Set the event "before timestamp" filter.
		 */
		void setBeforeTimestamp(const time_t &beforeTime);

		/*!
		 * Is the event "before timestamp" filter set?
		 * @return true if it is set, false otherwise
		 */
		bool hasBeforeTimestamp() { return m_beforeTimestampValue; }

		/*!
		 * Fetch the event "before timestamp" filter value.
		 * @throw Exception if not set
		 */
		time_t getBeforeTimestamp() throw (framework::Exception);

		/*!
		 * Set the event "after timestamp" filter.
		 */
		void setAfterTimestamp(const time_t &beforeTime);

		/*!
		 * Is the event "after timestamp" filter set?
		 * @return true if it is set, false otherwise
		 */
		bool hasAfterTimestamp() { return m_afterTimestampValue; }

		/*!
		 * Fetch the event "after timestamp" filter value.
		 * @throw Exception if not set
		 */
		time_t getAfterTimestamp() throw (framework::Exception);

	protected:
		// Member variables
		bool m_eventIdValueSet; //!< is the event ID filter value set
		framework::UINT32 m_eventIdValue; //!< event ID

		bool m_severityValueSet; //!< is the severity filter value set
		framework::UINT32 m_severityValue; //!< event severity enum

		bool m_typeValueSet; //!< is the event type filter value set
		framework::UINT32 m_typeValue; //!< event type enum

		bool m_codeValueSet; //!< is the event code filter value set
		framework::UINT16 m_codeValue; //!< event code

		bool m_uidValueSet; //!< is the UID filter value set
		std::string m_uidValue; //!< UID as string

		bool m_actionReqValueSet; //!< is the action required filter value set
		bool m_actionReqValue; //!< action required on event

		bool m_beforeTimestampValueSet; //!< is the timestamp before filter value set
		time_t m_beforeTimestampValue; //!< events before the specified timestamp

		bool m_afterTimestampValueSet; //!< is the timestamp after filter value set
		time_t m_afterTimestampValue; //!< events after the specified timestamp
};

} /* namespace support */
} /* namespace wbem */

#endif /* EVENTLOGFILTER_H_ */
