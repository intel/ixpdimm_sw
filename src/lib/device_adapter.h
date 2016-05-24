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

#include "device_fw.h"
#include "nvm_management.h"
#include "platform_capabilities.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define	MAX_NUMBER_OF_BLOCK_SIZES 16
#define	MAX_DIAGNOSTIC_RESULTS 1024

/*
 * ------------------------------------------------------------------------------------------------
 * Enums
 * ------------------------------------------------------------------------------------------------
 */

#define	NVM_DETAILS_SIZE_UNKNOWN	(NVM_UINT64)-1

/*
 * Describes the memory type associated with the DIMM
 * From DMTF SMBIOS spec
 * 7.18.2 Memory Device - Type
 */
enum smbios_memory_type
{
	// TODO: from SMBIOS 7.18.2 Memory Device <97> Type, NGNVM value TBD
	SMBIOS_MEMORY_TYPE_NVMDIMM = 0x15,
	SMBIOS_MEMORY_TYPE_DDR4 = 0x1A
};

enum smbios_memory_type_detail
{
	SMBIOS_MEMORY_TYPE_DETAIL_NONVOLATILE = 1 << 12
};

enum smbios_form_factor
{
	SMBIOS_FORM_FACTOR_DIMM = 0x9,
	SMBIOS_FORM_FACTOR_SODIMM = 0xD
};

/*
 * Defines driver specific diagnostics
 */
enum driver_diagnostic
{
	DRIVER_DIAGNOSTIC_PM_METADATA_CHECK = 0
};

/*
 * Defines type of the result from a driver metadata check diagnostic
 */
enum driver_diagnostic_health_event_type
{

	HEALTH_EVENT_TYPE_UNKNOWN = 0,
	HEALTH_EVENT_TYPE_NAMESPACE = 1,
	HEALTH_EVENT_TYPE_LABEL_AREA = 2
};

/*
 * Defines health status for namespace related metadata
 */
enum ns_health_result
{
	NAMESPACE_HEALTH_RESULT_OK = 0,
	NAMESPACE_HEALTH_RESULT_MISSING,
	NAMESPACE_HEALTH_RESULT_MISSING_LABEL,
	NAMESPACE_HEALTH_RESULT_CORRUPT_INTERLEAVE_SET,
	NAMESPACE_HEALTH_RESULT_INCONSISTENT_LABELS,
	NAMESPACE_HEALTH_RESULT_INVALID_BLOCK_SIZE,
	NAMESPACE_HEALTH_RESULT_CORRUPT_BTT_METADATA
};

/*
 * Defines health status for label area related metadata
 */
enum label_area_health_result
{
	LABEL_AREA_HEALTH_RESULT_OK = 0,
	LABEL_AREA_HEALTH_RESULT_MISSING_PCD,
	LABEL_AREA_HEALTH_RESULT_MISSING_VALID_INDEX_BLOCK,
	LABEL_AREA_HEALTH_RESULT_INSUFFICIENT_PERSISTENT_MEMORY,
	LABEL_AREA_HEALTH_RESULT_LABELS_OVERLAP
};

/*
 * ------------------------------------------------------------------------------------------------
 * Structures
 * ------------------------------------------------------------------------------------------------
 */
/*
 * Feature flags for the driver
 * Indicate whether a given feature is enabled or disabled by the driver.
 */
struct driver_feature_flags
{
	NVM_UINT32 get_platform_capabilities : 1;
	NVM_UINT32 get_topology : 1;
	NVM_UINT32 get_interleave : 1;
	NVM_UINT32 get_dimm_detail : 1;
	NVM_UINT32 get_namespaces : 1;
	NVM_UINT32 get_namespace_detail : 1;
	NVM_UINT32 get_address_scrub_data : 1;
	NVM_UINT32 get_platform_config_data : 1;
	NVM_UINT32 get_boot_status : 1;
	NVM_UINT32 get_power_data : 1;
	NVM_UINT32 get_security_state : 1;
	NVM_UINT32 get_log_page : 1;
	NVM_UINT32 get_features : 1;
	NVM_UINT32 set_features : 1;
	NVM_UINT32 create_namespace : 1;
	NVM_UINT32 rename_namespace : 1;
	NVM_UINT32 grow_namespace : 1;
	NVM_UINT32 shrink_namespace : 1;
	NVM_UINT32 delete_namespace : 1;
	NVM_UINT32 enable_namespace : 1;
	NVM_UINT32 disable_namespace : 1;
	NVM_UINT32 set_security_state : 1;
	NVM_UINT32 enable_logging : 1;
	NVM_UINT32 run_diagnostic : 1;
	NVM_UINT32 set_platform_config : 1;
	NVM_UINT32 passthrough : 1;
	NVM_UINT32 start_address_scrub : 1;
	NVM_UINT32 app_direct_mode : 1;
	NVM_UINT32 storage_mode : 1;
};

/*
 * Capabilities of the driver
 */
struct nvm_driver_capabilities
{
	NVM_UINT32 block_sizes[MAX_NUMBER_OF_BLOCK_SIZES]; // in bytes
	NVM_UINT32 num_block_sizes;
	NVM_UINT64 min_namespace_size; // in bytes
	NVM_BOOL namespace_memory_page_allocation_capable;
	struct driver_feature_flags features;
};

/*
 * Describes the system-level view of a physical memory module's properties.
 */
struct nvm_topology {
	NVM_NFIT_DEVICE_HANDLE device_handle;
	NVM_UINT16 id; /* The SMBIOS physical id of the memory device */
	NVM_UINT16 vendor_id; /* The vendor identifier */
	NVM_UINT16 device_id; /* The device identifier */
	NVM_UINT16 revision_id; /* The revision identifier */

	NVM_UINT16 subsystem_vendor_id;		// vendor identifier of the AEP DIMM non-volatile
										// memory subsystem controller
	NVM_UINT16 subsystem_device_id;		// device identifier of the AEP DIMM non-volatile
										// memory subsystem controller
	NVM_UINT16 subsystem_revision_id; 	// revision identifier of the AEP DIMM non-volatile
										// memory subsystem controller
	NVM_BOOL manufacturing_info_valid;	// manufacturing location and date validity
	NVM_UINT8 manufacturing_location;	// AEP DIMM manufacturing location assigned by vendor
										// only valid if manufacturing_info_valid=1
	NVM_UINT16 manufacturing_date;		// Date the AEP DIMM was manufactured, assigned by vendor
										// only valid if manufacturing_info_valid=1

	NVM_UINT8 type; /* From SMBIOS 7.18.2 Memory Device <97> Type */
	NVM_UINT16 fmt_interface_codes[NVM_MAX_IFCS_PER_DIMM]; /* The device type(s) from NFIT */
};

/*
 * Detailed information about a specific DIMM including information from the
 *  SMBIOS Type 17 tables
 */
struct nvm_details {
	NVM_UINT16 id; // SMBIOS handle
	NVM_UINT8 type; // SMBIOS memory type
	NVM_UINT32 type_detail_bits; // SMBIOS memory type detail bitfield
	NVM_UINT16 form_factor; /* DIMM Form Factor */
	/* Width in bits used to store user data */
	NVM_UINT64 data_width;
	NVM_UINT64 total_width; /* Width in bits for data and ECC */
	NVM_UINT64 size; //!< raw size reported by SMBIOS in bytes
	NVM_UINT64 speed; /* Speed in MHz */
	char part_number[NVM_PART_NUM_LEN]; /* DIMM part number */
	/* Socket or board pos */
	char device_locator[NVM_DEVICE_LOCATOR_LEN];
	/* Bank label */
	char bank_label[NVM_BANK_LABEL_LEN];
	char manufacturer[NVM_MANUFACTURERSTR_LEN]; // SMBIOS manufacturer string
};

/*
 * Interleave set information
 */
struct nvm_interleave_set
{
	NVM_UINT64 size; // the size of the interleave set in bytes
	NVM_UINT64 available_size; // Available capacity in the interleave set in bytes
	NVM_UINT8 socket_id; // The socket where the interleave set resides
	struct interleave_format settings; // The interleave format
	NVM_UINT8 dimm_count; // The number of dimm in the interleave set
	// Unique ids of the dimms in the interleave set
	NVM_NFIT_DEVICE_HANDLE dimms[NVM_MAX_DEVICES_PER_SOCKET];
	NVM_UINT32 set_index; // Unique identifier from the PCD
	NVM_UINT32 driver_id; // Unique identifier from the NFIT
	NVM_BOOL mirrored; // Whether the interleave set it mirrored
};

/*
 * Describes the features pertaining to the unique construction and usage of a
 * pool of memory.
 */
struct nvm_pool
{
	NVM_UID pool_uid; // Unique identifier of the pool.
	enum pool_type type; // The type of pool.
	NVM_UINT64 capacity; // Size of the pool in bytes.
	NVM_UINT64 free_capacity; // Available size of the pool in bytes.
	NVM_INT16 socket_id; // The processor socket identifier.  -1 for system level pool.
	NVM_UINT8 dimm_count; // number of dimms in pool
	NVM_UINT8 ilset_count; // the number of interleave sets in the pool
	// The memory capacities of the dimms in the pool
	NVM_UINT64 memory_capacities[NVM_MAX_DEVICES_PER_POOL];
	// The raw capacities of the dimms in the pool in bytes
	NVM_UINT64 raw_capacities[NVM_MAX_DEVICES_PER_POOL];
	// The storage ONLY capacities of the dimms in the pool in bytes
	NVM_UINT64 storage_capacities[NVM_MAX_DEVICES_PER_POOL];
	NVM_NFIT_DEVICE_HANDLE dimms[NVM_MAX_DEVICES_PER_POOL]; // Unique ID's of underlying NVM-DIMMs.
	// The interleave sets in this pool
	struct nvm_interleave_set ilsets[NVM_MAX_DEVICES_PER_SOCKET * 2];
	enum pool_health health; // The overall health of the pool
};

/*
 * Describes the storage capacity (total and available) of a DIMM.
 */
struct nvm_storage_capacities
{
	NVM_NFIT_DEVICE_HANDLE device_handle; // DIMM handle
	NVM_UINT64 total_storage_capacity; // total storage capacity - storage-only + App Direct
	NVM_UINT64 free_storage_capacity; // portion of total storage capacity not used by a namespace
	NVM_UINT64 storage_only_capacity; // storage-only capacity - unmapped
};

/*
 * A lightweight description of a namespace.
 */
struct nvm_namespace_discovery
{
	NVM_UID namespace_uid; // Unique identifier of the namespace
	char friendly_name[NVM_NAMESPACE_NAME_LEN]; // Human assigned name of the namespace
};
/*
 * Detailed namespace information
 */
struct nvm_namespace_details
{
	struct nvm_namespace_discovery discovery; // Discovery information
	NVM_UINT32 block_size; // The size of a block in bytes
	NVM_UINT64 block_count; // The number of blocks in the namespace
	enum namespace_type type; // The type of namespace
	enum namespace_health health; // Health of the underlying NVM-DIMMs
	enum namespace_enable_state enabled; // Exposure to OS
	NVM_BOOL btt; // optimized for speed
	union
	{
		NVM_NFIT_DEVICE_HANDLE device_handle; // Used when creating a Storage Namespace
		NVM_UINT32 interleave_setid; // Used when creating an App Direct Namespace
	} namespace_creation_id; // the identifier used by the driver when creating a Namespace
	enum namespace_memory_page_allocation memory_page_allocation;
};

/*
 * Caller specified settings for creating a new namespace.
 */
struct nvm_namespace_create_settings
{
	char friendly_name[NVM_NAMESPACE_NAME_LEN]; // User supplied friendly name.
	NVM_UINT16 block_size; // Block size in bytes.
	NVM_UINT64 block_count; // The number of blocks.
	enum namespace_type type; // The type of namespace.
	enum namespace_enable_state enabled; // If the namespace is exposed to OS after creation.
	NVM_BOOL btt; // optimized for speed
	union
	{
		NVM_NFIT_DEVICE_HANDLE device_handle; // Used when creating a Storage Namespace
		NVM_UINT32 interleave_setid; // Used when creating a App Direct Namespace
	} namespace_creation_id; // the identifier used by the driver when creating a Namespace
	enum namespace_memory_page_allocation memory_page_allocation;
};

/*
 * Caller specified settings for modifying a namespace
 */
struct nvm_namespace_modify_settings
{
	char friendly_name[NVM_NAMESPACE_NAME_LEN]; // User supplied friendly name.
	NVM_UINT64 block_count; // The number of blocks.
	enum namespace_enable_state enabled; // If the namespace is exposed to OS
};

/*
 * Result for a namespace metadata check
 */
struct namespace_health_event
{
	enum ns_health_result health_flag;
	NVM_UID namespace_uid;
};

/*
 * Result for a label area metadata check
 */
struct label_area_health_event
{
	enum label_area_health_result health_flag;
	NVM_UINT32 device_handle;
};

/*
 * Result of a driver metadata diagnostic check
 */
struct health_event
{
	enum driver_diagnostic_health_event_type event_type;
	union
	{
		struct namespace_health_event namespace_event;
		struct label_area_health_event label_area_event;
	} health;
};

/*
 * ------------------------------------------------------------------------------------------------
 * Functions
 * ------------------------------------------------------------------------------------------------
 */

/*
 * Determine if a supported AEP DIMM driver is installed.
 * Returns 1 if a compatible driver is detected, 0 otherwise.
 */
NVM_BOOL is_supported_driver_available();

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
int get_vendor_driver_revision(NVM_VERSION version_str, const NVM_SIZE str_len);

/*
 * Get the capabilities of the host platform
 * @param[out] p_capabilities
 * 		A pointer to a target buffer where the system's platform capabilities will be output
 * @param[in] cap_len
 *		Size of the buffer
 * @return
 */
int get_platform_capabilities(struct bios_capabilities *p_capabilities, const NVM_UINT32 cap_len);

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
int get_topology_count();

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
int get_topology(const NVM_UINT8 count, struct nvm_topology *p_dimm_topo);

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
int get_dimm_details(NVM_NFIT_DEVICE_HANDLE id, struct nvm_details *p_dimm_details);

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
int get_smbios_inventory_count();

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
int get_smbios_inventory(const NVM_UINT8 count, struct nvm_details *p_smbios_inventory);

/*
 * Fetch the interleave set count.
 */
int get_interleave_set_count();

/*
 * Fetch the list of interleave sets.
 */
int get_interleave_sets(const NVM_UINT32 count, struct nvm_interleave_set *p_interleaves);

/*
 * Get the number of existing namespaces
 * @return
 */
int get_namespace_count();

/*
 * Get the discovery information for a given number of namespaces
 * @param[in] count
 * 		The size of the array of namespaces.
 * @param[out] p_namespaces
 * 		A pointer to the list of namespace discovery structures
 * @return
 */
int get_namespaces(const NVM_UINT32 count,
		struct nvm_namespace_discovery *p_namespaces);

/*
 * Get the details for a specific namespace
 * @param[in] namespace_uid
 * 		The namespace identifier to retrieve.
 * @param[out] p_details
 * 		The detailed namespace information returned
 * @return
 */
int get_namespace_details(
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
int create_namespace(
		NVM_UID *p_namespace_uid,
		const struct nvm_namespace_create_settings *p_settings);

/*
 * Delete an existing namespace
 * @param[in] namespace_uid
 * 		The namespace to delete
 * @return
 */
int delete_namespace(const NVM_UID namespace_uid);

/*
 * Modify an existing namespace name property
 * @param[in] namespace_uid
 * 		The namespace to modify
 * @param[in] name
 * 		The new name of the namespace
 * @return
 */
int modify_namespace_name(
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
int modify_namespace_block_count(
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
int modify_namespace_enabled(
		const NVM_UID namespace_uid,
		const enum namespace_enable_state enabled);

/*
 * Retrieve the storage capacities of all DIMMs
 */
int get_dimm_storage_capacities(const NVM_UINT32 count,
		struct nvm_storage_capacities *p_capacities);

/*
 * Return the capabilities supported by the device driver
 * @param[out] p_capabilities
 * @return One of the following:
 * 		NVM_SUCCESS
 * 		NVM_ERR_INVALIDPARAMETER
 * 		NVM_ERR_DRIVERFAILED
 */
int get_driver_capabilities(struct nvm_driver_capabilities *p_capabilities);

/*
 * Execute a passthrough IOCTL
 * @param[in] p_cmd
 * 		The command structure for the passthrough command
 * @return
 */
int ioctl_passthrough_cmd(struct fw_cmd *p_cmd);

/*
 * Determine if power is limited
 * @param[in] socket_id
 * 		Socket ID that the DIMM belongs too
 * @return
 * 		Error code or whether or not power is limited
 */
int get_dimm_power_limited(NVM_UINT16 socket_id);

int get_job_count();

/*
 * Get the expected result count of a test run
 */
int get_test_result_count(enum driver_diagnostic diagnostic);

/*
 * Run Diagnostic
 */
int run_test(enum driver_diagnostic diagnostic, const NVM_UINT32 count,
		struct health_event results[]);



#ifdef __cplusplus
}
#endif

#endif /* _DEVICE_ADAPTER_H_ */
