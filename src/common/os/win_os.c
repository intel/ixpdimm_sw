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
 * This file contains the Windows implementation of the os_adapter.h
 * system call wrappers.
 */

#include "os_adapter.h"
#include "win_msgs.h"
#include "common_types.h"
#include <sys/stat.h>
#include <windows.h>
#include <winnt.h>
#include <stdio.h>
#include <tchar.h> // todo: remove this header and replace associated functions
#include <direct.h> // for _getcwd

#include <string/s_str.h>
#include <string/unicode_utilities.h>
#ifdef _WIN32
#include <intrin.h>
#else
#include <cpuid.h>
#endif

#define	MGMTSW_REG_KEY "SOFTWARE\\INTEL\\INTEL DIMM GEN 1"
#define	INSTALLDIR_REG_SUBKEY "InstallDir"

#define	EVENT_SOURCE	"IntelASM"

// used by get_os_name
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef int (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

int database_is_present(COMMON_PATH install_dir);

extern int parse_revision(unsigned short int **pp_parts, int parts_count,
		const char * const revision, size_t revision_len);

const char *WIN_VISTA = TEXT("Microsoft Windows Vista");
const char *WIN_SERVER_2008 = TEXT("Microsoft Windows Server 2008");
const char *WIN_7 = TEXT("Microsoft Windows 7");
const char *WIN_SERVER_2008_R2 = TEXT("Microsoft Windows Server 2008 R2");
const char *WIN_8 = TEXT("Microsoft Windows 8");
const char *WIN_SERVER_2012 = TEXT("Microsoft Windows Server 2012");
const char *WIN_8_1 = TEXT("Microsoft Windows 8.1");
const char *WIN_SERVER_2012_R2 = TEXT("Microsoft Windows Server 2012 R2");
const char *WIN_10 = TEXT("Microsoft Windows 10");
const char *WIN_SERVER_2016 = TEXT("Microsoft Windows Server 2016");
const char *WIN_UNKNOWN = TEXT("Unrecognized Microsoft Windows operating system");

/*
 * Get current working directory
 */
char *get_cwd(COMMON_PATH buffer, size_t size)
{
	return _getcwd(buffer, size);
}

/*
 * Start a process and get its PID
 */
int start_process(const char *process_name, unsigned int *p_process_id)
{
	int rc = COMMON_ERR_FAILED;

	COMMON_WPATH w_process_name;
	utf8_to_wchar(w_process_name, (size_t)COMMON_PATH_LEN, process_name, (int)COMMON_PATH_LEN);

	STARTUPINFOW start_up_info;
	memset(&start_up_info, 0, sizeof (STARTUPINFOW));
	start_up_info.cb = sizeof (STARTUPINFOW);

	PROCESS_INFORMATION process_info;
	memset(&process_info, 0, sizeof (PROCESS_INFORMATION));

	int success = CreateProcessW(w_process_name, NULL, NULL, NULL, FALSE, 0, NULL, NULL,
			&start_up_info, &process_info);
	if (success)
	{
		*p_process_id = process_info.dwProcessId;
		rc = COMMON_SUCCESS;
		// close handles
		CloseHandle(process_info.hProcess);
		CloseHandle(process_info.hThread);
	}

	return rc;
}

/*
 * Stop a process given the process handle
 */
int stop_process(unsigned int process_id)
{
	int rc = COMMON_ERR_FAILED;

	// get the process handle from the process id
	unsigned long access = PROCESS_TERMINATE;
	HANDLE process_handle = OpenProcess(access, 0, process_id);
	if (process_handle != NULL)
	{
		unsigned int exitCode = 0;
		int success = TerminateProcess(process_handle, exitCode);
		if (success)
		{
			rc = COMMON_SUCCESS;
		}
		CloseHandle(process_handle);
	}

	return rc;
}

/*
 * Blocks for the specified number of msecs.
 */
void nvm_sleep(unsigned long time)
{
	Sleep(time);
}

/*
 * Create a thread on the current process
 */
void create_thread(COMMON_UINT64 *p_thread_id, void *(*callback)(void *), void * callback_arg)
{
	CreateThread(
			NULL, // default security
			0,  // default stack size
			(LPTHREAD_START_ROUTINE)callback,
			(LPVOID)callback_arg,
			0, // Immediately run thread
			(LPDWORD)p_thread_id);
}

/*
 * Retrieve the id of the current thread
 */
COMMON_UINT64 get_thread_id()
{
	return GetCurrentThreadId();
}

/*
 * Creates & Initializes a mutex.
 */
int mutex_init(OS_MUTEX *p_mutex, const char *name)
{
	int rc = 0;
	if (p_mutex)
	{
		HANDLE *p_handle = (HANDLE *)p_mutex;

		*p_handle = CreateMutex(NULL, FALSE, name);

		// return failure if the pointer is null
		rc = (p_handle != NULL);
	}
	return rc;
}

/*
 * Locks the given mutex.
 */
int mutex_lock(OS_MUTEX *p_mutex)
{
	// default return to failure state
	int rc = 0;
	if (p_mutex)
	{
		HANDLE *p_handle = (HANDLE *)p_mutex;
		DWORD f_rc = WaitForSingleObject(*p_handle, INFINITE);
		switch (f_rc)
		{
			// this is the only case of success
			case WAIT_OBJECT_0:
				rc = 1;
				break;

			// all others are failure
			case WAIT_ABANDONED:
			case WAIT_TIMEOUT:
			case WAIT_FAILED:
			default:
				rc = 0;
				break;
		}
	}
	return rc;
}

/*
 * Unlocks a locked mutex
 */
int mutex_unlock(OS_MUTEX *p_mutex)
{
	int rc = 0;
	if (p_mutex)
	{
		HANDLE *p_handle = (HANDLE *)p_mutex;

		// failure when ReleaseMutex(..) == 0
		rc = (ReleaseMutex(*p_handle) != 0);
	}
	return rc;
}

/*
 * Deletes the mutex
 */
int mutex_delete(OS_MUTEX *p_mutex, const char *name)
{
	int rc = 1;
	if (p_mutex)
	{
		HANDLE *p_handle = (HANDLE *)p_mutex;

		// failure when CloseHandle(..) == 0
		rc = (CloseHandle(*p_handle) != 0);
        *p_handle = NULL;

	}
	return rc;
}

/*
 * Initializes a rwlock
 */
int rwlock_init(OS_RWLOCK *p_rwlock)
{
	SRWLOCK *p_handle = (SRWLOCK *)p_rwlock;

	// Win32 API provides no indication of success for this function
	InitializeSRWLock(p_handle);
	return 1;
}

/*
 * Applies a shared read-lock to the rwlock
 */
int rwlock_r_lock(OS_RWLOCK *p_rwlock)
{
	SRWLOCK *p_handle = (SRWLOCK *)p_rwlock;

	// Win32 API provides no indication of success for this function
	AcquireSRWLockShared(p_handle);
	return 1;
}

/*
 * Unlocks an shared-read lock
 */
int rwlock_r_unlock(OS_RWLOCK *p_rwlock)
{
	SRWLOCK *p_handle = (SRWLOCK *)p_rwlock;

	// Win32 API provides no indication of success for this function
	ReleaseSRWLockShared(p_handle);
	return 1;
}

/*
 * Applies an exclusive write-lock to the rwlock
 */
int rwlock_w_lock(OS_RWLOCK *p_rwlock)
{
	SRWLOCK *p_handle = (SRWLOCK *)p_rwlock;

	// Win32 API provides no indication of success for this function
	AcquireSRWLockExclusive(p_handle);
	return 1;
}

/*
 * Unlocks an exclusive-write lock
 */
int rwlock_w_unlock(OS_RWLOCK *p_rwlock)
{
	SRWLOCK *p_handle = (SRWLOCK *)p_rwlock;

	// Win32 API provides no indication of success for this function
	ReleaseSRWLockExclusive(p_handle);
	return 1;
}

/*
 * Deletes the rwlock
 */
int rwlock_delete(OS_RWLOCK *p_rwlock)
{
	// SRW Locks do not need to be explicitly destroyed
	// see: http:// msdn.microsoft.com/en-us/library/windows/desktop/ms683483%28v=vs.85%29.aspx
	return 1;
}

/*
 * Retrieve the name of the host server.
 */
int get_host_name(char *name, const COMMON_SIZE name_len)
{
	int rc = COMMON_SUCCESS;

	// check input parameters
	if (name == NULL || name_len == 0)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		// have to initialize the networking stuff to retrieve host name
		WSADATA wsaData;
		WORD wVersionRequested;
		wVersionRequested = MAKEWORD(2, 2);
		int err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0)
		{
			rc = COMMON_ERR_UNKNOWN;
		}
		else
		{
			err = gethostname(name, name_len);
			if (err != 0)
			{
				return COMMON_ERR_UNKNOWN;
			}
			WSACleanup();
		}
	}
	return rc;
}

int get_os_version_parts(unsigned short *p_major, unsigned short *p_minor,
		unsigned short *p_build)
{
	char version_str[128];
	int rc = get_os_version(version_str, sizeof (version_str));
	if (rc == COMMON_SUCCESS)
	{
		unsigned short *pp_parts[] = { p_major, p_minor, p_build };
		parse_revision(pp_parts, 3, version_str, sizeof (version_str));
	}

	return rc;
}

const char *get_server_release_for_version(const unsigned short major_version,
		const unsigned short minor_version)
{
	const char *release_name = WIN_UNKNOWN;

	// Modern versions of Windows Server
	// Older versions unsupported
	if (major_version == 6)
	{
		switch (minor_version)
		{
		case 0:
			release_name = WIN_SERVER_2008;
			break;
		case 1:
			release_name = WIN_SERVER_2008_R2;
			break;
		case 2:
			release_name = WIN_SERVER_2012;
			break;
		case 3:
			release_name = WIN_SERVER_2012_R2;
			break;
		default: // Unrecognized
			break;
		}
	}
	else if (major_version == 10)
	{
		switch (minor_version)
		{
		case 0:
			release_name = WIN_SERVER_2016;
			break;
		default:
			break;
		}
	}

	return release_name;
}

const char *get_workstation_release_for_version(const unsigned short major_version,
		const unsigned short minor_version)
{
	const char *release_name = WIN_UNKNOWN;

	// Modern versions of Windows
	// Older versions unsupported
	if (major_version == 6)
	{
		switch (minor_version)
		{
		case 0:
			release_name = WIN_VISTA;
			break;
		case 1:
			release_name = WIN_7;
			break;
		case 2:
			release_name = WIN_8;
			break;
		case 3:
			release_name = WIN_8_1;
			break;
		default: // Unrecognized
			break;
		}
	}
	else if (major_version == 10)
	{
		switch (minor_version)
		{
		case 0:
			release_name = WIN_10;
			break;
		default:
			break;
		}
	}

	return release_name;
}

COMMON_BOOL is_windows_server()
{
	OSVERSIONINFOEX version_info;
	ZeroMemory(&version_info, sizeof (version_info));
	version_info.dwOSVersionInfoSize = sizeof (version_info);

	// Anything that's not a workstation counts as a server
	version_info.wProductType = VER_NT_WORKSTATION;

	DWORDLONG conditionMask = 0;
	VER_SET_CONDITION(conditionMask, VER_PRODUCT_TYPE, VER_EQUAL);

	return !VerifyVersionInfo(&version_info, VER_PRODUCT_TYPE, conditionMask);
}

const char *get_windows_release_name_for_version(const unsigned short major_version,
		const unsigned short minor_version)
{
	const char *win_release;
	if (is_windows_server())
	{
		win_release = get_server_release_for_version(major_version, minor_version);
	}
	else
	{
		win_release = get_workstation_release_for_version(major_version, minor_version);
	}

	return win_release;
}

void get_system_info(SYSTEM_INFO *p_system_info)
{
	ZeroMemory(p_system_info, sizeof (SYSTEM_INFO));

	// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
	PGNSI p_gnsi = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),
		"GetNativeSystemInfo");
	if (NULL != p_gnsi)
	{
		p_gnsi(p_system_info);
	}
	else
	{
		GetSystemInfo(p_system_info);
	}
}

DWORD get_current_os_edition(const unsigned short major, const unsigned short minor)
{
	PGPI p_gpi;
	DWORD edition = 0;
	p_gpi = (PGPI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),
		"GetProductInfo");
	p_gpi(major, minor, 0, 0, &edition);

	return edition;
}

const char *get_current_os_edition_name(const unsigned short major_version,
		const unsigned short minor_version)
{
	const char *edition_name = "";

	DWORD edition_type = get_current_os_edition(major_version, minor_version);
	switch (edition_type)
	{
		case PRODUCT_ULTIMATE:
			edition_name = TEXT("Ultimate Edition");
			break;
		case PRODUCT_PROFESSIONAL:
			edition_name = TEXT("Professional");
			break;
		case PRODUCT_HOME_PREMIUM:
			edition_name = TEXT("Home Premium Edition");
			break;
		case PRODUCT_HOME_BASIC:
			edition_name = TEXT("Home Basic Edition");
			break;
		case PRODUCT_ENTERPRISE:
			edition_name = TEXT("Enterprise Edition");
			break;
		case PRODUCT_BUSINESS:
			edition_name = TEXT("Business Edition");
			break;
		case PRODUCT_STARTER:
			edition_name = TEXT("Starter Edition");
			break;
		case PRODUCT_CLUSTER_SERVER:
			edition_name = TEXT("Cluster Server Edition");
			break;
		case PRODUCT_DATACENTER_SERVER:
			edition_name = TEXT("Datacenter Edition");
			break;
		case PRODUCT_DATACENTER_SERVER_CORE:
			edition_name = TEXT("Datacenter Edition (core installation)");
			break;
		case PRODUCT_ENTERPRISE_SERVER:
			edition_name = TEXT("Enterprise Edition");
			break;
		case PRODUCT_ENTERPRISE_SERVER_CORE:
			edition_name = TEXT("Enterprise Edition (core installation)");
			break;
		case PRODUCT_ENTERPRISE_SERVER_IA64:
			edition_name = TEXT("Enterprise Edition for Itanium-based Systems");
			break;
		case PRODUCT_SMALLBUSINESS_SERVER:
			edition_name = TEXT("Small Business Server");
			break;
		case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
			edition_name = TEXT("Small Business Server Premium Edition");
			break;
		case PRODUCT_STANDARD_SERVER:
			edition_name = TEXT("Standard Edition");
			break;
		case PRODUCT_STANDARD_SERVER_CORE:
			edition_name = TEXT("Standard Edition (core installation)");
			break;
		case PRODUCT_WEB_SERVER:
			edition_name = TEXT("Web Server Edition");
			break;
	}

	return edition_name;
}

void append_edition_name(const unsigned short major_version,
	const unsigned short minor_version,
	char *os_name, const COMMON_SIZE os_name_len)
{
	s_strcat(os_name, os_name_len, " ");
	s_strcat(os_name, os_name_len,
			get_current_os_edition_name(major_version, minor_version));
}

void append_build_number(const unsigned short build_version,
		char *os_name, const COMMON_SIZE os_name_len)
{
	TCHAR buf[80];
	s_snprintf(buf, 80, " (build %u)", build_version);
	s_strcat(os_name, os_name_len, buf);
}

void append_processor_architecture(char *os_name, const COMMON_SIZE os_name_len)
{
	SYSTEM_INFO si;
	get_system_info(&si);

	switch (si.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_AMD64:
		s_strcat(os_name, os_name_len, TEXT(", 64-bit"));
		break;
	case PROCESSOR_ARCHITECTURE_INTEL:
		s_strcat(os_name, os_name_len, TEXT(", 32-bit"));
		break;
	default:
		break;
	}
}

int get_os_name_from_version(const unsigned short major_version,
	const unsigned short minor_version, const unsigned short build_version,
	char *os_name, const COMMON_SIZE os_name_len)
{
	int rc = COMMON_SUCCESS;

	s_strcpy(os_name, get_windows_release_name_for_version(major_version,
			minor_version), os_name_len);
	if (major_version >= 6) // Vista or later
	{
		append_edition_name(major_version, minor_version, os_name, os_name_len);
		append_build_number(build_version, os_name, os_name_len);
		append_processor_architecture(os_name, os_name_len);
	}

	return rc;
}

/*
 * Retrieve the operating system name.
 */
int get_os_name(char *os_name, const COMMON_SIZE os_name_len)
{
	int rc = COMMON_SUCCESS;

	// check input parameters
	if (os_name == NULL || os_name_len == 0)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		unsigned short major = 0;
		unsigned short minor = 0;
		unsigned short build = 0;
		get_os_version_parts(&major, &minor, &build);

		rc = get_os_name_from_version(major, minor, build,
				os_name, os_name_len);
	}

	return rc;
}

int get_file_version_info_for_system(LPVOID *pp_version_info)
{
	int rc = COMMON_SUCCESS;

	LPCSTR system_filename = TEXT("kernel32.dll");
	DWORD version_data_size = GetFileVersionInfoSize(system_filename, 0);
	if (version_data_size > 0)
	{
		*pp_version_info = calloc(1, version_data_size);
		if (*pp_version_info == NULL)
		{
			rc = COMMON_ERR_NOMEMORY;
		}
		else
		{
			if (!GetFileVersionInfo(system_filename, 0, version_data_size, *pp_version_info))
			{
				free(*pp_version_info);
				*pp_version_info = NULL;

				rc = COMMON_ERR_UNKNOWN;
			}
		}
	}
	else
	{
		rc = COMMON_ERR_UNKNOWN;
	}

	return rc;
}

// The language and code page numbers for Windows FileVersionInfo.
// Used to come up with paths for version strings in FileVersionInfo.
struct LANGANDCODEPAGE
{
	WORD wLanguage;
	WORD wCodePage;
};

// Returns a pointer into existing memory p_version_info
struct LANGANDCODEPAGE *get_first_lang_and_code_page(LPVOID p_version_info)
{
	struct LANGANDCODEPAGE *p_first = NULL;
	UINT bytes = 0;
	if (VerQueryValue(p_version_info, TEXT("\\VarFileInfo\\Translation"),
			(LPVOID *)&p_first, &bytes))
	{
		UINT language_count = bytes / sizeof (struct LANGANDCODEPAGE);
		if (language_count == 0) // inconsistent data
		{
			p_first = NULL;
		}
	}

	return p_first;
}

int get_sub_block_path_for_file_version_info(LPVOID p_file_version_info,
		const char *sub_block_name,
		char *p_sub_block_path, const unsigned int sub_block_path_len)
{
	int rc = COMMON_SUCCESS;

	struct LANGANDCODEPAGE *p_sub_block_lang =
			get_first_lang_and_code_page(p_file_version_info);
	if (p_sub_block_lang)
	{
		s_snprintf(p_sub_block_path, sub_block_path_len,
				"\\StringFileInfo\\%04x%04x\\%s",
				p_sub_block_lang->wLanguage, p_sub_block_lang->wCodePage,
				sub_block_name);
	}
	else
	{
		// Something is wrong with the version info
		rc = COMMON_ERR_UNKNOWN;
	}

	return rc;
}

int get_version_string_from_system(char *os_version, const COMMON_SIZE os_version_len)
{
	LPVOID p_version_info = NULL;
	int rc = get_file_version_info_for_system(&p_version_info);
	if (rc == COMMON_SUCCESS)
	{
		char product_version_sub_block[256];
		rc = get_sub_block_path_for_file_version_info(p_version_info,
				"ProductVersion",
				product_version_sub_block, sizeof (product_version_sub_block));
		if (rc == COMMON_SUCCESS)
		{
			LPVOID p_version = NULL;
			UINT version_size = 0;
			if (VerQueryValue(p_version_info, TEXT(product_version_sub_block),
					&p_version, &version_size))
			{
				// The sub-block is formatted as a string
				s_strncpy(os_version, os_version_len, p_version, version_size);
			}
		}
	}
	free(p_version_info);

	return rc;
}

/*
 * Retrieve the operating system version as a string.
 */
int get_os_version(char *os_version, const COMMON_SIZE os_version_len)
{
	int rc = COMMON_SUCCESS;

	// check input parameters
	if (os_version == NULL || os_version_len == 0)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		rc = get_version_string_from_system(os_version, os_version_len);
	}
	return rc;
}

/*
 * Determine if the caller has permission to make changes to the system
 */
int check_admin_permissions()
{
	int rc = COMMON_SUCCESS;
	SID_IDENTIFIER_AUTHORITY authority = { SECURITY_NT_AUTHORITY };
	PSID group;
	BOOL is_member = FALSE;
	DWORD user_type = DOMAIN_ALIAS_RID_ADMINS;

	if (AllocateAndInitializeSid(&authority, 2, SECURITY_BUILTIN_DOMAIN_RID,
			user_type, 0, 0, 0, 0, 0, 0, &group))
	{
		if (!CheckTokenMembership(NULL, group, &is_member) || !is_member)
		{
			rc = COMMON_ERR_INVALIDPERMISSIONS;
		}
		FreeSid(group);
	}

	return rc;
}

/*
 *  Check the given user is an administrator
 */
int is_admin(HANDLE hUserToken)
{
	int rc = 0;
	DWORD i = 0, dwSize = 0, dwResult = 0;
	SID_IDENTIFIER_AUTHORITY ntAuth = {SECURITY_NT_AUTHORITY};
	PSID pAdminSid;
	PTOKEN_GROUPS pGroupInfo;

	// calculate the token buffer for the given user token
	if (!GetTokenInformation(hUserToken, TokenGroups, NULL, dwSize, &dwSize))
	{
		dwResult = GetLastError();
		if (dwResult != ERROR_INSUFFICIENT_BUFFER)
		{
			return rc;
		}
	}

	// get token information for the given user token
	pGroupInfo = (PTOKEN_GROUPS) GlobalAlloc(GPTR, dwSize);
	if ((GetTokenInformation(hUserToken, TokenGroups, pGroupInfo, dwSize, &dwSize)) &&
			(NULL != pGroupInfo))
	{
		// SID for the Administrators group
		if (AllocateAndInitializeSid(&ntAuth, 2, SECURITY_BUILTIN_DOMAIN_RID,
				DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdminSid))
		{
			// loop and compare through groups in token
			for (i = 0; i < pGroupInfo->GroupCount; i++)
			{
				if (EqualSid(pAdminSid, pGroupInfo->Groups[i].Sid))
				{
					rc = 1;
					break;
				}
			}
			FreeSid(pAdminSid);
		}
	}

	// close handle
	CloseHandle(hUserToken);
	return rc;
}

/*
 * Map event type enum to event type defined in message file
 */
WORD get_windows_event_type(enum system_event_type type)
{
	WORD ret;
	switch (type)
	{

		case SYSTEM_EVENT_TYPE_WARNING:
			ret = WARNING_EVENT;
			break;
		case SYSTEM_EVENT_TYPE_ERROR:
			ret = ERROR_EVENT;
			break;
		case SYSTEM_EVENT_TYPE_DEBUG:
		case SYSTEM_EVENT_TYPE_INFO:
		default:
			ret = INFORMATIONAL_EVENT;
			break;
	}

	return ret;
}

/*
 * Map event type enum to event id defined in message file
 */
DWORD get_event_id(enum system_event_type type)
{
	DWORD ret;

	switch (type)
	{
		case SYSTEM_EVENT_TYPE_WARNING:
			ret = NVMDIMM_WARNING;
			break;
		case SYSTEM_EVENT_TYPE_ERROR:
			ret = NVMDIMM_ERROR;
			break;
		case SYSTEM_EVENT_TYPE_DEBUG:
		case SYSTEM_EVENT_TYPE_INFO:
		default:
			ret = NVMDIMM_INFORMATIONAL;
			break;
	}

	return ret;
}

/*
 * Logs a message in the operating system event log.
 */
void log_system_event(enum system_event_type type, const char *source, const char *message)
{
	HANDLE hEventSource;
	LPCTSTR lpszStrings[2];

	lpszStrings[0] = source;
	lpszStrings[1] = message;

	hEventSource = RegisterEventSource(NULL, EVENT_SOURCE);

	if (NULL != hEventSource)
	{
		ReportEvent(hEventSource,			// event log handle
				get_windows_event_type(type),	// event type
				0,				// event category
				get_event_id(type),			// event identifier
				NULL,				// no security identifier
				2,				// size of lpszStrings array
				0,				// no binary data
				lpszStrings,			// array of strings
				NULL);				// no binary data

		DeregisterEventSource(hEventSource);
	}
}

/*
 * Return the base path for the language catalog
 */
void get_locale_dir(COMMON_PATH locale_dir)
{
	get_install_dir(locale_dir);
}

/*
 * Return the path to the installation directory
 */
void get_install_dir(COMMON_PATH install_dir)
{
	// GetModuleFileName puts full path to DLL into install_dir
	// ex: C:\output\build\windows\debug\libixpdimm.dll
	if (GetModuleFileName((HINSTANCE)&__ImageBase, install_dir, COMMON_PATH_LEN) > 0)
	{
		// find last '/' or '\'
		int len = strlen(install_dir) - 1;
		while (len >= 0)
		{
			if (install_dir[len] == '\\' ||
					install_dir[len] == '/')
			{
				install_dir[len + 1] = '\0'; // keep the last '\'
				len  = 0; // all done
			}
			len --;
		}
	}

	if (!database_is_present(install_dir))
	{
		HKEY regKey;
		DWORD buffersize = COMMON_PATH_LEN;

		int ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, MGMTSW_REG_KEY, 0, KEY_READ, &regKey);

		if (ret == ERROR_SUCCESS)
		{
			RegQueryValueEx(regKey, INSTALLDIR_REG_SUBKEY, NULL, NULL,
					(LPBYTE)install_dir, &buffersize);

			RegCloseKey(regKey);
		}
		s_strncat(install_dir, COMMON_PATH_LEN, "\\", COMMON_PATH_LEN);
	}
}

int database_is_present(COMMON_PATH install_dir)
{
	int is_present = 1;
	struct stat st;

	COMMON_PATH database_file;
	s_strncpy(database_file, COMMON_PATH_LEN, install_dir, COMMON_PATH_LEN);
	s_strncat(database_file, COMMON_PATH_LEN, "\\", COMMON_PATH_LEN);
	s_strncat(database_file, COMMON_PATH_LEN, CONFIG_FILE, COMMON_PATH_LEN);

	// make sure the file is there
	if (stat(database_file, &st) < 0)
	{
		is_present = 0;
	}
	return is_present;
}

/*
 * Create a new stream connected to a pipe running the given command
 */
FILE *common_popen(const char *cmd, const char *mode)
{
	return _popen(cmd, mode);
}

/*
 * Close a stream opened by popen() and return the status of its child
 */
int common_pclose(FILE *p_fstream)
{
	return _pclose(p_fstream);
}

/*
 * dynamically load a library
 */
void *dlib_load(const char *lib_path)
{
	return LoadLibraryExA(lib_path, 0, 0);
}

/*
 * close a dynamically loaded library
 */
int dlib_close(void *handle)
{
	int rc = 0;
	if (handle)
	{
		rc = FreeLibrary(handle);
	}
	return rc;
}

/*
 * .dll for windows
 */
char *dlib_suffix(char *buffer, COMMON_SIZE buffer_len)
{
	s_strncpy(buffer, buffer_len, ".dll", s_strnlen(".dll", 5));
	return buffer;
}

/*
 * Get the symbol name from a loaded library
 */
void *dlib_find_symbol(void *handle, const char *symbol)
{
	return (void*) GetProcAddress((HMODULE) handle, symbol);
}

int get_cpuid(unsigned int level, unsigned int *eax,
		unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
    if (NULL == eax || NULL == ebx || NULL == ecx || NULL == edx)
    {
        return 0;
    }

#ifdef _WIN32
    int cpuInfo[4];

    __cpuid(cpuInfo, level);

    *eax = cpuInfo[0];
    *ebx = cpuInfo[1];
    *ecx = cpuInfo[2];
    *edx = cpuInfo[3];

    return 1;
#else
	return __get_cpuid(level, eax, ebx, ecx, edx);
#endif
}

void s_memset(void *ptr, size_t num)
{
	SecureZeroMemory(ptr, num);
}

int get_filesize(const char *filename, size_t *filesize)
{
    BOOL                        fOk;
    WIN32_FILE_ATTRIBUTE_DATA   fileInfo;

    if (NULL == filename && NULL != filesize)
        return -1;

    fOk = GetFileAttributesEx(filename, GetFileExInfoStandard, (void*)&fileInfo);
    if (!fOk)
        return -1;

    //assert(0 == fileInfo.nFileSizeHigh);
    *filesize = (size_t)fileInfo.nFileSizeLow;

    return 0;
}

int does_file_exist(const char *filename)
{
    DWORD dwAttrib = GetFileAttributes(filename);

    if (!((dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))))
    {
        return FALSE;
    }
    return TRUE;
}

