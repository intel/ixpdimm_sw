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
 * Describes the internal interface for the monitoring functions of the Native API.
 */

#ifndef	_NVM_MONITOR_H_
#define	_NVM_MONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <persistence/event.h>

/*
 * Structure to hold caller's callback function pointer, and which type of events interest them
 */
struct event_notify_callback
{
	enum event_type type;
	void (*p_event_callback)(struct event *p_event);
};


/*
 * Changes a uid to a string and copies it into an NVM_EVENT_ARG
 */
void uid_to_event_arg(const NVM_UID uid, NVM_EVENT_ARG arg);

/*
 * Logs a management event to the event log, and to syslog if
 * applicable.
 */
int log_mgmt_event(const enum event_severity severity, const NVM_UINT16 code,
		const NVM_UID device_uid, const NVM_BOOL action_required, const NVM_EVENT_ARG arg1,
		const NVM_EVENT_ARG arg2, const NVM_EVENT_ARG arg3);


#ifdef __cplusplus
}
#endif

#endif  /* _NVM_MONITOR_H_ */
