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

#include <support/EventLogFilter.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <persistence/logging.h>

wbem::support::EventLogFilter::EventLogFilter() :
		m_eventIdValueSet(false),
		m_eventIdValue(0),
		m_severityValueSet(false),
		m_severityValue(0),
		m_typeValueSet(false),
		m_typeValue(0),
		m_codeValueSet(false),
		m_codeValue(0),
		m_uidValueSet(false),
		m_actionReqValueSet(false),
		m_actionReqValue(false),
		m_beforeTimestampValueSet(false),
		m_beforeTimestampValue(0),
		m_afterTimestampValueSet(false),
		m_afterTimestampValue(0)
{
}

wbem::support::EventLogFilter::~EventLogFilter()
{
}

/*
 * Set the event ID filter.
 */
void wbem::support::EventLogFilter::setEventId(const framework::UINT32 &eventId)
{
	m_eventIdValue = eventId;
	m_eventIdValueSet = true;
}

/*
 * Fetch the event ID filter value.
 */
wbem::framework::UINT32 wbem::support::EventLogFilter::getEventId() throw (framework::Exception)
{
	if (!hasEventId())
	{
		COMMON_LOG_ERROR("requested event ID filter when none set");
		throw framework::ExceptionBadParameter("eventId");
	}
	return m_eventIdValue;
}

/*
 * Set the event severity filter.
 */
void wbem::support::EventLogFilter::setSeverity(const framework::UINT32 &severity)
{
	m_severityValue = severity;
	m_severityValueSet = true;
}

/*
 * Fetch the event severity filter value.
 * @throw Exception if not set
 */
wbem::framework::UINT32 wbem::support::EventLogFilter::getSeverity() throw (framework::Exception)
{
	if (!hasSeverity())
	{
		COMMON_LOG_ERROR("requested severity filter when none set");
		throw framework::ExceptionBadParameter("severity");
	}
	return m_severityValue;
}

/*
 * Set the event type filter.
 */
void wbem::support::EventLogFilter::setType(const framework::UINT32 &type)
{
	m_typeValue = type;
	m_typeValueSet = true;
}

/*
 * Fetch the event type filter value.
 * @throw Exception if not set
 */
wbem::framework::UINT32 wbem::support::EventLogFilter::getType() throw (framework::Exception)
{
	if (!hasType())
	{
		COMMON_LOG_ERROR("requested type filter when none set");
		throw framework::ExceptionBadParameter("type");
	}
	return m_typeValue;
}

/*
 * Set the event code filter.
 */
void wbem::support::EventLogFilter::setCode(const framework::UINT16 &code)
{
	m_codeValue = code;
	m_codeValueSet = true;
}

/*
 * Fetch the event code filter value.
 */
wbem::framework::UINT16 wbem::support::EventLogFilter::getCode() throw (framework::Exception)
{
	if (!hasCode())
	{
		COMMON_LOG_ERROR("requested code filter when none set");
		throw framework::ExceptionBadParameter("code");
	}
	return m_codeValue;
}

/*
 * Set the UID filter.
 */
void wbem::support::EventLogFilter::setUid(const std::string &uid)
{
	m_uidValue = uid;
	m_uidValueSet = true;
}

/*
 * Fetch the UID filter value.
 */
std::string wbem::support::EventLogFilter::getUid() throw (framework::Exception)
{
	if (!hasUid())
	{
		COMMON_LOG_ERROR("requested UID filter when none set");
		throw framework::ExceptionBadParameter("uid");
	}
	return m_uidValue;
}

/*
 * Set the action required filter.
 */
void wbem::support::EventLogFilter::setActionRequired(const bool &actionRequired)
{
	m_actionReqValue = actionRequired;
	m_actionReqValueSet = true;
}

/*
 * Fetch the action required filter value.
 */
bool wbem::support::EventLogFilter::getActionRequired() throw (framework::Exception)
{
	if (!hasActionRequired())
	{
		COMMON_LOG_ERROR("requested action required filter when none set");
		throw framework::ExceptionBadParameter("actionRequired");
	}
	return m_actionReqValue;
}

/*
 * Set the event "before timestamp" filter.
 */
void wbem::support::EventLogFilter::setBeforeTimestamp(const time_t &beforeTime)
{
	m_beforeTimestampValue = beforeTime;
	m_beforeTimestampValueSet = true;
}

/*
 * Fetch the event "before timestamp" filter value.
 */
time_t wbem::support::EventLogFilter::getBeforeTimestamp() throw (framework::Exception)
{
	if (!hasBeforeTimestamp())
	{
		COMMON_LOG_ERROR("requested before time filter when none set");
		throw framework::ExceptionBadParameter("beforeTimestamp");
	}
	return m_beforeTimestampValue;
}

/*
 * Set the event "after timestamp" filter.
 */
void wbem::support::EventLogFilter::setAfterTimestamp(const time_t &afterTime)
{
	m_afterTimestampValue = afterTime;
	m_afterTimestampValueSet = true;
}

/*!
 * Fetch the event "after timestamp" filter value.
 */
time_t wbem::support::EventLogFilter::getAfterTimestamp() throw (framework::Exception)
{
	if (!hasAfterTimestamp())
	{
		COMMON_LOG_ERROR("requested after timestamp filter when none set");
		throw framework::ExceptionBadParameter("afterTimestamp");
	}
	return m_afterTimestampValue;
}
