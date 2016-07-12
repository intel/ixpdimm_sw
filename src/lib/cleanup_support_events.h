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
 * Scrub the events in the support DB to remove sensitive data.
 */

#ifndef	CLEANUP_SUPPORT_EVENTS_H_
#define	CLEANUP_SUPPORT_EVENTS_H_

#include <persistence/lib_persistence.h>
#include <nvm_types.h>
#include <nvm_management.h>
#include "event_field_metadata.h"

int change_serial_num_in_events(PersistentStore *p_ps, enum event_type event_type,
		unsigned int event_code);
void clear_serial_num_in_db_event(struct db_event *p_event,
		const struct event_field_metadata *p_metadata);

int change_dimm_uid_in_events(PersistentStore *p_ps);
void obfuscate_dimm_uid_in_db_event(struct db_event *p_event,
		const struct event_field_metadata *p_metadata);
int generate_obfuscated_dimm_uid(const NVM_UID real_uid, NVM_UID obfuscated_uid);

#endif /* CLEANUP_SUPPORT_EVENTS_H_ */
