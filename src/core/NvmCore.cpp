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
 * This file contains the entry point for the Core DLL entry point and
 * functionality dynamically executed upon library load.
 */

#include <string/s_str.h>
#include <os/os_adapter.h>
#include <file_ops/file_ops_adapter.h>
#include <persistence/lib_persistence.h>
#include <common_types.h>
#include <nvm_types.h>

int nvm_open_lib();
int nvm_close_lib();

#ifdef __WINDOWS__
#include <Windows.h>
#include <process.h>

/*!
 * DLL Entry Point
 */
extern "C" BOOL WINAPI NVM_API DllMain(HANDLE hModule, DWORD ulReason, LPVOID lpReserved)
{
	int rc = COMMON_SUCCESS;
	switch (ulReason)
	{
		case DLL_PROCESS_ATTACH:
			if (nvm_open_lib())
			{
				rc = COMMON_ERR_FAILED;
			}
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
#include <assert.h>
#include <pthread.h>

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
	// assert if the db is not able to be loaded
	assert(nvm_open_lib() == COMMON_SUCCESS);
}

/*
 * Called when the library is unloaded, free resources
 */
void lib_unload()
{
	// Since lib_unload doesn't allow return values,
	// assert if we fail to unload the db
	assert(nvm_close_lib() == COMMON_SUCCESS);
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

	return rc;
}

/*
 * Close the library
 */
int nvm_close_lib()
{
	int rc = NVM_SUCCESS;

	// close the database
	if (close_lib_store() != NVM_SUCCESS)
	{
		rc = NVM_ERR_UNKNOWN;
	}
	return rc;
}
