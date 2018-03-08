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
* This file contains the implementation of the common eventing functionality.
*/
#ifndef _EVENT_STRINGS_H_
#define _EVENT_STRINGS_H_

#include <stdlib.h>
#include <cr_i18n.h>
#include <export_common.h>

#ifdef __cplusplus
extern "C" {
#endif

    /*
    * ****************************************************************************
    * EVENT STRING CONSTANTS
    * **************************************************************************
    */

    /*
    * Config event message strings.
    * This array ordered by event_code_config enumeration for easy lookup
    */
    static const char *EVENT_MESSAGES_CONFIG[EVENT_CODE_CONFIG_UNKNOWN -
        EVENT_CODE_OFFSET_CONFIG + 1] =
    {
        // EVENT_CODE_CONFIG_CONFIG_GOAL_APPLIED
        N_TR("The BIOS has successfully applied the configuration goal on " NVM_DIMM_NAME " %s."),
        // EVENT_CODE_CONFIG_TOPOLOGY_ADDED_CONFIGURED_DEVICE
        N_TR("The BIOS has detected and successfully configured " NVM_DIMM_NAME " %s."),
        // EVENT_CODE_CONFIG_TOPOLOGY_ADDED_NEW_DEVICE
        N_TR("The BIOS has detected a new " NVM_DIMM_NAME " %s. The new " NVM_DIMM_NAME " must "
        "be configured in order to be used."),
        // EVENT_CODE_CONFIG_TOPOLOGY_MISSING_DEVICE
        N_TR(NVM_DIMM_NAME" %s is missing."),
        // EVENT_CODE_CONFIG_TOPOLOGY_REPLACED_CONFIGURED_DEVICE
        N_TR(NVM_DIMM_NAME" %s has been replaced with " NVM_DIMM_NAME " %s."),
        // EVENT_CODE_CONFIG_TOPOLOGY_REPLACED_NEW_DEVICE
        N_TR(NVM_DIMM_NAME" %s has been replaced with a new " NVM_DIMM_NAME " %s. "
            "The new " NVM_DIMM_NAME " must be configured in order to be used."),
        // EVENT_CODE_CONFIG_TOPOLOGY_MOVED_DEVICE
        N_TR(NVM_DIMM_NAME" %s has been moved to a new position."),

        // EVENT_CODE_CONFIG_UNKNOWN
        N_TR("A platform configuration change has logged an unknown event code %d.")
    };

    /*
    * Management event message strings.
    * This array ordered by event_code_mgmt enumeration for easy lookup
    */
    static const char *EVENT_MESSAGES_MGMT[EVENT_CODE_MGMT_UNKNOWN -
        EVENT_CODE_OFFSET_MGMT + 1] =
    {
        // EVENT_CODE_MGMT_CONFIG_GOAL_CREATED
        N_TR("A new configuration goal has been saved on " NVM_DIMM_NAME " %s."),
        // EVENT_CODE_MGMT_CONFIG_GOAL_DELETED
        N_TR("A configuration goal has been deleted from " NVM_DIMM_NAME " %s."),
        // EVENT_CODE_MGMT_NAMESPACE_CREATED
        N_TR("A new namespace has been created. (Name: %s, UID: %s)"),
        // EVENT_CODE_MGMT_NAMESPACE_DELETED
        N_TR("A namespace has been deleted. (Name: %s, UID: %s)"),
        // EVENT_CODE_MGMT_NAMESPACE_MODIFIED
        N_TR("A namespace's settings have been modified. (Name: %s, UID: %s)"),
        // EVENT_CODE_MGMT_SENSOR_SETTINGS_CHANGE
        N_TR("The %s sensor settings have been changed on " NVM_DIMM_NAME " %s."),
        // EVENT_CODE_MGMT_FIRMWARE_UPDATE
        N_TR("The firmware has been updated on " NVM_DIMM_NAME " %s. (New version: %s)"),
        // EVENT_CODE_MGMT_SECURITY_PASSWORD_SET
        N_TR("The security password has been set on " NVM_DIMM_NAME " %s."),
        // EVENT_CODE_MGMT_SECURITY_PASSWORD_REMOVED
        N_TR("The security password has been removed from " NVM_DIMM_NAME " %s."),
        // EVENT_CODE_MGMT_SECURITY_SECURE_ERASE
        N_TR("The persistent data on " NVM_DIMM_NAME " %s has been securely erased."),
        // EVENT_CODE_MGMT_SECURITY_FROZEN
        N_TR("The security lock state is frozen on " NVM_DIMM_NAME " %s."),
        // EVENT_CODE_DIAG_QUICK_UNKNOWN
        N_TR("The management software logged an unknown event code %d.")
    };

    /*
    * " NVM_DIMM_NAME " event health messages
    * This array ordered by event_code_health enumeration for easy lookup
    */
    static const char *EVENT_MESSAGES_HEALTH[EVENT_CODE_HEALTH_UNKNOWN -
        EVENT_CODE_OFFSET_HEALTH + 1] =
    {
        // EVENT_CODE_HEALTH_HEALTH_STATE_CHANGED
        N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
        "is reporting a change in health state from %s to %s."),
        // EVENT_CODE_HEALTH_NAMESPACE_HEALTH_STATE_CHANGED
        N_TR("The health monitor has detected that namespace %s "
            "is reporting a change in health state from %s to %s."),
        // EVENT_CODE_HEALTH_NEW_FWERRORS_FOUND
        N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
            "firmware has reported %s new errors."),
        // EVENT_CODE_HEALTH_SANITIZE_INPROGRESS
        N_TR("The health monitor has detected that a sanitize operation "
            "is in progress on " NVM_DIMM_NAME " %s. A reboot will be "
            "required when complete to use the " NVM_DIMM_NAME "."),
        // EVENT_CODE_HEALTH_SANITIZE_COMPLETE
        N_TR("The health monitor has detected that a sanitize operation "
            "has completed on " NVM_DIMM_NAME " %s. A reboot will be "
            "required to use the " NVM_DIMM_NAME "."),
        // EVENT_CODE_HEALTH_SMART_HEALTH
        N_TR("The health monitor has detected a smart health event "
            "has occured on " NVM_DIMM_NAME " %s. %s error log entry details: %s"),
        // EVENT_CODE_HEALTH_UNKNOWN
        N_TR("The health monitor has logged an unknown error code %d."),
    };

    /*
    * Quick check diagnostic event message strings.
    * This array ordered by event_code_diag_quick enumeration for easy lookup
    */
    static const char *EVENT_MESSAGES_QUICK_DIAG[EVENT_CODE_DIAG_QUICK_UNKNOWN -
        EVENT_CODE_OFFSET_DIAG_QUICK + 1] =
    {
        // EVENT_CODE_DIAG_QUICK_SUCCESS
        N_TR("The quick health check succeeded."),
        // EVENT_CODE_DIAG_QUICK_NOT_MANAGEABLE_VENDOR_ID
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
        "is not manageable because subsystem vendor ID %s is not "
            "supported."),
        // EVENT_CODE_DIAG_QUICK_NOT_MANAGEABLE_DEVICE_ID
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
            "is not manageable because subsystem device ID %s is not "
            "supported."),
        // EVENT_CODE_DIAG_QUICK_NOT_MANAGEABLE_FW_API
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
            "is not manageable because firmware API version %s is not "
            "supported."),
        // EVENT_CODE_DIAG_QUICK_BAD_HEALTH
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
            "is reporting a bad health state %s, expected %s."),
        // EVENT_CODE_DIAG_QUICK_BAD_MEDIA_TEMP
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
            "is reporting a media temperature of %s C which is above "
            "the alarm threshold %s C."),
        // EVENT_CODE_DIAG_QUICK_BAD_SPARE
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
            "is reporting remaining spare capacity at %s% which is "
            "less than the alarm threshold %s%."),
        // EVENT_CODE_DIAG_QUICK_BAD_PERCENT_USED
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
            "is reporting percentage used at %s% which is above "
            "the recommended threshold %s%."),
        // Available event 508
        N_TR("Placeholder"),
        // Available event 509
        N_TR("Placeholder"),
        // Available event 510
        N_TR("Placeholder"),
        // EVENT_CODE_DIAG_QUICK_BAD_CORE_TEMP
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
            "is reporting a core temperature of %s C which is above "
            "the alarm threshold %s C."),
        // Available event 512
        N_TR("Placeholder"),
        // EVENT_CODE_DIAG_QUICK_UNREADABLE_BSR
        N_TR("The quick health check detected that the boot status register "
            "of " NVM_DIMM_NAME " %s is not readable."),
        // EVENT_CODE_DIAG_QUICK_MEDIA_NOT_READY
        N_TR("The quick health check detected that the firmware "
            "on " NVM_DIMM_NAME " %s is reporting that the media is not ready."),
        // EVENT_CODE_DIAG_QUICK_MEDIA_READY_ERROR
        N_TR("The quick health check detected that the firmware "
            "on " NVM_DIMM_NAME " %s is reporting an error in the media."),
        // Available event 516
        N_TR("Placeholder"),
        // Available event 517
        N_TR("Placeholder"),
        // Available event 518
        N_TR("Placeholder"),
        // EVENT_CODE_DIAG_QUICK_NO_POST_CODE
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s failed "
            "to initialize BIOS POST testing."),
        // EVENT_CODE_DIAG_QUICK_FW_INITIALIZATION_INCOMPLETE
        N_TR("The quick health check detected that the firmware "
            "on " NVM_DIMM_NAME " %s has not initialized "
            "successfully, last known Major:Minor Checkpoint is %s."),
        // EVENT_CODE_DIAG_QUICK_FW_HIT_ASSERT
        N_TR("The quick health check detected that the firmware "
            "on " NVM_DIMM_NAME " %s reported an assert."),
        // EVENT_CODE_DIAG_QUICK_FW_STALLED
        N_TR("The quick health check detected that the firmware on " NVM_DIMM_NAME
            " %s has stalled the media interface engine."),
        // EVENT_CODE_DIAG_QUICK_VIRAL_STATE
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
            "is reporting a viral state. The " NVM_DIMM_NAME " is now read-only."),
        // EVENT_CODE_DIAG_QUICK_BAD_DRIVER
        N_TR("The quick health check detected that the underlying software is missing "
            "or incompatible with this version of the management software."),
        // Available event 525
        N_TR("Placeholder"),
        // EVENT_CODE_DIAG_QUICK_BAD_MEDIA_TEMP_THROTTLING
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s is reporting a media "
            "temperature of %s C which is above the throttling threshold %s C."),
        // EVENT_CODE_DIAG_QUICK_BAD_MEDIA_TEMP_SHUTDOWN
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s is reporting a media "
            "temperature of %s C which is above the shutdown threshold %s C."),
        // EVENT_CODE_DIAG_QUICK_BAD_CORE_TEMP_SHUTDOWN
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s is reporting a core "
            "temperature of %s C which is above the shutdown threshold %s C."),
        // EVENT_CODE_DIAG_QUICK_SPARE_DIE_CONSUMED
        N_TR("The quick health check detected that " NVM_DIMM_NAME " %s is reporting that it "
            "has 0 spare die available."),
        // EVENT_CODE_DIAG_QUICK_UNSAFE_SHUTDOWN
        N_TR("The quick health check detected that the firmware on " NVM_DIMM_NAME " %s "
            "experienced an unsafe shutdown before its latest restart."),
        // Available event 531
        N_TR("Placeholder"),
        // Available event 532
        N_TR("Placeholder"),
        // EVENT_CODE_DIAG_QUICK_AIT_DRAM_NOT_READY
        N_TR("The quick health check detected that the firmware on " NVM_DIMM_NAME " %s "
            "is reporting that the AIT Dram is not ready."),
        // EVENT_CODE_DIAG_QUICK_MEDIA_DISABLED
        N_TR("The quick health check detected that the firmware on " NVM_DIMM_NAME " %s "
            "is reporting that the media is disabled."),
        N_TR("The quick health check detected that the firmware on " NVM_DIMM_NAME " %s "
            "is reporting that the AIT Dram is disabled."),
        N_TR("The quick health check detected that the firmware on " NVM_DIMM_NAME " %s "
            "failed to load successfully."),
        // EVENT_CODE_DIAG_QUICK_INTERNAL_CPU_ERROR
        N_TR("The quick health check detected that the firmware on " NVM_DIMM_NAME " %s "
            "has an internal CPU_error, last known Major:Minor Checkpoint is %s."),
        // EVENT_CODE_DIAG_QUICK_UNKNOWN
        N_TR("The quick health check logged an unknown error code %d."),
    };
    /*
    * Security check diagnostic event message strings.
    * This array ordered by event_code_diag_quick enumeration for easy lookup
    */
    static const char *EVENT_MESSAGES_SECURITY_DIAG[EVENT_CODE_DIAG_SECURITY_UNKNOWN -
        EVENT_CODE_OFFSET_DIAG_SECURITY + 1] =
    {
        // EVENT_CODE_DIAG_SECURITY_SUCCESS
        N_TR("The security check succeeded."),
        // EVENT_CODE_DIAG_SECURITY_NO_DIMMS
        N_TR("The security check detected that there are no " NVM_DIMM_NAME "s."),
        // EVENT_CODE_DIAG_SECURITY_INCONSISTENT
        N_TR("The security check detected that security settings are inconsistent. "
        "%s."),
        // EVENT_CODE_DIAG_SECURITY_ALL_DISABLED
        N_TR("The security check detected that security is disabled on all "
            NVM_DIMM_NAME"s."),
        // EVENT_CODE_DIAG_SECURITY_ALL_NOTSUPPORTED
        N_TR("The security check detected that security is not supported on all "
            NVM_DIMM_NAME"s."),
        // EVENT_CODE_DIAG_SECURITY_UNKNOWN
        N_TR("The security check logged an unknown error code %d."),
    };

    /*
    * Firmware consistency and settings check diagnostic event message strings.
    * This array ordered by event_code_diag_quick enumeration for easy lookup
    */
    static const char *EVENT_MESSAGES_FW_DIAG[EVENT_CODE_DIAG_FW_UNKNOWN -
        EVENT_CODE_OFFSET_DIAG_FW_CONSISTENCY + 1] =
    {
        // EVENT_CODE_DIAG_FW_SUCCESS
        N_TR("The firmware consistency and settings check succeeded."),
        // EVENT_CODE_DIAG_FW_NO_DIMMS
        N_TR("The firmware consistency and settings check detected that there are no "
        NVM_DIMM_NAME"s."),
        // EVENT_CODE_DIAG_FW_INCONSISTENT
        N_TR("The firmware consistency and settings check detected that firmware version "
            "on " NVM_DIMM_NAME "s %s with subsystem device ID %s "
            "is non-optimal, preferred version is %s"),
        // EVENT_CODE_DIAG_FW_BAD_TEMP_MEDIA_THRESHOLD
        N_TR("The firmware consistency and settings check detected that " NVM_DIMM_NAME " %s is "
            "reporting a media temperature threshold of %s C which is above the recommended "
            "threshold %s C."),
        // EVENT_CODE_DIAG_FW_BAD_TEMP_CONTROLLER_THRESHOLD
        N_TR("The firmware consistency and settings check detected that " NVM_DIMM_NAME " %s is "
            "reporting a core temperature threshold of %s C which is above the recommended "
            "threshold %s C."),
        // EVENT_CODE_DIAG_FW_BAD_SPARE_BLOCK
        N_TR("The firmware consistency and settings check detected that " NVM_DIMM_NAME " %s is "
            "reporting a spare capacity threshold of %s%% which is below the recommended "
            "threshold %s%%."),
        // EVENT_CODE_DIAG_FW_BAD_FW_LOG_LEVEL
        N_TR("The firmware consistency and settings check detected that " NVM_DIMM_NAME " %s is "
            "reporting a firmware log level of %s, default log level is %s."),
        // EVENT_CODE_DIAG_FW_SYSTEM_TIME_DRIFT
        N_TR("The firmware consistency and settings check detected that " NVM_DIMM_NAME " %s is %s "
            "the system time by %s seconds."),
        // EVENT_CODE_DIAG_FW_BAD_POWER_MGMT_POLICY
        N_TR("The firmware consistency and settings check detected that " NVM_DIMM_NAME " %s is "
            "reporting that power management policy is not set in accordance with best "
            "practice, due to field '%s'. Valid range is %s."),
        // EVENT_CODE_DIAG_FW_BAD_DIE_SPARING_POLICY
        N_TR("The firmware consistency and settings check detected that " NVM_DIMM_NAME " %s is "
            "reporting that die sparing policy is not set in accordance with best practice, "
            "due to field '%s'. Valid range is %s."),
        // EVENT_CODE_DIAG_FW_UNKNOWN
        N_TR("The firmware consistency and settings check logged an unknown error code %d."),
    };

    /*
    * Platform configuration check diagnostic event message strings.
    * This array ordered by event_code_diag_quick enumeration for easy lookup
    */
    static const char *EVENT_MESSAGES_PCONFIG_DIAG[EVENT_CODE_DIAG_PCONFIG_UNKNOWN -
        EVENT_CODE_OFFSET_DIAG_PLATFORM_CONFIG + 1] =
    {
        // EVENT_CODE_DIAG_PCONFIG_SUCCESS
        N_TR("The platform configuration check succeeded."),
        // EVENT_CODE_DIAG_PCONFIG_NO_DIMMS
        N_TR("The platform configuration check detected that there are no " NVM_DIMM_NAME "s."),
        // EVENT_CODE_DIAG_PCONFIG_INVALID_NFIT
        N_TR("The platform configuration check detected that the NFIT table is invalid."),
        // EVENT_CODE_DIAG_PCONFIG_INVALID_PCAT
        N_TR("The platform configuration check detected that the platform capability table is "
        "invalid."),
        // EVENT_CODE_DIAG_PCONFIG_INVALID_PCD
        N_TR("The platform configuration check detected that the platform configuration data is "
            "invalid for " NVM_DIMM_NAME " %s."),
        // EVENT_CODE_DIAG_PCONFIG_INVALID_CURRENT_PCD
        N_TR("The platform configuration check detected that the current configuration table is "
            "invalid for " NVM_DIMM_NAME " %s."),
        // EVENT_CODE_DIAG_PCONFIG_UNCONFIGURED
        N_TR("The platform configuration check detected that " NVM_DIMM_NAME " %s is not configured."),
        // EVENT_CODE_DIAG_PCONFIG_BROKEN_ISET
        N_TR("The platform configuration check detected that interleave set %s is broken due to "
            "missing " NVM_DIMM_NAME "(s): %s."),
        // EVENT_CODE_DIAG_PCONFIG_DUPLICATE_SERIAL_NUMBERS
        N_TR("The platform configuration check detected %s DIMMs installed on the platform with "
            "the same serial number %s."),
        // EVENT_CODE_DIAG_PCONFIG_REBOOT_NEEDED_TO_APPLY_GOAL
        N_TR("The platform configuration check detected that " NVM_DIMM_NAME " %s has "
            "a goal configuration that has not yet been applied. A system reboot "
            "is required for the new configuration to take effect."),
        // Available event 610
        N_TR("The platform configuration check detected that the capacity of App Direct "
            "namespace %s is smaller than its containing interleave set. The remaining %s "
            "of the interleave set cannot be accessed as App Direct."),
        // EVENT_CODE_DIAG_PCONFIG_POOL_NEEDS_APP_DIRECT_NAMESPACES
        N_TR("The platform configuration check detected that pool %s contains empty App Direct "
            "capacity. Create App Direct namespaces to access this capacity."),
        // Available event 612
        N_TR("Placeholder"),
        // EVENT_CODE_DIAG_PCONFIG_DIMM_CONFIG_UNBALANCED
        N_TR("The platform configuration check detected that " NVM_DIMM_NAME "s on "
            "socket %s are arranged in an unbalanced configuration. "
            "These " NVM_DIMM_NAME "s may experience reduced performance."),
        // EVENT_CODE_DIAG_PCONFIG_DIMMS_DIFFERENT_SIZES
        N_TR("The platform configuration check detected that manageable " NVM_DIMM_NAME "s on "
            "socket %s have different capacities. Memory allocations on "
            "this socket may be non-optimal."),
        // EVENT_CODE_DIAG_PCONFIG_DIMM_SECURITY_SKUS_MIXED
        N_TR("The platform configuration check detected that the " NVM_DIMM_NAME " security "
            "capability SKUs are inconsistent within the system. Functionality will be "
            "limited until the situation is corrected."),
        // EVENT_CODE_DIAG_PCONFIG_DIMM_MODE_SKUS_MIXED
        N_TR("The platform configuration check detected that the " NVM_DIMM_NAME " mode "
            "capability SKUs are inconsistent within the system. Functionality will "
            "be limited until the situation is corrected."),
        // EVENT_CODE_DIAG_PCONFIG_DIMM_DIE_SPARING_SKUS_MIXED
        N_TR("The platform configuration check detected that the " NVM_DIMM_NAME "s die "
            "sparing capability SKUs are inconsistent within the system. Functionality "
            "will be limited until the situation is corrected."),
        // EVENT_CODE_DIAG_PCONFIG_DIMM_INIT_FAILED
        N_TR("The platform configuration check detected that an " NVM_DIMM_NAME " with "
            "physical ID %s is present in the system but failed to initialize."),
        // EVENT_CODE_DIAG_PCONFIG_DIMM_SKU_VIOLATION
        N_TR("The platform configuration check detected that " NVM_DIMM_NAME " %s is "
            "configured with %s capacity which is in violation of the system supported "
            "capabilities."),
        // EVENT_CODE_DIAG_PCONFIG_DIMM_GOAL_SKU_VIOLATION
        N_TR("The platform configuration check detected that " NVM_DIMM_NAME " %s has a "
            "configuration goal request for %s capacity which will be in violation "
            "of the system supported capabilities."),
        // EVENT_CODE_DIAG_PCONFIG_POOLS_FAILED
        N_TR("The platform configuration check was unable to retrieve the pool information."),
        // EVENT_CODE_DIAG_PCONFIG_NAMESPACES_FAILED
        N_TR("The platform configuration check was unable to retrieve the namespace information."),
        // EVENT_CODE_DIAG_PCONFIG_NO_BIOS_SUPPORT
        N_TR("The platform configuration check detected that the BIOS settings do not currently "
            "allow memory provisioning from this software."),
        // EVENT_CODE_DIAG_PCONFIG_GOAL_FAILED_CONFIG_ERROR
        N_TR("The platform configuration check detected that the BIOS could not apply the "
            "configuration goal on " NVM_DIMM_NAME " %s because of errors in the goal data. "
            "The detailed status is %s."),
        // EVENT_CODE_DIAG_PCONFIG_GOAL_FAILED_INSUFFICIENT_RESOURCES
        N_TR("The platform configuration check detected that the BIOS could not apply the "
            "configuration goal on " NVM_DIMM_NAME " %s because the system has insufficient "
            "resources. The detailed status is %s."),
        // EVENT_CODE_DIAG_PCONFIG_GOAL_FAILED_FW_ERROR
        N_TR("The platform configuration check detected that the BIOS could not apply the "
            "configuration goal on " NVM_DIMM_NAME " %s because of a firmware error. "
            "The detailed status is %s."),
        // EVENT_CODE_DIAG_PCONFIG_GOAL_FAILED_UNKNOWN
        N_TR("The platform configuration check detected that the BIOS could not apply the "
            "configuration goal on " NVM_DIMM_NAME " %s for an unknown reason. "
            "The detailed status is %s."),
        // EVENT_CODE_DIAG_PCONFIG_BROKEN_ISET_MOVED
        N_TR("The platform configuration check detected that interleave set %s is broken "
            "because the " NVM_DIMM_NAME "(s) were moved."),

        // EVENT_CODE_DIAG_PCONFIG_UNKNOWN
        N_TR("The platform configuration check logged an unknown error code %d."),
    };

#ifdef __cplusplus
}
#endif

#endif //_EVENT_STRINGS_H_
