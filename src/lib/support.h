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
 * This file defines the structures and functions specific to generating a support file.
 */

#ifndef	_SUPPORT_H_
#define	_SUPPORT_H_

#include <export_api.h>

#ifdef __cplusplus
extern "C"
{
#endif

NVM_API int change_serial_num_in_topology_state(PersistentStore *p_ps);
NVM_API int change_serial_num_in_interleave_set_dimm_info(PersistentStore *p_ps);
NVM_API int change_serial_num_in_identify_dimm(PersistentStore *p_ps);
NVM_API int change_hostname_in_host(PersistentStore *p_ps);

/*
 * Enum values must be a bitmask, unique, non-overlapping values correlating to
 * SQL_KEY_GATHER_SUPPORT_FILTER
 */
enum gather_support_filters
{
	GSF_HOST_DATA =			(1 << 0),	// clear out host and host_history tables
	GSF_NAMESPACE_DATA =		(1 << 1),	// clear out namespace & namespace_history tables
	GSF_SYSTEM_LOG =		(1 << 2),	// clear out system log
	GSF_SERIAL_NUMS =		(1 << 3)	// clear out all serial numbers
};


#ifdef __cplusplus
}
#endif

#endif /* _SUPPORT_H_ */
