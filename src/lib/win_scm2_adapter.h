/*
 * Copyright (c) 2017, Intel Corporation
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

#ifndef CR_MGMT_WIN_SCM2_ADAPTER_H
#define	CR_MGMT_WIN_SCM2_ADAPTER_H

#include "nvm_types.h"
#include "fis_types.h"
#include "adapter_types.h"

#define	SCM_LOG_ENTRY()
#define	SCM_LOG_ERROR(str)
#define	SCM_LOG_ERROR_F(fmt, ...)
#define	SCM_LOG_EXIT_RETURN_I(i)

#define	WIN_SCM2_IS_SUCCESS(rc) (rc) == WIN_SCM2_SUCCESS
enum WIN_SCM2_RETURN_CODES
{
	WIN_SCM2_SUCCESS = 0,
	WIN_SCM2_ERR_UNKNOWN = -1,
	WIN_SCM2_ERR_DRIVERFAILED = -2,
	WIN_SCM2_ERR_NOMEMORY = -3,
};

NVM_BOOL win_scm_adp_is_supported_driver_available();

int win_scm_adp_get_vendor_driver_revision(NVM_VERSION version_str, const NVM_SIZE str_len);

int win_scm_adp_ioctl_passthrough_cmd(struct fw_cmd *p_cmd);

int win_scm_adp_get_driver_capabilities(struct nvm_driver_capabilities *p_caps);

int win_scm2_get_namespace_count();

int win_scm2_get_namespaces(const NVM_UINT32 count,
	struct nvm_namespace_discovery *p_namespaces);

int win_scm2_get_namespace_details(
	const NVM_UID namespace_uid,
	struct nvm_namespace_details *p_details);

#endif // CR_MGMT_WIN_SCM2_ADAPTER_H
