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

#include "namespace_labels.h"
#include "device_utilities.h"
#include "nvm_context.h"
#include <firmware_interface/fis_commands.h>
#include <common_types.h>
#include <persistence/logging.h>
#include <uid/uid.h>
#include <guid/guid.h>
#include <string/s_str.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define	NS_LABEL_PCDPARTITION	2

#define	NS_INDEX_SIGNATURE	"NAMESPACE_INDEX"
#define	NS_INDEX_SIG_LEN	16
#define	NS_INDEX_LEN	256
#define	NS_INDEX_FREEMAP_LEN	128	// Map for 1024 labels
#define	NS_INDEX_PADDING	56	// Alignment to 256B boundary
#define	NS_INDEX_MAJOR	1
#define	NS_INDEX_MINOR	1
#define	MAX_NS_LABELS	1020
#define	MAX_NAMESPACES	(MAX_NS_LABELS - 1) // Need to leave one free for updates
#define	NS_FLAGS_UPDATING(flags)	(flags & 0b100)
#define	NS_FLAGS_LOCAL(flags)	(flags & 0b010)
#define	NS_NAME_LEN	64
#define	NS_BTT_GUID	"8aed63a2-29a2-4c66-8b12-f05d15d3922a"
#define	NS_PFN_GUID	"266400ba-fb9f-4677-bcb0-968f11d0d225"
#define	NS_DAX_GUID	"97a86d9c-3cdd-4eda-986f-5068b4f80088"

// Macro used in interleave calculations - hold onto more severe health
#define	KEEP_NS_HEALTH(health, new_health)	\
{ \
	if (new_health > health) \
	{ \
		health = new_health; \
	} \
}

struct ns_data
{
	int dimm_count;
	struct device_discovery *dimm_list;
	struct pt_output_namespace_labels *dimm_nslsa_list;
	int ns_count;
	struct nvm_namespace_details *ns_list;
	int iset_count;
	struct nvm_interleave_set *iset_list;
};

struct v1_1_cookie_data
{
	NVM_UINT64 region_offset;
	NVM_UINT32 serial_number;
	NVM_UINT32 reserved;
} __attribute__((packed));

struct v1_2_cookie_data
{
	NVM_UINT64 region_offset;
	NVM_UINT32 serial_number;
	NVM_UINT16 vendor_id;
	NVM_UINT16 manufacturing_date;
	NVM_UINT8 manufacturing_location;
	NVM_UINT8 reserved[31];
} __attribute__((packed));


/*
 * Calculate a fletcher 64 checksum
 */
NVM_BOOL checksum_fletcher64(void *p_data, NVM_UINT32 length,
		NVM_UINT64 *p_checksum, NVM_BOOL update)
{
	NVM_UINT32 *p_start = p_data;
	NVM_UINT32 *p_end = (NVM_UINT32 *)((NVM_UINT8 *)p_start + length);
	NVM_UINT32 lo32 = 0;
	NVM_UINT32 hi32 = 0;
	NVM_UINT64 checksum = 0;
	NVM_BOOL checksum_match = 0;

	if (p_data == NULL || p_checksum == NULL)
	{
		COMMON_LOG_INFO("The address or checksum pointer equal NULL");
	}
	else if ((length % sizeof (NVM_UINT32)) != 0)
	{
		COMMON_LOG_INFO("The size specified for the checksum is not properly aligned");
	}
	else if (((uintptr_t)p_data % sizeof (NVM_UINT32)) !=
			((uintptr_t)checksum % sizeof (NVM_UINT32)))
	{
		COMMON_LOG_INFO("The address and the checksum address are not aligned together");
	}
	else
	{
		while (p_start < p_end)
		{
			if (p_start == (NVM_UINT32 *) p_checksum)
			{
				/* Lo32 += 0; treat first 32-bits as zero */
				p_start++;
				hi32 += lo32;
				/* Lo32 += 0; treat second 32-bits as zero */
				p_start++;
				hi32 += lo32;
			}
			else
			{
				lo32 += *p_start;
				++p_start;
				hi32 += lo32;
			}
		}

		checksum = (NVM_UINT64) hi32 << 32 | lo32;
		if (update)
		{
			*p_checksum = checksum;
			checksum_match = 1;
		}
		else
		{
			checksum_match = (*p_checksum == checksum);
		}
	}
	return checksum_match;
}

void free_ns_data(struct ns_data *p_ns_data)
{
	COMMON_LOG_ENTRY();
	if (p_ns_data)
	{
		if (p_ns_data->dimm_count)
		{
			if (p_ns_data->dimm_list)
			{
				free(p_ns_data->dimm_list);
			}
			if (p_ns_data->dimm_nslsa_list)
			{
				free(p_ns_data->dimm_nslsa_list);
			}
		}
		if (p_ns_data->iset_count)
		{
			if (p_ns_data->iset_list)
			{
				free(p_ns_data->iset_list);
			}
		}
		if (p_ns_data->ns_count)
		{
			if (p_ns_data->ns_list)
			{
				free(p_ns_data->ns_list);
			}
		}
		free(p_ns_data);
	}
	COMMON_LOG_EXIT();
}

/*
 * Retrieve a list of manageable DIMMs
 */
int collect_ns_data_manageable_dimms(struct ns_data **pp_ns_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	(*pp_ns_data)->dimm_count = get_manageable_dimms(&(*pp_ns_data)->dimm_list);
	if ((*pp_ns_data)->dimm_count < 0)
	{
		rc = (*pp_ns_data)->dimm_count;
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve the namespace label storage area from each manageable DIMM
 */
int collect_dimm_nslsa_list(struct ns_data **pp_ns_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	(*pp_ns_data)->dimm_nslsa_list = calloc((*pp_ns_data)->dimm_count,
			sizeof (struct pt_output_namespace_labels));
	if (!(*pp_ns_data)->dimm_nslsa_list)
	{
		COMMON_LOG_ERROR("No memory to collect namespace information");
		rc = NVM_ERR_NOMEMORY;
	}
	else
	{
		struct pt_input_namespace_labels input_payload;
		memset(&input_payload, 0, sizeof (input_payload));
		input_payload.partition_id = NS_LABEL_PCDPARTITION;

		for (int i = 0; i < (*pp_ns_data)->dimm_count; i ++)
		{
			struct fw_cmd cfg_cmd;
			memset(&cfg_cmd, 0, sizeof (cfg_cmd));
			cfg_cmd.device_handle = (*pp_ns_data)->dimm_list[i].device_handle.handle;
			cfg_cmd.opcode = PT_GET_ADMIN_FEATURES;
			cfg_cmd.sub_opcode = SUBOP_PLATFORM_DATA_INFO;
			cfg_cmd.input_payload_size = sizeof (input_payload);

#if __LARGE_PAYLOAD__
			cfg_cmd.input_payload = &input_payload;
			cfg_cmd.large_output_payload_size = sizeof (struct pt_output_namespace_labels);
			cfg_cmd.large_output_payload = &(*pp_ns_data)->dimm_nslsa_list[i];
			rc = ioctl_passthrough_cmd(&cfg_cmd);
			if (rc != NVM_SUCCESS)
			{
				break;
			}
#else
			NVM_SIZE lsa_size = sizeof (struct pt_output_namespace_labels);
			char out_buf[DEV_SMALL_PAYLOAD_SIZE];
			NVM_UINT32 offset = 0;
			input_payload.command_option = DEV_PLT_CFG_OPT_SMALL_DATA;
			input_payload.offset = offset;
			cfg_cmd.input_payload = &input_payload;
			cfg_cmd.output_payload_size = DEV_SMALL_PAYLOAD_SIZE;
			cfg_cmd.output_payload = &out_buf;

			while (offset < lsa_size && rc == NVM_SUCCESS)
			{
				memset(&out_buf, 0, DEV_SMALL_PAYLOAD_SIZE);
				input_payload.offset = offset;
				if ((rc = ioctl_passthrough_cmd(&cfg_cmd)) == NVM_SUCCESS)
				{
					NVM_SIZE transfer_size = DEV_SMALL_PAYLOAD_SIZE;
					if ((offset + DEV_SMALL_PAYLOAD_SIZE) > lsa_size)
					{
						transfer_size = lsa_size - offset;
					}
					memmove(((void *)&(*pp_ns_data)->dimm_nslsa_list[i]) + offset,
							out_buf, transfer_size);
					offset += DEV_SMALL_PAYLOAD_SIZE;
				}
				else
				{
					break;
				}
			}
#endif
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_ns_data_dimm_index_from_handle(struct ns_data *p_ns_data, NVM_UINT32 handle)
{
	int index = -1;
	for (int i = 0; i < p_ns_data->dimm_count; i++)
	{
		if (p_ns_data->dimm_list->device_handle.handle == handle)
		{
			index = i;
			break;
		}
	}
	return index;
}

void sort_cookie_data(struct v1_1_cookie_data *p_data_v1_1,
		struct v1_2_cookie_data *p_data_v1_2, int dimm_count)
{
	COMMON_LOG_ENTRY();
	for (int i = 0; i < (dimm_count - 1); i++)
	{
		for (int j = 0; j < (dimm_count - i - 1); j++)
		{
			if (p_data_v1_1[j].region_offset > p_data_v1_1[j+1].region_offset)
			{
				// swap
				struct v1_1_cookie_data p_tmp_v1_1 = p_data_v1_1[j];
				p_data_v1_1[j] = p_data_v1_1[j+1];
				p_data_v1_1[j+1] = p_tmp_v1_1;
			}
			if (p_data_v1_2[j].region_offset > p_data_v1_2[j+1].region_offset)
			{
				struct v1_2_cookie_data p_tmp_v1_2 = p_data_v1_2[j];
				p_data_v1_2[j] = p_data_v1_2[j+1];
				p_data_v1_2[j+1] = p_tmp_v1_2;
			}
		}
	}
	COMMON_LOG_EXIT();
}

void calculate_iset_cookies(struct ns_data **pp_ns_data)
{
	COMMON_LOG_ENTRY();
	for (int iset_idx = 0; iset_idx < (*pp_ns_data)->iset_count; iset_idx++)
	{
		struct nvm_interleave_set *p_set = &(*pp_ns_data)->iset_list[iset_idx];

		struct v1_1_cookie_data data_v1_1[p_set->dimm_count];
		NVM_SIZE data_v1_1_size = sizeof (struct v1_1_cookie_data) * p_set->dimm_count;
		memset(&data_v1_1, 0, data_v1_1_size);
		struct v1_2_cookie_data data_v1_2[p_set->dimm_count];
		NVM_SIZE data_v1_2_size = sizeof (struct v1_2_cookie_data) * p_set->dimm_count;
		memset(&data_v1_2, 0, data_v1_2_size);

		for (int dimm_idx = 0; dimm_idx < p_set->dimm_count; dimm_idx++)
		{
			int index = get_ns_data_dimm_index_from_handle(*pp_ns_data, p_set->dimms[dimm_idx]);
			if (index >= 0)
			{
				struct device_discovery dimm = (*pp_ns_data)->dimm_list[index];
				data_v1_1[dimm_idx].region_offset = p_set->dimm_region_offsets[dimm_idx];
				memmove(&data_v1_1[dimm_idx].serial_number,
					dimm.serial_number, sizeof (NVM_SERIAL_NUMBER));

				data_v1_2[dimm_idx].region_offset = p_set->dimm_region_offsets[dimm_idx];
				memmove(&data_v1_2[dimm_idx].serial_number,
					dimm.serial_number, sizeof (NVM_SERIAL_NUMBER));
				data_v1_2[dimm_idx].vendor_id = dimm.vendor_id;
				data_v1_2[dimm_idx].manufacturing_date = dimm.manufacturing_date;
				data_v1_2[dimm_idx].manufacturing_location = dimm.manufacturing_location;
			}
		}

		// sort the DIMMs by region offset
		sort_cookie_data(data_v1_1, data_v1_2, p_set->dimm_count);

		// cookie is the fletcher64 checksum of the cookie data data
		checksum_fletcher64((void *)&data_v1_1, data_v1_1_size, &p_set->cookie_v1_1, 1);
		checksum_fletcher64((void *)&data_v1_2, data_v1_2_size, &p_set->cookie_v1_2, 1);
	}
	COMMON_LOG_EXIT();
}

/*
 * Retrieve interleave sets and store them in the pool data structure
 */
int collect_ns_data_interleave_sets(struct ns_data **pp_ns_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	(*pp_ns_data)->iset_count = get_interleave_set_count();
	if ((*pp_ns_data)->iset_count < 0)
	{
		rc = (*pp_ns_data)->iset_count;
		(*pp_ns_data)->iset_count = 0;
	}
	else if ((*pp_ns_data)->iset_count > 0)
	{
		(*pp_ns_data)->iset_list = calloc((*pp_ns_data)->iset_count,
				sizeof (struct nvm_interleave_set));
		if (!(*pp_ns_data)->iset_list)
		{
			COMMON_LOG_ERROR("No memory to collect pool information");
			rc = NVM_ERR_NOMEMORY;
		}
		else
		{
			(*pp_ns_data)->iset_count = get_interleave_sets(
					(*pp_ns_data)->iset_count, (*pp_ns_data)->iset_list);
			if ((*pp_ns_data)->iset_count < 0)
			{
				rc = (*pp_ns_data)->iset_count;
			}
			else
			{
				calculate_iset_cookies(pp_ns_data);
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


int collect_required_ns_data(struct ns_data **pp_ns_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// gather the manageable DIMMs
	*pp_ns_data = calloc(1, sizeof (struct ns_data));
	if (!(*pp_ns_data))
	{
		COMMON_LOG_ERROR("No memory to collect namespace information");
		rc = NVM_ERR_NOMEMORY;
	}
	// get the manageable DIMMs
	else if ((rc = collect_ns_data_manageable_dimms(pp_ns_data)) != NVM_SUCCESS)
	{
		free_ns_data(*pp_ns_data);
	}
	else if ((*pp_ns_data)->dimm_count > 0)
	{
		// collect ns lsa from each manageable DIMM
		if ((rc = collect_dimm_nslsa_list(pp_ns_data)) != NVM_SUCCESS)
		{
			free_ns_data(*pp_ns_data);
		}
		// get interleave sets
		else if ((rc = collect_ns_data_interleave_sets(pp_ns_data)) != NVM_SUCCESS)
		{
			free_ns_data(*pp_ns_data);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL is_valid_index_block(struct pt_output_ns_index *p_ns_index,
	const int index_pos)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL rc = 1;
	// check checksum
	if (!checksum_fletcher64((void *)p_ns_index, sizeof (*p_ns_index),
			&p_ns_index->checksum, 0))
	{
		COMMON_LOG_INFO_F("Checksum failed on NS Index Block %d", index_pos);
		rc = 0;
	}
	// check signature
	if (s_strncmp(NS_INDEX_SIGNATURE, (char *)p_ns_index->signature, NS_INDEX_SIG_LEN) != 0)
	{
		COMMON_LOG_INFO_F("Invalid signature on NS Index Block %d", index_pos);
		rc = 0;
	}
	else if (p_ns_index->label_major_version != NS_INDEX_MAJOR)
	{
		COMMON_LOG_INFO_F("Invalid major version on NS Index Block %d", index_pos);
		rc = 0;
	}
	else if (p_ns_index->label_minor_version != NS_INDEX_MINOR)
	{
		COMMON_LOG_INFO_F("Invalid minor version on NS Index Block %d", index_pos);
		rc = 0;
	}
	// check myoff, mysize, otheroff
	else if (p_ns_index->my_offset != (index_pos * NS_INDEX_LEN))
	{
		COMMON_LOG_INFO_F("Invalid my_offset on NS Index Block %d", index_pos);
		rc = 0;
	}
	else if (p_ns_index->my_size != NS_INDEX_LEN)
	{
		COMMON_LOG_INFO_F("Invalid my_size on NS Index Block %d", index_pos);
		rc = 0;
	}
	else if (p_ns_index->other_offset != (index_pos == 0 ? NS_INDEX_LEN : 0))
	{
		COMMON_LOG_INFO_F("Invalid other_offset on NS Index Block %d", index_pos);
		rc = 0;
	}
	// check sequence number > 0
	else if (p_ns_index->sequence == 0)
	{
		COMMON_LOG_INFO_F("Invalid sequence number on NS Index Block %d", index_pos);
		rc = 0;
	}
	// check number of labels
	else if (p_ns_index->nlabel > MAX_NS_LABELS)
	{
		COMMON_LOG_INFO_F("Invalid label count on NS Index Block %d", index_pos);
		rc = 0;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL is_valid_label(struct pt_output_ns_label_v1_2 *p_label)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL rc = 1;

	if (!checksum_fletcher64((void *)p_label, sizeof (*p_label), &p_label->checksum, 0))
	{
		COMMON_LOG_INFO_F("Invalid namespace in slot %d, checksum failed",
			p_label->label.slot);
		rc = 0;
	}
	else if (NS_FLAGS_UPDATING(p_label->label.flags))
	{
		COMMON_LOG_INFO_F("Invalid namespace label in slot %d, raw size is 0",
			p_label->label.slot);
		rc = 0;
	}
	else if (p_label->label.rawsize == 0)
	{
		COMMON_LOG_INFO_F("Invalid namespace label in slot %d, raw size is 0",
			p_label->label.slot);
		rc = 0;
	}
	// ignore block ns
	else if (NS_FLAGS_LOCAL(p_label->label.flags))
	{
		COMMON_LOG_INFO_F(
			"Storage namespace label in slot %d ignored", p_label->label.slot);
		rc = 0;
	}
	// app direct ns checks
	else
	{
		if (p_label->label.position > p_label->label.nlabel)
		{
			COMMON_LOG_INFO_F(
				"Invalid namespace label in slot %d, position is > nlabel", p_label->label.slot);
			rc = 0;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int choose_index_block(struct pt_output_namespace_labels *p_labels)
{
	COMMON_LOG_ENTRY();
	int rc = 0; // no namespace index blocks = no namespaces

	NVM_BOOL index1_valid = is_valid_index_block(&p_labels->index1, 0);
	NVM_BOOL index2_valid = is_valid_index_block(&p_labels->index2, 1);

	// only index block 1 is valid
	if (index1_valid && !index2_valid)
	{
		rc = 1;
	}
	// only index block 2 is valid
	else if (index2_valid && !index1_valid)
	{
		rc = 2;
	}
	// both are valid
	else if (index1_valid && index2_valid)
	{
		// sequence #'s match, choose the higher offset
		if (p_labels->index1.sequence == p_labels->index2.sequence)
		{
			if (p_labels->index1.other_offset > p_labels->index1.my_offset)
			{
				rc = 2;
			}
			else
			{
				rc = 1;
			}
		}
		// newest is clockwise to the older 1->2->3->1...
		else
		{
			int seq_sum = p_labels->index1.sequence + p_labels->index2.sequence;
			// sequence numbers [1,2] or [2,3], higher one is newer
			if (seq_sum == 3 || seq_sum == 5)
			{
				rc = (p_labels->index1.sequence > p_labels->index2.sequence) ? 1 : 2;
			}
			// sequence numbers [1,3], lower one is newer
			else
			{
				rc = (p_labels->index1.sequence < p_labels->index2.sequence) ? 1 : 2;
			}
		}
	}
	// no valid index block = no namespaces
	else
	{
		rc = 0;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL is_slot_free(const NVM_UINT8 *free_map, const int slot)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL is_free = 0;
	NVM_UINT16 bits_in_block = sizeof (NVM_UINT8) * 8; // how many bits in a block
	NVM_UINT16 block = slot / bits_in_block; // subsequent block number in the bitmap
	NVM_UINT8 bit = (NVM_UINT8)(slot % bits_in_block); // subsequent bit number in a block
	if (free_map[block] & (1 << bit))
	{
		is_free = 1;
	}
	return is_free;
}

/*
 * Check if a namespace is already in the list by checking GUIDs. If
 * found, return the index in the list.
 */
int get_namespace_index_in_list(const struct ns_data *p_ns_data,
	const struct pt_output_ns_label_v1_2 *p_label)
{
	COMMON_LOG_ENTRY();
	COMMON_UID label_uid;
	guid_to_uid(p_label->label.uuid, label_uid);
	int index = -1;
	for (int i = 0; i < p_ns_data->ns_count; i++)
	{
		if (uid_cmp(label_uid, p_ns_data->ns_list[i].discovery.namespace_uid))
		{
			index = i;
			break;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(index);
	return index;
}

int get_isetid_from_nslabel_cookie(const struct ns_data *p_ns_data,
	const NVM_UINT64 cookie)
{
	int iset_id = -1;

	for (int i = 0; i < p_ns_data->iset_count; i++)
	{
		struct nvm_interleave_set set = p_ns_data->iset_list[i];
		if (cookie == set.cookie_v1_1 || cookie == set.cookie_v1_2)
		{
			iset_id = set.id;
			break;
		}
	}

	if (iset_id < 0)
	{
		COMMON_LOG_INFO("Failed to find an interleave set with the matching cookie");
		iset_id = 1;
	}

	return iset_id;
}

int namespace_has_abstraction_guid(
		const COMMON_GUID ns_abstraction_guid, const NVM_UID match_guid)
{
	int has_guid = 0;
	NVM_UID abstraction_uid;
	guid_to_uid(ns_abstraction_guid, abstraction_uid);
	if (uid_cmp(abstraction_uid, match_guid))
	{
		has_guid = 1;
	}
	return has_guid;
}

/*
 * Convert a namespace label to nvm_namespace_details struct
 * and add it to the list
 */
int init_namespace_from_label(struct ns_data **pp_ns_data,
	const int dimm_index, struct pt_output_ns_label_v1_2 *p_label)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (is_valid_label(p_label))
	{
		// is it already added to the list?
		int ns_index = get_namespace_index_in_list((*pp_ns_data), p_label);
		if (ns_index < 0)
		{
			(*pp_ns_data)->ns_list = realloc((*pp_ns_data)->ns_list,
				((*pp_ns_data)->ns_count + 1) * sizeof (struct nvm_namespace_details));
			if (!(*pp_ns_data)->ns_list)
			{
				COMMON_LOG_ERROR("No memory to collect namespace information");
				rc = NVM_ERR_NOMEMORY;
			}
			else
			{
#if 0 // print the label for easy debug
				NVM_UID uid;
				guid_to_uid(p_label->label.uuid, uid);
				printf("Uuid: %s\n", uid);
				printf("Name: %.64s\n", p_label->label.name);
				printf("Flags: 0x%x\n", p_label->label.flags);
				printf("Nlabel: 0x%x\n", p_label->label.nlabel);
				printf("Position: 0x%x\n", p_label->label.position);
				printf("IsetCookie: 0x%llx\n", p_label->label.iset_cookie);
				printf("LbaSize: 0x%llx\n", p_label->label.lba_size);
				printf("Dpa: 0x%llx\n", p_label->label.dpa);
				printf("Rawsize: 0x%llx\n", p_label->label.rawsize);
				printf("Slot: 0x%x\n", p_label->label.slot);
				printf("Alignment: 0x%x\n", p_label->alignment);
				NVM_UID type_uid;
				guid_to_uid(p_label->type_guid, type_uid);
				printf("TypeGuid: %s\n", type_uid);
				NVM_UID abstraction_uid;
				guid_to_uid(p_label->address_abstraction_guid, abstraction_uid);
				printf("AddressAbstractionGuid: %s\n", abstraction_uid);
#endif
				struct nvm_namespace_details *p_ns_details =
					&(*pp_ns_data)->ns_list[(*pp_ns_data)->ns_count];
				(*pp_ns_data)->ns_count++;
				guid_to_uid(p_label->label.uuid, p_ns_details->discovery.namespace_uid);
				s_strcpy(p_ns_details->discovery.friendly_name,
						(char *)p_label->label.name, NS_NAME_LEN);
				p_ns_details->type = NAMESPACE_TYPE_APP_DIRECT;
				p_ns_details->enabled = NAMESPACE_ENABLE_STATE_ENABLED;
				p_ns_details->block_size = p_label->label.lba_size ? p_label->label.lba_size : 1;
				p_ns_details->block_count = p_label->label.rawsize / p_ns_details->block_size;

				p_ns_details->namespace_creation_id.interleave_setid =
					get_isetid_from_nslabel_cookie(*pp_ns_data, p_label->label.iset_cookie);

				// is BTT
				p_ns_details->btt = namespace_has_abstraction_guid(
						p_label->address_abstraction_guid, NS_BTT_GUID);

				// is PFN
				if (namespace_has_abstraction_guid(
						p_label->address_abstraction_guid, NS_PFN_GUID))
				{
					p_ns_details->memory_page_allocation =
							NAMESPACE_MEMORY_PAGE_ALLOCATION_APP_DIRECT;
				}
				p_ns_details->nlabels = p_label->label.nlabel;
				// + 1 because positions start at 0
				p_ns_details->label_sum = (p_label->label.position + 1);
			}
		}
		else
		{
			// Update capacity
			struct nvm_namespace_details *p_ns_details = &(*pp_ns_data)->ns_list[ns_index];
			p_ns_details->block_size = p_label->label.lba_size ? p_label->label.lba_size : 1;
			p_ns_details->block_count += p_label->label.rawsize / p_ns_details->block_size;

			// Update completeness
			// + 1 because positions start at 0
			p_ns_details->label_sum += (p_label->label.position + 1);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int parse_dimm_ns_labels(struct ns_data **pp_ns_data, const int dimm_index)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	struct pt_output_namespace_labels dimm_lsa = (*pp_ns_data)->dimm_nslsa_list[dimm_index];
	int index_to_use = choose_index_block(&dimm_lsa);
	if (index_to_use) // 0 index block = no labels
	{
		struct pt_output_ns_index ns_index =
			index_to_use == 1 ? dimm_lsa.index1 : dimm_lsa.index2;
		for (int i = 0; i < ns_index.nlabel; i++)
		{
			if (!is_slot_free(ns_index.free, i))
			{
				if (ns_index.label_major_version == 1 &&
					ns_index.label_minor_version == 1)
				{
					int offset = i * sizeof (struct pt_output_ns_label_v1_1);
					struct pt_output_ns_label_v1_1 *p_label =
							(struct pt_output_ns_label_v1_1 *)(dimm_lsa.labels + offset);

					// convert to a v1.2 label
					struct pt_output_ns_label_v1_2 label;
					memset(&label, 0, sizeof (struct pt_output_ns_label_v1_2));
					memmove(&label.label, &p_label->label, sizeof (struct pt_output_ns_label));
					// create a valid checksum
					checksum_fletcher64((void *)&label, sizeof (label),
							&label.checksum, 1);

				    rc = init_namespace_from_label(pp_ns_data, dimm_index, &label);
				}
				else if (ns_index.label_major_version == 1 &&
						ns_index.label_minor_version == 2)
				{
					int offset = i * sizeof (struct pt_output_ns_label_v1_2);
						struct pt_output_ns_label_v1_2 *p_label =
							(struct pt_output_ns_label_v1_2 *)(dimm_lsa.labels + offset);
				    rc = init_namespace_from_label(pp_ns_data, dimm_index, p_label);
				}
			}
			if (rc != NVM_SUCCESS)
			{
				break;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Ensure all labels have been found on the specified namespace
 */
NVM_BOOL is_namespace_complete(const struct nvm_namespace_details *p_ns)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL complete = 1;
	NVM_UINT32 expected = (p_ns->nlabels * (p_ns->nlabels + 1)) / 2;
	if (p_ns->label_sum != expected)
	{
		NVM_UINT32 missing = expected - p_ns->label_sum;
		COMMON_LOG_INFO_F("Namespace is incomplete, missing label in position %u", missing);
		complete = 0;
	}
	COMMON_LOG_EXIT_RETURN_I(complete);
	return complete;
}

/*
 * Populate the namespace list from context or by reading the namepace
 * labels from all manageable DIMMs
 */
int populate_namespaces(int ns_count, struct nvm_namespace_details *p_namespaces)
{
	COMMON_LOG_ENTRY();

	// get the namespaces from the context
	int rc = get_nvm_context_pcd_namespace_count();
	if (rc > 0 && ns_count && p_namespaces)
	{
		rc = get_nvm_context_pcd_namespaces(ns_count, p_namespaces);
	}
	else if (rc < 0)
	{
		struct ns_data *p_ns_data = NULL;
		if ((rc = collect_required_ns_data(&p_ns_data) == NVM_SUCCESS))
		{
			for (int i = 0; i < p_ns_data->dimm_count; i ++)
			{
				rc = parse_dimm_ns_labels(&p_ns_data, i);
				if (rc != NVM_SUCCESS)
				{
					break;
				}
			}
			// copy and set context
			if (rc == NVM_SUCCESS && p_ns_data->ns_count)
			{
				// only want to parse namespaces once, so set
				// context even if the user didn't pass in a list of
				// namespaces
				struct nvm_namespace_details tmp_nslist[p_ns_data->ns_count];
				memset(tmp_nslist, 0,
					p_ns_data->ns_count * sizeof (struct nvm_namespace_details));
				int copy_count = 0;
				for (int i = 0; i < p_ns_data->ns_count; i++)
				{
					if (is_namespace_complete(&p_ns_data->ns_list[i]))
					{
						memmove(&tmp_nslist[copy_count], &p_ns_data->ns_list[i],
							sizeof (struct nvm_namespace_details));
						if (ns_count && p_namespaces)
						{
							if (copy_count >= ns_count)
							{
								rc = NVM_ERR_ARRAYTOOSMALL;
							}
							else
							{
								memmove(&p_namespaces[copy_count], &p_ns_data->ns_list[i],
									sizeof (struct nvm_namespace_details));
							}
						}
						copy_count++;
					}
				}
				if (rc == NVM_SUCCESS)
				{
					rc = copy_count;
					set_nvm_context_pcd_namespaces(copy_count, tmp_nslist);
				}
			}
			free_ns_data(p_ns_data);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve the count of namespaces from the PCD data on all manageable DIMMs
 */
int get_namespace_count_from_pcd()
{
	COMMON_LOG_ENTRY();
	int rc = populate_namespaces(0, NULL);
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve namespace discovery information from the PCD data on all manageable DIMMs
 */
int get_namespaces_from_pcd(const NVM_UINT32 count, struct nvm_namespace_discovery *p_namespaces)
{
	COMMON_LOG_ENTRY();
	struct nvm_namespace_details pcd_namespaces[count];
	memset(pcd_namespaces, 0, sizeof (struct nvm_namespace_details) * count);
	int rc = populate_namespaces(count, pcd_namespaces);
	if (rc >= 0)
	{
		for (int i = 0; i < rc; i++)
		{
			memmove(&p_namespaces[i], &pcd_namespaces[i].discovery,
				sizeof (struct nvm_namespace_discovery));
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve namespace details from the PCD data on all manageable DIMMs
 */
int get_namespace_details_from_pcd(
		const NVM_UID namespace_uid,
		struct nvm_namespace_details *p_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_BADNAMESPACE;
	int ns_count = get_namespace_count_from_pcd();
	if (ns_count > 0)
	{
		struct nvm_namespace_details pcd_namespaces[ns_count];
		memset(pcd_namespaces, 0, sizeof (struct nvm_namespace_details) * ns_count);
		ns_count = populate_namespaces(ns_count, pcd_namespaces);
		if (ns_count > 0)
		{
			for (int i = 0; i < ns_count; i++)
			{
				if (uid_cmp(namespace_uid, pcd_namespaces[i].discovery.namespace_uid))
				{
					memmove(p_details, &pcd_namespaces[i],
						sizeof (struct nvm_namespace_details));
					rc = NVM_SUCCESS;
					break;
				}
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Zero the namespace label PCD partition on the specified DIMM
 */
int zero_dimm_namespace_labels(const NVM_UINT32 device_handle)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct fw_cmd cfg_cmd;
	memset(&cfg_cmd, 0, sizeof (cfg_cmd));
	cfg_cmd.device_handle = device_handle;
	cfg_cmd.opcode = PT_SET_ADMIN_FEATURES;
	cfg_cmd.sub_opcode = SUBOP_PLATFORM_DATA_INFO;

	struct pt_payload_set_platform_cfg_data input_payload;
	memset(&input_payload, 0, sizeof (input_payload));
	input_payload.partition_id = NS_LABEL_PCDPARTITION;
	input_payload.payload_type = DEV_PLT_CFG_LARGE_PAY;
	cfg_cmd.input_payload_size = sizeof (input_payload);
	cfg_cmd.input_payload = &input_payload;
	struct pt_output_namespace_labels labels;
	memset(&labels, 0, sizeof (labels));
	cfg_cmd.large_input_payload_size = sizeof (labels);
	cfg_cmd.large_input_payload = &labels;

	rc = ioctl_passthrough_cmd(&cfg_cmd);
	if (rc == NVM_SUCCESS)
	{
		invalidate_namespaces();
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
