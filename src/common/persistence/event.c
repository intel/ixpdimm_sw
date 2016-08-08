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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "event.h"
#include "lib_persistence.h"
#include "config_settings.h"
#include "logging.h"
#include <string/s_str.h>
#include <uid/uid.h>
#include <cr_i18n.h>
#include <os/os_adapter.h>
#include "schema.h"

/*
 * ****************************************************************************
 * EVENT STRING CONSTANTS
 * **************************************************************************
 */

/*
 * Config event message strings.
 * This array ordered by event_code_config enumeration for easy lookup
 */
const char *EVENT_MESSAGES_CONFIG[EVENT_CODE_CONFIG_UNKNOWN -
	EVENT_CODE_OFFSET_CONFIG + 1] =
{
	// EVENT_CODE_CONFIG_PLATFORM_CONFIG_INVALID
	N_TR("The platform configuration data on " NVM_DIMM_NAME " %s is invalid."),
	// EVENT_CODE_CONFIG_CONFIG_GOAL_APPLIED
	N_TR("The BIOS has successfully applied the configuration goal on " NVM_DIMM_NAME " %s."),
	// EVENT_CODE_CONFIG_CONFIG_GOAL_FAILED_CONFIG_ERROR
	N_TR("The BIOS could not apply the configuration goal on " NVM_DIMM_NAME " %s because of "
			"errors in the goal data."),
	// EVENT_CODE_CONFIG_CONFIG_GOAL_FAILED_INSUFFICIENT_RESOURCES
	N_TR("The BIOS could not apply the configuration goal on " NVM_DIMM_NAME " %s "
			"because the system has insufficient resources."),
	// EVENT_CODE_CONFIG_CONFIG_GOAL_FAILED_FW_ERROR
	N_TR("The BIOS could not apply the configuration goal on " NVM_DIMM_NAME " %s because of a "
			"firmware error."),
	// EVENT_CODE_CONFIG_CONFIG_GOAL_FAILED_UNKNOWN
	N_TR("The BIOS could not apply the configuration goal on " NVM_DIMM_NAME " %s "
			"for an unknown reason."),
	// EVENT_CODE_CONFIG_DEVICE_ADDED_CONFIGURED
	N_TR("The BIOS has detected and successfully configured " NVM_DIMM_NAME " %s."),
	// EVENT_CODE_CONFIG_DEVICE_ADDED_NEW
	N_TR("The BIOS has detected a new " NVM_DIMM_NAME " %s. The new " NVM_DIMM_NAME " must "
			"be configured in order to be used."),
	// EVENT_CODE_CONFIG_DEVICE_MISSING
	N_TR(NVM_DIMM_NAME" %s is missing."),
	// EVENT_CODE_CONFIG_DEVICE_REPLACED_CONFIGURED
	N_TR(NVM_DIMM_NAME" %s has been replaced with " NVM_DIMM_NAME " %s."),
	// EVENT_CODE_CONFIG_DEVICE_REPLACED_NEW
	N_TR(NVM_DIMM_NAME" %s has been replaced with a new " NVM_DIMM_NAME " %s. "
			"The new " NVM_DIMM_NAME " must be configured in order to be used."),
	// EVENT_CODE_CONFIG_DEVICE_MOVED
	N_TR(NVM_DIMM_NAME" %s has been moved to a different position."),
	// EVENT_CODE_CONFIG_GOAL_SKU_VIOLATION
	N_TR("A configuration was requested for " NVM_DIMM_NAME " %s that "
			"is unsupported due to a licensing issue."),
	// EVENT_CODE_CONFIG_SKU_VIOLATION
	N_TR("The configuration of " NVM_DIMM_NAME " %s is unsupported due to a licensing issue."),
	// EVENT_CODE_CONFIG_BAD_DRIVER
	N_TR("The underlying software is missing or incompatible with this version of the "
			"management software."),
	// EVENT_CODE_CONFIG_NOT_MANAGEABLE
	N_TR(NVM_DIMM_NAME" %s is not manageable by this version of the management software."),

	// EVENT_CODE_CONFIG_UNKNOWN
	N_TR("A platform configuration change has logged an unknown event code %d.")
};

/*
 * Management event message strings.
 * This array ordered by event_code_mgmt enumeration for easy lookup
 */
const char *EVENT_MESSAGES_MGMT[EVENT_CODE_MGMT_UNKNOWN -
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
const char *EVENT_MESSAGES_HEALTH[EVENT_CODE_HEALTH_UNKNOWN -
	EVENT_CODE_OFFSET_HEALTH + 1] =
{
	// EVENT_CODE_HEALTH_LOW_SPARE_CAPACITY
	N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
			"is reporting percentage used at %s% which is above the alarm "
			"threshold %s%%."),
	// EVENT_CODE_HEALTH_MEDIA_TEMPERATURE_OVER_THRESHOLD
	N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
			"is reporting a media temperature of %s C which is above the alarm "
			"threshold of %s C."),
	// EVENT_CODE_HEALTH_MEDIA_TEMPERATURE_UNDER_THRESHOLD
	N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
			"is reporting a media temperature of %s C which is below the alarm "
			"threshold of %s C."),
	// EVENT_CODE_HEALTH_HIGH_WEARLEVEL
	N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
			"is reporting a wear level of %s% which is above the alarm threshold %s%%."),
	// EVENT_CODE_HEALTH_NEW_MEDIAERRORS_FOUND
	N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
			"is reporting new %s media errors, total count is %s."),
	// EVENT_CODE_HEALTH_HEALTH_STATE_CHANGED
	N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
			"is reporting a change in health state from %s to %s."),
	// EVENT_CODE_HEALTH_SPARE_DIE_CONSUMED
	N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
			"is reporting that it has consumed %s spare die."),
	// EVENT_CODE_HEALTH_NAMESPACE_HEALTH_STATE_CHANGED
	N_TR("The health monitor has detected that namespace %s "
			"is reporting a change in health state from %s to %s."),
	// EVENT_CODE_HEALTH_UNSAFE_SHUTDOWN
	N_TR("The firmware on " NVM_DIMM_NAME " %s experienced an unsafe shutdown "
			"before its latest restart."),
	// EVENT_CODE_HEALTH_CONTROLLER_TEMPERATURE_OVER_THRESHOLD
	N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
			"is reporting a controller temperature of %s C which is above the alarm "
			"threshold of %s C."),
	// EVENT_CODE_HEALTH_CONTROLLER_TEMPERATURE_UNDER_THRESHOLD
	N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
			"is reporting a controller temperature of %s C which is below the alarm "
			"threshold of %s C."),
	// EVENT_CODE_HEALTH_NEW_FWERRORS_FOUND
	N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
			"firmware has reported %s new errors."),
	// EVENT_CODE_HEALTH_VIRAL_STATE
	N_TR("The health monitor has detected that " NVM_DIMM_NAME " %s "
			"is reporting a viral state."),
	// EVENT_CODE_HEALTH_MIXED_SKU
	N_TR("The health monitor has detected that one or more " NVM_DIMM_NAME "s "
			"in the system have different SKUs."),
	// EVENT_CODE_HEALTH_SANITIZE_IN_PROGRESS
	N_TR("The health monitor has detected that a sanitize operation is in progress "
			"on " NVM_DIMM_NAME " %s. A reboot will be required when complete "
			"to use the " NVM_DIMM_NAME "."),
	// EVENT_CODE_HEALTH_SANITIZE_COMPLETE
	N_TR("The health monitor has detected that a sanitize operation has completed "
			"on " NVM_DIMM_NAME " %s. A reboot is required to use the " NVM_DIMM_NAME "."),
	// EVENT_CODE_HEALTH_UNKNOWN
	N_TR("The health monitor has logged an unknown error code %d."),
};

/*
 * Quick check diagnostic event message strings.
 * This array ordered by event_code_diag_quick enumeration for easy lookup
 */
const char *EVENT_MESSAGES_QUICK_DIAG[EVENT_CODE_DIAG_QUICK_UNKNOWN -
	EVENT_CODE_OFFSET_DIAG_QUICK + 1] =
{
	// EVENT_CODE_DIAG_QUICK_SUCCESS
	N_TR("The quick health check succeeded."),
	// EVENT_CODE_DIAG_QUICK_INVALID_VENDORID
	N_TR("The quick health check detected an unrecognized " NVM_DIMM_NAME " %s "
			"due to an invalid vendor identifier %s, expected %s. "
			"No further testing performed on this " NVM_DIMM_NAME "."),
	// EVENT_CODE_DIAG_QUICK_INVALID_MANUFACTURER
	N_TR("The quick health check detected an unrecognized " NVM_DIMM_NAME " %s "
			"due to an invalid manufacturer %s, expected %s. "
			"No further testing performed on this " NVM_DIMM_NAME "."),
	// EVENT_CODE_DIAG_QUICK_INVALID_MODEL_NUMBER
	N_TR("The quick health check detected an unrecognized " NVM_DIMM_NAME " %s "
			"due to an invalid model number %s, expected %s. "
			"No further testing performed on this " NVM_DIMM_NAME "."),
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
			"the alarm threshold %s%."),
	// EVENT_CODE_DIAG_QUICK_BAD_UNCORRECTABLE_MEDIA_ERRORS
	N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
			"is reporting %s uncorrectable media errors."),
	// EVENT_CODE_DIAG_QUICK_BAD_CORRECTED_MEDIA_ERRORS
	N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
			"is reporting %s corrected media errors."),
	// EVENT_CODE_DIAG_QUICK_BAD_ERASURE_CODED_CORRECTED_MEDIA_ERRORS
	N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
			"is reporting %s Erasure Coded Corrected media errors."),
	// EVENT_CODE_DIAG_QUICK_BAD_CONTROLLER_TEMP
	N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
			"is reporting a controller temperature of %s C which is above "
			"the alarm threshold %s C."),
	// EVENT_CODE_DIAG_QUICK_BAD_POWER_LIMITATION
	N_TR("The quick health check detected that power is limited on socket %s."),
	// EVENT_CODE_DIAG_QUICK_UNREADABLE_BSR
	N_TR("The quick health check detected that the boot status register "
			"of " NVM_DIMM_NAME " %s is not readable."),
	// EVENT_CODE_DIAG_QUICK_MEDIA_NOT_READY
	N_TR("The quick health check detected that the firmware "
			"on " NVM_DIMM_NAME " %s is reporting that the media is not ready."),
	// EVENT_CODE_DIAG_QUICK_MEDIA_READY_ERROR
	N_TR("The quick health check detected that the firmware "
			"on " NVM_DIMM_NAME " %s is reporting an error in the media."),
	// EVENT_CODE_DIAG_QUICK_DDRT_IO_INIT_NOT_READY
	N_TR("The quick health check detected that the firmware "
			"on " NVM_DIMM_NAME " %s is reporting that the DDRT "
			"I/O Initialization has not completed."),
	// EVENT_CODE_DIAG_QUICK_DDRT_IO_INIT_ERROR
	N_TR("The quick health check detected that the firmware "
			"on " NVM_DIMM_NAME " %s is reporting an error initializing DDRT I/O."),
	// EVENT_CODE_DIAG_QUICK_MAILBOX_INTERFACE_NOT_READY
	N_TR("The quick health check detected that the firmware "
			"on " NVM_DIMM_NAME " %s is reporting that "
			"the mailbox interface is not ready."),
	// EVENT_CODE_DIAG_QUICK_NO_POST_CODE
	N_TR("The quick health check detected that " NVM_DIMM_NAME " %s failed "
			"to initialize BIOS POST testing."),
	// EVENT_CODE_DIAG_QUICK_FW_INITIALIZATION_INCOMPLETE
	N_TR("The quick health check detected that the firmware "
			"on " NVM_DIMM_NAME " %s has not initialized "
			"successfully, the last known Major:Minor Checkpoint is %s."),
	// EVENT_CODE_DIAG_QUICK_FW_HIT_ASSERT
	N_TR("The quick health check detected that the firmware "
			"on " NVM_DIMM_NAME " %s reported an assert."),
	// EVENT_CODE_DIAG_QUICK_FW_STALLED
	N_TR("The quick health check detected that the firmware on "NVM_DIMM_NAME
			" %s has stalled the media interface engine."),
	// EVENT_CODE_DIAG_QUICK_VIRAL_STATE
	N_TR("The quick health check detected that " NVM_DIMM_NAME " %s "
			"is reporting a viral state."),
	// EVENT_CODE_DIAG_QUICK_BAD_DRIVER
	N_TR("The quick health check detected that the underlying software is missing "
			"or incompatible with this version of the management software."),
	// EVENT_CODE_DIAG_QUICK_NOT_MANAGEABLE
	N_TR("The quick health check detected that " NVM_DIMM_NAME " %s is not manageable by "
			"this version of the management software."),
	// EVENT_CODE_DIAG_QUICK_UNKNOWN
	N_TR("The quick health check logged an unknown error code %d."),
};
/*
 * Security check diagnostic event message strings.
 * This array ordered by event_code_diag_quick enumeration for easy lookup
 */
const char *EVENT_MESSAGES_SECURITY_DIAG[EVENT_CODE_DIAG_SECURITY_UNKNOWN -
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
const char *EVENT_MESSAGES_FW_DIAG[EVENT_CODE_DIAG_FW_UNKNOWN -
    EVENT_CODE_OFFSET_DIAG_FW_CONSISTENCY + 1] =
{
	// EVENT_CODE_DIAG_FW_SUCCESS
	N_TR("The firmware consistency and settings check succeeded."),
	// EVENT_CODE_DIAG_FW_NO_DIMMS
	N_TR("The firmware consistency and settings check detected that there are no "
			NVM_DIMM_NAME"s."),
	// EVENT_CODE_DIAG_FW_INCONSISTENT
	N_TR("The firmware consistency and settings check detected that firmware version "
			"on " NVM_DIMM_NAME "s %s with model number %s "
			"is non-optimal, preferred version is %s"),
	// EVENT_CODE_DIAG_FW_BAD_TEMP_MEDIA_THRESHOLD
	N_TR("The firmware consistency and settings check detected that " NVM_DIMM_NAME " %s is "
			"reporting a media temperature threshold of %s C which is above the recommended "
			"threshold %s C."),
	// EVENT_CODE_DIAG_FW_BAD_TEMP_CONTROLLER_THRESHOLD
	N_TR("The firmware consistency and settings check detected that " NVM_DIMM_NAME " %s is "
			"reporting a controller temperature threshold of %s C which is above the recommended "
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
const char *EVENT_MESSAGES_PCONFIG_DIAG[EVENT_CODE_DIAG_PCONFIG_UNKNOWN -
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
	N_TR("The platform configuration check detected that " NVM_DIMM_NAME " %s is unconfigured. "
			"Note that the capacity is unmapped by the BIOS because it's unconfigured."),
	// EVENT_CODE_DIAG_PCONFIG_BROKEN_ISET
	N_TR("The platform configuration check detected that interleave set %s is broken, due to a "
			"missing " NVM_DIMM_NAME " with serial number %s."),
	// EVENT_CODE_DIAG_PCONFIG_MAPPED_CAPACITY_MISMATCH
	N_TR("The platform configuration check detected that the BIOS reported mapped %s capacity "
			"%s doesn't match the driver reported capacity %s."),
	// EVENT_CODE_DIAG_PCONFIG_RECOMMENDED_INTERLEAVE_SIZE_IMC
	N_TR("The platform configuration check detected interleave set %s has an inter-memory "
			"controller interleave size of %s. Interleave sizes of 4KB and greater may improve "
			"performance."),
	// EVENT_CODE_DIAG_PCONFIG_RECOMMENDED_INTERLEAVE_SIZE_CHANNEL
	N_TR("The platform configuration check detected interleave set %s has an inter-channel "
			"interleave size of %s. Interleave sizes of 4KB and greater may improve "
			"performance."),
	// EVENT_CODE_DIAG_PCONFIG_RECOMMENDED_INTERLEAVE_WAYS
	N_TR("The platform configuration check detected that interleave set %s is interleaved "
			"across %s " NVM_DIMM_NAME "s, less than the recommended %s. Interleaving "
			"across as many " NVM_DIMM_NAME "s as possible may improve performance."),
	// EVENT_CODE_DIAG_PCONFIG_REBOOT_NEEDED_TO_APPLY_GOAL
	N_TR("The platform configuration check detected that " NVM_DIMM_NAME " %s has "
			"a goal configuration that has not yet been applied. A system reboot "
			"is required for the new configuration to take effect."),
	// EVENT_CODE_DIAG_PCONFIG_APP_DIRECT_NAMESPACE_TOO_SMALL
	N_TR("The platform configuration check detected that the capacity of App Direct "
			"namespace %s is smaller than the capacity of its containing extent. The remaining %s "
			"of the extent cannot be accessed as App Direct. This capacity may "
			"be included in Storage namespaces on each " NVM_DIMM_NAME "."),
	// EVENT_CODE_DIAG_PCONFIG_POOL_NEEDS_APP_DIRECT_NAMESPACES
	N_TR("The platform configuration check detected that pool %s contains empty App Direct "
			"capacity. Create App Direct namespaces in the pool to access this "
			"capacity."),
	// EVENT_CODE_DIAG_PCONFIG_POOL_NEEDS_STORAGE_NAMESPACES
	N_TR("The platform configuration check detected that pool %s contains empty Storage "
			"capacity. Create Storage namespaces in the pool to access this capacity."),
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
			"currently configured with %s capacity which is in violation "
			"of the system supported capabilities."),
	// EVENT_CODE_DIAG_PCONFIG_DIMM_GOAL_SKU_VIOLATION
	N_TR("The platform configuration check detected that " NVM_DIMM_NAME " %s has a "
			"configuration goal request for %s capacity which will be in violation "
			"of the system supported capabilities."),
	// EVENT_CODE_DIAG_PCONFIG_POOLS_FAILED
	N_TR("The platform configuration check was unable to retrieve the pool information."),
	// EVENT_CODE_DIAG_PCONFIG_NAMESPACES_FAILED
	N_TR("The platform configuration check was unable to retrieve the namespace information."),

	// EVENT_CODE_DIAG_PCONFIG_UNKNOWN
	N_TR("The platform configuration check logged an unknown error code %d."),
};

/*
 * Driver metadata check diagnostic event message strings.
 */
const char *EVENT_MESSAGES_DIAG_PM_METADATA_CHECK[EVENT_CODE_DIAG_PM_META_UNKNOWN -
	EVENT_CODE_OFFSET_DIAG_PM_META] =
{
	// EVENT_CODE_DIAG_PM_META_NS_OK - 0
	N_TR("The PM Metadata check detected no namespace health issues."),
	// EVENT_CODE_DIAG_PM_META_NS_MISSING - 1
	N_TR("The PM Metadata check detected that a namespace is missing."),
	// EVENT_CODE_DIAG_PM_META_NS_MISSING_LABEL - 2
	N_TR("The PM Metadata check detected that the number of labels for the namespace "
			"from the label area of the Platform Configuration Data does not match "
			"the nlabel count in the first label."),
	// EVENT_CODE_DIAG_PM_META_NS_CORRUPT_INTERLEAVE_SET - 3
	N_TR("The PM Metadata check detected more than one App Direct namespace "
			"for a given interleave set."),
	// EVENT_CODE_DIAG_PM_META_NS_INCONSISTENT_LABELS - 4
	N_TR("The PM Metadata check detected that the nlabel or lbasize fields "
			"in the Namespace Labels do not all have the same values."),
	// EVENT_CODE_DIAG_PM_META_NS_INVALID_BLOCK_SIZE - 5
	N_TR("The PM Metadata check detected the namespace contains an unsupported "
			"logical block size."),
	// EVENT_CODE_DIAG_PM_META_NS_CORRUPT_BTT_METADATA - 6
	N_TR("The PM Metadata check detected that the BTT metadata checks fail.")
	// EVENT_CODE_DIAG_PM_META_LABEL_OK - 7
	N_TR("The PM Metadata check detected no label area health issues."),
	// EVENT_CODE_DIAG_PM_META_LABEL_MISSING_PCD - 8
	N_TR("The PM Metadata check was unable to retrieve Platform Configuration Data "
			"from the " NVM_DIMM_NAME "."),
	// EVENT_CODE_DIAG_PM_META_LABEL_MISSING_VALID_INDEX_BLOCK - 9
	N_TR("The PM Metadata check detected no valid Namespace Index Blocks or an uninitialized "
			"label area in the Platform Configuration Data."),
	// EVENT_CODE_DIAG_PM_META_LABEL_INSUFFICIENT_PM - 10
	N_TR("The PM Metadata check detected a Storage Namespace described in the "
			"Platform Configuration Data is bigger than the " NVM_DIMM_NAME "'s "
			"available persistent memory."),
	// EVENT_CODE_DIAG_PM_META_LABEL_LABELS_OVERLAP - 11
	N_TR("The PM Metadata check detected Namespace Labels overlapping each other.")
	// EVENT_CODE_DIAG_PM_META_UNKNOWN
	N_TR("The PM Metadata check logged an unknown error code %d."),
};

/*
 * Set mgmt library event messages
 */
void populate_event_message(struct event *p_event)
{
	switch (p_event->type)
	{
		case EVENT_TYPE_DIAG_QUICK:
			// check for a valid offset
			if ((p_event->code >= EVENT_CODE_OFFSET_DIAG_QUICK) &&
					(p_event->code < EVENT_CODE_DIAG_QUICK_UNKNOWN))
			{
				s_strcpy(p_event->message,
						EVENT_MESSAGES_QUICK_DIAG[p_event->code - EVENT_CODE_OFFSET_DIAG_QUICK],
						NVM_EVENT_MSG_LEN);
			}
			else
			{
				// unknown error code
				s_snprintf(p_event->message, NVM_EVENT_MSG_LEN,
						EVENT_MESSAGES_QUICK_DIAG[EVENT_CODE_DIAG_QUICK_UNKNOWN -
						EVENT_CODE_OFFSET_DIAG_QUICK], p_event->code);
			}
			break;
		case EVENT_TYPE_DIAG_SECURITY:
			// check for a valid offset
			if ((p_event->code >= EVENT_CODE_OFFSET_DIAG_SECURITY) &&
					(p_event->code < EVENT_CODE_DIAG_SECURITY_UNKNOWN))
			{
				s_strcpy(p_event->message,
				EVENT_MESSAGES_SECURITY_DIAG[p_event->code - EVENT_CODE_OFFSET_DIAG_SECURITY],
				NVM_EVENT_MSG_LEN);
			}
			else
			{
				// unknown error code
				s_snprintf(p_event->message, NVM_EVENT_MSG_LEN,
						EVENT_MESSAGES_SECURITY_DIAG[EVENT_CODE_DIAG_SECURITY_UNKNOWN -
						EVENT_CODE_OFFSET_DIAG_SECURITY], p_event->code);
			}
			break;
		case EVENT_TYPE_DIAG_FW_CONSISTENCY:
			// check for a valid offset
			if ((p_event->code >= EVENT_CODE_OFFSET_DIAG_FW_CONSISTENCY) &&
					(p_event->code < EVENT_CODE_DIAG_FW_UNKNOWN))
			{
				s_strcpy(p_event->message, EVENT_MESSAGES_FW_DIAG[p_event->code
						- EVENT_CODE_OFFSET_DIAG_FW_CONSISTENCY], NVM_EVENT_MSG_LEN);
			}
			else
			{
				// unknown error code
				s_snprintf(p_event->message, NVM_EVENT_MSG_LEN,
						EVENT_MESSAGES_FW_DIAG[EVENT_CODE_DIAG_FW_UNKNOWN
								- EVENT_CODE_OFFSET_DIAG_FW_CONSISTENCY],
						p_event->code);
			}
			break;

		case EVENT_TYPE_MGMT:
			// check for a valid offset
			if ((p_event->code >= EVENT_CODE_OFFSET_MGMT) &&
					(p_event->code < EVENT_CODE_MGMT_UNKNOWN))
			{
				s_strcpy(p_event->message, EVENT_MESSAGES_MGMT[p_event->code
						- EVENT_CODE_OFFSET_MGMT], NVM_EVENT_MSG_LEN);
			}
			else
			{
				// unknown error code
				s_snprintf(p_event->message, NVM_EVENT_MSG_LEN,
						EVENT_MESSAGES_MGMT[EVENT_CODE_MGMT_UNKNOWN
								- EVENT_CODE_OFFSET_MGMT],
						p_event->code);
			}
			break;

		case EVENT_TYPE_HEALTH:
			// check for a valid offset
			if ((p_event->code >= EVENT_CODE_OFFSET_HEALTH) &&
					(p_event->code < EVENT_CODE_HEALTH_UNKNOWN))
			{
				s_strcpy(p_event->message, EVENT_MESSAGES_HEALTH[p_event->code
						- EVENT_CODE_OFFSET_HEALTH], NVM_EVENT_MSG_LEN);
			}
			else
			{
				// unknown error code
				s_snprintf(p_event->message, NVM_EVENT_MSG_LEN,
						EVENT_MESSAGES_HEALTH[EVENT_CODE_HEALTH_UNKNOWN
						- EVENT_CODE_OFFSET_HEALTH],
						p_event->code);
			}
			break;

		case EVENT_TYPE_CONFIG:
			// check for a valid offset
			if ((p_event->code >= EVENT_CODE_OFFSET_CONFIG) &&
					(p_event->code < EVENT_CODE_CONFIG_UNKNOWN))
			{
				s_strcpy(p_event->message, EVENT_MESSAGES_CONFIG[p_event->code
						- EVENT_CODE_OFFSET_CONFIG], NVM_EVENT_MSG_LEN);
			}
			else
			{
				// unknown error code
				s_snprintf(p_event->message, NVM_EVENT_MSG_LEN,
						EVENT_MESSAGES_CONFIG[EVENT_CODE_CONFIG_UNKNOWN
								- EVENT_CODE_OFFSET_CONFIG],
						p_event->code);
			}
			break;

		case EVENT_TYPE_DIAG_PLATFORM_CONFIG:
			// check for a valid offset
			if ((p_event->code >= EVENT_CODE_OFFSET_DIAG_PLATFORM_CONFIG) &&
					(p_event->code < EVENT_CODE_DIAG_PCONFIG_UNKNOWN))
			{
				s_strcpy(p_event->message, EVENT_MESSAGES_PCONFIG_DIAG[p_event->code
						- EVENT_CODE_OFFSET_DIAG_PLATFORM_CONFIG], NVM_EVENT_MSG_LEN);
			}
			else
			{
				// unknown error code
				s_snprintf(p_event->message, NVM_EVENT_MSG_LEN,
						EVENT_MESSAGES_PCONFIG_DIAG[EVENT_CODE_DIAG_PCONFIG_UNKNOWN
								- EVENT_CODE_OFFSET_DIAG_PLATFORM_CONFIG],
						p_event->code);
			}
			break;

		// TODO: Not yet supported
		case EVENT_TYPE_DIAG_PM_META:

		// invalid type, leave empty
		case EVENT_TYPE_DIAG:
		case EVENT_TYPE_ALL:
		default:
			break;
	}
}


/*
 * Helper function to copy a list of params into an event struct
 */
void params_to_event(const enum event_type type, const enum event_severity severity,
		const NVM_UINT16 code, const NVM_UID device_uid, const NVM_BOOL action_required,
		const NVM_EVENT_ARG arg1, const NVM_EVENT_ARG arg2, const NVM_EVENT_ARG arg3,
		const enum diagnostic_result result, struct event *p_event)
{
	COMMON_LOG_ENTRY();

	if (p_event != NULL)
	{
		p_event->type = type;
		p_event->severity = severity;
		p_event->code = code;
		p_event->action_required = action_required;
		if (device_uid)
		{
			memmove(p_event->uid, device_uid, NVM_MAX_UID_LEN);
		}
		s_strcpy(p_event->args[0], arg1, NVM_EVENT_ARG_LEN);
		s_strcpy(p_event->args[1], arg2, NVM_EVENT_ARG_LEN);
		s_strcpy(p_event->args[2], arg3, NVM_EVENT_ARG_LEN);
		p_event->diag_result = result;
	}

	COMMON_LOG_EXIT();
}

/*
 * Store an event log entry in the db
 */
int store_event(struct event *p_event, COMMON_BOOL syslog)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	PersistentStore *p_store = get_lib_store();
	if (!p_store)
	{
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		struct db_event db_event;
		memset(&db_event, 0, sizeof (struct db_event));

		// Note: db_event.id will be auto generated by sqlite
		// get current time
		time_t time_now;
		time_now = time(NULL);
		db_event.time = time_now;

		// translate event struct into db event struct
		db_event.type = p_event->type;
		db_event.severity = p_event->severity;
		db_event.code = p_event->code;
		db_event.action_required = p_event->action_required;
		uid_copy(p_event->uid, db_event.uid);
		s_strcpy(db_event.arg1, p_event->args[0], NVM_EVENT_ARG_LEN);
		s_strcpy(db_event.arg2, p_event->args[1], NVM_EVENT_ARG_LEN);
		s_strcpy(db_event.arg3, p_event->args[2], NVM_EVENT_ARG_LEN);
		db_event.diag_result = p_event->diag_result;

		// store it
		if (db_add_event(p_store, &db_event) == DB_SUCCESS)
		{
			// roll table
			int max_events = 10000; // default if key is missing
			int defaultTrimPercent = 10;
			int trim_percent = defaultTrimPercent;
			get_bounded_config_value_int(SQL_KEY_EVENT_LOG_MAX, &max_events);
			get_bounded_config_value_int(SQL_KEY_EVENT_LOG_TRIM_PERCENT, &trim_percent);
			if (trim_percent < 0)
			{
				trim_percent = defaultTrimPercent;
			}

			int event_count = 0;
			table_row_count(p_store, "event", &event_count);
			if (event_count >= max_events)
			{
				int trim_events = (trim_percent * max_events/100);
				char sql[1024];
				s_snprintf(sql, 1024,
						"DELETE FROM event "
						"where id IN "
						"(SELECT id FROM event where action_required = 0 ORDER BY id LIMIT %d)",
						trim_events);
				if (db_run_custom_sql(p_store, sql) != DB_SUCCESS)
				{
					COMMON_LOG_ERROR("Failed to trim the event log");
				}
			}
		}
		else
		{
			// database issue
			COMMON_LOG_ERROR("Failed to store an event in the database");
			rc = NVM_ERR_UNKNOWN;
		}
	}

	// store the event in the syslog
	if (syslog)
	{
		// get the message
		populate_event_message(p_event);
		log_event_in_syslog(p_event, NVM_SYSLOG_SOURCE);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Store an event log entry in the db
 */
int store_event_by_parts(const enum event_type type, const enum event_severity severity,
		const NVM_UINT16 code, const NVM_UID device_uid, const NVM_BOOL action_required,
		const NVM_EVENT_ARG arg1, const NVM_EVENT_ARG arg2, const NVM_EVENT_ARG arg3,
		const enum diagnostic_result result)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct event event;
	memset(&event, 0, sizeof (struct event));
	params_to_event(type, severity, code, device_uid, action_required, arg1,
			arg2, arg3, result, &event);

	rc = store_event(&event, 1);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL log_event_in_syslog(const struct event *p_event, const char *source)
{
	COMMON_LOG_ENTRY();

	NVM_BOOL result = 0;
	if (p_event != NULL)
	{
		// limit tracing to critical and fatal events
		if ((int)p_event->severity >= EVENT_SEVERITY_CRITICAL)
		{
			log_system_event(SYSTEM_EVENT_TYPE_ERROR, source, p_event->message);

			result = 1;
		}
	}

	COMMON_LOG_EXIT();
	return result;
}

NVM_BOOL event_matches_filter(const struct event_filter *p_filter,
		const struct db_event *p_db_event)
{
	int rc = 1;
	COMMON_LOG_ENTRY();

	if (p_filter) // no filter is a match
	{
		// match type
		if (p_filter->filter_mask & NVM_FILTER_ON_TYPE)
		{
			// allow filter all
			if (p_filter->type != EVENT_TYPE_ALL)
			{
				if (p_filter->type != p_db_event->type)
				{
					// allow filter all diag
					if (p_filter->type == EVENT_TYPE_DIAG)
					{
						if (p_db_event->type < EVENT_TYPE_DIAG)
						{
							rc = 0;
						}
					}
					else // types don't match
					{
						rc = 0;
					}
				}
			}
		}
		// match severity
		if (rc && (p_filter->filter_mask & NVM_FILTER_ON_SEVERITY))
		{
			if (p_db_event->severity < p_filter->severity)
			{
				rc = 0;
			}
		}
		// match code
		if (rc && (p_filter->filter_mask & NVM_FILTER_ON_CODE))
		{
			if (p_db_event->code != p_filter->code)
			{
				rc = 0;
			}
		}
		// match uid
		if (rc && (p_filter->filter_mask & NVM_FILTER_ON_UID))
		{
			NVM_UID db_uid;
			uid_copy(p_db_event->uid, db_uid);
			if (uid_cmp(db_uid, p_filter->uid) != 1)
			{
				rc = 0;
			}
		}
		// match time after
		if (rc && (p_filter->filter_mask & NVM_FILTER_ON_AFTER))
		{
			if (p_db_event->time <= p_filter->after)
			{
				rc = 0;
			}
		}
		// match time before
		if (rc && (p_filter->filter_mask & NVM_FILTER_ON_BEFORE))
		{
			if (p_db_event->time >= p_filter->before)
			{
				rc = 0;
			}
		}
		if (rc && (p_filter->filter_mask & NVM_FILTER_ON_EVENT))
		{
			if (p_db_event->id != p_filter->event_id)
			{
				rc = 0;
			}
		}
		// match on action_required
		if (rc && (p_filter->filter_mask & NVM_FILTER_ON_AR))
		{
			if (p_db_event->action_required != p_filter->action_required)
			{
				rc = 0;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve all events from the database and then filter on the specified
 * filter.
 * If purge is 1, delete the matching event from the database
 * Else if p_events is NULL or count = 0, just count the number matching.
 * Else copy to the provided structure.
 * Returns the count of matching events.
 */
int process_events_matching_filter(const struct event_filter *p_filter,
		struct event *p_events, const NVM_UINT16 count, const NVM_BOOL purge)
{
	int rc = 0;
	COMMON_LOG_ENTRY();

	PersistentStore *p_store = get_lib_store();
	if (!p_store)
	{
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		// get all the events
		int db_event_count = 0;
		if (db_get_event_count(p_store, &db_event_count) != DB_SUCCESS)
		{
			COMMON_LOG_ERROR("Unable to retrieve the number of events from the database");
			rc = NVM_ERR_UNKNOWN;
		}
		else if (db_event_count > 0)
		{
			// have to malloc the memory ... won't fit on stack
			struct db_event *db_events = malloc(db_event_count * sizeof (struct db_event));
			if (db_events)
			{
				db_event_count = db_get_events(p_store, db_events, db_event_count);
				if (db_event_count < 0)
				{
					COMMON_LOG_ERROR("Unable to retrieve the events from the database");
					rc = NVM_ERR_UNKNOWN;
				}
				else if (db_event_count > 0)
				{
					// only count events that match the filter
					for (int i = 0; i < db_event_count; i++)
					{
						if (event_matches_filter(p_filter, &db_events[i]))
						{
							rc++;

							// optionally remove it from the database
							if (purge)
							{
								if (db_delete_event_by_id(p_store, db_events[i].id) != DB_SUCCESS)
								{
									COMMON_LOG_ERROR_F("Failed to delete event Id %d",
											db_events[i].id);
								}
							}
							// optionally copy it
							else if (p_events && count > 0)
							{
								// check if the array is too small
								if (count < rc)
								{
									COMMON_LOG_ERROR(
											"Caller supplied event array \
											is too small to hold all matching events");
									rc = NVM_ERR_ARRAYTOOSMALL;
									break;
								}

								p_events[rc-1].event_id = db_events[i].id;
								p_events[rc-1].type = db_events[i].type;
								p_events[rc-1].severity = db_events[i].severity;
								p_events[rc-1].code = db_events[i].code;
								p_events[rc-1].time = db_events[i].time;
								p_events[rc-1].action_required = db_events[i].action_required;
								uid_copy(db_events[i].uid, p_events[rc - 1].uid);

								s_strcpy(p_events[rc-1].args[0], db_events[i].arg1,
										NVM_EVENT_ARG_LEN);
								s_strcpy(p_events[rc-1].args[1], db_events[i].arg2,
										NVM_EVENT_ARG_LEN);
								s_strcpy(p_events[rc-1].args[2], db_events[i].arg3,
										NVM_EVENT_ARG_LEN);

								// look up the message
								populate_event_message(&p_events[rc-1]);
								p_events[rc-1].diag_result = db_events[i].diag_result;
							}
						}
					}
				}
				free(db_events);
			}
			else
			{
				rc = NVM_ERR_NOMEMORY;
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*!
 * Acknowledge all events that meet the filter criteria
 */
int acknowledge_events(struct event_filter *p_filter)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	PersistentStore *p_store = get_lib_store();
	if (!p_store)
	{
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		// find events that match the filter
		int count = process_events_matching_filter(p_filter, NULL, 0, 0);
		if (count > 0) // at least one event needs acknowledging
		{
			// acknowledge event
			struct event events[count];
			memset(&events, 0, sizeof (struct event) * count);
			count = process_events_matching_filter(p_filter, events, count, 0);
			if (count > 0) // at least one event needs acknowledging
			{
				for (int i = 0; i < count; i++)
				{
					struct db_event db_event;
					rc = db_get_event_by_id(p_store, events[i].event_id, &db_event);
					if (rc != DB_SUCCESS)
					{
						COMMON_LOG_ERROR_F(
							"Failed to acknowledge event %d because it doesn't exist.",
							events[i].event_id);
						rc = NVM_ERR_UNKNOWN;
					}
					else
					{
						if (db_event.action_required)
						{
							db_event.action_required = 0;
							if (db_update_event_by_id(p_store,
									db_event.id, &db_event) != DB_SUCCESS)
							{
								COMMON_LOG_ERROR_F(
									"Failed to acknowledge event %d because of a database issue.",
									db_event.id);
								rc = NVM_ERR_UNKNOWN;
							}
						}
					}
				}
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
