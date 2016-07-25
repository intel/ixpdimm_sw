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
 * This file declares support/helper functions and structures for device management
 * in the native API.
 */

#ifndef	DEVICE_UTILITIES_H_
#define	DEVICE_UTILITIES_H_

#include "nvm_management.h"
#include "device_fw.h"
#include "device_adapter.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Convert a FW version array to a string
 */
#define	FW_VER_ARR_TO_STR(arr, str, len) \
	build_revision(str, len, \
			((((arr[4] >> 4) & 0xF) * 10) + (arr[4] & 0xF)), \
			((((arr[3] >> 4) & 0xF) * 10) + (arr[3] & 0xF)), \
			((((arr[2] >> 4) & 0xF) * 10) + (arr[2] & 0xF)), \
			(((arr[1] >> 4) & 0xF) * 1000) + (arr[1] & 0xF) * 100 + \
			(((arr[0] >> 4) & 0xF) * 10) + (arr[0] & 0xF));

int exists_and_manageable(const NVM_UID device_uid, struct device_discovery *p_dev,
		NVM_BOOL check_manageability);

int calculate_device_uid(struct device_discovery *p_device);
void calculate_uid_without_manufacturing_info(NVM_UID uid,
		NVM_UINT16 vendor_id, NVM_SERIAL_NUMBER serial_number);
void calculate_uid_with_valid_manufacturing_info(NVM_UID uid,
		NVM_UINT16 vendor_id, NVM_SERIAL_NUMBER serial_number,
		NVM_UINT8 manufacturing_loc, NVM_UINT16 manufacturing_date);

int lookup_dev_uid(const NVM_UID dev_uid, struct device_discovery *p_dev);

int lookup_dev_handle(const NVM_NFIT_DEVICE_HANDLE dev_handle, struct device_discovery *p_dev);

int lookup_dev_manufacturer_serial_model(const unsigned char *manufacturer,
		const unsigned char *serial_number, const char *model_number,
		struct device_discovery *p_dev);

void free_dev_table(COMMON_BOOL obtain_lock);

int init_dev_table(COMMON_BOOL obtain_lock, struct nvm_topology *p_dimm_topo, NVM_UINT8 count);

/*
 * Helper functions to get ars status
 */
int get_ars_status(const NVM_NFIT_DEVICE_HANDLE dimm_handle, enum device_ars_status *p_ars_status);

enum device_ars_status translate_to_ars_status(struct pt_payload_long_op_stat *long_op_payload);

/*
 * Derive DIMM manageability state from firmware
 */
void set_device_manageability_from_firmware(
	struct pt_payload_identify_dimm *p_id_dimm,
	enum manageability_state *p_manageability);

/*
 * Check device interface format code against those supported by mgmt sw
 */
NVM_BOOL is_device_interface_format_supported(struct device_discovery *p_device);

/*
 * Check device memory subsystem controller information against those supported by mgmt sw
 */
NVM_BOOL is_device_subsystem_controller_supported(struct device_discovery *p_device);

/*
 * Check whether the device is a type we can expect to respond to firmware commands
 */
NVM_BOOL can_communicate_with_device_firmware(struct device_discovery *p_device);

/*
 * Compare the firmware api version to the supported versions to determine
 * if the device is manageable.
 */
int check_firmware_revision(unsigned char fw_api_version);

/*
 * Helper function to get the health of a dimm. Used in get pools to roll up health info
 * to a pool
 */
int get_dimm_health(NVM_NFIT_DEVICE_HANDLE device_handle, enum device_health *p_health);

/*
 * Helper function to convert SMART health status to device health
 */
enum device_health smart_health_status_to_device_health(enum smart_health_status smart_health);

/*
 * Helper function to convert firmware type into firmware type enumeration
 */
enum device_fw_type firmware_type_to_enum(unsigned char fw_type);

/*
 * Helper function to get the partition info for a DIMM.
 */
int get_partition_info(const NVM_NFIT_DEVICE_HANDLE device_handle,
	struct pt_payload_get_dimm_partition_info *p_pi);

/*
 * Utility function to get DIMM manageability
 */
int get_dimm_manageability(const NVM_NFIT_DEVICE_HANDLE device_handle,
		enum manageability_state *p_manageability);

/*
 * Utility function to determine if a device is manageable
 */
int is_device_manageable(const NVM_NFIT_DEVICE_HANDLE handle);

/*
 * Determine if a device is erase capable depending on its security capabilities.
 */
NVM_BOOL device_is_erase_capable(struct device_security_capabilities security_capabilities);

/*
 * Determine if a device is encryption enabled depending on its lock state.
 */
NVM_BOOL device_is_encryption_enabled(enum lock_state lock_state);

void calculate_device_capabilities(struct device_discovery *p_dev);

void convert_sku_to_device_capabilities(const int sku_bits,
		struct device_capabilities *p_capabilities);

void map_sku_security_capabilities(unsigned int dimm_sku,
	struct device_security_capabilities *p_security_capabilities);

enum memory_type get_memory_type_from_smbios_memory_type(const NVM_UINT8 smbios_type,
		const NVM_UINT16 smbios_type_detail_bits);

enum device_form_factor get_device_form_factor_from_smbios_form_factor(
		const NVM_UINT16 smbios_form_factor);

int get_dimm_capacities(const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct nvm_capabilities *p_capabilities,
		struct device_capacities *p_capacities);

int dimm_has_namespaces_of_type(const NVM_NFIT_DEVICE_HANDLE dimm_handle,
		const enum namespace_type ns_type);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_UTILITIES_H_ */
