/*
 * Copyright (c) 2016, Intel Corporation
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
 * Get metadata about event string fields for event with a given type/code.
 */

#ifndef	EVENT_FIELD_METADATA_H_
#define	EVENT_FIELD_METADATA_H_

#include <nvm_management.h>

enum event_arg_type
{
	EVENT_ARG_TYPE_OTHER = 0,
	EVENT_ARG_TYPE_DEVICE_UID,
	EVENT_ARG_TYPE_DEVICE_UID_LIST,
	EVENT_ARG_TYPE_SERIAL_NUM
};

enum event_uid_type
{
	EVENT_UID_TYPE_NONE = 0,
	EVENT_UID_TYPE_DEVICE,
	EVENT_UID_TYPE_NAMESPACE,
	EVENT_UID_TYPE_POOL
};

struct event_field_metadata
{
	enum event_type event_type;
	NVM_UINT32 event_code;
	enum event_uid_type uid_type;
	enum event_arg_type arg1_type;
	enum event_arg_type arg2_type;
	enum event_arg_type arg3_type;
};

#define	EVENT_METADATA_HAS_TYPE_IN_ARG(metadata, arg_type, which_arg) \
	(metadata.arg## which_arg ##_type == arg_type)

struct event_field_metadata get_event_field_metadata(const enum event_type type,
		const NVM_UINT32 code);

NVM_BOOL event_field_metadata_includes_arg_type(const struct event_field_metadata *p_metadata,
		const enum event_arg_type arg_type);

#endif /* EVENT_FIELD_METADATA_H_ */
