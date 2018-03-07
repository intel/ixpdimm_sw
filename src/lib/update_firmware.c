/*
 * Copyright (c) 2017, Intel Corporation
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
 * This file contains the implementation of device firmware management functions.
 */

#include "nvm_management.h"
#include "system.h"
#include "monitor.h"
#include "nvm_context.h"
#include <fis_types.h>
#include <fw_header.h>
#include <persistence/logging.h>
#include <file_ops/file_ops_adapter.h>
#include <os/os_adapter.h>

/*
 * Retrieve the firmware image log information from the device specified.
 */
int nvm_get_device_fw_image_info(const NVM_UID device_uid,
		struct device_fw_info *p_fw_info)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct device_discovery discovery;
	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_fw_info == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_settings is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		struct pt_payload_fw_image_info fw_image_info;
		rc = fw_get_fw_image_info(discovery.device_handle.handle, &fw_image_info);
		if (rc != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F(
				"Unable to get firmware image information for handle: [%d]",
				discovery.device_handle.handle);
		}
		else
		{
			FW_VER_ARR_TO_STR(fw_image_info.fw_rev, p_fw_info->active_fw_revision,
					NVM_VERSION_LEN);

			FW_VER_ARR_TO_STR(fw_image_info.staged_fw_rev, p_fw_info->staged_fw_revision,
					NVM_VERSION_LEN);

			p_fw_info->active_fw_type = firmware_type_to_enum(fw_image_info.fw_type);
			memmove(p_fw_info->active_fw_commit_id, fw_image_info.commit_id, DEV_FW_COMMIT_ID_LEN);
			memmove(p_fw_info->active_fw_build_configuration, fw_image_info.build_configuration,
				DEV_FW_BUILD_CONFIGURATION_LEN);
			// make sure cstring is null terminated
			p_fw_info->active_fw_commit_id[NVM_COMMIT_ID_LEN-1] = 0;
			p_fw_info->active_fw_build_configuration[NVM_BUILD_CONFIGURATION_LEN-1] = 0;
			p_fw_info->fw_update_status =
				firmware_update_status_to_enum(fw_image_info.last_fw_update_status);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to read all bytes from a file and return an NVM error code
 */
int read_file_bytes(const NVM_PATH path, const NVM_SIZE path_len,
		unsigned char **pp_buf, unsigned int *p_buf_len)
{
	int rc;

	switch (copy_file_to_buffer(path, path_len, (void **)pp_buf, p_buf_len))
	{
		case COMMON_SUCCESS:
			rc = NVM_SUCCESS;
			break;
		case COMMON_ERR_NOMEMORY:
			COMMON_LOG_ERROR("Not enough memory to copy the FW file to a buffer");
			rc = NVM_ERR_NOMEMORY;
			break;
		case COMMON_ERR_BADFILE:
			COMMON_LOG_ERROR("The FW image file is not valid.");
			rc = NVM_ERR_BADFILE;
			break;
		default:
			COMMON_LOG_ERROR("Failed to copy the FW image file to a buffer.");
			rc = NVM_ERR_BADFILE;
			break;
	}
	return rc;
}

int send_new_firmware_to_device(const NVM_NFIT_DEVICE_HANDLE device_handle,
		unsigned char *p_fw_image, const unsigned int fw_image_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

#if __SET_LARGE_FW_UPDATE__
	// send a pass through command to update the fw
	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (cmd));
	cmd.device_handle = device_handle.handle;
	cmd.opcode = PT_UPDATE_FW;
	cmd.sub_opcode = SUBOP_UPDATE_FW;
	cmd.large_input_payload_size = fw_image_size;
	cmd.large_input_payload = p_fw_image;

	rc = ioctl_passthrough_cmd(&cmd);
#else
	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (cmd));
	cmd.device_handle = device_handle.handle;
	cmd.opcode = PT_UPDATE_FW;
	cmd.sub_opcode = SUBOP_UPDATE_FW;

	struct pt_update_fw_small_payload input_payload;
	memset(&input_payload, 0, sizeof (input_payload));
	input_payload.payload_selector = TRANSFER_VIA_SMALL_PAYLOAD;
	NVM_UINT16 packet_number = 0;
	unsigned int offset = 0;
	printf("Transferring the firmware to DIMM %u...\n",
		device_handle.handle);
	float num_packets = fw_image_size / TRANSFER_SIZE;
	int old_percent_complete = 0; // print status
	while (offset < fw_image_size)
	{
		// status
		int size = TRANSFER_SIZE;
		if (offset == 0)
		{
			input_payload.transfer_header =
				TRANSFER_HEADER(TRANSFER_TYPE_INITIATE, packet_number);
		}
		else if ((offset + TRANSFER_SIZE) < fw_image_size)
		{
			input_payload.transfer_header =
				TRANSFER_HEADER(TRANSFER_TYPE_CONTINUE, packet_number);
		}
		else if ((offset + TRANSFER_SIZE) == fw_image_size)
		{
			input_payload.transfer_header =
				TRANSFER_HEADER(TRANSFER_TYPE_END, packet_number);
		}
		else if ((offset + TRANSFER_SIZE) > fw_image_size)
		{
			size = fw_image_size - offset;
			input_payload.transfer_header =
				TRANSFER_HEADER(TRANSFER_TYPE_END, packet_number);
		}
		memmove(input_payload.data, (void *)p_fw_image + offset, size);
		cmd.input_payload = &input_payload;
		cmd.input_payload_size = sizeof (input_payload);
		rc = ioctl_passthrough_cmd(&cmd);
		if (rc != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Failed to transfer FW packet_number %u",
					packet_number);
			break;
		}
		offset += size;
		packet_number++;

		// update status
		int percent_complete = (int)(((float)packet_number / num_packets) * 100);
		if (percent_complete > old_percent_complete)
		{
			printf(".");
			old_percent_complete = percent_complete;
		}
	}
	printf("\n");
#endif

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL is_fw_update_operation(struct pt_payload_long_op_stat *p_long_op_status)
{
	union long_op_command
	{
		struct
		{
			unsigned char opcode;
			unsigned char subopcode;
		} parts;

		unsigned short command;
	} command;
	command.command = p_long_op_status->command;
	return command.parts.opcode == PT_UPDATE_FW &&
			command.parts.subopcode == SUBOP_UPDATE_FW;
}

int get_firmware_update_status(const NVM_NFIT_DEVICE_HANDLE device_handle)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_payload_long_op_stat long_op_payload;
	memset(&long_op_payload, 0, sizeof (long_op_payload));
	rc = fw_get_status_for_long_op(device_handle, &long_op_payload);
	if (rc == NVM_SUCCESS && is_fw_update_operation(&long_op_payload))
	{
		if (long_op_payload.status_code == MB_SUCCESS)
		{
			rc = NVM_SUCCESS;
		}
		else
		{
			rc = fw_mb_err_to_nvm_lib_err(
					long_op_payload.status_code << DSM_MAILBOX_ERROR_SHIFT);
		}
	}
	else
	{
		COMMON_LOG_WARN_F("No long op status available for FW update on device 0x%x, "
				"request returned %d", device_handle.handle, rc);
		rc = NVM_SUCCESS;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int update_firmware(const NVM_NFIT_DEVICE_HANDLE device_handle, const NVM_PATH path,
		const NVM_SIZE path_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// copy the file to the input buffer
	unsigned int fw_size = 0;
	unsigned char *p_fw = NULL;
	if ((rc = read_file_bytes(path, path_len, &p_fw, &fw_size)) == NVM_SUCCESS)
	{
		rc = send_new_firmware_to_device(device_handle, p_fw, fw_size);
		if (rc == NVM_SUCCESS)
		{
			while ((rc = get_firmware_update_status(device_handle)) == NVM_ERR_DEVICEBUSY)
			{
				nvm_sleep(1000); // 1 second
			}
			// Changing the state invalidates the device cache
			invalidate_devices();
		}
	}

	free(p_fw);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Push a new FW image to the device specified.
 */
int nvm_update_device_fw(const NVM_UID device_uid, const NVM_PATH path,
		const NVM_SIZE path_len, const NVM_BOOL force)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(modify_device_settings)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Modifying device settings is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path == NULL)
	{
		COMMON_LOG_ERROR("File path is NULL");
		rc = NVM_ERR_BADFILE;
	}
	else if (path_len >= NVM_PATH_LEN)
	{
		COMMON_LOG_ERROR_F(
				"Invalid parameter, path length is too big: %d; <= %d",
				path_len, NVM_PATH_LEN);
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, path length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (!file_exists(path, path_len))
	{
		COMMON_LOG_ERROR_F("File %s does not exist", path);
		rc = NVM_ERR_BADFILE;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		NVM_VERSION fw_version;
		rc = nvm_examine_device_fw(device_uid, path, path_len, fw_version, NVM_VERSION_LEN);
#if __EARLY_HW__ // not sure what header firmware is using currently
		rc = NVM_SUCCESS;
#endif
		if ((rc == NVM_SUCCESS) || (rc == NVM_ERR_REQUIRESFORCE && force == 1))
		{
			rc = update_firmware(discovery.device_handle, path, path_len);

			if (rc == NVM_SUCCESS)
			{
				// Log an event indicating we successfully updated
				NVM_EVENT_ARG uid_arg;
				uid_to_event_arg(device_uid, uid_arg);
				NVM_EVENT_ARG version_arg;
				s_strcpy(version_arg, fw_version, NVM_EVENT_ARG_LEN);
				log_mgmt_event(EVENT_SEVERITY_INFO,
						EVENT_CODE_MGMT_FIRMWARE_UPDATE,
						device_uid,
						0, // no action required
						uid_arg, version_arg, NULL);

			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Determine if the FW image is valid to load onto a device
 */
int nvm_examine_device_fw(const NVM_UID device_uid, const NVM_PATH path, const NVM_SIZE path_len,
		NVM_VERSION image_version, const NVM_SIZE image_version_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path == NULL)
	{
		COMMON_LOG_ERROR("File path is NULL");
		rc = NVM_ERR_BADFILE;
	}
	else if (path_len >= NVM_PATH_LEN)
	{
		COMMON_LOG_ERROR_F(
				"Invalid parameter, path length is too big: Path length (%d) must be less than %d",
				path_len, NVM_PATH_LEN);
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, path length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (image_version == NULL)
	{
		COMMON_LOG_ERROR("Image version is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (image_version_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, image_version_len length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (!file_exists(path, path_len))
	{
		COMMON_LOG_ERROR_F("File %s does not exist", path);
		rc = NVM_ERR_BADFILE;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		unsigned int buf_len = 0;
		unsigned char *p_buf = NULL;

		memset(image_version, 0, image_version_len);
		if ((rc = read_file_bytes(path, path_len, &p_buf, &buf_len)) == NVM_SUCCESS)
		{
			if (buf_len < sizeof (fwImageHeader))
			{
				COMMON_LOG_ERROR("The FW image file is not valid. Image is too small.");
				rc = NVM_ERR_BADFIRMWARE;
			}
			else
			{
				fwImageHeader *p_header = (fwImageHeader *)p_buf;

				// check some of the header values
				if (p_header->moduleType != FW_HEADER_MODULETYPE ||
						p_header->moduleVendor != FW_HEADER_MODULEVENDOR)
				{
					COMMON_LOG_ERROR("The FW image file is not valid. ");
					rc = NVM_ERR_BADFIRMWARE;
					// no need to continue checking already know the FW is bad
				}
				else
				{
					unsigned short int current_major;
					unsigned short int current_minor;
					unsigned short int current_hotfix;
					unsigned short int current_build;
					unsigned short int current_fwAPI_major;
					unsigned short int current_fwAPI_minor;
					int image_major = p_header->imageVersion.majorVer.version;
					int image_minor = p_header->imageVersion.minorVer.version;
					int image_hotfix = p_header->imageVersion.hotfixVer.version;
					int image_build = p_header->imageVersion.buildVer.build;
					unsigned int image_fwAPI_major =
							get_fw_api_major_version(p_header->fwApiVersion);
					unsigned int image_fwAPI_minor =
							get_fw_api_minor_version(p_header->fwApiVersion);

					unsigned long long bsr = 0;

					build_revision(image_version, image_version_len, image_major,
							image_minor, image_hotfix, image_build);

					parse_main_revision(&current_major, &current_minor, &current_hotfix,
							&current_build, discovery.fw_revision, NVM_VERSION_LEN);

					parse_fw_revision(&current_fwAPI_major, &current_fwAPI_minor,
							discovery.fw_api_version, NVM_VERSION_LEN);

					if (image_major != current_major)
					{
						COMMON_LOG_ERROR("The FW image file is not valid. "
								"Product number cannot be changed.");
						rc = NVM_ERR_BADFIRMWARE;
					}
					else if (image_major == current_major && image_minor < current_minor)
					{
						COMMON_LOG_ERROR("The FW image file is not valid. "
								"Revision number cannot be downgraded.");
						rc = NVM_ERR_BADFIRMWARE;
					}
					else if (is_fw_api_version_downgraded(current_fwAPI_major, current_fwAPI_minor,
									image_fwAPI_major, image_fwAPI_minor))
					{
						if (!is_fw_api_version_supported(image_fwAPI_major, image_fwAPI_minor))
						{
							COMMON_LOG_ERROR("The firmware image is not compatible with this "
									"version of software.");
							rc = NVM_ERR_INCOMPATIBLEFW;
						}
						else
						{
							rc = NVM_ERR_REQUIRESFORCE;
						}
					}
					else if (image_hotfix < current_hotfix)
					{
						rc = fw_get_bsr(discovery.device_handle, &bsr);
						if (rc == NVM_SUCCESS)
						{
							if (!BSR_OPTIN_ENABLED(bsr))
							{
								COMMON_LOG_ERROR("The FW image file is not valid. "
										"Svn Downgrade Opt-In is disabled.");
								rc = NVM_ERR_BADFIRMWARE;
							}
							else
							{
								rc = NVM_ERR_REQUIRESFORCE;
							}
						}
						else
						{
							COMMON_LOG_ERROR("Could not get the BSR. "
									"Couldnot determine the Opt-In value");
						}
					}
					else if (image_build < current_build)
					{
						rc = NVM_ERR_REQUIRESFORCE;
					}
				}
			}
		}
		if (p_buf != NULL)
		{
			free(p_buf);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
