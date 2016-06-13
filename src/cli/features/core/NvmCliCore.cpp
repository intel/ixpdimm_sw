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
 * DLL entry point for the NVMCLI core features.
 */

#include <libinvm-cli/Framework.h>
#include "SystemFeature.h"
#include "NamespaceFeature.h"
#include "ValidationFeature.h"
#include "FieldSupportFeature.h"
#include "SensorFeature.h"
#include "SimulatorFeature.h"
#include "NvmCliCore.h"
#include <persistence/lib_persistence.h>
#include <nvm_types.h>

#ifdef __WINDOWS__
#include <Windows.h>
#include <process.h>

/*!
 * Windows DLL entry point
 */
extern "C" BOOL WINAPI NVM_API DllMain(HANDLE hModule, DWORD ulReason, LPVOID lpReserved)
{
	int rc = COMMON_SUCCESS;
	switch (ulReason)
	{
		case DLL_PROCESS_ATTACH:
			if (!open_default_lib_store())
			{
				rc = COMMON_ERR_FAILED;
			}
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_PROCESS_DETACH:
			rc = close_lib_store();
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
	assert(open_default_lib_store());
}

/*
 * Called when the library is unloaded, free resources
 */
void lib_unload()
{
	// Since lib_unload doesn't allow return values,
	// assert if we fail to unload the db
	assert(close_lib_store() == COMMON_SUCCESS);
}
#endif

void registerFeatures()
{
	// get instance of Framework, then call registerFeature for each feature
	cli::framework::Framework *pFrameworkInst = cli::framework::Framework::getFramework();

	cli::framework::FeatureBase *pSystemFeature = new cli::nvmcli::SystemFeature();
	pFrameworkInst->registerFeature("SystemFeature", pSystemFeature);

	cli::framework::FeatureBase *pNamespaceFeature = new cli::nvmcli::NamespaceFeature();
	pFrameworkInst->registerFeature("NamespaceFeature", pNamespaceFeature);

	cli::framework::FeatureBase *pSensorFeature = new cli::nvmcli::SensorFeature();
	pFrameworkInst->registerFeature("SensorFeature", pSensorFeature);

	cli::framework::FeatureBase *pValidationFeature = new cli::nvmcli::ValidationFeature();
	pFrameworkInst->registerFeature("ValidationFeature", pValidationFeature);

	cli::framework::FeatureBase *pFieldSupportFeature = new cli::nvmcli::FieldSupportFeature();
	pFrameworkInst->registerFeature("FieldSupportFeature", pFieldSupportFeature);

	cli::framework::FeatureBase *pSimulatorFeature = new cli::nvmcli::SimulatorFeature();
	pFrameworkInst->registerFeature("SimulatorFeature", pSimulatorFeature);
}

void unRegisterFeatures()
{
	cli::framework::Framework *pFrameworkInst = cli::framework::Framework::getFramework();

	// remove all previously registered features
	pFrameworkInst->removeFeature("SystemFeature");
	pFrameworkInst->removeFeature("NamespaceFeature");
	pFrameworkInst->removeFeature("SensorFeature");
	pFrameworkInst->removeFeature("ValidationFeature");
	pFrameworkInst->removeFeature("FieldSupportFeature");
	pFrameworkInst->removeFeature("SimulatorFeature");
}
