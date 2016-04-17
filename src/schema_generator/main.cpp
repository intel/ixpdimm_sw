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
 * This file contains schema for the NVM Native API configuration database.
 * It is used by the DB schema generator to produce the sqlite DB interface.
 */

#include <stdbool.h>

#include <CrudSchemaGenerator.h>
#include <vector>

/*!
 * Entry point for the library specific Schema Generator.
 * Format should be SchemaGenerator.exe path_to_template_files path_to_write_generated_files
 * @return 0
 * @details
 * This is where the schema model is defined.  See @ref Entity and @ref Attribute for details on
 * how to define the schema model.
 */
int main(int arg_count, char **args)
{
	std::vector<Entity> entities;

	/*
	 * Configuration
	 */
	Entity config("config", "User preferences and configuration settings.");
	config.addAttribute("key").isText(256).isPk();
	config.addAttribute("value").isText(1024);
	entities.push_back(config);

	/*
	 * Logging
	 */
	Entity log("log", "Software debug logs.");
	log.addAttribute("id").isInt32().isPk(true).orderByDesc();
	log.addAttribute("thread_id").isInt64().isUnsigned();
	log.addAttribute("time").isInt64().isUnsigned();
	log.addAttribute("level").isInt32();
	log.addAttribute("file_name").isText(1024);
	log.addAttribute("line_number").isInt32().isUnsigned();
	log.addAttribute("message").isText(2048);
	entities.push_back(log);

	/*
	 * Events
	 */
	Entity event("event", "Software generated events and diagnostic results.");
	event.addAttribute("id").isInt32().isPk(true).orderByDesc();
	// not really a FK but creates helpful functions
	event.addAttribute("type").isInt32().isUnsigned().isFk("event_type", "type");
	event.addAttribute("severity").isInt32().isUnsigned();
	event.addAttribute("code").isInt32().isUnsigned();
	event.addAttribute("action_required").isInt32().isUnsigned();
	event.addAttribute("guid").isText(37);
	event.addAttribute("time").isInt64().isUnsigned();
	event.addAttribute("arg1").isText(1024);
	event.addAttribute("arg2").isText(1024);
	event.addAttribute("arg3").isText(1024);
	event.addAttribute("diag_result").isInt32().isUnsigned();
	entities.push_back(event);

	/*
	 * Topology state - for detecting topology changes
	 */
	Entity topology_state("topology_state", "Monitor stored topology for detecting topology changes on restart.");
	topology_state.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	topology_state.addAttribute("guid").isText(37);
	topology_state.addAttribute("manufacturer").isInt32().isUnsigned();
	topology_state.addAttribute("serial_num").isInt32().isUnsigned();
	topology_state.addAttribute("model_num").isText(21);
	topology_state.addAttribute("current_config_status").isInt32();
	topology_state.addAttribute("config_goal_status").isInt32();
	entities.push_back(topology_state);

	/*
	 * Simulated System
	 */
	Entity host("host", "Host server information.");
	host.includesHistory();
	host.addAttribute("name").isText(256).isPk();
	host.addAttribute("os_type").isInt32();
	host.addAttribute("os_name").isText(256);
	host.addAttribute("os_version").isText(256);
	entities.push_back(host);

	Entity sw_inventory("sw_inventory", "Host software inventory.");
	sw_inventory.includesHistory();
	sw_inventory.addAttribute("name").isText(256).isPk();
	sw_inventory.addAttribute("mgmt_sw_rev").isText(25);
	sw_inventory.addAttribute("vendor_driver_rev").isText(25);
	entities.push_back(sw_inventory);

	Entity socket("socket", "Processor socket information.");
	socket.includesHistory();
	socket.addAttribute("socket_id").isInt32().isUnsigned().isPk();
	socket.addAttribute("type").isInt32().isUnsigned();
	socket.addAttribute("model").isInt32().isUnsigned();
	socket.addAttribute("brand").isInt32().isUnsigned();
	socket.addAttribute("family").isInt32().isUnsigned();
	socket.addAttribute("stepping").isInt32().isUnsigned();
	socket.addAttribute("manufacturer").isText(32);
	socket.addAttribute("logical_processor_count").isInt32().isUnsigned();
	socket.addAttribute("rapl_limited").isInt32().isUnsigned();
	entities.push_back(socket);

	Entity runtime_config_validation("runtime_config_validation", "Platform Configuration Attributes Table (PCAT): Re-Configuration input validation Interface Table - Type 2.");
	runtime_config_validation.includesHistory();
	runtime_config_validation.addAttribute("id").isIndexPk();
	runtime_config_validation.addAttribute("type").isInt32().isUnsigned();
	runtime_config_validation.addAttribute("length").isInt32().isUnsigned();
	runtime_config_validation.addAttribute("address_space_id").isInt32().isUnsigned();
	runtime_config_validation.addAttribute("bit_width").isInt32().isUnsigned();
	runtime_config_validation.addAttribute("bit_offset").isInt32().isUnsigned();
	runtime_config_validation.addAttribute("access_size").isInt32().isUnsigned();
	runtime_config_validation.addAttribute("address").isInt64().isUnsigned();
	runtime_config_validation.addAttribute("operation_type_1").isInt32().isUnsigned();
	runtime_config_validation.addAttribute("value").isInt64().isUnsigned();
	runtime_config_validation.addAttribute("mask_1").isInt64().isUnsigned();
	runtime_config_validation.addAttribute("gas_structure").isInt32().isUnsigned().isArray(12);
	runtime_config_validation.addAttribute("operation_type_2").isInt32().isUnsigned();
	runtime_config_validation.addAttribute("mask_2").isInt64().isUnsigned();
	entities.push_back(runtime_config_validation);

	Entity interleave_cap("interleave_capability", "Platform Configuration Attributes Table (PCAT): Memory Interleave Capability Information Table - Type 1.");
	interleave_cap.includesHistory();
	interleave_cap.addAttribute("id").isIndexPk();
	interleave_cap.addAttribute("type").isInt32().isUnsigned();
	interleave_cap.addAttribute("length").isInt32().isUnsigned();
	interleave_cap.addAttribute("memory_mode").isInt32().isUnsigned();
	interleave_cap.addAttribute("interleave_alignment_size").isInt32().isUnsigned();
	interleave_cap.addAttribute("supported_interleave_count").isInt32().isUnsigned();
	interleave_cap.addAttribute("interleave_format_list").isInt32().isUnsigned().isArray(32);
	entities.push_back(interleave_cap);

	Entity platform_info_cap("platform_info_capability", "Platform Configuration Attributes Table (PCAT): Platform Capability Information Table - Type 0.");
	platform_info_cap.includesHistory();
	platform_info_cap.addAttribute("id").isIndexPk();
	platform_info_cap.addAttribute("type").isInt32().isUnsigned();
	platform_info_cap.addAttribute("length").isInt32().isUnsigned();
	platform_info_cap.addAttribute("mgmt_sw_config_support").isInt32().isUnsigned();
	platform_info_cap.addAttribute("mem_mode_capabilities").isInt32().isUnsigned();
	platform_info_cap.addAttribute("current_mem_mode").isInt32().isUnsigned();
	platform_info_cap.addAttribute("pmem_ras_capabilities").isInt32().isUnsigned();
	entities.push_back(platform_info_cap);

	// should only be one platform_cap so make signature the PK
	Entity platform_capabilities("platform_capabilities", "Platform Configuration Attributes Table (PCAT): Header.");
	platform_capabilities.includesHistory();
	platform_capabilities.addAttribute("signature").isText(4).isPk();
	platform_capabilities.addAttribute("length").isInt32().isUnsigned();
	platform_capabilities.addAttribute("revision").isInt32().isUnsigned();
	platform_capabilities.addAttribute("checksum").isInt32().isUnsigned();
	platform_capabilities.addAttribute("oem_id").isText(6);
	platform_capabilities.addAttribute("oem_table_id").isText(8);
	platform_capabilities.addAttribute("oem_revision").isInt32().isUnsigned();
	platform_capabilities.addAttribute("creator_id").isInt32().isUnsigned().isArray(4);
	platform_capabilities.addAttribute("creator_revision").isInt32().isUnsigned();
	entities.push_back(platform_capabilities);

	Entity driver_capabilities("driver_capabilities", "Driver capabilities and limits.");
	driver_capabilities.includesHistory();
	driver_capabilities.addAttribute("id").isInt32().isPk(true); // artificial so can update
	driver_capabilities.addAttribute("min_namespace_size").isInt64().isUnsigned();
	driver_capabilities.addAttribute("max_non_continguous_namespaces").isInt64().isUnsigned();
	driver_capabilities.addAttribute("block_sizes").isInt32().isUnsigned().isArray(16);
	driver_capabilities.addAttribute("num_block_sizes").isInt32().isUnsigned();
	driver_capabilities.addAttribute("namespace_memory_page_allocation_capable").isInt32().isUnsigned();
	entities.push_back(driver_capabilities);

	Entity driver_features("driver_features", "Driver supported features.");
	driver_features.includesHistory();
	driver_features.addAttribute("id").isInt32().isPk(true);
	driver_features.addAttribute("get_platform_capabilities").isInt32().isUnsigned();
	driver_features.addAttribute("get_topology").isInt32().isUnsigned();
	driver_features.addAttribute("get_interleave").isInt32().isUnsigned();
	driver_features.addAttribute("get_dimm_detail").isInt32().isUnsigned();
	driver_features.addAttribute("get_namespaces").isInt32().isUnsigned();
	driver_features.addAttribute("get_namespace_detail").isInt32().isUnsigned();
	driver_features.addAttribute("get_address_scrub_data").isInt32().isUnsigned();
	driver_features.addAttribute("get_platform_config_data").isInt32().isUnsigned();
	driver_features.addAttribute("get_boot_status").isInt32().isUnsigned();
	driver_features.addAttribute("get_power_data").isInt32().isUnsigned();
	driver_features.addAttribute("get_security_state").isInt32().isUnsigned();
	driver_features.addAttribute("get_log_page").isInt32().isUnsigned();
	driver_features.addAttribute("get_features").isInt32().isUnsigned();
	driver_features.addAttribute("set_features").isInt32().isUnsigned();
	driver_features.addAttribute("create_namespace").isInt32().isUnsigned();
	driver_features.addAttribute("rename_namespace").isInt32().isUnsigned();
	driver_features.addAttribute("grow_namespace").isInt32().isUnsigned();
	driver_features.addAttribute("shrink_namespace").isInt32().isUnsigned();
	driver_features.addAttribute("delete_namespace").isInt32().isUnsigned();
	driver_features.addAttribute("enable_namespace").isInt32().isUnsigned();
	driver_features.addAttribute("disable_namespace").isInt32().isUnsigned();
	driver_features.addAttribute("set_security_state").isInt32().isUnsigned();
	driver_features.addAttribute("enable_logging").isInt32().isUnsigned();
	driver_features.addAttribute("run_diagnostic").isInt32().isUnsigned();
	driver_features.addAttribute("set_platform_config").isInt32().isUnsigned();
	driver_features.addAttribute("passthrough").isInt32().isUnsigned();
	driver_features.addAttribute("start_address_scrub").isInt32().isUnsigned();
	driver_features.addAttribute("app_direct_mode").isInt32().isUnsigned();
	driver_features.addAttribute("storage_mode").isInt32().isUnsigned();
	entities.push_back(driver_features);

	Entity dimm_topology("dimm_topology", "NVDIMM Firmware Interface Table (NFIT) DIMM topology.");
	dimm_topology.includesHistory();
	dimm_topology.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_topology.addAttribute("id").isInt32().isUnsigned();
	dimm_topology.addAttribute("vendor_id").isInt32().isUnsigned();
	dimm_topology.addAttribute("device_id").isInt32().isUnsigned();
	dimm_topology.addAttribute("revision_id").isInt32().isUnsigned();
	dimm_topology.addAttribute("type").isInt32().isUnsigned();
	entities.push_back(dimm_topology);

	Entity nvm_namespace("namespace", "Persistent memory namespaces.");
	nvm_namespace.includesHistory();
	nvm_namespace.addAttribute("namespace_guid").isText(37).isPk();
	nvm_namespace.addAttribute("friendly_name").isText(64);
	nvm_namespace.addAttribute("block_size").isInt32().isUnsigned();
	nvm_namespace.addAttribute("block_count").isInt64().isUnsigned();
	nvm_namespace.addAttribute("type").isInt32().isUnsigned();
	nvm_namespace.addAttribute("health").isInt32().isUnsigned();
	nvm_namespace.addAttribute("enabled").isInt32().isUnsigned();
	nvm_namespace.addAttribute("btt").isInt32().isUnsigned();
	nvm_namespace.addAttribute("device_handle").isInt32().isUnsigned().isFk("dimm_topology", "device_handle");
	// for simulator, PCD index = NFIT driver ID
	nvm_namespace.addAttribute("interleave_set_index").isInt32().isUnsigned().isFk("interleave_set_dimm_info", "index_id");
	nvm_namespace.addAttribute("memory_page_allocation").isInt32().isUnsigned();
	entities.push_back(nvm_namespace);

	/*
	 * Tables to support simulated IOCTL calls: Passthrough FW Cmds
	 */
	Entity identify_dimm("identify_dimm", "Vendor firmware command: Identify DIMM.");
	identify_dimm.includesHistory();
	identify_dimm.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	identify_dimm.addAttribute("vendor_id").isInt32().isUnsigned();
	identify_dimm.addAttribute("device_id").isInt32().isUnsigned();
	identify_dimm.addAttribute("revision_id").isInt32().isUnsigned();
	identify_dimm.addAttribute("interface_format_code").isInt32().isUnsigned();
	identify_dimm.addAttribute("fw_revision").isText(14);
	identify_dimm.addAttribute("fw_api_version").isInt32().isUnsigned();
	identify_dimm.addAttribute("fw_sw_mask").isInt32().isUnsigned();
	identify_dimm.addAttribute("dimm_sku").isInt32().isUnsigned();
	identify_dimm.addAttribute("block_windows").isInt32().isUnsigned();
	identify_dimm.addAttribute("write_flush_addresses").isInt32().isUnsigned();
	identify_dimm.addAttribute("write_flush_address_start").isInt64().isUnsigned();
	identify_dimm.addAttribute("block_control_region_offset").isInt32().isUnsigned();
	identify_dimm.addAttribute("raw_cap").isInt64().isUnsigned();
	identify_dimm.addAttribute("manufacturer").isInt32().isUnsigned();
	identify_dimm.addAttribute("serial_num").isInt32().isUnsigned().isClearable();
	identify_dimm.addAttribute("model_num").isText(21);
	entities.push_back(identify_dimm);

	Entity dimm_partition("dimm_partition", "Vendor Firmware: Get Admin Features - DIMM Partition Info");
	dimm_partition.includesHistory();
	dimm_partition.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_partition.addAttribute("volatile_capacity").isInt32();
	dimm_partition.addAttribute("volatile_start").isInt64();
	dimm_partition.addAttribute("pmem_capacity").isInt32();
	dimm_partition.addAttribute("pm_start").isInt64();
	dimm_partition.addAttribute("raw_capacity").isInt32();
	entities.push_back(dimm_partition);

	Entity dimm_smart("dimm_smart", "Vendor Firmware: Get Log Page - SMART and Health Info");
	dimm_smart.includesHistory();
	dimm_smart.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_smart.addAttribute("validation_flags").isInt32().isUnsigned();
	dimm_smart.addAttribute("health_status").isInt32().isUnsigned();
	dimm_smart.addAttribute("media_temperature").isInt32().isUnsigned();
	dimm_smart.addAttribute("spare").isInt32().isUnsigned();
	dimm_smart.addAttribute("alarm_trips").isInt32().isUnsigned();
	dimm_smart.addAttribute("percentage_used").isInt32().isUnsigned();
	dimm_smart.addAttribute("lss").isInt32().isUnsigned();
	dimm_smart.addAttribute("vendor_specific_data_size").isInt32().isUnsigned();
	dimm_smart.addAttribute("power_cycles").isInt64().isUnsigned();
	dimm_smart.addAttribute("power_on_seconds").isInt64().isUnsigned();
	dimm_smart.addAttribute("uptime").isInt64().isUnsigned();
	dimm_smart.addAttribute("unsafe_shutdowns").isInt32().isUnsigned();
	dimm_smart.addAttribute("lss_details").isInt32().isUnsigned();
	dimm_smart.addAttribute("last_shutdown_time").isInt64().isUnsigned();
	dimm_smart.addAttribute("controller_temperature").isInt32().isUnsigned();
	entities.push_back(dimm_smart);

	/*
	 * For event monitoring of dimm state transitions
	 */
	Entity dimm_state("dimm_state", "Monitor stored DIMM state for detecting changes.");
	dimm_state.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_state.addAttribute("spare_capacity_state").isInt32();
	dimm_state.addAttribute("wearlevel_state").isInt32();
	dimm_state.addAttribute("mediaerrors_corrected").isInt64().isUnsigned();
	dimm_state.addAttribute("mediaerrors_uncorrectable").isInt64().isUnsigned();
	dimm_state.addAttribute("mediaerrors_erasurecoded").isInt64().isUnsigned();
	dimm_state.addAttribute("health_state").isInt32();
	dimm_state.addAttribute("die_spares_used").isInt32().isUnsigned();
	dimm_state.addAttribute("mediatemperature_state").isInt32();
	dimm_state.addAttribute("controllertemperature_state").isInt32();
	entities.push_back(dimm_state);

	/*
	 * For event monitoring of namespace health state transitions
	 */
	Entity namespace_state("namespace_state", "Monitor stored namespace state for detecting changes.");
	namespace_state.addAttribute("namespace_guid").isText(37).isPk();
	namespace_state.addAttribute("health_state").isInt32();
	entities.push_back(namespace_state);

	Entity dimm_alarm_thresholds("dimm_alarm_thresholds", "Vendor Firmware: Get Features - Alarm Thresholds.");
	dimm_alarm_thresholds.includesHistory();
	dimm_alarm_thresholds.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_alarm_thresholds.addAttribute("enable").isInt32().isUnsigned();
	dimm_alarm_thresholds.addAttribute("media_temperature").isInt32().isUnsigned();
	dimm_alarm_thresholds.addAttribute("controller_temperature").isInt32().isUnsigned();
	dimm_alarm_thresholds.addAttribute("spare").isInt32().isUnsigned();
	entities.push_back(dimm_alarm_thresholds);

	Entity dimm_power_management("dimm_power_management", "Vendor Firmware: Get Features - Power Management Policy.");
	dimm_power_management.includesHistory();
	dimm_power_management.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_power_management.addAttribute("enable").isInt32().isUnsigned();
	dimm_power_management.addAttribute("tdp_power_limit").isInt32().isUnsigned();
	dimm_power_management.addAttribute("peak_power_budget").isInt32().isUnsigned();
	dimm_power_management.addAttribute("avg_power_budget").isInt32().isUnsigned();
	entities.push_back(dimm_power_management);

	Entity dimm_die_sparing("dimm_die_sparing", "Vendor Firmware: Get Features - Die Sparing Policy.");
	dimm_die_sparing.includesHistory();
	dimm_die_sparing.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_die_sparing.addAttribute("enable").isInt32().isUnsigned();
	dimm_die_sparing.addAttribute("aggressiveness").isInt32().isUnsigned();
	// currently 4 ranks in an NVM DIMM
	dimm_die_sparing.addAttribute("supported_by_rank").isInt32().isUnsigned().isArray(4);
	entities.push_back(dimm_die_sparing);

	Entity dimm_optional_config_data("dimm_optional_config_data", "Vendor Firmware: Get Features - Optional Configuration Data Policy.");
	dimm_optional_config_data.includesHistory();
	dimm_optional_config_data.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_optional_config_data.addAttribute("first_fast_refresh_enable").isInt32().isUnsigned();
	entities.push_back(dimm_optional_config_data);

	Entity dimm_err_correction("dimm_err_correction", "Obsolete.");
	dimm_err_correction.includesHistory();
	dimm_err_correction.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_err_correction.addAttribute("unrefreshed_enable").isInt32().isUnsigned();
	dimm_err_correction.addAttribute("refreshed_enable").isInt32().isUnsigned();
	dimm_err_correction.addAttribute("unrefreshed_force_write").isInt32().isUnsigned();
	dimm_err_correction.addAttribute("refreshed_force_write").isInt32().isUnsigned();
	entities.push_back(dimm_err_correction);

	Entity dimm_erasure_coding("dimm_erasure_coding", "Obsolete.");
	dimm_erasure_coding.includesHistory();
	dimm_erasure_coding.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_erasure_coding.addAttribute("verify_erc").isInt32().isUnsigned();
	dimm_erasure_coding.addAttribute("unrefreshed_enable").isInt32().isUnsigned();
	dimm_erasure_coding.addAttribute("refreshed_enable").isInt32().isUnsigned();
	dimm_erasure_coding.addAttribute("unrefreshed_force_write").isInt32().isUnsigned();
	dimm_erasure_coding.addAttribute("refreshed_force_write").isInt32().isUnsigned();
	entities.push_back(dimm_erasure_coding);

	Entity dimm_thermal("dimm_thermal", "Obsolete.");
	dimm_thermal.includesHistory();
	dimm_thermal.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_thermal.addAttribute("throttling_enable").isInt32().isUnsigned();
	dimm_thermal.addAttribute("alerting_enable").isInt32().isUnsigned();
	dimm_thermal.addAttribute("critical_shutdown_enable").isInt32().isUnsigned();
	entities.push_back(dimm_thermal);

	Entity dimm_fw_image("dimm_fw_image", "Vendor Firmware: Get Log - Firmware Image Info.");;
	dimm_fw_image.includesHistory();
	dimm_fw_image.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_fw_image.addAttribute("fw_rev").isText(14);
	dimm_fw_image.addAttribute("fw_type").isInt32().isUnsigned();
	dimm_fw_image.addAttribute("staged_fw_status").isInt32().isUnsigned();
	dimm_fw_image.addAttribute("staged_fw_rev").isText(14);
	dimm_fw_image.addAttribute("staged_fw_type").isInt32().isUnsigned();
	dimm_fw_image.addAttribute("commit_id").isText(40);
	entities.push_back(dimm_fw_image);

	Entity dimm_fw_debug_log("dimm_fw_debug_log", "Vendor Firmware: Get Log - Firmware Debug Log.");
	dimm_fw_debug_log.includesHistory();
	dimm_fw_debug_log.addAttribute("device_handle").isInt32().isUnsigned().isFk("dimm_topology", "device_handle");
	dimm_fw_debug_log.addAttribute("fw_log").isText(1024).isUnsigned().isPk();
	entities.push_back(dimm_fw_debug_log);

	Entity dimm_memory_info_page0("dimm_memory_info_page0", "Vendor Firmware: Get Log - Memory Info Page 0.");
	dimm_memory_info_page0.includesHistory();
	dimm_memory_info_page0.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_memory_info_page0.addAttribute("bytes_read").isInt64().isUnsigned();
	dimm_memory_info_page0.addAttribute("bytes_written").isInt64().isUnsigned();
	dimm_memory_info_page0.addAttribute("read_reqs").isInt64().isUnsigned();
	dimm_memory_info_page0.addAttribute("write_reqs").isInt64().isUnsigned();
	dimm_memory_info_page0.addAttribute("block_read_reqs").isInt64().isUnsigned();
	dimm_memory_info_page0.addAttribute("block_write_reqs").isInt64().isUnsigned();
	entities.push_back(dimm_memory_info_page0);

	Entity dimm_memory_info_page1("dimm_memory_info_page1", "Vendor Firmware: Get Log - Memory Info Page 1.");
	dimm_memory_info_page1.includesHistory();
	dimm_memory_info_page1.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_memory_info_page1.addAttribute("total_bytes_read").isInt64().isUnsigned();
	dimm_memory_info_page1.addAttribute("total_bytes_written").isInt64().isUnsigned();
	dimm_memory_info_page1.addAttribute("total_read_reqs").isInt64().isUnsigned();
	dimm_memory_info_page1.addAttribute("total_write_reqs").isInt64().isUnsigned();
	dimm_memory_info_page1.addAttribute("total_block_read_reqs").isInt64().isUnsigned();
	dimm_memory_info_page1.addAttribute("total_block_write_reqs").isInt64().isUnsigned();
	entities.push_back(dimm_memory_info_page1);

	Entity dimm_memory_info_page2("dimm_memory_info_page2", "Vendor Firmware: Get Log - Memory Info Page 2.");
	dimm_memory_info_page2.includesHistory();
	dimm_memory_info_page2.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_memory_info_page2.addAttribute("write_count_max").isInt64().isUnsigned();
	dimm_memory_info_page2.addAttribute("write_count_average").isInt64().isUnsigned();
	dimm_memory_info_page2.addAttribute("uncorrectable_host").isInt32().isUnsigned();
	dimm_memory_info_page2.addAttribute("uncorrectable_non_host").isInt32().isUnsigned();
	dimm_memory_info_page2.addAttribute("media_errors_uc").isInt32().isUnsigned();
	dimm_memory_info_page2.addAttribute("media_errors_ce").isInt64().isUnsigned();
	dimm_memory_info_page2.addAttribute("media_errors_ecc").isInt64().isUnsigned();
	entities.push_back(dimm_memory_info_page2);

	Entity dimm_long_op_status("dimm_long_op_status", "Obsolete.");
	dimm_long_op_status.includesHistory();
	dimm_long_op_status.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_long_op_status.addAttribute("command0").isInt64();
	dimm_long_op_status.addAttribute("command1").isInt64();
	dimm_long_op_status.addAttribute("percent_complete0").isInt64();
	dimm_long_op_status.addAttribute("percent_complete1").isInt64();
	dimm_long_op_status.addAttribute("etc").isText(13);
	entities.push_back(dimm_long_op_status);

	Entity dimm_details("dimm_details", "SMBIOS Type 17 Table.");
	dimm_details.includesHistory();
	dimm_details.addAttribute("device_handle").isInt32().isUnsigned();
	dimm_details.addAttribute("form_factor").isInt32().isUnsigned();
	dimm_details.addAttribute("data_width").isInt64().isUnsigned();
	dimm_details.addAttribute("total_width").isInt64().isUnsigned();
	dimm_details.addAttribute("size").isInt64().isUnsigned();
	dimm_details.addAttribute("speed").isInt64().isUnsigned();
	dimm_details.addAttribute("part_number").isText(32);
	dimm_details.addAttribute("device_locator").isText(128);
	dimm_details.addAttribute("bank_label").isText(128);
	dimm_details.addAttribute("manufacturer").isText(256);
	dimm_details.addAttribute("type").isInt32().isUnsigned();
	dimm_details.addAttribute("type_detail").isInt32().isUnsigned();
	dimm_details.addAttribute("id").isInt32().isUnsigned().isPk();
	entities.push_back(dimm_details);

	Entity dimm_security_info("dimm_security_info", "Vendor Firmware: Get Security Info - Get Security State.");
	dimm_security_info.includesHistory();
	dimm_security_info.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_security_info.addAttribute("security_state").isInt32();
	entities.push_back(dimm_security_info);

	Entity dimm_sanitize_info("dimm_sanitize_info", "Vendor Firmware: Get Security Info - Get Sanitize Status.");
	dimm_sanitize_info.includesHistory();
	dimm_sanitize_info.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_sanitize_info.addAttribute("sanitize_state").isInt32();
	dimm_sanitize_info.addAttribute("sanitize_progress").isInt32();
	entities.push_back(dimm_sanitize_info);

	Entity fw_media_low_log_entry("fw_media_low_log_entry", "Vendor Firmware: Get Log - Error Log Low Media Log.");
	fw_media_low_log_entry.includesHistory();
	fw_media_low_log_entry.addAttribute("device_handle").isInt32().isUnsigned().isFk("dimm_topology", "device_handle");
	fw_media_low_log_entry.addAttribute("system_timestamp").isInt64().isUnsigned().isPk();
	fw_media_low_log_entry.addAttribute("dpa").isInt64().isUnsigned();
	fw_media_low_log_entry.addAttribute("pda").isInt64().isUnsigned();
	fw_media_low_log_entry.addAttribute("range").isInt32().isUnsigned();
	fw_media_low_log_entry.addAttribute("error_type").isInt32().isUnsigned();
	fw_media_low_log_entry.addAttribute("error_flags").isInt32().isUnsigned();
	fw_media_low_log_entry.addAttribute("transaction_type").isInt32().isUnsigned();
	entities.push_back(fw_media_low_log_entry);

	Entity fw_media_high_log_entry("fw_media_high_log_entry", "Vendor Firmware: Get Log - Error Log High Media Log.");
	fw_media_high_log_entry.addAttribute("device_handle").isInt32().isUnsigned().isFk("dimm_topology", "device_handle");
	fw_media_high_log_entry.includesHistory();
	fw_media_high_log_entry.addAttribute("system_timestamp").isInt64().isUnsigned().isPk();
	fw_media_high_log_entry.addAttribute("dpa").isInt64().isUnsigned();
	fw_media_high_log_entry.addAttribute("pda").isInt64().isUnsigned();
	fw_media_high_log_entry.addAttribute("range").isInt32().isUnsigned();
	fw_media_high_log_entry.addAttribute("error_type").isInt32().isUnsigned();
	fw_media_high_log_entry.addAttribute("error_flags").isInt32().isUnsigned();
	fw_media_high_log_entry.addAttribute("transaction_type").isInt32().isUnsigned();
	entities.push_back(fw_media_high_log_entry);

	Entity fw_thermal_low_log_entry("fw_thermal_low_log_entry", "Vendor Firmware: Get Log - Error Log Low Thermal Log.");
	fw_thermal_low_log_entry.includesHistory();
	fw_thermal_low_log_entry.addAttribute("device_handle").isInt32().isUnsigned().isFk("dimm_topology", "device_handle");
	fw_thermal_low_log_entry.addAttribute("system_timestamp").isInt64().isUnsigned().isPk();
	fw_thermal_low_log_entry.addAttribute("host_reported_temp_data").isInt32().isUnsigned();
	entities.push_back(fw_thermal_low_log_entry);

	Entity fw_thermal_high_log_entry("fw_thermal_high_log_entry", "Vendor Firmware: Get Log - Error Log High Thermal Log.");
	fw_thermal_high_log_entry.addAttribute("device_handle").isInt32().isUnsigned().isFk("dimm_topology", "device_handle");
	fw_thermal_high_log_entry.includesHistory();
	fw_thermal_high_log_entry.addAttribute("system_timestamp").isInt64().isUnsigned().isPk();
	fw_thermal_high_log_entry.addAttribute("host_reported_temp_data").isInt32().isUnsigned();
	entities.push_back(fw_thermal_high_log_entry);

	Entity dimm_fw_log_level("dimm_fw_log_level", "Vendor Firmware: Get Admin Features - FW Debug Log Level.");
	dimm_fw_log_level.includesHistory();
	dimm_fw_log_level.addAttribute("device_handle").isInt32().isPk();
	dimm_fw_log_level.addAttribute("log_level").isInt32();
	entities.push_back(dimm_fw_log_level);

	Entity dimm_fw_time("dimm_fw_time", "Vendor Firmware: Get Admin Features - System Time.");
	dimm_fw_time.includesHistory();
	dimm_fw_time.addAttribute("device_handle").isInt32().isPk();
	dimm_fw_time.addAttribute("time").isInt64().isUnsigned();
	entities.push_back(dimm_fw_time);

	// platform configuration data header table for a dimm
	Entity dimm_platform_config("dimm_platform_config", "Vendor Firmware: Get Admin Features - Platform Config Data Header.");
	dimm_platform_config.includesHistory();
	dimm_platform_config.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_platform_config.addAttribute("signature").isText(4);
	dimm_platform_config.addAttribute("length").isInt32().isUnsigned(); // calculated?
	dimm_platform_config.addAttribute("revision").isInt32().isUnsigned();
	dimm_platform_config.addAttribute("checksum").isInt32().isUnsigned();
	dimm_platform_config.addAttribute("oem_id").isText(6);
	dimm_platform_config.addAttribute("oem_table_id").isText(8);
	dimm_platform_config.addAttribute("oem_revision").isInt32().isUnsigned();
	dimm_platform_config.addAttribute("creator_id").isInt32().isUnsigned();
	dimm_platform_config.addAttribute("creator_revision").isInt32().isUnsigned();
	dimm_platform_config.addAttribute("current_config_size").isInt32().isUnsigned(); // calculated?
	dimm_platform_config.addAttribute("current_config_offset").isInt32().isUnsigned(); // calculated?
	dimm_platform_config.addAttribute("config_input_size").isInt32().isUnsigned(); // calculated?
	dimm_platform_config.addAttribute("config_input_offset").isInt32().isUnsigned(); // calculated?
	dimm_platform_config.addAttribute("config_output_size").isInt32().isUnsigned(); // calculated?
	dimm_platform_config.addAttribute("config_output_offset").isInt32().isUnsigned(); // calculated?
	entities.push_back(dimm_platform_config);

	// platform configuration data current configuration table for a dimm
	Entity dimm_current_config("dimm_current_config", "Vendor Firmware: Get Admin Features - Platform Config Data Current Config.");
	dimm_current_config.includesHistory();
	dimm_current_config.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_current_config.addAttribute("signature").isText(4);
	dimm_current_config.addAttribute("length").isInt32().isUnsigned(); // calculated?
	dimm_current_config.addAttribute("revision").isInt32().isUnsigned();
	dimm_current_config.addAttribute("checksum").isInt32().isUnsigned();
	dimm_current_config.addAttribute("oem_id").isText(6);
	dimm_current_config.addAttribute("oem_table_id").isText(8);
	dimm_current_config.addAttribute("oem_revision").isInt32().isUnsigned();
	dimm_current_config.addAttribute("creator_id").isInt32().isUnsigned();
	dimm_current_config.addAttribute("creator_revision").isInt32().isUnsigned();
	dimm_current_config.addAttribute("config_status").isInt32().isUnsigned();
	dimm_current_config.addAttribute("mapped_memory_capacity").isInt64().isUnsigned();
	dimm_current_config.addAttribute("mapped_app_direct_capacity").isInt64().isUnsigned();
	entities.push_back(dimm_current_config);

	// platform configuration data configuration input table for a dimm
	Entity dimm_config_input("dimm_config_input", "Vendor Firmware: Get Admin Features - Platform Config Data Config Input.");
	dimm_config_input.includesHistory();
	dimm_config_input.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_config_input.addAttribute("signature").isText(4);
	dimm_config_input.addAttribute("length").isInt32().isUnsigned();
	dimm_config_input.addAttribute("revision").isInt32().isUnsigned();
	dimm_config_input.addAttribute("checksum").isInt32().isUnsigned();
	dimm_config_input.addAttribute("oem_id").isText(6);
	dimm_config_input.addAttribute("oem_table_id").isText(8);
	dimm_config_input.addAttribute("oem_revision").isInt32().isUnsigned();
	dimm_config_input.addAttribute("creator_id").isInt32().isUnsigned();
	dimm_config_input.addAttribute("creator_revision").isInt32().isUnsigned();
	dimm_config_input.addAttribute("sequence_number").isInt32().isUnsigned();
	entities.push_back(dimm_config_input);

	// platform configuration data configuration output table for a dimm
	Entity dimm_config_output("dimm_config_output", "Vendor Firmware: Get Admin Features - Platform Config Data Config Output.");
	dimm_config_output.includesHistory();
	dimm_config_output.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	dimm_config_output.addAttribute("signature").isText(4);
	dimm_config_output.addAttribute("length").isInt32().isUnsigned();
	dimm_config_output.addAttribute("revision").isInt32().isUnsigned();
	dimm_config_output.addAttribute("checksum").isInt32().isUnsigned();
	dimm_config_output.addAttribute("oem_id").isText(6);
	dimm_config_output.addAttribute("oem_table_id").isText(8);
	dimm_config_output.addAttribute("oem_revision").isInt32().isUnsigned();
	dimm_config_output.addAttribute("creator_id").isInt32().isUnsigned();
	dimm_config_output.addAttribute("creator_revision").isInt32().isUnsigned();
	dimm_config_output.addAttribute("sequence_number").isInt32().isUnsigned();
	dimm_config_output.addAttribute("validation_status").isInt32().isUnsigned();
	entities.push_back(dimm_config_output);

	// Partition Size change
	Entity dimm_partition_change("dimm_partition_change", "Vendor Firmware: Get Admin Features - Platform Config Data Partition Size Change Table Type 4.");
	dimm_partition_change.includesHistory();
	dimm_partition_change.addAttribute("device_handle").isInt32().isUnsigned().isFk("dimm_topology", "device_handle");
	dimm_partition_change.addAttribute("id").isIndexPk();
	// config input or config output
	dimm_partition_change.addAttribute("config_table_type").isInt32().isUnsigned();
	dimm_partition_change.addAttribute("extension_table_type").isInt32().isUnsigned();
	dimm_partition_change.addAttribute("length").isInt32().isUnsigned();
	dimm_partition_change.addAttribute("partition_size").isInt64().isUnsigned();
	dimm_partition_change.addAttribute("status").isInt32().isUnsigned();
	entities.push_back(dimm_partition_change);

	// Interleave set info table
	Entity dimm_interleave_set("dimm_interleave_set", "Vendor Firmware: Get Admin Features - Platform Config Data DIMM Interleave Information Table Type 5.");
	dimm_interleave_set.includesHistory();
	dimm_interleave_set.addAttribute("id").isIndexPk();
	// uniqueness is a combination of index, dimm id and table_type
	dimm_interleave_set.addAttribute("device_handle").isInt32().isUnsigned().isFk("dimm_topology", "device_handle");
	// current config, config input or config output
	dimm_interleave_set.addAttribute("config_table_type").isInt32().isUnsigned();
	dimm_interleave_set.addAttribute("extension_table_type").isInt32().isUnsigned();
	dimm_interleave_set.addAttribute("length").isInt32().isUnsigned();
	// Can't use "index" as it's a sql keyword
	dimm_interleave_set.addAttribute("index_id").isInt32().isUnsigned();
	dimm_interleave_set.addAttribute("dimm_count").isInt32().isUnsigned();
	dimm_interleave_set.addAttribute("memory_type").isInt32().isUnsigned();
	dimm_interleave_set.addAttribute("interleave_format").isInt32().isUnsigned();
	dimm_interleave_set.addAttribute("mirror_enable").isInt32().isUnsigned();
	dimm_interleave_set.addAttribute("status").isInt32().isUnsigned();
	entities.push_back(dimm_interleave_set);

	// Interleave set dimm info table
	Entity interleave_set_dimm_info("interleave_set_dimm_info", "Vendor Firmware: Get Admin Features - Platform Config Data DIMM Interleave Information Table Type 5 - DIMM Identification Information Extension Table.");
	interleave_set_dimm_info.includesHistory();
	interleave_set_dimm_info.addAttribute("id").isIndexPk();
	// uniqueness is a combination of index, dimm id and table_type
	// current config, config input or config output
	interleave_set_dimm_info.addAttribute("config_table_type").isInt32().isUnsigned();
	// NOTE: Can't use "index" as it's a sql keyword
	interleave_set_dimm_info.addAttribute("index_id").isInt32().isUnsigned().isFk("dimm_interleave_set", "index_id");
	// which dimm this interleave set info is stored on
	interleave_set_dimm_info.addAttribute("device_handle").isInt32().isUnsigned().isFk("dimm_topology", "device_handle");
	interleave_set_dimm_info.addAttribute("manufacturer").isInt32().isUnsigned();
	interleave_set_dimm_info.addAttribute("serial_num").isInt32().isUnsigned().isClearable();
	interleave_set_dimm_info.addAttribute("model_num").isText(21);
	interleave_set_dimm_info.addAttribute("offset").isInt64().isUnsigned();
	interleave_set_dimm_info.addAttribute("size").isInt64().isUnsigned();
	entities.push_back(interleave_set_dimm_info);

	// Temperature Error injection info table
	Entity temperature_error_injection_info("temperature_error_injection_info", "Simulation only injected temperature errors.");
	temperature_error_injection_info.includesHistory();
	temperature_error_injection_info.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	temperature_error_injection_info.addAttribute("temperature").isInt32().isUnsigned();
	entities.push_back(temperature_error_injection_info);

	// Poison Error injection info table
	Entity poison_error_injection_info("poison_error_injection_info", "Simulation only injected poison errors.");
	poison_error_injection_info.addAttribute("id").isInt32().isPk(true).orderByDesc();
	poison_error_injection_info.includesHistory();
	poison_error_injection_info.addAttribute("device_handle").isInt32().isUnsigned().isFk("dimm_topology", "device_handle");
	poison_error_injection_info.addAttribute("dpa_address").isInt64().isUnsigned();
	entities.push_back(poison_error_injection_info);

	// Dimm Performance
	Entity performance("performance", "Monitor stored DIMM performance metrics.");
	performance.addAttribute("id").isInt32().isPk(true);
	performance.addAttribute("dimm_guid").isText(37);
	performance.addAttribute("time").isInt64().isUnsigned().orderByDesc();
	performance.addAttribute("bytes_read").isInt64().isUnsigned();
	performance.addAttribute("bytes_written").isInt64().isUnsigned();
	performance.addAttribute("read_reqs").isInt64().isUnsigned();
	performance.addAttribute("host_write_cmds").isInt64().isUnsigned();
	performance.addAttribute("block_reads").isInt64().isUnsigned();
	performance.addAttribute("block_writes").isInt64().isUnsigned();
	entities.push_back(performance);

	// Driver Metadata check diagnostic results
	Entity driver_metadata_check_diag_result("driver_metadata_check_diag_result", "Simulation only driver metadata diagnostic results.");
	driver_metadata_check_diag_result.addAttribute("id").isInt32().isPk(true);
	driver_metadata_check_diag_result.addAttribute("result_type").isInt32();
	driver_metadata_check_diag_result.addAttribute("ns_guid").isText(37);
	driver_metadata_check_diag_result.addAttribute("device_handle").isInt32().isUnsigned();
	driver_metadata_check_diag_result.addAttribute("health_flag").isInt32().isUnsigned();
	entities.push_back(driver_metadata_check_diag_result);

	// Boot status register
	Entity boot_status_register("boot_status_register", "NVDIMM Controller boot status");
	boot_status_register.addAttribute("device_handle").isInt32().isUnsigned().isPk();
	boot_status_register.addAttribute("bsr").isInt64().isUnsigned();
	entities.push_back(boot_status_register);

	if (arg_count == 3) // allows caller to pass where the templates are and where to put the files
	{
		CrudSchemaGenerator::Generate(entities, args[1], args[2]);
		CrudSchemaGenerator::CreateDoc(entities, args[2]);
	}
	else
	{
		CrudSchemaGenerator::Generate(entities, "", "");
	}
	return 0;
}

