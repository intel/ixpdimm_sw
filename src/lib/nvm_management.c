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
 * This file contains the functionality that is dynamically executed upon library load.
 */

#include <string/s_str.h>
#include <os/os_adapter.h>

#include "nvm_management.h"
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include <persistence/logging.h>


#ifdef __WINDOWS__
#include <Windows.h>
#include <process.h>
// TODO: need actual registry key from installer
#define	APP_REGISTRY_ENTRY	"SOFTWARE\\Intel\\TBD"
HANDLE g_eventmonitor_lock;
HANDLE g_context_lock;
#else
#include <assert.h>
pthread_mutex_t g_eventmonitor_lock;
pthread_mutex_t g_context_lock;
#endif

/*
 * Local methods
 */
int nvm_open_lib();
int nvm_close_lib();

#ifdef __WINDOWS__
/*
 * Windows API call that deals with DLL administrative tasks.
 */
BOOL __stdcall DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	int rc = NVM_SUCCESS;

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			rc = nvm_open_lib();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_PROCESS_DETACH:
			rc = nvm_close_lib();
			break;
		case DLL_THREAD_DETACH:
			break;
	}

	if (rc >= 0)
		return TRUE;
	else
		return FALSE;
}

#else
/*
 * Tell gcc to generate code that calls these functions when the shared
 * library is loaded and unloaded.
 */
void __attribute__((constructor)) lib_load();
void __attribute__((destructor)) lib_unload();

/*
 * Called when the library is loaded, open the config database
 */
void lib_load()
{
	// Since lib_load doesn't allow return values,
	// assert if the library is not able to be loaded
	assert(nvm_open_lib() == NVM_SUCCESS);
}

/*
 * Called when the library is unloaded, free resources
 */
void lib_unload()
{
	// Since lib_unload doesn't allow return values,
	// assert if the library fails while unloading
	assert(nvm_close_lib() == NVM_SUCCESS);
}
#endif

/*
 * Open the library
 */
int nvm_open_lib()
{
	int rc = NVM_SUCCESS;

	// initialize the connection to the database
	if (!open_default_lib_store())
	{
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		char sim_path[CONFIG_VALUE_LEN];
		// attempt to open a default simulator
		if (get_config_value(SQL_KEY_DEFAULT_SIMULATOR, sim_path) == COMMON_SUCCESS)
		{
			// don't care about failures. sim_adapter will log any
			// errors to the config database. other adapters will just
			// return not supported
			COMMON_LOG_DEBUG_F("Opening default simulator %s", sim_path);
			nvm_add_simulator(sim_path, s_strnlen(sim_path, CONFIG_VALUE_LEN));
		}

		// initialize the event monitor lock
		// event monitoring is per process so no need to be cross-process safe
		// thus no name on the mutex
		if (!mutex_init((OS_MUTEX*)&g_eventmonitor_lock, NULL))
		{
			rc = NVM_ERR_UNKNOWN;
		}

		// initialize the context lock
		// context is per proces so no need to be cross-process safe
		// thus no name on the mutex
		if (!mutex_init((OS_MUTEX*)&g_context_lock, NULL))
		{
			rc = NVM_ERR_UNKNOWN;
		}
	}
	return rc;
}

/*
 * Close the library
 */
int nvm_close_lib()
{
	int rc = NVM_SUCCESS;

	// remove any simulators that were loaded
	// don't care about any failures here because there may
	// not be a simulator loaded
	nvm_remove_simulator();

	// close the database
	if (close_lib_store() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_UNKNOWN;
	}
	// clean up the locks
	if (!mutex_delete((OS_MUTEX*)&g_eventmonitor_lock, NULL))
	{
		rc = NVM_ERR_UNKNOWN;
	}
	if (!mutex_delete((OS_MUTEX*)&g_context_lock, NULL))
	{
		rc = NVM_ERR_UNKNOWN;
	}

	return rc;
}
