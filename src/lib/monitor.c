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
 * Implementation of the Native API monitoring functions.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "device_adapter.h"
#include "nvm_management.h"
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include "monitor.h"
#include <persistence/logging.h>
#include <string/s_str.h>
#include <uid/uid.h>
#include <cr_i18n.h>
#include <os/os_adapter.h>
#include <persistence/schema.h>
#include "device_utilities.h"
#include "capabilities.h"
#include "nvm_context.h"
#include "system.h"
#include <time/time_utilities.h>

/*
 * global (to this file) variables
 */
#ifdef __WINDOWS__
#include <windows.h>
	extern HANDLE g_eventmonitor_lock;
#else
	extern pthread_mutex_t g_eventmonitor_lock;
#endif

// Have we added a hook to get called back when the db changes?
static int g_is_polling = 0;
// event callback list
static struct event_notify_callback g_event_callback_list[MAX_EVENT_SUBSCRIBERS];
static int g_current_event_id = 0; // When polling this stores the most recent event id
static NVM_UINT32 g_poll_interval_sec = 60; // Default poll interval. Overridden in config database

/*
 * Helper functions
 */
static void *poll_events(void *arg); // run in a seperate thread for polling the event table
static int timeToQuit(unsigned long timeoutSeconds); // helps polling to know when to stop
static int get_nvm_event_id(); // get most recent id from event table


/*
 * Convert a uid to an event argument (string)
 */
void uid_to_event_arg(const NVM_UID uid, NVM_EVENT_ARG arg)
{
	if (uid && arg)
	{
		NVM_UID uid_str;
		uid_copy(uid, uid_str);
		s_strcpy(arg, uid_str, NVM_EVENT_ARG_LEN);
	}
}

/*
 * Log a management software event to the config DB and to syslog if applicable
 */
int log_mgmt_event(const enum event_severity severity, const NVM_UINT16 code,
		const NVM_UID device_uid, const NVM_BOOL action_required, const NVM_EVENT_ARG arg1,
		const NVM_EVENT_ARG arg2, const NVM_EVENT_ARG arg3)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// store the event in the db and syslog
	rc = store_event_by_parts(EVENT_TYPE_MGMT, severity, code, device_uid,
			action_required, arg1, arg2, arg3, DIAGNOSTIC_RESULT_UNKNOWN);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


/*
 * Retrieve the number of events in the native API library event database.
 */
int nvm_get_event_count(const struct event_filter *p_filter)
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		rc = process_events_matching_filter(p_filter, NULL, 0, 0);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve a list of stored events from the native API library database and
 * optionally filter the results.
 */
int nvm_get_events(const struct event_filter *p_filter,
		struct event *p_events, const NVM_UINT16 count)
{
	COMMON_LOG_ENTRY_PARAMS("count: %d", count);
	int rc = 0;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (p_events == NULL)
	{
		rc = NVM_ERR_INVALIDPARAMETER;
		COMMON_LOG_ERROR("p_events is NULL");
	}
	else if (count == 0)
	{
		rc = NVM_ERR_INVALIDPARAMETER;
		COMMON_LOG_ERROR("count is 0");
	}
	else
	{
		memset(p_events, 0, count * sizeof (struct event));
		rc = process_events_matching_filter(p_filter, p_events, count, 0);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int nvm_purge_events(const struct event_filter *p_filter)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		rc = process_events_matching_filter(p_filter, NULL, 0, 1);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Be sure we are listening for database updates
 */
int init_event_notify()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;


	if (mutex_lock(&g_eventmonitor_lock))
	{
		// if we're not already polling for new events
		if (!g_is_polling)
		{
			// first time registering, so zero out callback pointers
			for (int i = 0; i < MAX_EVENT_SUBSCRIBERS; i++)
			{
				g_event_callback_list[i].p_event_callback = 0;
			}

			g_current_event_id = get_nvm_event_id();
			int poll_interval = 0;
			if (NVM_SUCCESS == get_bounded_config_value_int(
					SQL_KEY_EVENT_POLLING_INTERVAL_MINUTES, &poll_interval))
			{
				g_poll_interval_sec = (NVM_UINT32)poll_interval * SECONDSPERMINUTE;
			}

			g_is_polling = 1;

			mutex_unlock(&g_eventmonitor_lock);

			// start polling
			NVM_UINT64 thread_id;
			create_thread(&thread_id, poll_events, NULL);
		}
		else
		{
			mutex_unlock(&g_eventmonitor_lock);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Stop polling on event table
 */
int remove_event_notify()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (mutex_lock(&g_eventmonitor_lock))
	{
		// if we're listening for database changes, stop
		if (g_is_polling)
		{
			// trigger we are done polling
			g_is_polling = 0;
		}
		mutex_unlock(&g_eventmonitor_lock);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Register a callback for events
 */
int nvm_add_event_notify(const enum event_type type,
		void (*p_event_callback) (struct event *p_event))
{
	COMMON_LOG_ENTRY_PARAMS("type: %d", type);
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (p_event_callback == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_event_callback is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		// be sure we have the polling setup and running to get callback when a new event is created
		init_event_notify();
		int notify_added = 0;
		int i = 0;

		if (mutex_lock(&g_eventmonitor_lock))
		{
			// if there's an open callback on the list, fill it
			for (i = 0; (i < MAX_EVENT_SUBSCRIBERS) && (!notify_added); i++)
			{
				if (NULL == g_event_callback_list[i].p_event_callback)
				{
					g_event_callback_list[i].p_event_callback = p_event_callback;
					g_event_callback_list[i].type = type;
					notify_added = 1;
				}
			}
			mutex_unlock(&g_eventmonitor_lock);
		}

		// if no place to add one, return error
		if (!notify_added)
		{
			rc = NVM_ERR_EXCEEDSMAXSUBSCRIBERS;
			COMMON_LOG_ERROR("Max subscribers exceeded.");
		}

		// if successful, return the callback ID - just using the index so should be unique
		if (NVM_SUCCESS == rc)
		{
			rc = i - 1;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Remove the specified callback for device events.  Callback_id used is just the index
 * of the event_callback_list.
 */
int nvm_remove_event_notify(const int callback_id)
{
	COMMON_LOG_ENTRY_PARAMS("callback_id: %d", callback_id);
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	if (callback_id < 0)
	{
		COMMON_LOG_ERROR("Invalid callback_id < 0");
		rc = NVM_ERR_BADCALLBACK;
	}
	else if (callback_id >= MAX_EVENT_SUBSCRIBERS)
	{
		COMMON_LOG_ERROR("Invalid callback_id > MAX_EVENT_SUBSCRIBERS");
		rc = NVM_ERR_BADCALLBACK;
	}
	else
	{
		int still_listening = 0;
		int i = 0;
		if (mutex_lock(&g_eventmonitor_lock))
		{
			g_event_callback_list[callback_id].p_event_callback = NULL;

			// check to see if there are any others still listening for callbacks
			for (i = 0; i < MAX_EVENT_SUBSCRIBERS; i++)
			{
				if (NULL != g_event_callback_list[i].p_event_callback)
				{
					still_listening = 1;
					break;
				}
			}
			mutex_unlock(&g_eventmonitor_lock);
		}
		// if not, remove the device notify
		if (0 == still_listening)
		{
			remove_event_notify();
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 *  Acknowledges the specified event
 */
int nvm_acknowledge_event(NVM_UINT32 event_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		PersistentStore *p_store = get_lib_store();
		if (!p_store)
		{
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{
			struct db_event event;
			rc = db_get_event_by_id(p_store, event_id, &event);
			if (rc == DB_SUCCESS)
			{
				if (!event.action_required)
				{
					// error to acknowledge an event that does not have action_required set to true
					rc = NVM_ERR_NOTSUPPORTED;
				}
				else
				{
					event.action_required = 0;
					rc = db_update_event_by_id(p_store, event_id, &event);
				}
			}
			else
			{
				COMMON_LOG_ERROR_F("Invalid event id %d", event_id);
				rc = NVM_ERR_INVALIDPARAMETER;
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the most recent event id.
 */
static int get_nvm_event_id()
{
	COMMON_LOG_ENTRY();
	int event_id = 0;
	PersistentStore *p_store = get_lib_store();
	if (p_store)
	{
		if (run_scalar_sql(p_store, "select max(id) from event", &event_id) != DB_SUCCESS)
		{
			event_id = 0;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(event_id);
	return event_id;
}

/*
 * hold until it's time to stop polling or the wait time expires
 */
static int timeToQuit(unsigned long timeoutSeconds)
{
	COMMON_LOG_ENTRY();
	int time_to_quit = 0;
	time_t beg = time(NULL);
	time_t cur;
	unsigned long elapsedSeconds = 0;
	do
	{
		if (mutex_lock(&g_eventmonitor_lock))
		{
			time_to_quit = !g_is_polling;
			mutex_unlock(&g_eventmonitor_lock);
		}
		if (time_to_quit)
		{
			break;
		}
		// sleep
		cur = time(NULL);
		elapsedSeconds = (unsigned long)difftime(cur, beg);
		nvm_sleep(1000); // sleep for 1 second between checks
	}
	while (elapsedSeconds < timeoutSeconds);

	COMMON_LOG_EXIT_RETURN_I(time_to_quit);
	return time_to_quit;
}

/*
 * Look at the event table for any new records. For each new record notify any registered callbacks.
 */
static void *poll_events(void *arg)
{
	COMMON_LOG_ENTRY();
	struct event_filter filter; // used to get all events
	memset(&filter, 0, sizeof (filter));

	while (!timeToQuit(g_poll_interval_sec))
	{
		if (mutex_lock(&g_eventmonitor_lock))
		{
			// ensure nothing comes in while we are working
			db_begin_transaction(get_lib_store());
			int next_event_id = get_nvm_event_id();
			if (g_current_event_id < next_event_id)
			{
				int event_count = process_events_matching_filter(&filter, NULL, 0, 0);
				if (event_count < 0)
				{
					COMMON_LOG_ERROR_F("Error polling events while getting event count. Error: %d",
							event_count);
				}
				else
				{
					struct event events[event_count];
					event_count = process_events_matching_filter(&filter, events, event_count, 0);
					if (event_count < 0)
					{
						COMMON_LOG_ERROR_F("Error polling events while getting events. Error: %d",
								event_count);
					}
					else
					{
						// can't necessarily guarantee order of events,
						// so loop through each and only
						// execute callbacks if the event id is new.
						for (int e = 0; e < event_count; e++)
						{
							if (events[e].event_id > g_current_event_id)
							{
								for (int i = 0; (i < MAX_EVENT_SUBSCRIBERS); i++)
								{
									if (NULL != g_event_callback_list[i].p_event_callback)
									{
										// check if subscriber is interested in this event
										if (g_event_callback_list[i].type == events[e].type ||
												g_event_callback_list[i].type == EVENT_TYPE_ALL)
										{
											g_event_callback_list[i].p_event_callback(&events[e]);
										}
									}
								}
							}
						}
						// save new current event id so events aren't repeatedly sent to callbacks
						g_current_event_id = next_event_id;
					}
				}
			}
			else if (next_event_id < g_current_event_id) // Should never happen
			{
				COMMON_LOG_WARN("Most current event ID is less than the last event ID. Polling "
						"May have missed events to send to listeners.");
				g_current_event_id = next_event_id;
			}
			db_end_transaction(get_lib_store());

			mutex_unlock(&g_eventmonitor_lock);
		}
	}

	COMMON_LOG_EXIT();
	return NULL; // no return value expected
}
