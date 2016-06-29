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
 * This file declares common definitions and helper functions used throughout the Linux
 * device adapter.
 */

#ifndef LNX_ADAPTER_H_
#define	LNX_ADAPTER_H_

#include <stddef.h>
#include <linux/ndctl.h>
#include <ndctl/libndctl.h>

#define	SYSFS_ATTR_SIZE 1024

int linux_err_to_nvm_lib_err(int);

/*
 * This function is to swap the bytes revieved from the NFIT
 * this is in reference to https://bugzilla.kernel.org/show_bug.cgi?id=121161
 * TODO: remove this function as soon as the above mentioned bug is resolved
 */
unsigned short swap_bytes(unsigned short nfit_val);

int open_ioctl_target(int *p_target, const char *dev_name);
int send_ioctl_command(int fd, unsigned long request, void* parg);

int get_dimm_by_handle(struct ndctl_ctx *ctx, unsigned int handle, struct ndctl_dimm **dimm);

int get_unconfigured_namespace(struct ndctl_namespace **unconfigured_namespace,
	struct ndctl_region *region);

#endif /* LNX_ADAPTER_H_ */
