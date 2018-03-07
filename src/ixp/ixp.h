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


#ifndef SRC_IXP_H_
#define	SRC_IXP_H_

#include <nvm_types.h>
#include <ixp_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct ixp_context;

#define IXP_PROP_KEY  unsigned int

#define IXP_MAX_PROPERTY_NAME_SZ	256

struct ixp_prop_info
{
	IXP_PROP_KEY prop_key;
	char prop_name[IXP_MAX_PROPERTY_NAME_SZ];
	void *prop_value;
	int prop_value_size;
};


int ixp_create_ctx_nfit_handle(struct ixp_context **ctx, const NVM_NFIT_DEVICE_HANDLE handle, void *user_data);
int ixp_create_ctx_uid(struct ixp_context **ctx, const NVM_UID uid, void *user_data);
int ixp_get_ctx_user_data(struct ixp_context *ctx, void **user_data);
int ixp_free_ctx(struct ixp_context *ctx);
int ixp_init_prop(struct ixp_prop_info *prop, IXP_PROP_KEY prop_key);
int ixp_get_prop_value(struct ixp_prop_info *prop, void **prop_value, unsigned int *prop_value_size);
int ixp_get_prop_name(struct ixp_prop_info *prop, char **prop_name);
int ixp_get_prop(struct ixp_context *ctx, struct ixp_prop_info *prop);
int ixp_free_prop(struct ixp_prop_info *prop);
int ixp_get_props(struct ixp_context *ctx, struct ixp_prop_info *props, unsigned int num_props);
int ixp_free_props(struct ixp_prop_info *props, unsigned int num_props);
int ixp_get_props(struct ixp_context *ctx, struct ixp_prop_info *props, unsigned int num_props);
int ixp_get_prop_key_by_name(char * name, unsigned int length, IXP_PROP_KEY * key);

#ifdef __cplusplus
}
#endif

#endif /* SRC_IXP_H_ */
