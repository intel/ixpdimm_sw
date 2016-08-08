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
 * This file contains the implementation of security functions of the Native API.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nvm_management.h"
#include <persistence/logging.h>
#include <persistence/lib_persistence.h>
#include "device_adapter.h"
#include "monitor.h"
#include <uid/uid.h>
#include <string/s_str.h>
#include "device_utilities.h"
#include "capabilities.h"
#include "nvm_context.h"
#include "system.h"
#include <os/os_adapter.h>

/*
 * Helper functions
 */
int check_passphrase_capable(const NVM_UID device_uid);
int check_unlock_device_capable(const NVM_UID device_uid);

/*
 * Helper function to freeze lock a dimm
 */
int freeze_security(const struct device_discovery *p_discovery)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = p_discovery->device_handle.handle;
	cmd.opcode = PT_SET_SEC_INFO;
	cmd.sub_opcode = SUBOP_SEC_FREEZE_LOCK;
	rc = ioctl_passthrough_cmd(&cmd);

	// log event if it succeeded
	if (rc == NVM_SUCCESS)
	{
		store_event_by_parts(EVENT_TYPE_MGMT,
				EVENT_SEVERITY_INFO,
				EVENT_CODE_MGMT_SECURITY_FROZEN,
				p_discovery->uid,
				0, // no action required
				p_discovery->uid,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_UNKNOWN);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int security_change_prepare(struct device_discovery *p_discovery,
		const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len,
		NVM_BOOL check_enabled)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	NVM_UID uid_str;

	// Do not proceed if frozen
	if (p_discovery->lock_state == LOCK_STATE_FROZEN)
	{
		uid_copy(p_discovery->uid, uid_str);
		COMMON_LOG_ERROR_F("Failed to modify security on device %s \
				because security is frozen",
				uid_str);
		rc = NVM_ERR_SECURITYFROZEN;
	}
	// Do not proceed if max password attempts have been reached
	else if (p_discovery->lock_state == LOCK_STATE_PASSPHRASE_LIMIT)
	{
		uid_copy(p_discovery->uid, uid_str);
		COMMON_LOG_ERROR_F("Failed to modify security on device %s \
				because the passphrase limit has been reached",
				uid_str);
		rc = NVM_ERR_LIMITPASSPHRASE;
	}
	// Do not proceed if unknown
	else if (p_discovery->lock_state == LOCK_STATE_UNKNOWN)
	{
		uid_copy(p_discovery->uid, uid_str);
		COMMON_LOG_ERROR_F("Failed to modify security on device %s \
					because security is unknown",
					uid_str);
		rc = NVM_ERR_NOTSUPPORTED;
	}
	// Do not proceed if security is not enabled (only for certain commads)
	else if (check_enabled && p_discovery->lock_state == LOCK_STATE_DISABLED)
	{
		uid_copy(p_discovery->uid, uid_str);
		COMMON_LOG_ERROR_F("Failed to modify security on device %s \
				because the security is disabled",
				uid_str);
		rc = NVM_ERR_SECURITYDISABLED;
	}
	// Do not proceed if user sends passphrase in disabled state
	else if (passphrase_len > 0 && p_discovery->lock_state == LOCK_STATE_DISABLED)
	{
		uid_copy(p_discovery->uid, uid_str);
		COMMON_LOG_ERROR_F("Failed to modify security on device %s \
				because passphrase is provided when the security is disabled",
				uid_str);
		rc = NVM_ERR_SECURITYDISABLED;
	}
	// if locked, try to unlock
	else if (p_discovery->lock_state == LOCK_STATE_LOCKED)
	{
		// send the pass through ioctl to unlock the dimm
		struct pt_payload_passphrase input_payload;
		memset(&input_payload, 0, sizeof (input_payload));
		s_strncpy(input_payload.passphrase_current, NVM_PASSPHRASE_LEN,
				passphrase, passphrase_len);

		struct fw_cmd cmd;
		memset(&cmd, 0, sizeof (struct fw_cmd));
		cmd.device_handle = p_discovery->device_handle.handle;
		cmd.opcode = PT_SET_SEC_INFO;
		cmd.sub_opcode = SUBOP_UNLOCK_UNIT;
		cmd.input_payload_size = sizeof (input_payload);
		cmd.input_payload = &input_payload;
		rc = ioctl_passthrough_cmd(&cmd);
		s_memset(&input_payload, sizeof (input_payload));
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Check a passphrase for validity.
 * There are no restrictions on passphrase characters, just min and max length.
 * These restrictions come from the Security Architecture team.
 */
int check_passphrase(const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	if (passphrase == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, new_passphrase is NULL");
		rc = NVM_ERR_BADPASSPHRASE;
	}
	else if (passphrase_len > NVM_PASSPHRASE_LEN)
	{
		COMMON_LOG_ERROR_F("Invalid passphrase length (%d). Passphrase length must be <= %d",
				passphrase_len, NVM_PASSPHRASE_LEN);
		rc = NVM_ERR_BADPASSPHRASE;
	}
	// TODO: this might change to minimum of 10.
	else if (passphrase_len == 0)
	{
		COMMON_LOG_ERROR_F("Invalid passphrase length (%d). Passphrase length must be > 0",
				passphrase_len);
		rc = NVM_ERR_BADPASSPHRASE;
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * If data at rest security is not enabled, this method enables it
 * and sets the passphrase. The device will be locked on the next reset.
 * If data at rest security was previously enabled, this method changes
 * the passphrase to the new passphrase specified. The device will be
 * unlocked if it is currently locked.
 */
int nvm_set_passphrase(const NVM_UID device_uid,
		const NVM_PASSPHRASE old_passphrase, const NVM_SIZE old_passphrase_len,
		const NVM_PASSPHRASE new_passphrase, const NVM_SIZE new_passphrase_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(modify_device_security)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Modifying " NVM_DIMM_NAME " security is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (check_passphrase(new_passphrase, new_passphrase_len) != NVM_SUCCESS)
	{
		// since this is a new passphrase, change the error code from BADPASSPHRASE
		// to INVALIDPASSPHRASE because the passphrase is not BAD is
		// for an existing passphrase and INVALID is for a new.
		rc = NVM_ERR_INVALIDPASSPHRASE;
	}
	else if (old_passphrase != NULL)
	{
		rc = check_passphrase(old_passphrase, old_passphrase_len);
	}
	if (rc == NVM_SUCCESS &&
			((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS) &&
			((rc = check_passphrase_capable(device_uid)) == NVM_SUCCESS) &&
			((rc = security_change_prepare(&discovery, old_passphrase,
					old_passphrase_len, 0)) == NVM_SUCCESS))
	{
		// send the pass through ioctl
		struct pt_payload_set_passphrase input_payload;
		memset(&input_payload, 0, sizeof (input_payload));
		if (old_passphrase_len > 0 && old_passphrase != NULL)
		{
			s_strncpy(input_payload.passphrase_current, NVM_PASSPHRASE_LEN,
					old_passphrase, old_passphrase_len);
		}
		s_strncpy(input_payload.passphrase_new, NVM_PASSPHRASE_LEN,
				new_passphrase, new_passphrase_len);

		struct fw_cmd cmd;
		memset(&cmd, 0, sizeof (struct fw_cmd));
		cmd.device_handle = discovery.device_handle.handle;
		cmd.opcode = PT_SET_SEC_INFO;
		cmd.sub_opcode = SUBOP_SET_PASS;
		cmd.input_payload_size = sizeof (input_payload);
		cmd.input_payload = &input_payload;
		if ((rc = ioctl_passthrough_cmd(&cmd)) == NVM_SUCCESS)
		{
			// Log an event indicating we successfully set a passphrase
			NVM_EVENT_ARG uid_arg;
			uid_to_event_arg(device_uid, uid_arg);
			log_mgmt_event(EVENT_SEVERITY_INFO,
					EVENT_CODE_MGMT_SECURITY_PASSWORD_SET,
					device_uid,
					0, // no action required
					uid_arg, NULL, NULL);
		}
		s_memset(&input_payload, sizeof (input_payload));

		// clear any device context - security state has likely changed
		invalidate_devices();
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int check_passphrase_capable(const NVM_UID device_uid)
{
	int rc;

	struct device_discovery discovery;
	if ((rc = nvm_get_device_discovery(device_uid, &discovery)) == NVM_SUCCESS)
	{
		rc = discovery.security_capabilities.passphrase_capable == 1
				? NVM_SUCCESS : NVM_ERR_NOTSUPPORTED;
	}

	return rc;
}

int check_unlock_device_capable(const NVM_UID device_uid)

{
	int rc;

	struct device_discovery discovery;
	if ((rc = nvm_get_device_discovery(device_uid, &discovery)) == NVM_SUCCESS)
	{
		rc = discovery.security_capabilities.unlock_device_capable == 1
				? NVM_SUCCESS : NVM_ERR_NOTSUPPORTED;
	}

	return rc;
}

/*
 * Disables data at rest security and removes the passphrase.
 * The device will be unlocked if it is currently locked.
 */
int nvm_remove_passphrase(const NVM_UID device_uid,
		const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	// check user has permission to make changes
	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(modify_device_security)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Modifying " NVM_DIMM_NAME " security is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (((rc = check_passphrase(passphrase, passphrase_len)) == NVM_SUCCESS) &&
			((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS) &&
			((rc = check_passphrase_capable(device_uid)) == NVM_SUCCESS) &&
			((rc = security_change_prepare(&discovery, passphrase, passphrase_len, 1))
					== NVM_SUCCESS))
	{
		// send a pass through command to disable security
		struct pt_payload_passphrase input_payload;
		memset(&input_payload, 0, sizeof (input_payload));
		s_strncpy(input_payload.passphrase_current, NVM_PASSPHRASE_LEN,
				passphrase, passphrase_len);

		struct fw_cmd cmd;
		memset(&cmd, 0, sizeof (struct fw_cmd));
		cmd.device_handle = discovery.device_handle.handle;
		cmd.opcode = PT_SET_SEC_INFO;
		cmd.sub_opcode = SUBOP_DISABLE_PASS;
		cmd.input_payload_size = sizeof (input_payload);
		cmd.input_payload = &input_payload;
		rc = ioctl_passthrough_cmd(&cmd);
		if (rc == NVM_SUCCESS)
		{
			// Log an event indicating we successfully removed the passphrase
			NVM_EVENT_ARG uid_arg;
			uid_to_event_arg(device_uid, uid_arg);
			log_mgmt_event(EVENT_SEVERITY_INFO,
					EVENT_CODE_MGMT_SECURITY_PASSWORD_REMOVED,
					device_uid,
					0, // no action required
					uid_arg, NULL, NULL);
		}
		s_memset(&input_payload, sizeof (input_payload));

		// clear any device context - security state has likely changed
		invalidate_devices();
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Unlocks the device with the passphrase specified.
 */
int nvm_unlock_device(const NVM_UID device_uid,
		const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	// check user has permission to make changes
	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(modify_device_security)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Modifying " NVM_DIMM_NAME " security is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (((rc = check_passphrase(passphrase, passphrase_len)) == NVM_SUCCESS) &&
			((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS) &&
			((rc = check_unlock_device_capable(device_uid)) == NVM_SUCCESS) &&
			((rc = security_change_prepare(&discovery, passphrase, passphrase_len, 1))
					== NVM_SUCCESS))
	{
		// send a pass through command to unlock the device
		struct pt_payload_passphrase input_payload;
		memset(&input_payload, 0, sizeof (input_payload));
		s_strncpy(input_payload.passphrase_current, NVM_PASSPHRASE_LEN,
				passphrase, passphrase_len);

		struct fw_cmd cmd;
		memset(&cmd, 0, sizeof (struct fw_cmd));
		cmd.device_handle = discovery.device_handle.handle;
		cmd.opcode = PT_SET_SEC_INFO;
		cmd.sub_opcode = SUBOP_UNLOCK_UNIT;
		cmd.input_payload_size = sizeof (input_payload);
		cmd.input_payload = &input_payload;
		rc = ioctl_passthrough_cmd(&cmd);
		s_memset(&input_payload, sizeof (input_payload));

		// clear any device context - security state has likely changed
		invalidate_devices();
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;

}

/*
 * Prevent security lock state changes to the dimm until the next reboot
 */
int nvm_freezelock_device(const NVM_UID device_uid)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	// check user has permission to make changes
	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(modify_device_security)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Modifying "NVM_DIMM_NAME" security is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		rc = freeze_security(&discovery);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper method to make secure erase call
 */
int secure_erase(const NVM_PASSPHRASE passphrase,
		const NVM_SIZE passphrase_len,
		struct device_discovery *p_discovery)
{
	int rc = NVM_ERR_UNKNOWN;
	// try to send the prepare/erase command combo up to 5 times
	int count = 0;
	do
	{
		// erase
		struct pt_payload_passphrase input_payload;
		memset(&input_payload, 0, sizeof (input_payload));
		s_strncpy(input_payload.passphrase_current, NVM_PASSPHRASE_LEN,
				passphrase, passphrase_len);
		struct fw_cmd cmd;
		memset(&cmd, 0, sizeof (struct fw_cmd));
		cmd.device_handle = p_discovery->device_handle.handle;
		cmd.opcode = PT_SET_SEC_INFO;
		cmd.sub_opcode = SUBOP_SEC_ERASE_UNIT;
		cmd.input_payload_size = sizeof (input_payload);
		cmd.input_payload = &input_payload;
		rc = ioctl_passthrough_cmd(&cmd);
		s_memset(&input_payload, sizeof (input_payload));

		count++;
	}
	while (rc == NVM_ERR_DEVICEBUSY && count < 5);

	// log event if it succeeded
	if (rc == NVM_SUCCESS)
	{
		store_event_by_parts(EVENT_TYPE_MGMT,
				EVENT_SEVERITY_INFO,
				EVENT_CODE_MGMT_SECURITY_SECURE_ERASE,
				p_discovery->uid,
				0, // no action required
				p_discovery->uid,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_UNKNOWN);
	}

	return rc;
}

/*
 * Erases the data on the device specified.
 */
int nvm_erase_device(const NVM_UID device_uid,
		const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	// check user has permission to make changes
	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(modify_device_security)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Modifying " NVM_DIMM_NAME " security is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		if (discovery.security_capabilities.passphrase_capable)
		{
			// check passphrase length
			if (check_passphrase(passphrase, passphrase_len) != NVM_SUCCESS)
			{
				rc = NVM_ERR_BADPASSPHRASE;
			}
			// verify device is in the right state to accept a secure erase
			else if ((rc =
					security_change_prepare(&discovery, passphrase, passphrase_len, 1))
					== NVM_SUCCESS)
			{
				rc = secure_erase(passphrase, passphrase_len, &discovery);
				// clear any device context - security state has likely changed
				invalidate_devices();
			}
		}
		else
		{
			COMMON_LOG_ERROR("Invalid parameter. "
					"Crypto scramble erase is not valid in the current security state");
			rc = NVM_ERR_INVALIDPARAMETER;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
