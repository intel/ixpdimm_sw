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
 * This file implements the internal interface for system level functionality in
 * the Linux adapter.
 */

#include "system.h"
#include <numa.h>
#include <persistence/logging.h>
#include <os/os_adapter.h>
#include <system/system.h>
#include <smbios/smbios.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#define	EFI_SYSTAB "/sys/firmware/efi/systab"

// Helper function declarations
int get_numa_nodes(NVM_UINT16 *p_node_id, NVM_UINT16 count);
int get_numa_node_count();
int process_bitmask(struct bitmask *p_bitmask, NVM_UINT16 *p_bit_index, NVM_UINT16 bit_index_size);
int get_numa_node_logical_processor_count(NVM_UINT16 node_id);
int get_cpu_data_from_numa_node(NVM_UINT16 node_id, struct socket *p_node);
NVM_BOOL numa_node_id_exists(NVM_UINT16 node_id);

/*
 * Load a simulator file.
 */
int add_simulator(const NVM_PATH simulator, const NVM_SIZE simulator_len)
{
	COMMON_LOG_ENTRY();

	// not supported in Linux adapter
	int rc = NVM_ERR_NOTSUPPORTED;

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Remove a simulator file.
 */
int remove_simulator()
{
	COMMON_LOG_ENTRY();

	// not supported in Linux adapter
	int rc = NVM_ERR_NOTSUPPORTED;

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve basic information about the host server the native API library is running on.
 */
int get_host(struct host *p_host)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// get the host name
	if (get_host_name(p_host->name, NVM_COMPUTERNAME_LEN) != COMMON_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve the host name.");
		rc = NVM_ERR_UNKNOWN;
	}

	// set the OS type
	p_host->os_type = OS_TYPE_LINUX;

	// get the OS name
	if (get_os_name(p_host->os_name, NVM_OSNAME_LEN) != COMMON_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve the OS name.");
		rc = NVM_ERR_UNKNOWN;
	}

	// get the OS version
	if (get_os_version(p_host->os_version, NVM_OSVERSION_LEN) != COMMON_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve the OS version.");
		rc = NVM_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieves the number of NUMA nodes in the system, which is is equivalent
 * to the number of physical processors (or sockets).
 */
int get_socket_count()
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	// required to check if NUMA is available before getting the count
	if (numa_available() < 0)
	{
		COMMON_LOG_ERROR("NUMA API is not supported by the system.");
		rc = NVM_ERR_NOTSUPPORTED;
	}
	else
	{
		rc = get_numa_node_count();
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieves NUMA information about each physical processor in the system
 */
int get_sockets(struct socket *p_node, NVM_UINT16 count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (numa_available() < 0)
	{
		COMMON_LOG_ERROR("NUMA API is not supported by the system.");
		rc = NVM_ERR_NOTSUPPORTED;
	}
	else
	{
		unsigned int socket_count = get_socket_count();
		if (socket_count > 0)
		{
			// we can allocate here because the calling function nvm_get_numa_nodes(..) does the
			// input parameter checking for each adapter's implementation
			NVM_UINT16 node_ids[socket_count];

			// use our helper to get node_ids
			if ((rc = get_numa_nodes(node_ids, socket_count)) < NVM_SUCCESS)
			{
				COMMON_LOG_ERROR("Failed to identify the NUMA node_ids.");
			}
			else if (rc > 0)
			{
				int num_numa_nodes = rc;

				// clear the output buffer as defined by the user
				memset(p_node, 0, count * sizeof (struct socket));

				// gets the node mask that the current process runs within
				struct bitmask *p_process_node_mask = numa_get_run_node_mask();
				if (p_process_node_mask == NULL)
				{
					COMMON_LOG_ERROR("Failed to get the current process node mask");
					rc = NVM_ERR_UNKNOWN;
				}
				else
				{
					// construct the output, based on the return of the helper
					for (int i_node = 0; i_node < num_numa_nodes; i_node++)
					{
						if (i_node >= count)
						{
							rc = NVM_ERR_ARRAYTOOSMALL;
							COMMON_LOG_ERROR("Invalid parameter, "
									"count is smaller than number of sockets");
							break;
						}

						// if we fail to get the info from the NUMA node, stop looping
						if ((rc = get_cpu_data_from_numa_node(node_ids[i_node], &(p_node[i_node])))
								< NVM_SUCCESS)
						{
							COMMON_LOG_ERROR_F("Failed to get the NUMA information "
									"for node_id(%hu)",
									node_ids[i_node]);
							break;
						}

						// set rc to the number of NUMA nodes successfully queried thus far
						rc = i_node + 1;
					}

					// regardless of success or failure, try to restore the process'
					// original affinity
					if (numa_run_on_node_mask(p_process_node_mask) != 0)
					{
						COMMON_LOG_ERROR("Failed to restore process to original affinity.");
						// maintain any previous error
						KEEP_ERROR(rc, NVM_ERR_UNKNOWN);
					}

					// and free the nodemask
					numa_bitmask_free(p_process_node_mask);
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieves NUMA information about a given NUMA node number
 */
int get_socket(NVM_UINT16 node_id, struct socket *p_node)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (numa_available() < 0)
	{
		COMMON_LOG_ERROR("NUMA API is not supported by the system.");
		rc = NVM_ERR_NOTSUPPORTED;
	}
	else if (!(numa_node_id_exists(node_id)))
	{
		COMMON_LOG_ERROR("NUMA node_id does not exist");
	}
	else
	{
		// clear the output buffer as defined by the user
		memset(p_node, 0, sizeof (struct socket));

		// gets the node mask that the current process runs within
		struct bitmask *p_process_node_mask = numa_get_run_node_mask();
		if (p_process_node_mask == NULL)
		{
			COMMON_LOG_ERROR("Failed to get the current process node mask");
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{
			rc = get_cpu_data_from_numa_node(node_id, p_node);

			// regardless of success or failure, try to restore the process' original affinity
			if (numa_run_on_node_mask(p_process_node_mask) != 0)
			{
				COMMON_LOG_ERROR("Failed to restore process to original affinity.");
				// maintain any previous error
				KEEP_ERROR(rc, NVM_ERR_UNKNOWN);
			}

			// and free the nodemask
			numa_bitmask_free(p_process_node_mask);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * This will count the number of set bits in a NUMA bitmask, and optionally process
 * that mask into an array of the indices where those bits are set
 */
int process_bitmask(struct bitmask *p_bitmask, NVM_UINT16 *p_bit_index, NVM_UINT16 bit_index_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (p_bit_index && (bit_index_size == 0))
	{
		COMMON_LOG_ERROR("When p_bit_index exists, bit_index_size must be non-zero.");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		rc = 0;
		// need to iterate through each bit in the mask to find the node count
		// since the mask is an array of ulong segments, accumulate by segment
		NVM_UINT16 bits_per_ulong = (NVM_UINT16)(8 * sizeof (unsigned long));
		NVM_UINT16 num_segments = ((NVM_UINT16)(p_bitmask->size)) / bits_per_ulong;
		for (NVM_UINT16 i_seg = 0; i_seg < num_segments; i_seg++)
		{
			// do until all set bits in the segment have been accounted for,
			// remembering that the values returned are zero-indexed
			for (NVM_UINT16 i_ulong = 0; p_bitmask->maskp[i_seg]; i_ulong++)
			{
				// if the LSB is set, we have found a valid NUMA node number
				if (p_bitmask->maskp[i_seg] & 0x1)
				{
					// check if the caller is asking for the node_id as well
					if (p_bit_index)
					{
						// count should never be less than (index + 1)
						if (bit_index_size <= rc)
						{
							COMMON_LOG_ERROR("The provided array is too small.");
							rc = NVM_ERR_ARRAYTOOSMALL;

							// break out of the loop and exit
							break;
						}
						else
						{
							p_bit_index[rc] = i_ulong + (i_seg * bits_per_ulong);
						}
					}

					// increment the return
					rc++;
				}

				// right shift bits out of the ulong, one at a time
				p_bitmask->maskp[i_seg] >>= 1;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to get the number of valid NUMA nodes
 */
int get_numa_node_count()
{
	COMMON_LOG_ENTRY();
	int num_nodes = 0;

	int max_node = numa_max_node();
	for (int i = 0; i <= max_node; i++)
	{
		if (numa_bitmask_isbitset(numa_nodes_ptr, i))
		{
			num_nodes++;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(num_nodes);
	return num_nodes;
}

/*
 * Helper function that gets valid NUMA node_ids, and returns the number of nodes
 */
int get_numa_nodes(NVM_UINT16 *p_node_id, const NVM_UINT16 count)
{
	COMMON_LOG_ENTRY();
	int actual_node_count = 0;

	int max_node = numa_max_node();
	for (int i = 0; i <= max_node; i++)
	{
		if (numa_bitmask_isbitset(numa_nodes_ptr, i))
		{
			if (count <= actual_node_count)
			{
				COMMON_LOG_ERROR("The provided array is too small.");
				actual_node_count = NVM_ERR_ARRAYTOOSMALL;
				break;
			}
			p_node_id[actual_node_count] = i;
			actual_node_count++;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(actual_node_count);
	return actual_node_count;
}

/*
 * Gets the number of logical processors for the given NUMA node
 */
int get_numa_node_logical_processor_count(NVM_UINT16 node_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// get the (logical) cpumask for the given NUMA node_id
	struct bitmask *p_lpmask = numa_allocate_cpumask();
	if (p_lpmask == NULL)
	{
		COMMON_LOG_ERROR("Failed to allocate a cpumask");
	}
	else
	{
		// get the (logical) cpumask for the given node_id
		if (numa_node_to_cpus(node_id, p_lpmask) != 0)
		{
			COMMON_LOG_ERROR_F("Failed to get the logical processor mask for NUMA node_id[%hu].",
					node_id);
		}
		else
		{
			// count the number of set bits in the cpumask
			rc = process_bitmask(p_lpmask, NULL, 0);
		}

		// free the cpumask
		numa_bitmask_free(p_lpmask);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function that switches the current process' node affinity, and retrieves the NUMA info
 */
int get_cpu_data_from_numa_node(NVM_UINT16 node_id, struct socket *p_node)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// check input struct pointer
	if (p_node == NULL)
	{
		COMMON_LOG_ERROR("p_node cannot be NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	// move process to given node_id
	else if (numa_run_on_node((int)node_id) != 0)
	{
		COMMON_LOG_ERROR_F("Failed to move process to NUMA node [%hu]", node_id);
	}
	// get the information for the node, from the node
	else
	{
		int temp_rc = 0;
		processor_info info;

		// CPUID derived
		if ((temp_rc = get_processor_manufacturer(p_node->manufacturer,
				NVM_SOCKET_MANUFACTURER_LEN)) != COMMON_SUCCESS)
		{
			COMMON_LOG_ERROR_F("get_processor_manufacturer failed on node_id(%hu) with rc: %d",
					node_id, temp_rc);
			switch (temp_rc)
			{
				case COMMON_ERR_INVALIDPARAMETER:
					rc = NVM_ERR_INVALIDPARAMETER;
					break;
				case COMMON_ERR_UNKNOWN:
				default:
					rc = NVM_ERR_UNKNOWN;
					break;
			}
		}
		// CPUID derived
		else if ((temp_rc = get_processor_info(&info)) != COMMON_SUCCESS)
		{
			COMMON_LOG_ERROR_F("get_processor_info failed on node_id(%hu) with rc: %d", node_id,
					temp_rc);
			switch (temp_rc)
			{
				case COMMON_ERR_INVALIDPARAMETER:
					rc = NVM_ERR_INVALIDPARAMETER;
					break;
				case COMMON_ERR_UNKNOWN:
				default:
					rc = NVM_ERR_UNKNOWN;
					break;
			}
		}
		// NUMA derived
		else if ((temp_rc = get_numa_node_logical_processor_count(node_id)) < NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Failed to get the logical processor mask for NUMA node_id[%hu].",
					node_id);
			rc = temp_rc;
		}
		else
		{
			p_node->id = node_id;
			p_node->type = info.type;
			p_node->model = info.model;
			p_node->family = info.family;
			p_node->stepping = info.stepping;
			p_node->brand = info.brand_index;
			p_node->logical_processor_count = (NVM_UINT16)temp_rc;

			rc = NVM_SUCCESS;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to identify if a node_id is valid
 */
NVM_BOOL numa_node_id_exists(NVM_UINT16 node_id)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL node_exists = 0;

	int numa_node_count = get_numa_node_count();
	if (numa_node_count > 0)
	{
		NVM_UINT16 node_ids[numa_node_count];
		numa_node_count = get_numa_nodes(node_ids, numa_node_count);

		if (numa_node_count > 0)
		{
			for (int i = 0; i < numa_node_count; i++)
			{
				// if the node is found, then return success
				if (node_id == node_ids[i])
				{
					node_exists = 1;
					break;
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(node_exists);
	return node_exists;
}

int get_smbios_entry_point_from_offset(int mem_fd, size_t entry_point_offset,
		struct smbios_entry_point *p_entry_point)
{
	int rc = NVM_SUCCESS;

	size_t size_to_read = 0;
	NVM_UINT8 *p_data = NULL;
	if (p_entry_point->type == SMBIOS_ENTRY_POINT_64BIT)
	{
		size_to_read = sizeof (struct smbios_64bit_entry_point);
		p_data = (NVM_UINT8 *)&(p_entry_point->data.entry_point_64_bit);
	}
	else
	{
		size_to_read = sizeof (struct smbios_32bit_entry_point);
		p_data = (NVM_UINT8 *)&(p_entry_point->data.entry_point_32_bit);
	}

	if (lseek(mem_fd, entry_point_offset, SEEK_SET) < 0)
	{
		rc = NVM_ERR_UNKNOWN;
	}
	else if (read(mem_fd, p_data, size_to_read) != size_to_read)
	{
		rc = NVM_ERR_UNKNOWN;
	}

	return rc;
}

int get_smbios_entry_point_from_efi(int mem_fd, struct smbios_entry_point *p_entry_point)
{
	int rc = NVM_ERR_UNKNOWN;

	FILE *fp = fopen(EFI_SYSTAB, "r");
	if (fp == NULL)
	{
		COMMON_LOG_ERROR("Couldn't open systab");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		char str_buff[128];
		while (fgets(str_buff, 128, fp) != NULL)
		{
			char *smbios_str = strstr(str_buff, "SMBIOS");

			if (smbios_str != NULL)
			{
				char *smbios_offset_str = strstr(smbios_str, "0x");
				if (smbios_offset_str != NULL)
				{
					size_t offset = strtoul(smbios_offset_str, NULL, 0);
					rc = get_smbios_entry_point_from_offset(mem_fd, offset, p_entry_point);
					break;
				}
			}
		}
		fclose(fp);
	}

	return rc;
}

int get_smbios_entry_point(int mem_fd, struct smbios_entry_point *p_entry_point)
{
	int rc = NVM_ERR_UNKNOWN;

	// Look for anchor string on paragraph (16-byte) alignments
	const size_t paragraph_size = 16;
	unsigned char tmpbuf[paragraph_size];

	// SMBIOS entry point only appears within a specific physical memory range
	if (lseek(mem_fd, SMBIOS_ADDR_RANGE_START, SEEK_SET) >= 0)
	{
		size_t offset = SMBIOS_ADDR_RANGE_START;

		ssize_t result = 1;
		while ((result > 0) && (offset < SMBIOS_ADDR_RANGE_END))
		{
			result = read(mem_fd, tmpbuf, paragraph_size);

			if (memcmp(tmpbuf, SMBIOS_64BIT_ANCHOR_STR, sizeof (SMBIOS_64BIT_ANCHOR_STR)) == 0)
			{
				p_entry_point->type = SMBIOS_ENTRY_POINT_64BIT;
				rc = get_smbios_entry_point_from_offset(mem_fd, offset, p_entry_point);
				break;
			}
			else if (memcmp(tmpbuf, SMBIOS_32BIT_ANCHOR_STR, sizeof (SMBIOS_32BIT_ANCHOR_STR)) == 0)
			{
				p_entry_point->type = SMBIOS_ENTRY_POINT_32BIT;
				rc = get_smbios_entry_point_from_offset(mem_fd, offset, p_entry_point);
				break;
			}

			offset += paragraph_size;
		}

		// Try getting offset from GPT if MBG didn't work
		if (rc == NVM_ERR_UNKNOWN)
		{
			rc = get_smbios_entry_point_from_efi(mem_fd, p_entry_point);
		}
	}
	return rc;
}

int copy_smbios_table_from_mem_alloc(int mem_fd, size_t address, size_t size_to_allocate,
		NVM_UINT8 **pp_smbios_table, size_t *p_allocated_size)
{
	int rc = NVM_SUCCESS;

	NVM_UINT8* p_smbios_table = calloc(1, size_to_allocate);
	if (p_smbios_table)
	{
		if (lseek(mem_fd, address, SEEK_SET) >= 0 &&
			read(mem_fd, p_smbios_table, size_to_allocate) == size_to_allocate)
		{
			*pp_smbios_table = p_smbios_table;
			*p_allocated_size = size_to_allocate;
		}
		else
		{
			COMMON_LOG_ERROR_F("couldn't read SMBIOS structure table from address 0x%llx", address);
			free(p_smbios_table);
			rc = NVM_ERR_UNKNOWN;
		}
	}
	else
	{
		COMMON_LOG_ERROR("couldn't allocate memory for SMBIOS table copy");
		rc = NVM_ERR_NOMEMORY;
	}

	return rc;
}

/*
 * Harvest the raw SMBIOS table data from memory and allocate a copy
 * to parse.
 */
int get_smbios_table_alloc(NVM_UINT8 **pp_smbios_table, size_t *p_allocated_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	int fd = open("/dev/mem", O_RDONLY);
	if (fd < 0)
	{
		COMMON_LOG_ERROR("Couldn't open /dev/mem to read SMBIOS table");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		struct smbios_entry_point entry_point;
		memset(&entry_point, 0, sizeof (entry_point));
		rc = get_smbios_entry_point(fd, &entry_point);
		if (rc == NVM_SUCCESS)
		{
			size_t address = 0;
			size_t size_to_allocate = 0;
			if (entry_point.type == SMBIOS_ENTRY_POINT_64BIT)
			{
				size_to_allocate = entry_point.data.entry_point_64_bit.structure_table_max_length;
				address = entry_point.data.entry_point_64_bit.structure_table_address;
			}
			else // 32-bit entry point
			{
				size_to_allocate = entry_point.data.entry_point_32_bit.structure_table_length;
				address = entry_point.data.entry_point_32_bit.structure_table_address;
			}

			rc = copy_smbios_table_from_mem_alloc(fd, address, size_to_allocate, pp_smbios_table,
					p_allocated_size);
		}

		close(fd);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Determine if the caller has permission to make changes to the system
 */
int check_caller_permissions()
{
	int rc = COMMON_SUCCESS;
	rc = check_admin_permissions();
	return rc;
}
