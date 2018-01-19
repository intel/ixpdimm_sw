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

#include <os/os_adapter.h>
#include <stdio.h>
#include <windows.h>
#define WINVER _WIN32_WINNT_WIN8
#include <CfgMgr32.h>
#include <initguid.h>
#include <winerror.h>
#include "device_adapter.h"
#include "nfit_utilities.h"
#include "win_scm2_adapter.h"

#pragma comment(lib, "cfgmgr32.lib")
//
// When a physical NVDIMM driver detects a change in the health status of an NVDIMM,
// it must trigger a PNP custom event (through TARGET_DEVICE_CUSTOM_NOTIFICATION) to
// alert any registered components. The custom event's GUID is GUID_NVDIMM_HEALTH_NOTIFICATION
// and its payload is NVDIMM_HEALTH_NOTIFICATION_DATA
//
// {9DA2D386-72F5-4EE3-8155-ECA0678E3B06}
DEFINE_GUID(GUID_NVDIMM_HEALTH_NOTIFICATION, 0x9da2d386, 0x72f5, 0x4ee3, 0x81, 0x55, 0xec, 0xa0, 0x67, 0x8e, 0x3b, 0x6);

struct nvm_dimm_acpi_event_ctx
{
	NVM_NFIT_DEVICE_HANDLE		dimm_handle;
	HANDLE						h_nvdimm;
	CM_NOTIFY_FILTER			notify_filter;
	HCMNOTIFICATION				h_notification;
	HANDLE						h_event;
	NVM_UINT32					monitored_events;
	NVM_UINT32					triggered_events;
};

int acpi_health_notification_callback (HCMNOTIFICATION h_notify, void *context, CM_NOTIFY_ACTION action, PCM_NOTIFY_EVENT_DATA event_data, int event_data_size)
{
	struct nvm_dimm_acpi_event_ctx * ctx = context;
	SCM_LOG_INFO("Received a health notification");
	// Check the received context
	if(NULL == ctx)
	{
		SCM_LOG_ERROR("Notification callback with the NULL context pointer.\n");
		return ERROR_INVALID_DATA;
	}

	// We only care about disk health events.
	if (action == CM_NOTIFY_ACTION_DEVICECUSTOMEVENT)
	{
		if (event_data->FilterType == CM_NOTIFY_FILTER_TYPE_DEVICEHANDLE)
		{
			if ((memcmp(&event_data->u.DeviceHandle.EventGuid, &GUID_NVDIMM_HEALTH_NOTIFICATION, sizeof(GUID)) == 0) &&
				(ctx->monitored_events & DIMM_ACPI_EVENT_SMART_HEALTH_MASK))
			{
				ctx->triggered_events |= DIMM_ACPI_EVENT_SMART_HEALTH_MASK;
				// Send an event notification to the waiting thread
				if (!SetEvent(ctx->h_event))
				{
					SCM_LOG_ERROR_F("SetEvent failed (%d)\n", (int)GetLastError());
					return ERROR_WRITE_FAULT;
				}
			}
			else
			{
				SCM_LOG_INFO_F("Event not sent, monitored_events mask: 0x%x, event received mask: 0x%x.\n",
								 ctx->monitored_events, DIMM_ACPI_EVENT_SMART_HEALTH_MASK);
			}
		}
	}

	return NO_ERROR;
}

/*
* Create a context for a particular dimm to be used by all other acpi_event_* APIs
*
* @param[in] dimm_handle - NFIT dimm handle
*
* @return Returns a pointer to the context.  Note, this context needs to be freed
*	by acpi_event_free_ctx.
*/
int acpi_event_create_ctx(NVM_NFIT_DEVICE_HANDLE dimm_handle, void ** ctx)
{
	struct nvm_dimm_acpi_event_ctx * win_ctx = NULL;
	char ioctl_target[256];
	int status;

	// allocate the context
	win_ctx = *ctx = (struct nvm_dimm_acpi_event_ctx *)malloc(sizeof(struct nvm_dimm_acpi_event_ctx));
	if (NULL != win_ctx)
	{
		// Initialize memory with zero values
		memset(win_ctx, 0, sizeof(struct nvm_dimm_acpi_event_ctx));
		// Create the device handle
		win_ctx->dimm_handle = dimm_handle;
		//	\\.\PhysicalNvdimm{NFIT handle in hex}
		sprintf_s(ioctl_target, 256, "\\\\.\\PhysicalNvdimm%x", win_ctx->dimm_handle);
		win_ctx->h_nvdimm = CreateFile(ioctl_target,
											0,
											FILE_SHARE_READ | FILE_SHARE_WRITE,
											NULL, OPEN_EXISTING, 0, NULL);
		if (win_ctx->h_nvdimm == INVALID_HANDLE_VALUE)
		{
			// cound not open handle to the Nvdimm
			SCM_LOG_ERROR_F("CreateFile failed for target: %s. Verify if driver installed. Error: %d",
								 ioctl_target,
								 (int)GetLastError());
			free(win_ctx);
			return NVM_ERR_UNKNOWN;
		}
		// Create event
		win_ctx->h_event = CreateEvent(
								NULL,// default security attributes
								FALSE,// no manual-reset event
								FALSE,// initial state is nonsignaled
								TEXT("ACPIEventHdl")//object name
								);
		if (win_ctx->h_event == NULL)
		{
			SCM_LOG_ERROR_F("CreateEvent for target: %s. Error: %d\n", ioctl_target, (int)GetLastError());
			free(win_ctx);
			return NVM_ERR_UNKNOWN;
		}
		// Register for a notification by using a handle to the disk.
		win_ctx->notify_filter.cbSize = sizeof(win_ctx->notify_filter);
		win_ctx->notify_filter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEHANDLE;
		win_ctx->notify_filter.u.DeviceHandle.hTarget = win_ctx->h_nvdimm;
		win_ctx->h_notification = NULL;
		status = CM_Register_Notification(&win_ctx->notify_filter, (void *)win_ctx, (PCM_NOTIFY_CALLBACK) acpi_health_notification_callback, &win_ctx->h_notification);
		if (CR_SUCCESS != status) {
			SCM_LOG_ERROR_F("Couldn't register for notification. Target: %s, Error: %d", ioctl_target, status);
			free(win_ctx);
			return NVM_ERR_UNKNOWN;
		}
	}
	return NVM_SUCCESS;
}

/*
* Free a context previously created by acpi_event_create_ctx.
*
* @param[in] ctx - pointer to a context created by acpi_event_create_ctx
*/
int acpi_event_free_ctx(void * context)
{
	struct nvm_dimm_acpi_event_ctx * ctx = context;
	if (NULL != ctx)
	{
		// Unregister a notification callback
		CM_Unregister_Notification(ctx->h_notification);
		// Close the event handle
		CloseHandle(ctx->h_event);
		// Close file
		CloseHandle(ctx->h_nvdimm);
		// Free Context
		free(ctx);
	}
	return NVM_SUCCESS;
}

/*
* Retrieve the NFIT dimm handle associated with the context.
*
* @param[in] ctx - pointer to a context created by acpi_event_create_ctx
*
* @return Returns the NFIT dimm handle associated with the context
*/
int acpi_event_ctx_get_dimm_handle(void * ctx, NVM_NFIT_DEVICE_HANDLE * dev_handle)
{
	struct nvm_dimm_acpi_event_ctx * acpi_event_ctx = (struct nvm_dimm_acpi_event_ctx *)ctx;
	if (NULL != acpi_event_ctx)
	{
		*dev_handle = acpi_event_ctx->dimm_handle;
		return NVM_SUCCESS;
	}
	else
	{
		return NVM_ERR_INVALIDPARAMETER; // no handle
	}
}

/*
* Retrieve an ACPI notification state of a DIMM.
*
* @param[in] ctx - pointer to a context created by acpi_event_create_ctx
*
* @return Returns one of the following
*		ACPI_EVENT_SIGNALLED
*		ACPI_EVENT_NOT_SIGNALLED
*		ACPI_EVENT_UNKNOWN
*/
int acpi_event_get_event_state(void * ctx, enum acpi_event_type event_type, enum acpi_event_state *event_state)
{
	struct nvm_dimm_acpi_event_ctx * acpi_event_ctx = (struct nvm_dimm_acpi_event_ctx *)ctx;
	if (NULL != acpi_event_ctx)
	{
		*event_state = (acpi_event_ctx->triggered_events & (1 << event_type)) ? ACPI_EVENT_SIGNALLED : ACPI_EVENT_NOT_SIGNALLED;
		return NVM_SUCCESS;
	}
	else
	{
		return NVM_ERR_INVALIDPARAMETER;
	}
}

/*
* Set which ACPI events should be monitored.
*
* @param[in] ctx - pointer to a context created by acpi_event_create_ctx
*
*/
int acpi_event_set_monitor_mask(void * ctx, const unsigned int acpi_monitored_event_mask)
{
	struct nvm_dimm_acpi_event_ctx * acpi_event_ctx = (struct nvm_dimm_acpi_event_ctx *)ctx;
	if (NULL != acpi_event_ctx)
	{
		acpi_event_ctx->monitored_events = acpi_monitored_event_mask;
	}
	return NVM_SUCCESS;
}

/*
* Set which ACPI events should be monitored.
*
* @param[in] ctx - pointer to a context created by acpi_event_create_ctx
*
*/
int acpi_event_get_monitor_mask(void * ctx, unsigned int * mask)
{
	struct nvm_dimm_acpi_event_ctx * acpi_event_ctx = (struct nvm_dimm_acpi_event_ctx *)ctx;
	if (NULL != acpi_event_ctx)
	{
		*mask = acpi_event_ctx->monitored_events;
		return NVM_SUCCESS;
	}
	else
		return NVM_ERR_INVALIDPARAMETER;
}

/*
* Wait for an asynchronous ACPI notification. This function will return when the timeout expires or an acpi notification
* occurs for any dimm, whichever happens first.
*
*
* @param[in] acpi_event_contexts - Array of contexts
* @param[in] dimm_cnt - Number of contexts in the array
* @param[in] timeout_sec - -1 - No timeout, all other non-negative values represent a second granularity timeout value
*
* @return Returns one of the following
*		ACPI_EVENT_SIGNALLED_RESULT
*		ACPI_EVENT_TIMED_OUT_RESULT
*		ACPI_EVENT_UNKNOWN_RESULT
*/
int acpi_wait_for_event(void * acpi_event_contexts[], const NVM_UINT32 dimm_cnt, const int timeout_sec, enum acpi_get_event_result * event_result)
{
	COMMON_LOG_ENTRY();
	struct nvm_dimm_acpi_event_ctx *ctx;
	unsigned int index;
	unsigned long event;
	int rc = NVM_SUCCESS;

	// prepare the handle table
	HANDLE *p_h_event = (HANDLE*) malloc(sizeof(HANDLE)*dimm_cnt);
	if (p_h_event)
	{
		// copy event handlers to the table and clear all passed events
		for (index = 0; index < dimm_cnt; index++)
		{
			ctx = (struct nvm_dimm_acpi_event_ctx *) acpi_event_contexts[index];
			if (NULL == ctx)
			{
				free(p_h_event);
				return NVM_ERR_INVALIDPARAMETER;
			}
			p_h_event[index] = ctx->h_event;
		}
		// Wait for the thread to signal one of the event objects
		event = WaitForMultipleObjects(dimm_cnt,				// number of objects in array
										p_h_event,			// array of objects
										FALSE,			// wait for any object
										timeout_sec*1000);		// miliseconds wait
		if (WAIT_TIMEOUT == event)
		{
			SCM_LOG_ERROR("Wait for event: Timeout\n");
			*event_result = ACPI_EVENT_TIMED_OUT_RESULT;
		}
		else if ((WAIT_FAILED == event) || (WAIT_ABANDONED_0 == event))
		{
			SCM_LOG_ERROR("Wait for event: Failed\n");
			*event_result = ACPI_EVENT_UNKNOWN_RESULT;
		}
		else
		{
			ctx = (struct nvm_dimm_acpi_event_ctx *) acpi_event_contexts[event-WAIT_OBJECT_0];
			SCM_LOG_INFO_F("Wait for event: Signalled. event: 0x%lu; monitored_events 0x%xu, triggered_events 0x%xu\n",
							event-WAIT_OBJECT_0,
							ctx->monitored_events,
							ctx->triggered_events);
			*event_result = ACPI_EVENT_SIGNALLED_RESULT;
		}
		free(p_h_event);
	}

	COMMON_LOG_EXIT();
	return rc;
}
