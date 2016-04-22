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
 * This file contains the implementation of the library smart caching interface.
 */

#include "nvm_context.h"
#include <os/os_adapter.h>
#include <persistence/logging.h>
#include <uid/uid.h>

#ifdef __WINDOWS__
#include <Windows.h>

extern HANDLE g_context_lock;
#else
extern pthread_mutex_t g_context_lock;
#endif

// Current context pointer - one per process
struct nvm_context *p_context = NULL;

/*
 * Initialize the context. This is a lazy context meaning
 * details are added as they are requested rather than up
 * front. This call should be pair with free_nvm_context.
 */
int nvm_create_context()
{
	COMMON_LOG_ENTRY();
	// clean up existing
	int rc = nvm_free_context();
	if (rc == NVM_SUCCESS)
	{
		// lock
		if (!mutex_lock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not obtain the context lock");
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{
			// create new pointer
			p_context = calloc(1, sizeof (struct nvm_context));
			if (!p_context)
			{
				COMMON_LOG_ERROR("Failed to allocate memory for the context.");
				rc = NVM_ERR_NOMEMORY;
			}
			else
			{
				// initialize everything
				p_context->p_capabilities = NULL;
				p_context->device_count = -1;
				p_context->p_devices = NULL;
				p_context->pool_count = -1;
				p_context->p_pools = NULL;
				p_context->namespace_count = -1;
				p_context->p_namespaces = NULL;
			}

			// unlock
			if (!mutex_unlock(&g_context_lock))
			{
				COMMON_LOG_ERROR("Could not release the context lock.");
				rc = NVM_ERR_UNKNOWN;
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to free the entire device list
 * NOTE: This function assumes the caller has obtained the lock
 */
void free_device_list()
{
	COMMON_LOG_ENTRY();
	// clean up the device list
	if (p_context && p_context->device_count > 0 && p_context->p_devices)
	{
		for (int i = 0; i < p_context->device_count; i++)
		{
			if (p_context->p_devices[i].p_device_discovery)
			{
				free(p_context->p_devices[i].p_device_discovery);
				p_context->p_devices[i].p_device_discovery = NULL;
			}
			if (p_context->p_devices[i].p_device_details)
			{
				free(p_context->p_devices[i].p_device_details);
				p_context->p_devices[i].p_device_details = NULL;
			}
			if (p_context->p_devices[i].p_pcd)
			{
				free(p_context->p_devices[i].p_pcd);
				p_context->p_devices[i].p_pcd = NULL;
				p_context->p_devices[i].pcd_size = -1;
			}
		}
		free(p_context->p_devices);
		p_context->p_devices = NULL;
		p_context->device_count = -1;
	}
	COMMON_LOG_EXIT();
}

/*
 * Helper function to free the entire pool list
 * NOTE: This function assumes the caller has obtained the lock
 */
void free_pool_list()
{
	COMMON_LOG_ENTRY();
	// clean up the device list
	if (p_context && p_context->pool_count > 0 && p_context->p_pools)
	{
		free(p_context->p_pools);
		p_context->p_pools = NULL;
		p_context->pool_count = -1;
	}
	COMMON_LOG_EXIT();
}

/*
 * Helper function to free the entire namespace list
 * NOTE: This function assumes the caller has obtained the lock
 */
void free_namespace_list()
{
	COMMON_LOG_ENTRY();
	// clean up the namespace list
	if (p_context && p_context->namespace_count > 0 && p_context->p_namespaces)
	{
		for (int i = 0; i < p_context->namespace_count; i++)
		{
			if (p_context->p_namespaces[i].p_namespace_discovery)
			{
				free(p_context->p_namespaces[i].p_namespace_discovery);
				p_context->p_namespaces[i].p_namespace_discovery = NULL;
			}
			if (p_context->p_namespaces[i].p_namespace_details)
			{
				free(p_context->p_namespaces[i].p_namespace_details);
				p_context->p_namespaces[i].p_namespace_details = NULL;
			}
		}
		free(p_context->p_namespaces);
		p_context->p_namespaces = NULL;
		p_context->namespace_count = -1;
	}
	COMMON_LOG_EXIT();
}
/*
 * Clean up the resources allocated by nvm_create_context
 */
int nvm_free_context()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context)
		{
			// clean up capabilities
			if (p_context->p_capabilities)
			{
				free(p_context->p_capabilities);
				p_context->p_capabilities = NULL;
			}

			free_device_list();
			free_pool_list();
			free_namespace_list();

			// clean up pointer
			free(p_context);
			p_context = NULL;
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_nvm_context_capabilities(struct nvm_capabilities *p_capabilities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->p_capabilities)
		{
			memmove(p_capabilities, p_context->p_capabilities, sizeof (struct nvm_capabilities));
			rc = NVM_SUCCESS;
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int set_nvm_context_capabilities(const struct nvm_capabilities *p_capabilities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context)
		{
			// clean up existing
			if (p_context->p_capabilities)
			{
				free(p_context->p_capabilities);
				p_context->p_capabilities = NULL;
			}
			// create new
			p_context->p_capabilities = calloc(1, sizeof (struct nvm_capabilities));
			if (p_context->p_capabilities)
			{
				memmove(p_context->p_capabilities, p_capabilities,
						sizeof (struct nvm_capabilities));
				rc = NVM_SUCCESS;
			}
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

// devices
int get_nvm_context_device_count()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->device_count >= 0)
		{
			rc = p_context->device_count;
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_nvm_context_devices(struct device_discovery *p_devices, const int dev_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->device_count > 0 && p_context->p_devices)
		{
			int copy_count = dev_count;
			rc = copy_count;
			if (dev_count > p_context->device_count)
			{
				copy_count = p_context->device_count;
				rc = copy_count;
			}
			else if (dev_count < p_context->device_count)
			{
				rc = NVM_ERR_ARRAYTOOSMALL;
			}
			memset(p_devices, 0, copy_count * sizeof (struct device_discovery));

			for (int i = 0; i < copy_count; i++)
			{
				if (p_context->p_devices[i].p_device_discovery)
				{
					memmove(&p_devices[i], p_context->p_devices[i].p_device_discovery,
							sizeof (struct device_discovery));
				}
				else
				{
					// pointers are invalid, clear the device cache and quit
					free_device_list();
					rc = NVM_ERR_UNKNOWN;
					break;
				}
			}
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int set_nvm_context_devices(const struct device_discovery *p_devices, const int dev_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context)
		{
			// clean up existing
			free_device_list();

			// create new list
			p_context->p_devices = calloc(dev_count, sizeof (struct nvm_device_context));
			if (!p_context->p_devices)
			{
				COMMON_LOG_ERROR("Failed to allocate memory for device context structure");
				rc = NVM_ERR_NOMEMORY;
			}
			else
			{
				p_context->device_count = dev_count;
				rc = NVM_SUCCESS;
				for (int i = 0; i < dev_count; i++)
				{
					p_context->p_devices[i].p_device_details = NULL;
					p_context->p_devices[i].p_pcd = NULL;
					p_context->p_devices[i].pcd_size = -1;
					p_context->p_devices[i].p_device_discovery =
							calloc(1, sizeof (struct device_discovery));
					if (!p_context->p_devices[i].p_device_discovery)
					{
						COMMON_LOG_ERROR("Failed to allocate memory for discovery structure");
						rc = NVM_ERR_NOMEMORY;
						break;
					}
					else
					{
						memmove(p_context->p_devices[i].uid,
								p_devices[i].uid, sizeof (NVM_UID));
						memmove(p_context->p_devices[i].p_device_discovery,
								&p_devices[i], sizeof (struct device_discovery));
					}
				}
			}
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Clear the device list
 */
void invalidate_devices()
{
	COMMON_LOG_ENTRY();
	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
	}
	else
	{
		free_device_list();

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
		}
	}
	COMMON_LOG_EXIT();
}

/*
 * Clear the pcd from a specific device
 */
void invalidate_device_pcd(const NVM_UID device_uid)
{
	COMMON_LOG_ENTRY();
	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
	}
	else
	{
		if (p_context && p_context->device_count > 0 && p_context->p_devices)
		{
			for (int i = 0; i < p_context->device_count; i++)
			{
				if (uid_cmp(device_uid, p_context->p_devices[i].uid))
				{
					// found it
					if (p_context->p_devices[i].p_pcd)
					{
						free(p_context->p_devices[i].p_pcd);
						p_context->p_devices[i].p_pcd = NULL;
						p_context->p_devices[i].pcd_size = -1;
					}
					break;
				}
			}
		}


		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
		}
	}
	COMMON_LOG_EXIT();
}

int get_nvm_context_device_details(const NVM_UID device_uid, struct device_details *p_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->device_count > 0 && p_context->p_devices)
		{
			for (int i = 0; i < p_context->device_count; i++)
			{
				if (uid_cmp(device_uid, p_context->p_devices[i].uid))
				{
					// found it
					if (p_context->p_devices[i].p_device_details)
					{
						memset(p_details, 0, sizeof (struct device_details));
						memmove(p_details, p_context->p_devices[i].p_device_details,
								sizeof (struct device_details));
						rc = NVM_SUCCESS;
					}
					break;
				}
			}
		}


		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int set_nvm_context_device_details(const NVM_UID device_uid,
		const struct device_details *p_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->device_count > 0 && p_context->p_devices)
		{
			for (int i = 0; i < p_context->device_count; i++)
			{
				if (uid_cmp(device_uid, p_context->p_devices[i].uid))
				{
					// found it
					// clear any existing details
					if (p_context->p_devices[i].p_device_details)
					{
						free(p_context->p_devices[i].p_device_details);
						p_context->p_devices[i].p_device_details = NULL;
					}
					// allocate new memory
					p_context->p_devices[i].p_device_details =
							calloc(1, sizeof (struct device_details));
					if (!p_context->p_devices[i].p_device_details)
					{
						rc = NVM_ERR_NOMEMORY;
						COMMON_LOG_ERROR("Failed to allocate memory for device details structure");
					}
					else
					{
						// success, do the copy
						memmove(p_context->p_devices[i].p_device_details, p_details,
								sizeof (struct device_details));
						rc = NVM_SUCCESS;
					}
					break;
				}
			}
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_nvm_context_device_pcd(const NVM_UID device_uid,
		struct platform_config_data **pp_pcd, NVM_SIZE *p_pcd_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->device_count > 0 && p_context->p_devices)
		{
			for (int i = 0; i < p_context->device_count; i++)
			{
				if (uid_cmp(device_uid, p_context->p_devices[i].uid))
				{
					// found it
					if (p_context->p_devices[i].pcd_size > 0 && p_context->p_devices[i].p_pcd)
					{
						// allocate memory for return
						*pp_pcd = calloc(1, p_context->p_devices[i].pcd_size);
						if (*pp_pcd == NULL)
						{
							COMMON_LOG_ERROR("Failed to allocate memory for the pcd structure");
							rc = NVM_ERR_NOMEMORY;
						}
						else
						{
							memmove(*pp_pcd, p_context->p_devices[i].p_pcd,
									p_context->p_devices[i].pcd_size);
							*p_pcd_size = p_context->p_devices[i].pcd_size;
							rc = NVM_SUCCESS;
						}
					}
					break;
				}
			}
		}


		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int set_nvm_context_device_pcd(const NVM_UID device_uid,
		const struct platform_config_data *p_pcd, const NVM_SIZE pcd_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->device_count > 0 && p_context->p_devices)
		{
			for (int i = 0; i < p_context->device_count; i++)
			{
				if (uid_cmp(device_uid, p_context->p_devices[i].uid))
				{
					// found it
					// clear any existing pcd
					if (p_context->p_devices[i].pcd_size > 0 && p_context->p_devices[i].p_pcd)
					{
						free(p_context->p_devices[i].p_pcd);
						p_context->p_devices[i].p_pcd = NULL;
						p_context->p_devices[i].pcd_size = -1;
					}
					// allocate new memory
					p_context->p_devices[i].p_pcd = calloc(1, pcd_size);
					if (!p_context->p_devices[i].p_pcd)
					{
						COMMON_LOG_ERROR("Failed to allocate memory for pcd structure");
						rc = NVM_ERR_NOMEMORY;
					}
					else
					{
						// success, do the copy
						p_context->p_devices[i].pcd_size = pcd_size;
						memmove(p_context->p_devices[i].p_pcd, p_pcd, pcd_size);
						rc = NVM_SUCCESS;
					}
					break;
				}
			}
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void invalidate_pools()
{
	COMMON_LOG_ENTRY();
	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
	}
	else
	{
		free_pool_list();

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
		}
	}
	COMMON_LOG_EXIT();
}

int get_nvm_context_pool_count()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->pool_count >= 0)
		{
			rc = p_context->pool_count;
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_nvm_context_pools(struct pool *p_pools, const int pool_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->pool_count > 0 && p_context->p_pools)
		{
			int copy_count = pool_count;
			rc = pool_count;
			if (pool_count > p_context->pool_count)
			{
				copy_count = p_context->pool_count;
				rc = p_context->pool_count;
			}
			else if (pool_count < p_context->pool_count)
			{
				rc = NVM_ERR_ARRAYTOOSMALL;
			}
			memset(p_pools, 0, (copy_count * sizeof (struct pool)));
			memmove(p_pools, p_context->p_pools, (copy_count * sizeof (struct pool)));
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int set_nvm_context_pools(const struct pool *p_pools, const int pool_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context)
		{
			// clean up existing
			free_pool_list();

			// create new list
			p_context->p_pools = calloc(pool_count, sizeof (struct pool));
			if (!p_context->p_pools)
			{
				COMMON_LOG_ERROR("Failed to allocate memory for pool list");
				rc = NVM_ERR_NOMEMORY;
			}
			else
			{
				p_context->pool_count = pool_count;
				memmove(p_context->p_pools, p_pools, pool_count * sizeof (struct pool));
				rc = NVM_SUCCESS;
			}
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;

}

int get_nvm_context_pool(const NVM_UID pool_uid, struct pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->pool_count > 0 && p_context->p_pools)
		{
			for (int i = 0; i < p_context->pool_count; i++)
			{
				if (uid_cmp(pool_uid, p_context->p_pools[i].pool_uid))
				{
					// found it
					memset(p_pool, 0, sizeof (struct pool));
					memmove(p_pool, &p_context->p_pools[i],  sizeof (struct pool));
					rc = NVM_SUCCESS;
					break;
				}
			}
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void invalidate_namespaces()
{
	COMMON_LOG_ENTRY();
	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
	}
	else
	{
		free_namespace_list();

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
		}
	}
	COMMON_LOG_EXIT();
}

int get_nvm_context_namespace_count()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->namespace_count >= 0)
		{
			rc = p_context->namespace_count;
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_nvm_context_namespaces(struct namespace_discovery *p_namespaces,
		const int namespace_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->namespace_count > 0 && p_context->p_namespaces)
		{
			int copy_count = namespace_count;
			rc = namespace_count;
			if (namespace_count > p_context->namespace_count)
			{
				copy_count = p_context->namespace_count;
				rc = p_context->namespace_count;
			}
			else if (namespace_count < p_context->namespace_count)
			{
				rc = NVM_ERR_ARRAYTOOSMALL;
			}

			memset(p_namespaces, 0, (copy_count * sizeof (struct namespace_discovery)));
			for (int i = 0; i < namespace_count; i++)
			{
				memmove(&p_namespaces[i], p_context->p_namespaces[i].p_namespace_discovery,
						(sizeof (struct namespace_discovery)));
			}
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int set_nvm_context_namespaces(const struct namespace_discovery *p_namespaces,
		const int namespace_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context)
		{
			// clean up existing
			free_namespace_list();

			// create new list
			p_context->p_namespaces = calloc(namespace_count,
					sizeof (struct nvm_namespace_context));
			if (!p_context->p_namespaces)
			{
				COMMON_LOG_ERROR("Failed to allocate memory for context structure");
				rc = NVM_ERR_NOMEMORY;
			}
			else
			{
				p_context->namespace_count = namespace_count;
				rc = NVM_SUCCESS;
				for (int i = 0; i < namespace_count; i++)
				{
					p_context->p_namespaces[i].p_namespace_details = NULL;
					p_context->p_namespaces[i].p_namespace_discovery =
							calloc(1, sizeof (struct namespace_discovery));
					if (!p_context->p_namespaces[i].p_namespace_discovery)
					{
						COMMON_LOG_ERROR("Failed to allocate memory for discovery structure");
						rc = NVM_ERR_NOMEMORY;
						break;
					}
					else
					{
						memmove(p_context->p_namespaces[i].uid,
								p_namespaces[i].namespace_uid, sizeof (NVM_UID));
						memmove(p_context->p_namespaces[i].p_namespace_discovery,
								&p_namespaces[i], sizeof (struct namespace_discovery));
					}
				}
			}
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_nvm_context_namespace_details(const NVM_UID namespace_uid,
		struct namespace_details *p_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->namespace_count > 0 && p_context->p_namespaces)
		{
			for (int i = 0; i < p_context->namespace_count; i++)
			{
				if (uid_cmp(namespace_uid, p_context->p_namespaces[i].uid))
				{
					// found it
					if (p_context->p_namespaces[i].p_namespace_details)
					{
						memset(p_details, 0, sizeof (struct namespace_details));
						memmove(p_details, p_context->p_namespaces[i].p_namespace_details,
								sizeof (struct namespace_details));
						rc = NVM_SUCCESS;
					}
					break;
				}
			}
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int set_nvm_context_namespace_details(const NVM_UID namespace_uid,
		const struct namespace_details *p_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	// lock
	if (!mutex_lock(&g_context_lock))
	{
		COMMON_LOG_ERROR("Could not obtain the context lock");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if (p_context && p_context->namespace_count > 0 && p_context->p_namespaces)
		{
			for (int i = 0; i < p_context->namespace_count; i++)
			{
				if (uid_cmp(namespace_uid, p_context->p_namespaces[i].uid))
				{
					// found it
					// clear any existing details
					if (p_context->p_namespaces[i].p_namespace_details)
					{
						free(p_context->p_namespaces[i].p_namespace_details);
						p_context->p_namespaces[i].p_namespace_details = NULL;
					}
					// allocate new memory
					p_context->p_namespaces[i].p_namespace_details =
							calloc(1, sizeof (struct namespace_details));
					if (!p_context->p_namespaces[i].p_namespace_details)
					{
						rc = NVM_ERR_NOMEMORY;
						COMMON_LOG_ERROR("Failed to allocate memory for details structure");
					}
					else
					{
						// success, do the copy
						memmove(p_context->p_namespaces[i].p_namespace_details, p_details,
								sizeof (struct namespace_details));
						rc = NVM_SUCCESS;
					}
					break;
				}
			}
		}

		// unlock
		if (!mutex_unlock(&g_context_lock))
		{
			COMMON_LOG_ERROR("Could not release the context lock.");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
