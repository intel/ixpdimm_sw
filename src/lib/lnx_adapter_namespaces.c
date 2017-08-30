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
 * This file implements the Linux driver adapter interface for managing namespaces.
 */

#include "device_adapter.h"
#include "lnx_adapter.h"
#include "namespace_labels.h"
#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>
#include <string/s_str.h>
#include <uid/uid.h>
#include <guid/guid.h>
#include "utility.h"
#include <errno.h>

#define	DEFAULT_BTT_SECTOR_SIZE	4096
#define	DEFAULT_PFN_NS_ALIGNMENT	0x00200000 // == 2MB recommended data offset alignment

void get_namespace_guid(struct ndctl_namespace *p_namespace, COMMON_UID guid);

/*
 * Get the number of existing namespaces
 */
int get_namespace_count()
{
	int rc = 0; // returns the namespace count

	int num_namespaces = 0;
	struct ndctl_ctx *ctx;

	if ((rc = ndctl_new(&ctx)) >= 0)
	{
		struct ndctl_bus *bus;
		ndctl_bus_foreach(ctx, bus)
		{
			struct ndctl_region *region;
			ndctl_region_foreach(bus, region)
			{
				int nstype = ndctl_region_get_nstype(region);
				if (ndctl_region_is_enabled(region) &&
					(nstype == ND_DEVICE_NAMESPACE_PMEM || nstype == ND_DEVICE_NAMESPACE_BLK))
				{
					struct ndctl_namespace *namespace;
					ndctl_namespace_foreach(region, namespace)
					{
						if (ndctl_namespace_is_configured(namespace))
						{
							num_namespaces++;
						}
					}
				}
			}
		}
		rc = num_namespaces;
		ndctl_unref(ctx);
	}
	else
	{
		rc = linux_err_to_nvm_lib_err(rc);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the discovery information for a given number of namespaces
 */
int get_namespaces(const NVM_UINT32 count,
		struct nvm_namespace_discovery *p_namespaces)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	int namespace_index = 0;
	struct ndctl_ctx *ctx;

	if (p_namespaces == NULL)
	{
		COMMON_LOG_ERROR("p_namespaces is NULL");
		rc = NVM_ERR_UNKNOWN;
	}
	else if ((rc = ndctl_new(&ctx)) >= 0)
	{
		rc = NVM_SUCCESS;
		memset(p_namespaces, 0, sizeof (struct nvm_namespace_discovery) * count);

		struct ndctl_bus *bus;
		ndctl_bus_foreach(ctx, bus)
		{
			struct ndctl_region *region;
			ndctl_region_foreach(bus, region)
			{
				int nstype = ndctl_region_get_nstype(region);
				if (ndctl_region_is_enabled(region) &&
					(nstype == ND_DEVICE_NAMESPACE_PMEM || nstype == ND_DEVICE_NAMESPACE_BLK))
				{
					struct ndctl_namespace *p_namespace;
					ndctl_namespace_foreach(region, p_namespace)
					{
						if (ndctl_namespace_is_configured(p_namespace))
						{
							if (namespace_index >= count)
							{
								rc = NVM_ERR_ARRAYTOOSMALL;
								COMMON_LOG_ERROR("Invalid parameter, "
										"count is smaller than number of " NVM_DIMM_NAME "s");
								break;
							}

							get_namespace_guid(p_namespace,
								p_namespaces[namespace_index].namespace_uid);
							s_strcpy(p_namespaces[namespace_index].friendly_name,
								ndctl_namespace_get_alt_name(p_namespace), NVM_NAMESPACE_NAME_LEN);

							namespace_index++;
						}
					}
				}
			}
		}
		if (rc == NVM_SUCCESS)
		{
			rc = namespace_index;
		}
		ndctl_unref(ctx);
	}
	else
	{
		rc = linux_err_to_nvm_lib_err(rc);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_namespace_lba_size(struct ndctl_namespace *namespace, NVM_UINT32 *lba_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	char ns_queue_lba_size_path[PATH_MAX];

	*lba_size = 0;

	snprintf(ns_queue_lba_size_path, PATH_MAX, "/sys/block/%s/queue/logical_block_size",
		ndctl_namespace_get_block_device(namespace));

	int fd = open(ns_queue_lba_size_path, O_RDONLY|O_CLOEXEC);
	int bytes_read;

	if (fd < 0)
	{
		COMMON_LOG_ERROR_F("Failed to open %s", ns_queue_lba_size_path);
		rc = NVM_ERR_DRIVERFAILED;
	}
	else
	{
		char buf[SYSFS_ATTR_SIZE];
		bytes_read = read(fd, buf, SYSFS_ATTR_SIZE);
		close(fd);
		if (bytes_read < 0 || bytes_read >= SYSFS_ATTR_SIZE)
		{
			COMMON_LOG_ERROR_F("Failed to read from %s", ns_queue_lba_size_path);
			rc = NVM_ERR_DRIVERFAILED;
		}
		else
		{
			buf[bytes_read] = 0;
			if (bytes_read && buf[bytes_read-1] == '\n')
			{
				buf[bytes_read-1] = 0;
			}

			*lba_size = strtoul(buf, NULL, 0);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int enable_namespace(struct ndctl_namespace *p_namespace)
{
	int rc = NVM_SUCCESS;

	struct ndctl_btt *p_btt = ndctl_namespace_get_btt(p_namespace);
	struct ndctl_pfn *p_pfn = ndctl_namespace_get_pfn(p_namespace);
	if ((!p_btt) && (!p_pfn))
	{
		// if not already enabled
		if (!ndctl_namespace_is_enabled(p_namespace))
		{
			int tmp_rc = ndctl_namespace_enable(p_namespace);
			if (tmp_rc < 0)
			{
				rc = linux_err_to_nvm_lib_err(tmp_rc);
				COMMON_LOG_ERROR("Failed to enable the namespace");
			}
		}
	}
	else
	{
		if (p_btt)
		{
			if (!ndctl_btt_is_enabled(p_btt))
			{
				int tmp_rc = ndctl_btt_enable(p_btt);
				if (tmp_rc < 0)
				{
					rc = linux_err_to_nvm_lib_err(tmp_rc);
					COMMON_LOG_ERROR("Failed to enable the BTT namespace");
				}
			}
		}

		if (p_pfn)
		{
			if (!ndctl_pfn_is_enabled(p_pfn))
			{
				int tmp_rc = ndctl_pfn_enable(p_pfn);
				if (tmp_rc < 0)
				{
					rc = linux_err_to_nvm_lib_err(tmp_rc);
					COMMON_LOG_ERROR("Failed to enable the PFN namespace");
				}
			}
		}
	}

	return rc;
}

int disable_namespace(struct ndctl_namespace *p_namespace)
{
	int rc = NVM_SUCCESS;

	struct ndctl_btt *p_btt = ndctl_namespace_get_btt(p_namespace);
	struct ndctl_pfn *p_pfn = ndctl_namespace_get_pfn(p_namespace);
	if ((!p_btt) && (!p_pfn))
	{
		// if not already disabled
		if (ndctl_namespace_is_enabled(p_namespace))
		{
			int tmp_rc = ndctl_namespace_disable(p_namespace);
			if (tmp_rc < 0)
			{
				rc = linux_err_to_nvm_lib_err(tmp_rc);
				COMMON_LOG_ERROR("Failed to disable the namespace");
			}
		}
	}
	else
	{
		if (p_btt)
		{
			int tmp_rc = ndctl_btt_delete(p_btt);
			if (tmp_rc < 0)
			{
				rc = linux_err_to_nvm_lib_err(tmp_rc);
				COMMON_LOG_ERROR("Failed to disable the btt namespace");
			}
		}

		if (p_pfn)
		{
			int tmp_rc = ndctl_pfn_delete(p_pfn);
			if (tmp_rc < 0)
			{
				rc = linux_err_to_nvm_lib_err(tmp_rc);
				COMMON_LOG_ERROR("Failed to disable the pfn namespace");
			}
		}
	}
	return rc;
}

struct ndctl_namespace *get_ndctl_namespace_from_guid(struct ndctl_ctx *p_ctx,
		const NVM_UID namespace_guid)
{
	struct ndctl_bus *p_bus;
	ndctl_bus_foreach(p_ctx, p_bus)
	{
		struct ndctl_region *p_region;
		ndctl_region_foreach(p_bus, p_region)
		{
			int nstype = ndctl_region_get_nstype(p_region);
			if (ndctl_region_is_enabled(p_region) &&
				(nstype == ND_DEVICE_NAMESPACE_PMEM || nstype == ND_DEVICE_NAMESPACE_BLK))
			{
				struct ndctl_namespace *p_namespace = NULL;
				ndctl_namespace_foreach(p_region, p_namespace)
				{
					NVM_UID index_guid;
					get_namespace_guid(p_namespace, index_guid);
					if (ndctl_namespace_is_configured(p_namespace) &&
						uid_cmp(namespace_guid, index_guid))
					{
						return p_namespace;
					}
				}
			}
		}
	}
	return NULL;
}

/*
 * Helper function to populate the enabled field
 */
int get_enable_namespace(struct ndctl_namespace *p_namespace)
{
	int enabled = 0;

	struct ndctl_btt *p_btt = ndctl_namespace_get_btt(p_namespace);
	struct ndctl_pfn *p_pfn = ndctl_namespace_get_pfn(p_namespace);

	if (p_pfn)
	{
		enabled = ndctl_pfn_is_enabled(p_pfn);
	}
	else if (p_btt)
	{
		enabled = ndctl_btt_is_enabled(p_btt);
	}
	else
	{
		enabled = ndctl_namespace_is_enabled(p_namespace);
	}

	return enabled;
}

/*
 * Helper function to get the memory page allocation information
 */
enum namespace_memory_page_allocation get_mem_page_allocation(struct ndctl_namespace *p_namespace)
{
	struct ndctl_pfn *p_pfn = ndctl_namespace_get_pfn(p_namespace);

	enum namespace_memory_page_allocation memory_page_allocation =
		NAMESPACE_MEMORY_PAGE_ALLOCATION_UNKNOWN;

	if (p_pfn)
	{
		enum ndctl_pfn_loc loc = ndctl_pfn_get_location(p_pfn);

		if (loc == NDCTL_PFN_LOC_PMEM)
		{
			memory_page_allocation =
					NAMESPACE_MEMORY_PAGE_ALLOCATION_APP_DIRECT;
		}
		else if (loc == NDCTL_PFN_LOC_RAM)
		{
			memory_page_allocation =
					NAMESPACE_MEMORY_PAGE_ALLOCATION_DRAM;
		}
		else
		{
			memory_page_allocation =
					NAMESPACE_MEMORY_PAGE_ALLOCATION_NONE;
		}

	}
	else
	{
		memory_page_allocation =
				NAMESPACE_MEMORY_PAGE_ALLOCATION_NONE;
	}

	return memory_page_allocation;
}

/*
 * Get the details for a specific namespace
 */
int get_namespace_details(
		const NVM_UID namespace_guid,
		struct nvm_namespace_details *p_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct ndctl_ctx *ctx;
	if (namespace_guid == NULL)
	{
			COMMON_LOG_ERROR("namespace guid cannot be NULL.");
			rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_details == NULL)
	{
		COMMON_LOG_ERROR("nvm_namespace_details is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = ndctl_new(&ctx)) >= 0)
	{
		memset(p_details, 0, sizeof (struct nvm_namespace_details));
		struct ndctl_namespace *p_namespace =
				get_ndctl_namespace_from_guid(ctx, namespace_guid);
		if (!p_namespace)
		{
			COMMON_LOG_ERROR("Specified namespace not found");
			rc = NVM_ERR_BADNAMESPACE;
		}
		else
		{
			struct ndctl_region *p_region =
					ndctl_namespace_get_region(p_namespace);
			switch (ndctl_namespace_get_type(p_namespace))
			{
				case ND_DEVICE_NAMESPACE_PMEM:
					p_details->type = NAMESPACE_TYPE_APP_DIRECT;
					p_details->namespace_creation_id.interleave_setid =
							ndctl_region_get_range_index(p_region);
					p_details->block_size = DEFAULT_BTT_SECTOR_SIZE;
					break;

				default:
					p_details->type = NAMESPACE_TYPE_UNKNOWN;
					break;
			}

			get_namespace_guid(p_namespace,
				p_details->discovery.namespace_uid);

			s_strcpy(p_details->discovery.friendly_name,
				ndctl_namespace_get_alt_name(p_namespace),
				NVM_NAMESPACE_NAME_LEN);

			p_details->enabled = get_enable_namespace(p_namespace) ?
					NAMESPACE_ENABLE_STATE_ENABLED :
					NAMESPACE_ENABLE_STATE_DISABLED;

			if (ndctl_namespace_get_btt(p_namespace))
			{
				p_details->btt = 1;
			}

			p_details->memory_page_allocation = get_mem_page_allocation(p_namespace);
			p_details->health = NAMESPACE_HEALTH_NORMAL;

			p_details->block_count =
				calculateBlockCount((ndctl_namespace_get_size(p_namespace)), p_details->block_size);
		}
		ndctl_unref(ctx);
	}
	else
	{
		rc = linux_err_to_nvm_lib_err(rc);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_idle_btt(struct ndctl_region *region, struct ndctl_btt **idle_btt)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_DRIVERFAILED;
	struct ndctl_btt *btt;

	*idle_btt = NULL;

	ndctl_btt_foreach(region, btt)
	{
		if (!ndctl_btt_is_enabled(btt) && !ndctl_btt_is_configured(btt))
		{
			*idle_btt = btt;
			rc = NVM_SUCCESS;
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_idle_pfn(struct ndctl_region *region, struct ndctl_pfn **idle_pfn)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_DRIVERFAILED;

	struct ndctl_pfn *pfn;
	*idle_pfn = NULL;

	ndctl_pfn_foreach(region, pfn)
	{
		if (!ndctl_pfn_is_enabled(pfn) && !ndctl_pfn_is_configured(pfn))
		{
			*idle_pfn = pfn;
			rc = NVM_SUCCESS;
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_unconfigured_namespace(struct ndctl_namespace **unconfigured_namespace,
	struct ndctl_region *region)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_DRIVERFAILED;

	*unconfigured_namespace = NULL;

	struct ndctl_namespace *namespace;
	ndctl_namespace_foreach(region, namespace)
	{
		if (!ndctl_namespace_is_enabled(namespace) && !ndctl_namespace_is_configured(namespace))
		{
			*unconfigured_namespace = namespace;
			rc = NVM_SUCCESS;
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_ndctl_app_direct_region_by_range_index(struct ndctl_ctx *ctx,
		struct ndctl_region **target_region, unsigned int spa_index)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_DRIVERFAILED;

	*target_region = NULL;

	struct ndctl_bus *bus;
	ndctl_bus_foreach(ctx, bus)
	{
		struct ndctl_region *region;
		ndctl_region_foreach(bus, region)
		{
			int nstype = ndctl_region_get_nstype(region);
			if (ndctl_region_is_enabled(region) &&
				(nstype == ND_DEVICE_NAMESPACE_PMEM))
			{
				if (spa_index == ndctl_region_get_range_index(region))
				{
					*target_region = region;
					rc = NVM_SUCCESS;
					break;
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_ndctl_storage_region_by_handle(struct ndctl_ctx *ctx,
		struct ndctl_region **target_region, unsigned int handle)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_DRIVERFAILED;

	*target_region = NULL;

	struct ndctl_bus *bus;
	ndctl_bus_foreach(ctx, bus)
	{
		struct ndctl_region *region;
		ndctl_region_foreach(bus, region)
		{
			int nstype = ndctl_region_get_nstype(region);
			if (ndctl_region_is_enabled(region) &&
				(nstype == ND_DEVICE_NAMESPACE_BLK))
			{
				struct ndctl_dimm *dimm = ndctl_region_get_first_dimm(region);
				if (dimm)
				{
					if (handle == ndctl_dimm_get_handle(dimm))
					{
						*target_region = region;
						rc = NVM_SUCCESS;
						break;
					}
				}
				else
				{
					rc = NVM_ERR_DRIVERFAILED;
					break;
				}

			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int create_pfn_namespace(struct ndctl_namespace *namespace,
		const struct nvm_namespace_create_settings *p_settings)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (ndctl_namespace_get_type(namespace) == ND_DEVICE_NAMESPACE_PMEM)
	{
		if ((p_settings->memory_page_allocation == NAMESPACE_MEMORY_PAGE_ALLOCATION_APP_DIRECT) ||
				(p_settings->memory_page_allocation == NAMESPACE_MEMORY_PAGE_ALLOCATION_DRAM))
		{
			struct ndctl_region *region = ndctl_namespace_get_region(namespace);
			struct ndctl_pfn *pfn;
			if (region != NULL && ((rc = get_idle_pfn(region, &pfn)) == NVM_SUCCESS))
			{
				COMMON_GUID pfn_guid;
				generate_guid(pfn_guid);

				// For AppDirect namespaces, the default is
				// NAMESPACE_MEMORY_PAGE_ALLOCATION_APPDIRECT
				enum ndctl_pfn_loc loc = NDCTL_PFN_LOC_PMEM;
				unsigned long align = DEFAULT_PFN_NS_ALIGNMENT;

				if (p_settings->memory_page_allocation ==
						NAMESPACE_MEMORY_PAGE_ALLOCATION_DRAM)
				{ // enable PFN and page structures in DRAM
					loc = NDCTL_PFN_LOC_RAM;
				}
				else if (p_settings->memory_page_allocation ==
						NAMESPACE_MEMORY_PAGE_ALLOCATION_APP_DIRECT)
				{ // enable PFN and page structures in PMEM
					loc = NDCTL_PFN_LOC_PMEM;
				}
				else if (p_settings->memory_page_allocation ==
						NAMESPACE_MEMORY_PAGE_ALLOCATION_NONE)
				{
					loc = NDCTL_PFN_LOC_NONE;
				}
				if (ndctl_pfn_set_uuid(pfn, pfn_guid))
				{
					COMMON_LOG_ERROR("Set pfn UUID failed");
					rc = NVM_ERR_DRIVERFAILED;
				}
				else if (ndctl_pfn_set_location(pfn, loc))
				{
					COMMON_LOG_ERROR("Set pfn location failed");
					rc = NVM_ERR_DRIVERFAILED;
				}
				else if (ndctl_pfn_set_align(pfn, align))
				{
					COMMON_LOG_ERROR("Set pfn align failed");
					rc = NVM_ERR_DRIVERFAILED;
				}
				else if (ndctl_pfn_set_namespace(pfn, namespace))
				{
					COMMON_LOG_ERROR("Set pfn backing namespace failed");
					rc = NVM_ERR_DRIVERFAILED;
				}
				else if (p_settings->enabled == NAMESPACE_ENABLE_STATE_ENABLED &&
						ndctl_pfn_enable(pfn) < 0)
				{ // if pfn fails to enable it remains in a disabled state, delete it
					COMMON_LOG_ERROR("Enable pfn failed");
					ndctl_pfn_delete(pfn);
					rc = NVM_ERR_DRIVERFAILED;
				}
			}
		}
	}
	else
	{
		COMMON_LOG_ERROR("blk namespace does not support memory mode.");
		rc = NVM_ERR_NOTSUPPORTED;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int create_btt_namespace(struct ndctl_namespace *namespace,
		const struct nvm_namespace_create_settings *p_settings)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct ndctl_region *region = ndctl_namespace_get_region(namespace);
	struct ndctl_btt *btt;
	if (region != NULL && ((rc = get_idle_btt(region, &btt)) == NVM_SUCCESS))
	{
		COMMON_GUID btt_guid;
		generate_guid(btt_guid);

		// always have to set a sector size for the btt backing
		// namespace. 1 is not valid for this, so use default of 4kB
		// for app direct namespaces
		unsigned int sector_size = DEFAULT_BTT_SECTOR_SIZE;

		if (ndctl_btt_set_uuid(btt, btt_guid))
		{
			COMMON_LOG_ERROR("Set btt UUID failed");
			rc = NVM_ERR_DRIVERFAILED;
		}
		else if (ndctl_btt_set_sector_size(btt, sector_size))
		{
			COMMON_LOG_ERROR("Set btt sector size failed");
			rc = NVM_ERR_DRIVERFAILED;
		}
		else if (ndctl_btt_set_namespace(btt, namespace))
		{
			COMMON_LOG_ERROR("Set btt backing namespace failed");
			rc = NVM_ERR_DRIVERFAILED;
		}
		else if (p_settings->enabled == NAMESPACE_ENABLE_STATE_ENABLED &&
				ndctl_btt_enable(btt) < 0)
		{
			COMMON_LOG_ERROR("Enable btt failed");
			// If a btt fails to enable it remains in a disabled state, so delete it
			ndctl_btt_delete(btt);
			rc = NVM_ERR_DRIVERFAILED;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Create a new namespace
 */
int create_namespace(
		NVM_UID *p_namespace_guid,
		const struct nvm_namespace_create_settings *p_settings)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct ndctl_ctx *ctx;

	if (p_settings == NULL)
	{
		COMMON_LOG_ERROR("namespace create settings structure cannot be NULL.");
		rc = NVM_ERR_UNKNOWN;
	}
	else if (p_namespace_guid == NULL)
	{
		COMMON_LOG_ERROR("namespace GUID pointer cannot be NULL.");
		rc = NVM_ERR_UNKNOWN;
	}
	else if ((rc = ndctl_new(&ctx)) >= 0)
	{
		struct ndctl_region *region;
		if (p_settings->type == NAMESPACE_TYPE_APP_DIRECT)
		{
			rc = get_ndctl_app_direct_region_by_range_index(ctx, &region,
				p_settings->namespace_creation_id.interleave_setid);
		}
		else
		{
			COMMON_LOG_ERROR("Cannot create unknown namespace type");
			rc = NVM_ERR_UNKNOWN;
		}

		struct ndctl_namespace *namespace;
		if (rc == NVM_SUCCESS &&
				((rc = get_unconfigured_namespace(&namespace, region)) == NVM_SUCCESS))
		{
			COMMON_GUID namespace_guid;
			generate_guid(namespace_guid);
			guid_to_uid(namespace_guid, *p_namespace_guid);

			if (ndctl_namespace_set_uuid(namespace, namespace_guid))
			{
				COMMON_LOG_ERROR("Set UUID Failed");
				rc = NVM_ERR_DRIVERFAILED;
			}
			else if (ndctl_namespace_set_alt_name(namespace, p_settings->friendly_name))
			{
				COMMON_LOG_ERROR("Set Alt Name Failed");
				rc = NVM_ERR_DRIVERFAILED;
			}
			else if (ndctl_namespace_set_size(namespace,
				adjust_namespace_size(p_settings->block_size, p_settings->block_count)))
			{
				COMMON_LOG_ERROR("Set SizeFailed");
				rc = NVM_ERR_DRIVERFAILED;
			}

			if (rc == NVM_SUCCESS && ndctl_namespace_is_configured(namespace))
			{
				if (p_settings->btt)
				{
					if ((rc = create_btt_namespace(namespace, p_settings)) != NVM_SUCCESS)
					{
						COMMON_LOG_ERROR("Create BTT Failed");
						ndctl_namespace_delete(namespace);
					}
				}
				if (p_settings->memory_page_allocation !=
						NAMESPACE_MEMORY_PAGE_ALLOCATION_NONE)
				{
					if ((rc = create_pfn_namespace(namespace, p_settings)) != NVM_SUCCESS)
					{
						COMMON_LOG_ERROR("Create PFN Failed");
						ndctl_namespace_delete(namespace);
					}
				}
				// enable the namespace if desired (only non-btt or non-pfn namespaces)
				else if (p_settings->enabled == NAMESPACE_ENABLE_STATE_ENABLED)
				{
					rc = enable_namespace(namespace);
				}
			}
			else
			{
				COMMON_LOG_ERROR("Failed to configure the namespace");
			}
		}
		ndctl_unref(ctx);
	}
	else
	{
		rc = linux_err_to_nvm_lib_err(rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Fetches the device ID for the namespace in /dev/.
 * Returns NULL if namespace is disabled.
 */
const char *get_enabled_namespace_dev_id(struct ndctl_namespace *p_namespace)
{
	COMMON_LOG_ENTRY();

	const char *dev_id = NULL;
	struct ndctl_pfn *pfn = ndctl_namespace_get_pfn(p_namespace);
	struct ndctl_btt *btt = ndctl_namespace_get_btt(p_namespace);

	if (pfn && ndctl_pfn_is_enabled(pfn))
	{
		dev_id = ndctl_pfn_get_block_device(pfn);
	}
	else if (btt && ndctl_btt_is_enabled(btt))
	{
		dev_id = ndctl_btt_get_block_device(btt);
	}
	else if (ndctl_namespace_is_enabled(p_namespace))
	{
		dev_id = ndctl_namespace_get_block_device(p_namespace);
	}

	COMMON_LOG_EXIT();
	return dev_id;
}

NVM_BOOL is_namespace_enabled(struct ndctl_namespace *p_namespace)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL enabled = 0;

	struct ndctl_pfn *pfn = ndctl_namespace_get_pfn(p_namespace);
	struct ndctl_btt *btt = ndctl_namespace_get_btt(p_namespace);

	enabled = (pfn && ndctl_pfn_is_enabled(pfn)) ||
			(btt && ndctl_btt_is_enabled(btt)) ||
			ndctl_namespace_is_enabled(p_namespace);

	COMMON_LOG_EXIT_RETURN_I(enabled);
	return enabled;
}

/*
 * Returns file descriptor, or -1 if couldn't open/lock namespace device
 */
int get_exclusive_namespace_fd(struct ndctl_namespace *p_namespace)
{
	COMMON_LOG_ENTRY();
	int fd = -1;

	// Disabled namespace has no dev ID
	const char *dev_id = get_enabled_namespace_dev_id(p_namespace);
	if (dev_id)
	{
		NVM_PATH ns_path;
		s_snprintf(ns_path, NVM_PATH_LEN, "/dev/%s", dev_id);

		// Linux keeps others from mounting with exclusive open
		fd = open(ns_path, O_RDWR | O_EXCL);
		if (fd < 0) // couldn't exclusively open
		{
			COMMON_LOG_INFO_F("Device %s is mounted", ns_path);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(fd);
	return fd;
}

NVM_BOOL file_descriptor_valid(const int fd)
{
	return (fd >= 0);
}

void release_namespace_fd(int fd)
{
	COMMON_LOG_ENTRY();

	if (file_descriptor_valid(fd))
	{
		if (close(fd) < 0)
		{
			COMMON_LOG_WARN_F("Couldn't close fd %d, errno=%d", fd, errno);
		}
	}

	COMMON_LOG_EXIT();
}

int check_namespace_filesystem_mounted(const NVM_BOOL namespace_enabled, const int namespace_fd)
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;
	if (namespace_enabled && !file_descriptor_valid(namespace_fd))
	{
		rc = NVM_ERR_NAMESPACEBUSY;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Delete an existing namespace
 */
int delete_namespace(const NVM_UID namespace_guid)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct ndctl_ctx *p_ctx;
	if (namespace_guid == NULL)
	{
			COMMON_LOG_ERROR("namespace guid cannot be NULL.");
			rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = ndctl_new(&p_ctx)) >= 0)
	{
		struct ndctl_namespace *p_namespace =
				get_ndctl_namespace_from_guid(p_ctx, namespace_guid);
		if (!p_namespace)
		{
			COMMON_LOG_ERROR("Specified namespace not found");
			rc = NVM_ERR_BADNAMESPACE;
		}
		else
		{
			int fd = get_exclusive_namespace_fd(p_namespace);
			if ((rc = check_namespace_filesystem_mounted(
					is_namespace_enabled(p_namespace), fd)) != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR_F("Can't delete namespace %s, filesystem is mounted",
						namespace_guid);
			}
			else
			{
				// disable it
				if ((rc = disable_namespace(p_namespace)) == NVM_SUCCESS)
				{
					// delete it
					rc = linux_err_to_nvm_lib_err(ndctl_namespace_delete(p_namespace));
				}
			}

			release_namespace_fd(fd);
		}

		ndctl_unref(p_ctx);
	}
	else
	{
		rc = linux_err_to_nvm_lib_err(rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Modify an existing namespace name
 */
int modify_namespace_name(
		const NVM_UID namespace_guid,
		const NVM_NAMESPACE_NAME name)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct ndctl_ctx *p_ctx;
	if (namespace_guid == NULL)
	{
			COMMON_LOG_ERROR("namespace guid cannot be NULL.");
			rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = ndctl_new(&p_ctx)) >= 0)
	{
		struct ndctl_namespace *p_namespace =
				get_ndctl_namespace_from_guid(p_ctx, namespace_guid);
		if (!p_namespace)
		{
			COMMON_LOG_ERROR("Specified namespace not found");
			rc = NVM_ERR_BADNAMESPACE;
		}
		else
		{
			NVM_BOOL ns_enabled = is_namespace_enabled(p_namespace);
			int fd = get_exclusive_namespace_fd(p_namespace);
			if ((rc = check_namespace_filesystem_mounted(
					ns_enabled, fd)) != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR_F("Can't modify namespace %s name, filesystem is mounted",
						namespace_guid);
			}
			else
			{
				// disable it
				if (ns_enabled && ((rc = disable_namespace(p_namespace)) != NVM_SUCCESS))
				{
					COMMON_LOG_ERROR("Failed to disable the namespace");
				}
				// change the name
				else if ((rc = linux_err_to_nvm_lib_err(
					ndctl_namespace_set_alt_name(p_namespace, name))) != NVM_SUCCESS)
				{
					COMMON_LOG_ERROR("Failed to set new friendly name");
				}
				// re-enable it
				else if (ns_enabled && ((rc = enable_namespace(p_namespace)) != NVM_SUCCESS))
				{
					COMMON_LOG_ERROR("Failed to re-enable namespace ");
				}
			}

			release_namespace_fd(fd);
		}
		ndctl_unref(p_ctx);
	}
	else
	{
		rc = linux_err_to_nvm_lib_err(rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Modify an existing namespace size
 */
int modify_namespace_block_count(
		const NVM_UID namespace_guid,
		const NVM_UINT64 block_count)
{
	int rc = NVM_ERR_NOTSUPPORTED;
	return rc;
}

/*
 * Modify an existing namespace enable
 */
int modify_namespace_enabled(
		const NVM_UID namespace_guid,
		const enum namespace_enable_state enabled)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct ndctl_ctx *p_ctx;
	if (namespace_guid == NULL)
	{
			COMMON_LOG_ERROR("namespace guid cannot be NULL.");
			rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = ndctl_new(&p_ctx)) >= 0)
	{
		struct ndctl_namespace *p_namespace =
				get_ndctl_namespace_from_guid(p_ctx, namespace_guid);
		if (!p_namespace)
		{
			COMMON_LOG_ERROR("Specified namespace not found");
			rc = NVM_ERR_BADNAMESPACE;
		}
		else
		{
			int fd = get_exclusive_namespace_fd(p_namespace);
			if ((rc = check_namespace_filesystem_mounted(
					is_namespace_enabled(p_namespace), fd)) != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR_F("Can't modify namespace %s state, filesystem is mounted",
						namespace_guid);
			}
			else if (enabled == NAMESPACE_ENABLE_STATE_DISABLED)
			{
				rc = disable_namespace(p_namespace);
			}
			else if (enabled == NAMESPACE_ENABLE_STATE_ENABLED)
			{
				rc = enable_namespace(p_namespace);
			}

			release_namespace_fd(fd);
		}
		ndctl_unref(p_ctx);
	}
	else
	{
		rc = linux_err_to_nvm_lib_err(rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


void get_namespace_guid(struct ndctl_namespace *p_namespace, COMMON_UID guid)
{
	COMMON_GUID tmp;
	ndctl_namespace_get_uuid(p_namespace, tmp);
	guid_to_uid(tmp, guid);
}
