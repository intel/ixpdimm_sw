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
 * This file defines the interface between the platform independent
 * Native API library code and a driver specific implementation.
 */

#ifndef	_DEVICE_ADAPTER_H_
#define	_DEVICE_ADAPTER_H_

#include "nvm_management.h"
#include "adapter_types.h"
#include "device_fw.h"
#include "nvm_types.h"
#include "platform_capabilities.h"
#include "export_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * ------------------------------------------------------------------------------------------------
 * Functions
 * ------------------------------------------------------------------------------------------------
 */

/*
 * Determine if a supported AEP DIMM driver is installed.
 * Returns 1 if a compatible driver is detected, 0 otherwise.
 */
NVM_API NVM_BOOL is_supported_driver_available();

/*
 * Retrieve the vendor specific NVDIMM driver version.
 * @return Returns one of the following return_codes:
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_NOSIMULATOR @n
 * 		#NVM_ERR_UNKNOWN @n
 */
NVM_API int get_vendor_driver_revision(NVM_VERSION version_str, const NVM_SIZE str_len);

/*
 * Get the capabilities of the host platform
 * @param[out] p_capabilities
 * 		A pointer to a target buffer where the system's platform capabilities will be output
 * @return
 */
NVM_API int get_platform_capabilities(struct bios_capabilities *p_capabilities);

/*
 * Get the number of DIMMs in the system's memory topology
 * @return
 * 		Returns the number of elements in the system's memory topology on success
 * 		or one of the following @link #return_code return_codes: @endlink @n
 *			#NVM_ERR_NOTSUPPORTED @n
 *			#NVM_ERR_NOSIMULATOR @n
 *			#NVM_ERR_DRIVERFAILED @n
 *			#NVM_ERR_UNKNOWN @n
 */
NVM_API int get_topology_count();

/*
 * Get the system's memory topology
 * @param[in] count
 * 		The number of @c nvm_topology structs allocated by the caller, which
 * 		@c p_dimm_topo points to
 * @param[out] p_dimm_topo
 * 		A pointer to a pre-allocated buffer of @c count @c nvm_topology structs
 * @return
 * 		Returns the number of elements in the system's memory topology successfully retrieved, up to
 * 		the count provided,	or one of the following @link #return_code return_codes: @endlink @n
 *			#NVM_ERR_NOTSUPPORTED @n
 *			#NVM_ERR_NOSIMULATOR @n
 *			#NVM_ERR_INVALIDPARAMETER @n
 *			#NVM_ERR_DRIVERFAILED @n
 *			#NVM_ERR_UNKNOWN @n
 */
NVM_API int get_topology(const NVM_UINT8 count, struct nvm_topology *p_dimm_topo);

/*
 * Get the details of a specific dimm
 * @param[in] device_handle
 *		The dimm device_handle from the nfit
 * @param[out] p_dimm_details
 * 		A pointer to a nvm_details structure allocated by the caller
 * @return
 * 		Returns a value describing the success or failure of the operation
 * 		@link #return_code return_codes: @endlink @n
 * 			#NVM_SUCCESS @n
 *			#NVM_ERR_NOTSUPPORTED @n
 *			#NVM_ERR_NOSIMULATOR @n
 *			#NVM_ERR_INVALIDPARAMETER @n
 *			#NVM_ERR_DRIVERFAILED @n
 *			#NVM_ERR_UNKNOWN @n
 */
NVM_API int get_dimm_details(NVM_NFIT_DEVICE_HANDLE id, struct nvm_details *p_dimm_details);

/*
 * Get the number of DIMMs in the SMBIOS Type 17 tables.
 * @return
 * 		Returns the number of elements in the SMBIOS Type 17 tables on success
 * 		or one of the following @link #return_code return_codes: @endlink @n
 *			#NVM_ERR_NOTSUPPORTED @n
 *			#NVM_ERR_NOSIMULATOR @n
 *			#NVM_ERR_DRIVERFAILED @n
 *			#NVM_ERR_UNKNOWN @n
 */
NVM_API int get_smbios_inventory_count();

/*
 * Get the list of DIMMs defined in the SMBIOS Type 17 tables.
 * @param[in] count
 * 		The number of @c nvm_details structs allocated by the caller, which
 * 		@c p_smbios_inventory points to
 * @param[out] p_smbios_inventory
 * 		A pointer to a pre-allocated buffer of @c count @c nvm_smbios_inventory structs
 * @return
 * 		Returns the number of elements in the SMBIOS Type 17 tables successfully retrieved, up to
 * 		the count provided,	or one of the following @link #return_code return_codes: @endlink @n
 *			#NVM_ERR_NOTSUPPORTED @n
 *			#NVM_ERR_NOSIMULATOR @n
 *			#NVM_ERR_INVALIDPARAMETER @n
 *			#NVM_ERR_DRIVERFAILED @n
 *			#NVM_ERR_UNKNOWN @n
 */
NVM_API int get_smbios_inventory(const NVM_UINT8 count, struct nvm_details *p_smbios_inventory);

/*
 * Fetch the interleave set count.
 */
NVM_API int get_interleave_set_count();

/*
 * Fetch the list of interleave sets.
 */
NVM_API int get_interleave_sets(const NVM_UINT32 count, struct nvm_interleave_set *p_interleaves);

/*
 * Get the number of existing namespaces
 * @return
 */
NVM_API int get_namespace_count();

/*
 * Get the discovery information for a given number of namespaces
 * @param[in] count
 * 		The size of the array of namespaces.
 * @param[out] p_namespaces
 * 		A pointer to the list of namespace discovery structures
 * @return
 */
NVM_API int get_namespaces(const NVM_UINT32 count,
		struct nvm_namespace_discovery *p_namespaces);

/*
 * Get the details for a specific namespace
 * @param[in] namespace_uid
 * 		The namespace identifier to retrieve.
 * @param[out] p_details
 * 		The detailed namespace information returned
 * @return
 */
NVM_API int get_namespace_details(
		const NVM_UID namespace_uid,
		struct nvm_namespace_details *p_details);

/*
 * Create a new namespace
 * @param[in,out] p_namespace_uid
 * 		uid of the created namespace
 * @param[in] p_settings
 * 		The namespace setting
 * @return
 */
NVM_API int create_namespace(
		NVM_UID *p_namespace_uid,
		const struct nvm_namespace_create_settings *p_settings);

/*
 * Delete an existing namespace
 * @param[in] namespace_uid
 * 		The namespace to delete
 * @return
 */
NVM_API int delete_namespace(const NVM_UID namespace_uid);

/*
 * Modify an existing namespace name property
 * @param[in] namespace_uid
 * 		The namespace to modify
 * @param[in] name
 * 		The new name of the namespace
 * @return
 */
NVM_API int modify_namespace_name(
		const NVM_UID namespace_uid,
		const NVM_NAMESPACE_NAME name);

/*
 * Modify an existing namespace block count property
 * @param[in] namespace_uid
 * 		The namespace to modify
 * @param[in] block_count
 * 		The new block count of the namespace
 * @return
 */
NVM_API int modify_namespace_block_count(
		const NVM_UID namespace_uid,
		const NVM_UINT64 block_count);

/*
 * Modify an existing namespace enabled  property
 * @param[in] namespace_uid
 * 		The namespace to modify
 * @param[in] enabled
 * 		The new enabled state of the namespace
 * @return
 */
NVM_API int modify_namespace_enabled(
		const NVM_UID namespace_uid,
		const enum namespace_enable_state enabled);

/*
 * Return the capabilities supported by the device driver
 * @param[out] p_capabilities
 * @return One of the following:
 * 		NVM_SUCCESS
 * 		NVM_ERR_INVALIDPARAMETER
 * 		NVM_ERR_DRIVERFAILED
 */
NVM_API int get_driver_capabilities(struct nvm_driver_capabilities *p_capabilities);

/*
 * Execute a passthrough IOCTL
 * @param[in] p_cmd
 * 		The command structure for the passthrough command
 * @return
 */
NVM_API int ioctl_passthrough_cmd(struct fw_cmd *p_cmd);

/*
 * Determine if power is limited
 * @param[in] socket_id
 * 		Socket ID that the DIMM belongs too
 * @return
 * 		Error code or whether or not power is limited
 */
NVM_API int get_dimm_power_limited(NVM_UINT16 socket_id);

NVM_API int get_job_count();

/*
 * Get the expected result count of a test run
 */
NVM_API int get_test_result_count(enum driver_diagnostic diagnostic);

/*
 * Run Diagnostic
 */
NVM_API int run_test(enum driver_diagnostic diagnostic, const NVM_UINT32 count,
		struct health_event results[]);

NVM_API int reenumerate_namespaces(NVM_NFIT_DEVICE_HANDLE device_handle);

 /*
 * Initialize the namespace label index to the specified version on the specified DIMM
 */
NVM_API int init_label_dimm(const NVM_UINT32 device_handle, NVM_UINT16 major_version, NVM_UINT16  minor_version);

NVM_API void nvm_topology_to_device(struct nvm_topology *p_topology, struct device_discovery *p_device);

NVM_API int acpi_event_create_ctx(NVM_NFIT_DEVICE_HANDLE dimm_handle, void ** ctx);
NVM_API int acpi_event_free_ctx(void * ctx);
NVM_API int acpi_wait_for_event(void * acpi_event_contexts[], const NVM_UINT32 dimm_cnt, const int timeout_sec, enum acpi_get_event_result * event_result);
NVM_API int acpi_event_get_event_state(void * ctx, enum acpi_event_type event_type, enum acpi_event_state *event_state);
NVM_API int acpi_event_get_monitor_mask(void * ctx, unsigned int * mask);
NVM_API int acpi_event_ctx_get_dimm_handle(void * ctx, NVM_NFIT_DEVICE_HANDLE * dev_handle);
NVM_API int acpi_event_set_monitor_mask(void * ctx, const unsigned int acpi_monitored_event_mask);
#ifdef __cplusplus
}
#endif

#endif /* _DEVICE_ADAPTER_H_ */
