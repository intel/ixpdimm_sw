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
 * This file contains the entry points for the OS into the DLL as a COM server
 * (Register and Unregister the DLL, GetClassObject)
 *
 * Much of this code is from Microsoft's example at
 * http://msdn.microsoft.com/en-us/library/windows/desktop/aa393677(v=vs.85).aspx
 * but it as been adapted to fit the project style and standards.
 */

#include <objbase.h>
#include <initguid.h>
#include <windows.h>
#include <libinvm-cim/IntelWmiProviderFactory.h>
#include <string/s_str.h>
#include <LogEnterExit.h>
#include <nvm_types.h>
#include <persistence/logging.h>

// Count number of objects and number of locks.
long       g_cObj=0;
long       g_cLock=0;

// used for path of dll to register in registry
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
HMODULE ghModule;

// will work for testing, but should generate own CLSID for production (use guidgen.exe)
// {7BADB4CD-9E85-429b-B5EA-4FB86BEF45EF}
DEFINE_GUID(CLSID_instprovider,
		0x7badb4cd, 0x9e85, 0x429b, 0xb5, 0xea, 0x4f, 0xb8, 0x6b, 0xef, 0x45, 0xef);
#define	CLSID_PATH "Software\\classes\\CLSID\\"

/*
 *
 *  DllGetClassObject
 *
 *  Purpose: Called by Ole when some client wants a class factory.
 *           Return one only if it is the sort of
 *           class this DLL supports.
 *
 */
STDAPI NVM_API DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
	COMMON_LOG_ENTRY();
	HRESULT rc = NOERROR;
	wbem::wmi::IntelWmiProviderFactory *pObj;

	if (CLSID_instprovider != rclsid)
	{
		rc = E_FAIL;
	}
	else
	{
		pObj = new wbem::wmi::IntelWmiProviderFactory();

		if (NULL == pObj)
		{
			rc = E_OUTOFMEMORY;
		}
		else
		{
			rc = pObj->QueryInterface(riid, ppv);
			if (FAILED(rc))
			{
				delete pObj;
			}
		}
	}
	log_gather();
	return rc;
}

/*
 * Called periodically by Ole to determine if the DLL can be freed.
 */
STDAPI NVM_API DllCanUnloadNow(void)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	//It is OK to unload if there are no objects or locks on the
	// class factory.
	SCODE sc = wbem::wmi::IntelWmiProviderFactory::CimFrameworkDLLCanUnloadNow();
	return sc;
}

/*
 * Register the DLL with Windows OS
 *  Called during setup or by regsvr32.
 *  Reused from
 *
 */
STDAPI NVM_API DllRegisterServer(void)
{
	COMMON_LOG_ENTRY();
	char       id[128];
	WCHAR		wId[128];
	char       clsid[128];
	TCHAR       module[MAX_PATH + 1];
	const char *name = "Intel WMI Instance Provider";
	const char *model = "Both";
	HKEY hKey1;
	HKEY hKey2;
	HRESULT rc = E_FAIL;

	// Create the path.
	memset(wId, 0, sizeof(wId));
	memset(id, 0, sizeof(id));
	StringFromGUID2(CLSID_instprovider, wId, sizeof(wId)/sizeof(WCHAR));
	wcstombs(id, wId, sizeof(id));
	s_strcpy(clsid, CLSID_PATH, sizeof(clsid));
	s_strcat(clsid, sizeof(clsid), id);

	// Create entries under CLSID
	LONG tmpRc = RegCreateKeyEx(HKEY_LOCAL_MACHINE, clsid, 0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_WRITE, NULL, &hKey1, NULL );
	if (tmpRc == ERROR_SUCCESS)
	{
		tmpRc = RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)name,
				strlen(name)+1);
		if (tmpRc == ERROR_SUCCESS)
		{
			tmpRc = RegCreateKeyEx(hKey1, "InprocServer32", 0, NULL,
					REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
					&hKey2, NULL );
			if (tmpRc == ERROR_SUCCESS)
			{
				memset(&module, 0, sizeof(module));
				GetModuleFileName((HINSTANCE)&__ImageBase, module,
						sizeof(module)/sizeof(TCHAR) - 1);

				tmpRc = RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)module,
						strlen(module)+1);
				if (tmpRc == ERROR_SUCCESS)
				{
					tmpRc = RegSetValueEx(hKey2, "ThreadingModel", 0, REG_SZ,
							(BYTE *)model, strlen(model)+1);
					if (tmpRc == ERROR_SUCCESS)
					{
						rc = NOERROR;
					}
				}
				RegCloseKey(hKey2);
			}
		}
		RegCloseKey(hKey1);
	}

	log_gather();
	return rc;
}

/*
 * Called when it is time to remove the registry entries.
 */
STDAPI NVM_API DllUnregisterServer(void)
{
	COMMON_LOG_ENTRY();
	TCHAR	id[128];
	WCHAR	wId[128];
	TCHAR	clsid[128];
	HKEY	hKey;

	// Create the path using the CLSID
	memset(wId, 0, sizeof(wId));
	memset(id, 0, sizeof(id));
	StringFromGUID2(CLSID_instprovider, wId,
			sizeof(wId)/sizeof(WCHAR));
	wcstombs(id, wId, sizeof(id));
	s_strcpy(clsid, CLSID_PATH, sizeof(clsid));
	s_strcat(clsid, sizeof(clsid), id);

	// First delete the InProcServer subkey.
	DWORD dwRet = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE, clsid, 0, KEY_WRITE, &hKey);
	if (dwRet == NO_ERROR)
	{
		RegDeleteKey(hKey, "InProcServer32");
		RegCloseKey(hKey);
	}

	dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			CLSID_PATH, 0, KEY_WRITE, &hKey);
	if (dwRet == NO_ERROR)
	{
		RegDeleteKey(hKey,id);
		RegCloseKey(hKey);
	}

	log_gather();
	return NOERROR;
}
