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
 * This file defines the interface to shared helper
 * function regarding capabilities
 */

#ifndef CAPABILITIES_H_
#define	CAPABILITIES_H_

#include "device_adapter.h"
#include "nvm_types.h"
#include "nvm_management.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Return the value of the specified nvm feature
 */
#define	VALUEOF_NVM_FEATURE(p_nvm_features, feature_name)	\
	*(NVM_BOOL *)(((char *)p_nvm_features) + offsetof(struct nvm_features, feature_name))

/*
 * Determine if the specified nvm_feature is supported by
 * retrieving the capabilities and checking the specified feature_name.
 */
#define	IS_NVM_FEATURE_SUPPORTED(feature_name)	\
(\
{ \
	struct nvm_capabilities capabilities; \
	int rc = nvm_get_nvm_capabilities(&capabilities); \
	if (rc == NVM_SUCCESS) \
	{ \
		if (VALUEOF_NVM_FEATURE(&capabilities.nvm_features, feature_name) != 1) \
		{ \
			rc = NVM_ERR_NOTSUPPORTED; \
		} \
	} \
	rc; \
} \
)

/*
 * Determine if the specified nvm_feature is supported by
 * retrieving the capabilities and checking the specified feature_name
 * and checking the
 */
#define	IS_NVM_FEATURE_LICENSED(feature_name)	\
(\
{ \
	struct nvm_capabilities capabilities; \
	int rc = nvm_get_nvm_capabilities(&capabilities); \
	if (rc == NVM_SUCCESS) \
	{ \
		if (VALUEOF_NVM_FEATURE(&capabilities.nvm_features, feature_name) != 1) \
		{ \
			rc = NVM_ERR_NOTSUPPORTED; \
		} \
		else \
		{ \
			NVM_BOOL sku_violation; \
			rc = system_in_sku_violation(&capabilities, &sku_violation); \
			if (rc == NVM_SUCCESS && sku_violation) \
			{ \
				rc = NVM_ERR_SKUVIOLATION; \
			} \
		} \
	} \
	rc; \
} \
)

int check_device_app_direct_namespaces_for_sku_violation(
		const struct nvm_capabilities *p_capabilities,
		const NVM_NFIT_DEVICE_HANDLE device_handle, NVM_BOOL *p_sku_violation);

int check_device_storage_namespaces_for_sku_violation(
		const struct nvm_capabilities *p_capabilities,
		const NVM_NFIT_DEVICE_HANDLE device_handle, NVM_BOOL *p_sku_violation);

int device_in_sku_violation(const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct nvm_capabilities *p_capabilities,
		NVM_BOOL *p_sku_violation);

int system_in_sku_violation(const struct nvm_capabilities *p_capabilities,
		NVM_BOOL *p_sku_violation);

int get_devices_is_supported();

#ifdef __cplusplus
}
#endif

#endif /* CAPABILITIES_H_ */
