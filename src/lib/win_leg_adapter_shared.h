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
 * This file declares common definitions and functions used throughout the Windows
 * driver adapter.
 */

#ifndef SRC_LIB_WIN_ADAPTER_H_
#define	SRC_LIB_WIN_ADAPTER_H_

#include <windows.h>
#include <windows/PrivateIoctlDefinitions.h>
#include <device_fw.h>

// SCSI port used as IOCTL target
extern short g_scsi_port;

int ind_err_to_nvm_lib_err(CR_RETURN_CODES ind_err);

int init_scsi_port();
int open_ioctl_target(PHANDLE p_handle, short scsi_port);
int send_ioctl_command(HANDLE handle,
		unsigned long io_controlcode,
		void *p_in_buffer,
		size_t in_size,
		void *p_out_buffer,
		size_t out_size);
int execute_ioctl(size_t bufSize, void *p_ioctl_data, unsigned long io_controlcode);

static inline unsigned int win_dsm_status_to_int(DSM_STATUS win_dsm_status)
{
	return win_dsm_status.BackgroundOperationState << DSM_BACKGROUND_OP_STATE_SHIFT
		| win_dsm_status.MailboxStatusCode << DSM_MAILBOX_ERROR_SHIFT
		| win_dsm_status.DsmStatus << DSM_VENDOR_ERROR_SHIFT;
}

void win_guid_to_uid(const GUID guid, COMMON_UID uid);
void win_uid_to_guid(const COMMON_UID uid, GUID *p_guid);

#endif /* SRC_LIB_WIN_ADAPTER_H_ */
