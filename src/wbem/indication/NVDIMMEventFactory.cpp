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
 * This file contains the implementation of NVMDIMMEvent provider.
 */


#include <uid/uid.h>
#include <server/BaseServerFactory.h>
#include <support/NVDIMMLogEntryFactory.h>
#include <LogEnterExit.h>
#include <persistence/event.h>

#include "NVDIMMEventFactory.h"

wbem::framework::Instance *wbem::indication::NVDIMMEventFactory::createIndication(
		struct event *pEvent)
		throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pResult = NULL;
	// NVDIMMEvent: Instances are generated for: over-temperature, nvm device failure,
	// spare capacity low, remaining life low and media errors.
	if (pEvent->type == EVENT_TYPE_HEALTH && (
			pEvent->code == EVENT_CODE_HEALTH_HEALTH_STATE_CHANGED ||
			pEvent->code == EVENT_CODE_HEALTH_LOW_SPARE_CAPACITY ||
			pEvent->code == EVENT_CODE_HEALTH_MEDIA_TEMPERATURE_OVER_THRESHOLD ||
			pEvent->code == EVENT_CODE_HEALTH_CONTROLLER_TEMPERATURE_OVER_THRESHOLD ||
			pEvent->code == EVENT_CODE_HEALTH_NEW_MEDIAERRORS_FOUND ||
			pEvent->code == EVENT_CODE_HEALTH_HIGH_WEARLEVEL))
	{
		COMMON_LOG_DEBUG_F("Type: %d, Code: %d is an NVDIMMEvent Indication",
				(int)pEvent->type, (int)pEvent->code);
		framework::attributes_t keys; // indications have no keys
		std::string hostname = server::getHostName();
		framework::ObjectPath path(hostname, NVM_NAMESPACE, NVDIMMEVENT_CLASSNAME, keys);
		pResult = new framework::Instance(path);

		pResult->setAttribute(SYSTEMCREATIONCLASSNAME_KEY,
				framework::Attribute(server::BASESERVER_CREATIONCLASSNAME, false));
		pResult->setAttribute(ALERTINGELEMENTFORMAT_KEY,
				framework::Attribute((NVM_UINT16)2u, false)); // CIMObjectPath
		pResult->setAttribute(ALERTTYPE_KEY,
				framework::Attribute((NVM_UINT16)5u, false)); // "Device Alert"
		pResult->setAttribute(SYSTEMNAME_KEY,
				framework::Attribute(hostname, false)); // "Device Alert"
		pResult->setAttribute(INDICATIONTIME_KEY, framework::Attribute((NVM_UINT64)pEvent->time,
				framework::DATETIME_SUBTYPE_DATETIME, false));

		NVM_UID uid;
		uid_copy(pEvent->uid, uid);
		pResult->setAttribute(ALERTINGMANAGEDELEMENT_KEY,
				framework::Attribute(std::string(uid), false));

		// These are attributes we want populated from NVDIMMLogEntryFactory
		framework::attribute_names_t attributes;
		attributes.push_back(PERCEIVEDSEVERITY_KEY);
		attributes.push_back(MESSAGEID_KEY);
		attributes.push_back(MESSAGE_KEY);
		attributes.push_back(MESSAGEARGS_KEY);
		support::NVDIMMLogEntryFactory::eventToInstance(pResult, pEvent, attributes);
	}

	return pResult;
}

/*
 * PerceivedSeverity value map:
 * 		0, 1, 2, 3, 4, 5, 6, 7
 * 		Unknown, Other, Information, Degraded/Warning, Minor, Major, Critical, Fatal/NonRecoverable
 */
NVM_UINT16 wbem::indication::NVDIMMEventFactory::getPerceivedSeverity(enum event_severity severity)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	NVM_UINT16 result = 0; // Unknown
	switch (severity)
	{
		case EVENT_SEVERITY_INFO:
			result = NVDIMMEVENT_INFO;
			break;
		case EVENT_SEVERITY_WARN:
			result = NVDIMMEVENT_WARN;
			break;
		case EVENT_SEVERITY_CRITICAL:
			result = NVDIMMEVENT_CRITICAL;
			break;
		case EVENT_SEVERITY_FATAL:
			result = NVDIMMEVENT_FATAL;
			break;
	}

	return result;
}
