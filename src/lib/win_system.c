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
 * the Windows adapter.
 */

#include "system.h"
#include <persistence/logging.h>
#include <os/os_adapter.h>
#include <system/system.h>
#include <windows.h>

int find_numa_nodes(NVM_UINT16 *p_node_id, NVM_UINT16 count);
int set_affinity_numa_node(NVM_UINT16 node_id);
int get_numa_nodes_logical_processor_count(NVM_UINT16 *p_lpcount, NVM_UINT16 *p_nodeids,
		int num_nodes);
int get_cpu_data_from_numa_nodes(NVM_UINT16 *p_node_ids, struct socket *p_sockets,
		NVM_UINT16 count);
int numa_node_id_exists(NVM_UINT16 node_id);

/*
 * Helper function - assumes string length >= 4
 */
extern DWORD string_to_dword(const char *str);

/*
 * Standard Windows structure for SMBIOS data.
 * GetSystemFirmwareTable returns a blob in this format.
 */
struct RawSMBIOSData
{
    BYTE    Used20CallingMethod;
    BYTE    SMBIOSMajorVersion;
    BYTE    SMBIOSMinorVersion;
    BYTE    DmiRevision;
    DWORD   Length;
    BYTE    SMBIOSTableData[];
} __attribute__((packed));

/*
 * Load a simulator file.
 */
int add_simulator(const NVM_PATH simulator, const NVM_SIZE simulator_len)
{
	COMMON_LOG_ENTRY();

	// not supported in Windows adapter
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

	// not supported in Windows adapter
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

	if (get_host_name(p_host->name, NVM_COMPUTERNAME_LEN) != COMMON_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve the host name.");
		rc = NVM_ERR_UNKNOWN;
	}

	p_host->os_type = OS_TYPE_WINDOWS;

	if (get_os_name(p_host->os_name, NVM_OSNAME_LEN) != COMMON_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve the OS name.");
		rc = NVM_ERR_UNKNOWN;
	}

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
	int rc = NVM_ERR_UNKNOWN;

	rc = find_numa_nodes(NULL, 0);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieves NUMA information about each physical processor in the system
 */
int get_sockets(struct socket *p_sockets, NVM_UINT16 count)
{
	COMMON_LOG_ENTRY();
	int rc = get_socket_count();
	if (rc >= 0)
	{
		unsigned int socket_count = rc;
		NVM_UINT16 node_ids[socket_count];
		if ((rc = find_numa_nodes(node_ids, socket_count)) < 0)
		{
			COMMON_LOG_ERROR("Failed to identify the NUMA node_ids.");
		}
		else
		{
			memset(p_sockets, 0, count * sizeof (struct socket));

			// get out the thread's current affinity to restore later
			GROUP_AFFINITY old_affinity;
			HANDLE thread_handle = GetCurrentThread();
			if (!GetThreadGroupAffinity(thread_handle, &old_affinity))
			{
				COMMON_LOG_ERROR("Failed to get the current affinity mask");
				rc = NVM_ERR_UNKNOWN;
			}
			else
			{
				rc = get_cpu_data_from_numa_nodes(node_ids, p_sockets, count);
				if (count < socket_count)
				{
					rc = NVM_ERR_ARRAYTOOSMALL;
				}

				// regardless of success or failure, try to restore the process' original affinity
				if (!SetThreadGroupAffinity(thread_handle, &old_affinity, NULL))
				{
					COMMON_LOG_ERROR("Failed to restore process to original affinity.");
					KEEP_ERROR(rc, NVM_ERR_UNKNOWN);
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
int get_socket(NVM_UINT16 node_id, struct socket *p_socket)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (!p_socket)
	{
		COMMON_LOG_ERROR("p_socket was NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = numa_node_id_exists(node_id)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("NUMA node_id %hu does not exist", node_id);
	}
	else
	{
		memset(p_socket, 0, sizeof (struct socket));

		// get out the thread's current affinity to restore later
		GROUP_AFFINITY old_affinity;
		HANDLE thread_handle = GetCurrentThread();
		if (!GetThreadGroupAffinity(thread_handle, &old_affinity))
		{
			COMMON_LOG_ERROR("Failed to get the current process affinity mask");
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{
			if ((rc = get_cpu_data_from_numa_nodes(&node_id, p_socket, 1)) < 0)
			{
				COMMON_LOG_ERROR_F("Failed to get processor information from NUMA node_id: %hu",
						node_id);
				KEEP_ERROR(rc, NVM_ERR_UNKNOWN);
			}
			else
			{
				rc = NVM_SUCCESS;
			}

			// regardless of success or failure, try to restore the process' original affinity
			if (!SetThreadGroupAffinity(thread_handle, &old_affinity, NULL))
			{
				COMMON_LOG_ERROR("Failed to restore process to original affinity.");
				KEEP_ERROR(rc, NVM_ERR_UNKNOWN);
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_logical_processor_count_for_node_from_system_logical_processor_info(NVM_UINT16 node_id,
		BYTE *p_buffer, DWORD buffer_size)
{
	int num_processors = NVM_ERR_BADSOCKET;

	/*
	 * SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX has variable size, so we have to
	 * use pointer arithmetic instead of polite array logic.
	 */
	DWORD offset = 0;
	while (offset < buffer_size)
	{
		SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *p_slpi =
				(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *)(p_buffer + offset);
		if (p_slpi->Relationship == RelationNumaNode &&
				p_slpi->NumaNode.NodeNumber == node_id)
		{
			KAFFINITY processor_mask = p_slpi->NumaNode.GroupMask.Mask;
			// count the bits in the mask
			for (num_processors = 0; processor_mask;
					processor_mask >>= 1)
			{
				num_processors += processor_mask & 1;
			}

			break;
		}

		offset += p_slpi->Size;
	}

	return num_processors;
}

int get_numa_nodes_logical_processor_count(NVM_UINT16 *p_lpcount, NVM_UINT16 *p_nodeids,
		int num_nodes)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	DWORD buffer_size = 0;
	// this will return in error, but it will get us the correct buffer_size
	GetLogicalProcessorInformationEx(RelationNumaNode, NULL, &buffer_size);

	BYTE buffer[buffer_size];
	memset(buffer, 0, buffer_size);
	if (!GetLogicalProcessorInformationEx(RelationNumaNode,
			(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *)buffer, &buffer_size))
	{
		COMMON_LOG_ERROR("Failed to get the logical processor information for all NUMA nodes.");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		memset(p_lpcount, 0, num_nodes * sizeof (NVM_UINT16));

		rc = 0;
		for (int node_idx = 0; node_idx < num_nodes; node_idx++)
		{
			int num_processors =
					get_logical_processor_count_for_node_from_system_logical_processor_info(
					p_nodeids[node_idx], buffer, buffer_size);
			if (num_processors < 0)
			{
				rc = num_processors;
				break;
			}
			else
			{
				p_lpcount[node_idx] = num_processors;
				rc++;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Sets the current process affinity to those logical processors associated with the
 * given NUMA node id.
 */
int set_affinity_numa_node(NVM_UINT16 node_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	GROUP_AFFINITY grp_affinity;
	if (!GetNumaNodeProcessorMaskEx(node_id, &grp_affinity))
	{
		COMMON_LOG_ERROR_F("Unable to obtain the logical processor mask for NUMA node_id: %hu",
				node_id);
	}
	else if (!SetThreadGroupAffinity(GetCurrentThread(), &grp_affinity, NULL))
	{
		COMMON_LOG_ERROR_F("Unable to set the process affinity to NUMA node_id: %hu", node_id);
	}
	else
	{
		rc = NVM_SUCCESS;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Gets the number of valid NUMA nodes, and (optionally) their node_ids
 */
int find_numa_nodes(NVM_UINT16 *p_node_id, NVM_UINT16 count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (p_node_id && (count == 0))
	{
		COMMON_LOG_ERROR("When p_nodes exists, count must be non-zero.");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		if (p_node_id)
		{
			memset(p_node_id, 0, count * sizeof (NVM_UINT16));
		}

		// retrieve the largest NUMA node number (zero-indexed)
		ULONG biggestNumaNodeNumber;
		if (!GetNumaHighestNodeNumber(&biggestNumaNodeNumber))
		{
			COMMON_LOG_ERROR("Failed to get the highest NUMA node_id");
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{
			rc = 0;

			// we have the largest node number, but there could be holes
			// windows does not provide a function to obtain the NUMA node mask, so
			// if we can retrieve the processor mask, it is a valid NUMA node
			GROUP_AFFINITY mask;
			for (ULONG i = 0; i <= biggestNumaNodeNumber; ++i)
			{
				if (GetNumaNodeProcessorMaskEx((USHORT)i, &mask))
				{
					if (p_node_id)
					{
						if (count <= rc)
						{
							COMMON_LOG_ERROR("The provided array is too small.");
							rc = NVM_ERR_ARRAYTOOSMALL;
							break;
						}
						else
						{
							p_node_id[rc] = (NVM_UINT16)i;
						}
					}

					rc++;
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Switches the current process' node affinity, and retrieves the NUMA info
 * for as many nodes are provided as input.  It was algorithmically more efficient to
 * do it this way, due to the NUMA implementation in Windows.
 */
int get_cpu_data_from_numa_nodes(NVM_UINT16 *p_node_ids, struct socket *p_sockets,
		NVM_UINT16 count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (p_sockets == NULL)
	{
		COMMON_LOG_ERROR("p_sockets cannot be NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_node_ids == NULL)
	{
		COMMON_LOG_ERROR("p_node_ids cannot be NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		NVM_UINT16 node_lpcount[count];

		rc = get_numa_nodes_logical_processor_count(node_lpcount, p_node_ids, count);
		if ((rc < NVM_SUCCESS) && (rc != NVM_ERR_ARRAYTOOSMALL))
		{
			COMMON_LOG_ERROR("Failed to find the logical processor count for each NUMA node");
		}
		else
		{
			int i_node;
			for (i_node = 0; i_node < count; i_node++)
			{
				int temp_rc = 0;

				// move current process to a logical processor residing on the current NUMA node
				if ((temp_rc = set_affinity_numa_node(p_node_ids[i_node])) != NVM_SUCCESS)
				{
					COMMON_LOG_ERROR_F("Failed to move process to NUMA node_id: %hu",
							p_node_ids[i_node]);
					rc = temp_rc;
					break;
				}

				// CPUID derived
				if ((temp_rc = get_processor_manufacturer(p_sockets[i_node].manufacturer,
						NVM_SOCKET_MANUFACTURER_LEN)) != COMMON_SUCCESS)
				{
					COMMON_LOG_ERROR_F(
							"get_processor_manufacturer failed on node_id(%hu) with rc: %d",
							p_node_ids[i_node], temp_rc);
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
					break;
				}

				// CPUID derived
				processor_info info;
				if ((temp_rc = get_processor_info(&info)) != COMMON_SUCCESS)
				{
					COMMON_LOG_ERROR_F("get_processor_info failed on node_id(%hu) with rc: %d",
							p_node_ids[i_node], temp_rc);
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
					break;
				}
				else
				{
					p_sockets[i_node].type = info.type;
					p_sockets[i_node].model = info.model;
					p_sockets[i_node].family = info.family;
					p_sockets[i_node].stepping = info.stepping;
					p_sockets[i_node].brand = info.brand_index;
				}

				p_sockets[i_node].id = p_node_ids[i_node];
				p_sockets[i_node].logical_processor_count = node_lpcount[i_node];
			}

			// return any error encountered, otherwise return the number of node processed
			KEEP_ERROR(rc, i_node);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to identify if a node_id is valid
 */
int numa_node_id_exists(NVM_UINT16 node_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_BADSOCKET;

	int numa_node_count = find_numa_nodes(NULL, 0);
	if (numa_node_count > NVM_SUCCESS)
	{
		NVM_UINT16 node_ids[numa_node_count];
		numa_node_count = find_numa_nodes(node_ids, numa_node_count);

		if (numa_node_count > NVM_SUCCESS)
		{
			for (int i = 0; i < numa_node_count; i++)
			{
				// if the node is found, then return success
				if (node_id == node_ids[i])
				{
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
 * Get the DWORD-formatted Windows signature for fetching the SMBIOS table
 */
DWORD get_smbios_table_signature()
{
	// Endian-flipped "RSMB"
	static const char *SMBIOS_TABLE_SIGNATURE = "BMSR";
	return string_to_dword(SMBIOS_TABLE_SIGNATURE);
}

size_t allocate_smbios_table_from_raw_data(const struct RawSMBIOSData *p_data,
		NVM_UINT8 **pp_smbios_table)
{
	COMMON_LOG_ENTRY();
	size_t allocated_size = (size_t)p_data->Length;

	*pp_smbios_table = calloc(1, allocated_size);
	if (*pp_smbios_table)
	{
		memmove(*pp_smbios_table, p_data->SMBIOSTableData, allocated_size);
	}
	else
	{
		COMMON_LOG_ERROR_F("Failed to allocate memory for SMBIOS table of size %llu",
				allocated_size);
		allocated_size = 0;
	}

	COMMON_LOG_EXIT();
	return allocated_size;
}

/*
 * Helper function to get a copy of the SMBIOS table dynamically allocated to the passed-in pointer.
 * p_allocated_size returns the size of the allocated data.
 * Caller is assumed to have passed non-NULL inputs.
 */
int get_smbios_table_alloc(NVM_UINT8 **pp_smbios_table, size_t *p_allocated_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	UINT buf_size = GetSystemFirmwareTable(get_smbios_table_signature(), 0, NULL, 0);
	if (buf_size > 0)
	{
		BYTE smbios_table_buf[buf_size];
		UINT size_fetched = GetSystemFirmwareTable(get_smbios_table_signature(), 0,
				smbios_table_buf, buf_size);
		if (size_fetched == 0)
		{
			COMMON_LOG_ERROR_F(
					"Windows reported no SMBIOS table after reporting a size (size = %u)",
					buf_size);
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{
			*p_allocated_size = allocate_smbios_table_from_raw_data(
					(struct RawSMBIOSData *)smbios_table_buf, pp_smbios_table);
			if (*p_allocated_size == 0)
			{
				rc = NVM_ERR_NOMEMORY;
			}
		}
	}
	else
	{
		COMMON_LOG_ERROR("Windows reported no SMBIOS table (size = 0)");
		rc = NVM_ERR_UNKNOWN;
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
