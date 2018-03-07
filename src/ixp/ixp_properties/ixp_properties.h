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


#ifndef SRC_IXP_PROPERTIES_H_
#define SRC_IXP_PROPERTIES_H_

#include <ixp.h>
#include <export_api.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define PROP_KEY_VALID(k) (k >= 0 && k < IXP_PROP_KEY_MAX)

struct ixp_lookup_t
{
	// For the fis call associated with this ixp_prop_key, populate all
	// relevant properties in props
	int (* f_populate)(unsigned int handle, struct ixp_prop_info props[], unsigned int num_props);
	// For the fis call associated with this ixp_prop_key, free all
	// relevant properties in props
	void (* f_free)(struct ixp_prop_info props[], unsigned int num_props);
	char prop_name[IXP_MAX_PROPERTY_NAME_SZ];
};

int ixp_set_g_ixp_lookup_entry(IXP_PROP_KEY key, struct ixp_lookup_t entry);
int ixp_get_g_ixp_lookup_entry(IXP_PROP_KEY key, struct ixp_lookup_t * entry);


#ifdef __cplusplus
}
#endif

#endif /* SRC_IXP_PROPERTIES_H_ */
