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

#include "nvm_types.h"
#include "adapter_types.h"

#ifndef NAMESPACE_LABELS_H
#define	NAMESPACE_LABELS_H

/*
 * Retrieve the count of namespaces from the PCD data on all manageable DIMMs
 */
int get_namespace_count_from_pcd();

/*
 * Retrieve namespace discovery information from the PCD data on all manageable DIMMs
 */
int get_namespaces_from_pcd(const NVM_UINT32 count,
		struct nvm_namespace_discovery *p_namespaces);

/*
 * Retrieve namespace details from the PCD data on all manageable DIMMs
 */
int get_namespace_details_from_pcd(const NVM_UID namespace_uid,
		struct nvm_namespace_details *p_details);

/*
 * Zero the namespace label PCD partition on the specified DIMM
 */
int zero_dimm_namespace_labels(const NVM_UINT32 device_handle);

#endif // NAMESPACE_LABELS_H
