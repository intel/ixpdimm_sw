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
#include "lnx_adapter.h"
#include "device_adapter.h"
#include "nfit_utilities.h"


struct nvm_dimm_acpi_event_ctx
{
	NVM_NFIT_DEVICE_HANDLE dimm_handle;
	NVM_UINT32 monitored_events;
	NVM_UINT32 triggered_events;
	NVM_INT32 smart_health_fd;
	struct ndctl_ctx *ndctl_lib_ctx;
	struct ndctl_dimm *ndctl_lib_dimm;
};

/*
* Create a context for a particular dimm to be used by all other acpi_event_* APIs
*
* @param[in] dimm_handle - NFIT dimm handle
* @param[out] ctx - pointer to new context. Note, this context needs to be freed by acpi_event_free_ctx
* @return Returns one of the following
*		NVM_SUCCESS
*		NVM_ERR_NOMEMORY
*/
int acpi_event_create_ctx(NVM_NFIT_DEVICE_HANDLE dimm_handle, void ** ctx)
{
	int rc = NVM_SUCCESS;
	struct nvm_dimm_acpi_event_ctx * new_ctx;
	new_ctx = *ctx = (struct nvm_dimm_acpi_event_ctx *)malloc(sizeof(struct nvm_dimm_acpi_event_ctx));
	if (new_ctx)
	{
		ndctl_new(&new_ctx->ndctl_lib_ctx);
		new_ctx->dimm_handle = dimm_handle;
		if (NVM_SUCCESS == (rc = get_dimm_by_handle(new_ctx->ndctl_lib_ctx, dimm_handle.handle, &new_ctx->ndctl_lib_dimm)))
		{
			new_ctx->smart_health_fd = ndctl_dimm_get_health_eventfd(new_ctx->ndctl_lib_dimm);
		}
		else
		{
			COMMON_LOG_ERROR("Failed to get dimm by handle.");
			return rc;
		}
	}
	else
	{
		COMMON_LOG_ERROR("Failed to allocate memory for ctx.");
		return NVM_ERR_NOMEMORY;
	}
	return rc;
}

/*
* Free a context previously created by acpi_event_create_ctx.
*
* @param[in] ctx - pointer to a context created by acpi_event_create_ctx
* @return Returns one of the following
*		NVM_SUCCESS
*/
int acpi_event_free_ctx(void * ctx)
{
	if (NULL != ctx)
	{
		struct nvm_dimm_acpi_event_ctx * p_ctx = (struct nvm_dimm_acpi_event_ctx *)ctx;
		ndctl_unref(p_ctx->ndctl_lib_ctx);
		free(ctx);
	}

	return NVM_SUCCESS;
}

/*
* Retrieve the NFIT dimm handle associated with the context.
*
* @param[in] ctx - pointer to a context created by acpi_event_create_ctx
* @param[out] dev_handle - the NFIT dimm handle associated with the context
* @return Returns one of the following
*		NVM_ERR_INVALIDPARAMETER
*		NVM_SUCCESS
*/
int acpi_event_ctx_get_dimm_handle(void * ctx, NVM_NFIT_DEVICE_HANDLE * dev_handle)
{
	struct nvm_dimm_acpi_event_ctx * acpi_event_ctx = (struct nvm_dimm_acpi_event_ctx *)ctx;
	if (NULL != ctx)
	{
		*dev_handle = acpi_event_ctx->dimm_handle;
		return NVM_SUCCESS;
	}
	else
	{
		COMMON_LOG_ERROR("Invalid ctx");
		return NVM_ERR_INVALIDPARAMETER;
	}
}

/*
* Retrieve an ACPI notification state of a DIMM.
*
* @param[in] ctx - pointer to a context created by acpi_event_create_ctx
* @param[in] event_type - which event type to obtain the state for
* @param[out] event_state - the state of the event type
* @return Returns one of the following
*		NVM_ERR_INVALIDPARAMETER
*		NVM_SUCCESS
*/
int acpi_event_get_event_state(void * ctx, enum acpi_event_type event_type, enum acpi_event_state *event_state)
{
	struct nvm_dimm_acpi_event_ctx * acpi_event_ctx = (struct nvm_dimm_acpi_event_ctx *)ctx;
	if (NULL != ctx)
	{
		*event_state = (acpi_event_ctx->triggered_events & (1 << event_type)) ? ACPI_EVENT_SIGNALLED : ACPI_EVENT_NOT_SIGNALLED;
		return NVM_SUCCESS;
	}
	else
	{
		COMMON_LOG_ERROR("Invalid ctx");
		return NVM_ERR_INVALIDPARAMETER;
	}
}

/*
* Set which ACPI events should be monitored.
*
* @param[in] ctx - pointer to a context created by acpi_event_create_ctx
* @param[out] acpi_monitored_event_mask - sets which events to actively monitor
* @return Returns one of the following
*		NVM_ERR_INVALIDPARAMETER
*		NVM_SUCCESS
*
*/
int acpi_event_set_monitor_mask(void * ctx, const unsigned int acpi_monitored_event_mask)
{
	struct nvm_dimm_acpi_event_ctx * acpi_event_ctx = (struct nvm_dimm_acpi_event_ctx *)ctx;
	if (NULL != ctx)
	{
		acpi_event_ctx->monitored_events = acpi_monitored_event_mask;
		return NVM_SUCCESS;
	}
	else
	{
		COMMON_LOG_ERROR("Invalid ctx");
		return NVM_ERR_INVALIDPARAMETER;
	}
}

/*
* Set which ACPI events should be monitored.
*
* @param[in] ctx - pointer to a context created by acpi_event_create_ctx
* @param[out] mask - retrieves bit mask which defines actively monitored events
* @return Returns one of the following
*		NVM_ERR_INVALIDPARAMETER
*		NVM_SUCCESS
*/
int acpi_event_get_monitor_mask(void * ctx, unsigned int * mask)
{
	struct nvm_dimm_acpi_event_ctx * acpi_event_ctx = (struct nvm_dimm_acpi_event_ctx *)ctx;
	if (NULL != ctx)
	{
		*mask = acpi_event_ctx->monitored_events;
		return NVM_SUCCESS;
	}
	else
	{
		COMMON_LOG_ERROR("Invalid ctx");
		return NVM_ERR_INVALIDPARAMETER;
	}
}

/*
* Wait for an asynchronous ACPI notification. This function will return when the timeout expires or an acpi notification
* occurs for any dimm, whichever happens first.
*
*
* @param[in] acpi_event_contexts - Array of contexts
* @param[in] dimm_cnt - Number of contexts in the array
* @param[in] timeout_sec - -1 - No timeout, all other non-negative values represent a second granularity timeout value
* @param[out] event_result - ACPI_EVENT_SIGNALLED_RESULT, ACPI_EVENT_TIMED_OUT_RESULT, ACPI_EVENT_UNKNOWN_RESULT
* @return Returns one of the following
*		NVM_ERR_UNKNOWN
*		NVM_SUCCESS
*/
int acpi_wait_for_event(void * acpi_event_contexts[], const NVM_UINT32 dimm_cnt, const int timeout_sec, enum acpi_get_event_result * event_result)
{
	COMMON_LOG_ENTRY();
	struct nvm_dimm_acpi_event_ctx * context;
	int rc = NVM_ERR_UNKNOWN;
	char buf[4096]; //4k based on ndctl example
	fd_set fds;
	int max_fd = 0;
	FD_ZERO(&fds);

	//add all dimm smart health FDs to the set
	//and re-arm them (lseek/pread)
	for (int i = 0; i < dimm_cnt; ++i)
	{
		context = (struct nvm_dimm_acpi_event_ctx *)acpi_event_contexts[i];
		context->triggered_events = 0;
		int fd = context->smart_health_fd;
		if (fd > max_fd)
			max_fd = fd;

		lseek(fd, 0, SEEK_SET);
		rc = pread(fd, buf, sizeof(buf), 0);
		FD_SET(fd, &fds);
	}

	struct timeval tm;
	tm.tv_sec = timeout_sec;
	tm.tv_usec = 0;
	//wait for event(s), can either have timeout or wait indefinitely for an event
	if (0 < (rc = select(max_fd + 1, NULL, NULL, &fds, ((timeout_sec >= 0) ? &tm : NULL))))
	{
		for (int i = 0; i < dimm_cnt; ++i)
		{
			context = (struct nvm_dimm_acpi_event_ctx *)acpi_event_contexts[i];
			if (FD_ISSET(context->smart_health_fd, &fds))
			{
				context->triggered_events |= DIMM_ACPI_EVENT_SMART_HEALTH_MASK;
				*event_result = ACPI_EVENT_SIGNALLED_RESULT;
			}
		}
	}
	else
	{
		*event_result = (rc == 0 ? ACPI_EVENT_TIMED_OUT_RESULT : ACPI_EVENT_UNKNOWN_RESULT);
	}
	return NVM_SUCCESS;
}
