/*
 * Copyright (c) 2016 2017, Intel Corporation
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
 * This file declares helper functions which support the native API in
 * configuration of NVM-DIMM capacity.
 */

#ifndef SRC_LIB_CONFIG_GOAL_UTILITIES_H_
#define	SRC_LIB_CONFIG_GOAL_UTILITIES_H_

#include "platform_config_data.h"

#ifdef __cplusplus
extern "C"
{
#endif

int get_devices_from_appdirect_attributes(struct device_discovery *p_devices,
		int num_devices,
		const struct app_direct_attributes *p_qos);

int verify_interleave_set_rules(struct device_discovery *p_devices,
		int num_devices);

int arrange_devices_in_interleave_set_for_pcat2(struct device_discovery *p_devices,
		int num_devices);

void populate_dimm_info_extension_table(struct dimm_info_extension_table *p_dimms_ext,
		NVM_UINT8 pcd_table_revision,
		struct device_discovery *p_devices,
		int num_devices,
		const NVM_UINT64 interleave_set_size,
		const NVM_UINT64 interleave_set_offset);

int populate_dimm_info_extension_tables(struct dimm_info_extension_table *p_dimms_ext,
		const struct app_direct_attributes *p_qos,
		const NVM_UINT8 pcat_revision,
		const NVM_UINT8 pcd_table_revision,
		const NVM_UINT64 interleave_set_size,
		const NVM_UINT64 interleave_set_offset);

#ifdef __cplusplus
}
#endif

#endif /* SRC_LIB_CONFIG_GOAL_UTILITIES_H_ */
