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

#ifndef CR_MGMT_WIN_LEG_ADAPTER_H
#define	CR_MGMT_WIN_LEG_ADAPTER_H

#include "device_adapter.h"

NVM_BOOL win_leg_adp_is_supported_driver_available();

int win_leg_adp_get_vendor_driver_revision(NVM_VERSION version_str, const NVM_SIZE str_len);

int win_leg_adp_get_driver_capabilities(struct nvm_driver_capabilities *p_capabilities);

int win_leg_adp_get_interleave_set_count();

int win_leg_adp_get_interleave_sets(const NVM_UINT32 count,
	struct nvm_interleave_set *p_interleaves);

int win_leg_adp_get_namespace_count();
int win_leg_adp_get_namespaces(const NVM_UINT32 count,
	struct nvm_namespace_discovery *p_namespaces);
int win_leg_adp_get_namespace_details(const NVM_UID namespace_uid,
	struct nvm_namespace_details *p_details);
int win_leg_adp_create_namespace(NVM_UID *p_namespace_uid,
	const struct nvm_namespace_create_settings *p_settings);
int win_leg_adp_delete_namespace(const NVM_UID namespace_uid);
int win_leg_adp_modify_namespace_name(const NVM_UID namespace_uid, const NVM_NAMESPACE_NAME name);
int win_leg_adp_modify_namespace_block_count(const NVM_UID namespace_uid,
	const NVM_UINT64 block_count);
int win_leg_adp_modify_namespace_enabled(const NVM_UID namespace_uid,
	const enum namespace_enable_state enabled);

int win_leg_adp_ioctl_passthrough_cmd(struct fw_cmd *p_cmd);

int win_leg_adp_get_test_result_count(enum driver_diagnostic diagnostic);


int win_leg_adp_run_test(enum driver_diagnostic diagnostic, const NVM_UINT32 count,
	struct health_event *results);

#endif // CR_MGMT_WIN_LEG_ADAPTER_H
