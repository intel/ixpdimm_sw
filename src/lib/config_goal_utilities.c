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
 * This file contains the implementation of helper functions which support
 * the native API in configuration of NVM-DIMM capacity.
 */

#include "nvm_management.h"
#include "config_goal_utilities.h"

int get_devices_from_appdirect_attributes(struct device_discovery *p_devices,
		int num_devices,
		const struct app_direct_attributes *p_qos)
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;

	for (NVM_UINT32 i = 0; i < num_devices; i++)
	{
		struct device_discovery discovery;
		// Look up the DIMM
		rc = exists_and_manageable(p_qos->dimms[i], &discovery, 1);
		if (rc != NVM_SUCCESS)
		{
			break;
		}

		memmove(&p_devices[i], &discovery, sizeof (struct device_discovery));
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL interleave_request_across_socket(struct device_discovery *p_devices, int num_devices)
{
	COMMON_LOG_ENTRY();

	NVM_BOOL across_socket = 0;
	int socket_id = p_devices[0].socket_id;
	for (int i = 1; i < num_devices; i++)
	{
		if (p_devices[i].socket_id != socket_id)
		{
			across_socket = 1;
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(across_socket);
	return across_socket;
}

NVM_BOOL dimms_are_across_controllers(struct device_discovery *p_devices, int num_devices)
{
	COMMON_LOG_ENTRY();

	NVM_BOOL across_controllers = 0;
	int controller_id = p_devices[0].memory_controller_id;
	for (int i = 1; i < num_devices; i++)
	{
		if (p_devices[i].memory_controller_id != controller_id)
		{
			across_controllers = 1;
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(across_controllers);
	return across_controllers;
}

NVM_BOOL dimms_across_controllers_and_channels_match(struct device_discovery *p_devices,
		int num_devices)
{
	COMMON_LOG_ENTRY();

	NVM_BOOL dimm_found_across_channel = 0;

	for (int i = 0; i < num_devices; i++)
	{
		dimm_found_across_channel = 0;
		for (int j = 0; j < num_devices; j++)
		{
			if ((p_devices[i].channel_id == p_devices[j].channel_id) &&
					(p_devices[i].memory_controller_id != p_devices[j].memory_controller_id))
			{
				dimm_found_across_channel = 1;
				break;
			}
		}
		if (!dimm_found_across_channel)
		{
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(dimm_found_across_channel);
	return dimm_found_across_channel;
}

/*
 * Helper function to populate dimms with following pcat rules
 *
 * 1. Interleave requests across sockets not supported
 *
 * 2. For 2-way interleave sets, the lowest numbered channel must be the first DIMM in
 * the two-way interleave set.
 *
 * 3. 2-way interleave sets between MCs must have DIMMs populated in same channels on both MCs.
 *
 * 4. For 3-way interleave sets, all the channels should belong to same contoller.
 *
 * 5. For 4-way interleave sets, interleave sets between MCs must have DIMMs populated in same
 * channels on both MCs.
 *
 * 7. Appdirect granularity must be 4k/4k .
 *
 */
int verify_interleave_set_rules(struct device_discovery *p_devices,
		int num_devices)
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;
	if (interleave_request_across_socket(p_devices, num_devices))
	{
		rc = NVM_ERR_CONFIGNOTSUPPORTED;
	}
	else
	{
		switch (num_devices)
		{

		case INTERLEAVE_WAYS_0:
			rc = NVM_ERR_UNKNOWN;
			break;

		case INTERLEAVE_WAYS_1:
			break;

		case INTERLEAVE_WAYS_2:
			if (dimms_are_across_controllers(p_devices, num_devices) &&
					!dimms_across_controllers_and_channels_match(p_devices, num_devices))
			{
				rc = NVM_ERR_CONFIGNOTSUPPORTED;
			}
			break;

		case INTERLEAVE_WAYS_3:
			if (dimms_are_across_controllers(p_devices, num_devices))
			{
				rc = NVM_ERR_CONFIGNOTSUPPORTED;
			}
			break;

		case INTERLEAVE_WAYS_4:
			if (!dimms_across_controllers_and_channels_match(p_devices, num_devices))
			{
				rc = NVM_ERR_CONFIGNOTSUPPORTED;
			}
			break;

		case INTERLEAVE_WAYS_6:
			break;

		case INTERLEAVE_WAYS_8:
		case INTERLEAVE_WAYS_12:
		case INTERLEAVE_WAYS_16:
		case INTERLEAVE_WAYS_24:
		default:
			rc = NVM_ERR_UNKNOWN;
		}
	}

	return rc;
}

/*
 * Compare Dimm channel and iMC to determine Dimms order in interleave set
 *
 * Thsi function supports 1-way, 2-way, 3-way and 4-way interleave sets
 *
 * @param[in] pFirst First item to compare
 * @param[in] pSecond Second item to compare
 *
 * @retval -1 if first is less than second
 * @retval  0 if first is equal to second
 * @retval 	1 if first is greater than second
 *
 */
int compare_dimm_order_in_interleave_set(const void *pFirst, const void *pSecond)
{
	struct device_discovery *pFirstDevice = NULL;
	struct device_discovery *pSecondDevice = NULL;

	if (pFirst == NULL || pSecond == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, One of the device is NULL ");
		return 0;
	}

	pFirstDevice = (struct device_discovery *)pFirst;
	pSecondDevice = (struct device_discovery *)pSecond;

	if (pFirstDevice->channel_id < pSecondDevice->channel_id)
	{
		return -1;
	}
	else if (pFirstDevice->channel_id > pSecondDevice->channel_id)
	{
		return 1;
	}
	else
	{
		if (pFirstDevice->memory_controller_id < pSecondDevice->memory_controller_id)
		{
			return -1;
		}
		else if (pFirstDevice->memory_controller_id > pSecondDevice->memory_controller_id)
		{
			return 1;
		}
		else
		{
			COMMON_LOG_ERROR("Two DIMMs plugged into same channel");
			return 0;
		}
	}
}

/*
 * Compare Dimm channel and iMC to determine Dimms order in interleave set
 *
 * Thsi function supports 6-way interleave sets
 *
 * @param[in] pFirst First item to compare
 * @param[in] pSecond Second item to compare
 *
 * @retval -1 if first is less than second
 * @retval  0 if first is equal to second
 * @retval 	1 if first is greater than second
 *
 */
int compare_dimm_order_six_way_interleave_set(const void *pFirst, const void *pSecond)
{
	struct device_discovery *pFirstDevice = NULL;
	struct device_discovery *pSecondDevice = NULL;
	NVM_UINT16 parityResultFirst = 0;
	NVM_UINT16 parityResultSecond = 0;

	if (pFirst == NULL || pSecond == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, One of the device is NULL ");
		return 0;
	}

	pFirstDevice = (struct device_discovery *)pFirst;
	pSecondDevice = (struct device_discovery *)pSecond;

	parityResultFirst = (pFirstDevice->channel_id + pFirstDevice->memory_controller_id) % 2;
	parityResultSecond = (pSecondDevice->channel_id + pSecondDevice->memory_controller_id) % 2;

	if (parityResultFirst < parityResultSecond)
	{
		return -1;
	}
	else if (parityResultFirst > parityResultSecond)
	{
		return 1;
	}
	else
	{
		if (pFirstDevice->channel_id < pSecondDevice->channel_id)
		{
			return -1;
		}
		else if (pFirstDevice->channel_id > pSecondDevice->channel_id)
		{
			return 1;
		}
		else
		{
			COMMON_LOG_ERROR("Two DIMMs plugged into same channel");
			return 0;
		}
	}
}

/*
 * Helper function to populate dimms with following pcat rules
 *
 * 1. For 2-way interleave sets, the lowest numbered channel must be the first DIMM in
 * the two-way interleave set.
 *
 * 2. The channel on the first MC must be the first DIMM in each two-way interleave set.
 *
 * 3. For 3-way interleave sets, CH0 of the memory controller must be the first DIMM in the
 * interleave set, followed by CH1, and CH2.
 *
 * 4. Interleave sets between MCs must have DIMMs ordered from lowest channel
 * on MC0, lowest channel on MC1, highest channel on MC0, and highest channel on MC1.
 *
 * 6. For 6-way interleave sets, the interleave should be MC0/CH0, MC1/CH1, MC0/CH2, MC1/CH0,
 * MC0/CH1, MC1/CH2 (or CH0, CH4, CH2, CH3, CH1, CH5)
 */
int arrange_devices_in_interleave_set_for_pcat2(struct device_discovery *p_devices,
		int num_devices)
{
	int rc = NVM_SUCCESS;

	if (p_devices)
	{
		if (num_devices <= INTERLEAVE_WAYS_4)
		{
			qsort(p_devices, num_devices, sizeof (struct device_discovery),
					compare_dimm_order_in_interleave_set);
		}
		if (num_devices == INTERLEAVE_WAYS_6)
		{
			qsort(p_devices, num_devices, sizeof (struct device_discovery),
					compare_dimm_order_six_way_interleave_set);
		}
	}
	else
	{
		rc = NVM_ERR_INVALIDPARAMETER;
	}

	return rc;
}

void populate_dimm_info_extension_table(struct dimm_info_extension_table *p_dimms_ext,
		struct device_discovery *p_devices,
		int num_devices,
		const NVM_UINT64 interleave_set_size,
		const NVM_UINT64 interleave_set_offset)
{
	for (NVM_UINT32 i = 0; i < num_devices; i++)
	{
		// The DIMM is OK - put it in the list
		p_dimms_ext[i].size = interleave_set_size * BYTES_PER_GIB;
		p_dimms_ext[i].offset = interleave_set_offset * BYTES_PER_GIB;
		memmove(p_dimms_ext[i].serial_number, p_devices[i].serial_number, NVM_SERIAL_LEN);
		memmove(p_dimms_ext[i].manufacturer, p_devices[i].manufacturer, NVM_MANUFACTURER_LEN);
		memmove(p_dimms_ext[i].model_number, p_devices[i].model_number, NVM_MODEL_LEN-1);
	}
}

int populate_dimm_info_extension_tables(struct dimm_info_extension_table *p_dimms_ext,
		const struct app_direct_attributes *p_qos,
		const NVM_UINT8 pcat_revision,
		const NVM_UINT64 interleave_set_size,
		const NVM_UINT64 interleave_set_offset)
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;
	int num_devices = p_qos->interleave.ways;
	struct device_discovery *p_devices =
			calloc(num_devices, sizeof (struct device_discovery));

	if (p_devices)
	{
		if ((rc = get_devices_from_appdirect_attributes(p_devices,
				num_devices, p_qos)) == NVM_SUCCESS)
		{
			rc = verify_interleave_set_rules(p_devices, num_devices);
		}
		if (rc == NVM_SUCCESS)
		{
			if (pcat_revision == 1)
			{
				populate_dimm_info_extension_table(p_dimms_ext, p_devices, num_devices,
						interleave_set_size, interleave_set_offset);
			}
			else if (pcat_revision == 2)
			{
				if ((rc = arrange_devices_in_interleave_set_for_pcat2(p_devices, num_devices))
						== NVM_SUCCESS)
				{
					populate_dimm_info_extension_table(p_dimms_ext, p_devices, num_devices,
							interleave_set_size, interleave_set_offset);
				}
			}
			else
			{
				rc = NVM_ERR_UNKNOWN;
			}
		}
	}
	else
	{
		rc = NVM_ERR_NOMEMORY;
	}

	free(p_devices);
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
