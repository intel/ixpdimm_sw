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
 * This file contains definition of the OS adapter interface
 * for general OS commands.
 */

#ifndef	_OS_ADAPTER_H_
#define	_OS_ADAPTER_H_

#include <common_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * ****************************************************************************
 * DEFINES
 * ****************************************************************************
 */

/*!
 * The standard page size for an x86 system
 */
#define	PAGE_SIZE_x86		4096

/*!
 * The standard cache line size
 */
#define	CACHE_LINE_SIZE 	64

/*!
 * Maximum size for a syslog message
 */
#define	SYSLOG_MESSAGE_MAX 1024

/*
 * ****************************************************************************
 * TYPEDEFS
 * ****************************************************************************
 */

/*!
 * The OS-specific type-definition of a mutex.
 * @remarks
 * 		in Windows, the mutex lives in kernel space @n
 * 			and is type @c HANDLE
 * 		in Linux, the mutex lives in user space @n
 * 			and is type @c pthread_mutex_t
 */
typedef void OS_MUTEX;

/*!
 * The OS-specific type-definition of a rwlock.
 * @remarks
 * 		in Windows, the type is @c SRWLOCK
 * 		in Linux, the type is @c pthread_rwlock_t
 */
typedef void OS_RWLOCK;

/*
 * ****************************************************************************
 * ENUMS
 * ****************************************************************************
 */

/*!
 * An enumeration set describing system event types
 */
enum system_event_type
{
	SYSTEM_EVENT_TYPE_INFO = 0,	//!< Informational event
	SYSTEM_EVENT_TYPE_WARNING = 1,	//!< Warning event
	SYSTEM_EVENT_TYPE_ERROR = 2,	//!< Error event
	SYSTEM_EVENT_TYPE_DEBUG = 3 //!< Debug event
};

/*!
 * Get the current working directory
 * @param[out] locale_dir
 * 		Base path for the language catalog
 */
extern void get_locale_dir(COMMON_PATH locale_dir);

/*
 * ***************************************************************
 * win_os & lnx_os functions (directory operations)
 * ***************************************************************
 */

/*!
 * Get the current working directory
 * @param[out] buffer
 * 		Storage location for tha path
 * @param[in] size
 * 		The allocated size of @c buffer (in @b char)
 * @return
 * 		A pointer to @c buffer. @n
 * 		@c NULL if an error occurred
 */
extern char *get_cwd(COMMON_PATH buffer, size_t size);

/*!
 * Function that gets the OS system root directory
 * @param[out] root_dir
 * 		The string containing the system root directory
 * @return
 * 		COMMON_ERR_INVALIDPARAMETER @n
 * 		COMMON_ERR_UNKNOWN @n
 * 		COMMON_SUCCESS
 */
extern int get_system_root(COMMON_PATH root_dir);

/*
 * ***************************************************************
 * win_os & lnx_os functions (process/thread execution)
 * ***************************************************************
 */

/*!
 * Start a process, and obtain its process id
 * @param[in] process_name
 * 		A string containing the full path name of the new process to start.  This must
 * 		include the file extension, if one exists.
 * @param[out] p_process_id
 * 		The ID of the process, if successfully started
 * @return
 * 		COMMON_SUCCESS @n
 * 		COMMON_ERR_FAILED
 */
extern int start_process(const char *process_name, unsigned int *p_process_id);

/*!
 * Stop a process using a process id
 * @param[in] process_id
 * 		The ID of the process to stop
 * @return
 * 		COMMON_SUCCESS @n
 * 		COMMON_ERR_FAILED
 */
extern int stop_process(unsigned int process_id);

/*!
 * Blocks for the specified number of msecs.
 * @param[in] time
 * 		The amount of time to sleep, in milliseconds.
 */
extern void nvm_sleep(unsigned long time);

/*!
 * Create a thread on the current process
 */
extern void create_thread(COMMON_UINT64 *p_thread_id, void *(*callback)(void *),
	void *callback_arg);

/*!
 * Gets the current threads ID.  Useful in logging.
 * @return
 * 		The thread ID
 */
extern COMMON_UINT64 get_thread_id();

/*!
 * A function that creates (in Windows only) and initializes a mutex.
 * @remarks
 *		The type of mutex created is a re-entrant mutex, meaning that the mutex can be
 *		locked by the same thread multiple times, while that thread already holds the
 *		lock.
 * @param p_mutex
 * 		A pointer to the mutex
 * @return
 * 		1 for success, 0 for failure
 */
extern int mutex_init(OS_MUTEX *p_mutex, const char *name);

/*!
 * A function that locks the mutex, guaranteeing thread independence where implemented.
 * @remarks
 *		Note that if this function is used recursively (re-entrant), mutex_unlock() must
 *		be called an equal number of times, to make the mutex available again for locking.
 * @param p_mutex
 * 		A pointer to the mutex
 * @return
 * 		1 for success, 0 for failure
 */
extern int mutex_lock(OS_MUTEX *p_mutex);

/*!
 * A function that unlocks the mutex, allowing other callers to mutex_lock to proceed
 * @param p_mutex
 * 		A pointer to the mutex
 * @return
 * 		1 for success, 0 for failure
 */
extern int mutex_unlock(OS_MUTEX *p_mutex);

/*!
 * A function that de-initializes and deallocates (Windows only) the mutex
 * @param p_mutex
 * 		A pointer to the mutex
 * @return
 * 		1 for success, 0 for failure
 */
extern int mutex_delete(OS_MUTEX *p_mutex, const char *name);

/*!
 * A function that initializes a rwlock.
 * @param p_rwlock
 * 		A pointer to the rwlock
 * @return
 * 		1 for success, 0 for failure
 */
extern int rwlock_init(OS_RWLOCK *p_rwlock);

/*!
 * A function that applies a shared-read lock to the rwlock
 * @param p_rwlock
 * 		A pointer to the rwlock
 * @return
 * 		1 for success, 0 for failure
 */
extern int rwlock_r_lock(OS_RWLOCK *p_rwlock);

/*!
 * A function that releases a shared-read lock
 * @param p_rwlock
 * 		A pointer to the rwlock
 * @return
 * 		1 for success, 0 for failure
 */
extern int rwlock_r_unlock(OS_RWLOCK *p_rwlock);

/*!
 * A function that applies an exclusive-write lock to the rwlock
 * @param p_rwlock
 * 		A pointer to the rwlock
 * @return
 * 		1 for success, 0 for failure
 */
extern int rwlock_w_lock(OS_RWLOCK *p_rwlock);

/*!
 * A function that releases an exclusive-write lock
 * @param p_rwlock
 * 		A pointer to the rwlock
 * @return
 * 		1 for success, 0 for failure
 */
extern int rwlock_w_unlock(OS_RWLOCK *p_rwlock);

/*!
 * A function that de-initializes and deallocates the rwlock
 * @param p_rwlock
 * 		A pointer to the rwlock
 * @return
 * 		1 for success, 0 for failure
 */
extern int rwlock_delete(OS_RWLOCK *p_rwlock);


/*
 * ***************************************************************
 * win_os & lnx_os functions (host information)
 * ***************************************************************
 */

/*!
 * Retrieve the name of the host server.
 * @param[in,out] name
 * 		A buffer to hold the name.
 * @param[in] name_len
 * 		The length of the buffer.  Should be <= 256.
 * @return
 * 		COMMON_SUCCESS
 * 		COMMON_ERR_INVALIDPARAMETER
 * 		COMMON_ERR_UNKNOWN
 */
extern int get_host_name(char *name, const COMMON_SIZE name_len);

/*!
 * Retrieve the operating system name.
 * @param[in,out] os_name
 * 		A buffer to hold the OS name.
 * @param[in] os_name_len
 * 		The length of the buffer.  Should be <= 256.
 * @return
 *  	COMMON_SUCCESS
 * 		COMMON_ERR_INVALIDPARAMETER
 * 		COMMON_ERR_UNKNOWN
 */
extern int get_os_name(char *os_name, const COMMON_SIZE os_name_len);

/*!
 * Retrieve the operating system version as a string.f
 * @param[in,out] os_version
 * 		A buffer to hold the OS version string.
 * @param[in] os_version_len
 * 		The length of the buffer.  Should be <= 256.
 * @return
 *  	COMMON_SUCCESS
 * 		COMMON_ERR_INVALIDPARAMETER
 * 		COMMON_ERR_UNKNOWN
 */
extern int get_os_version(char *os_version, const COMMON_SIZE os_version_len);

/*
 * ***************************************************************
 * win_os & lnx_os functions (user information)
 * ***************************************************************
 */

/*!
 * Determine if the caller has admin permissions
 * @return
 * 		COMMON_SUCCESS @n
 * 		COMMON_ERR_INVALIDPERMISSIONS
 */
extern int check_admin_permissions();

/*
 * ***************************************************************
 * win_os & lnx_os functions (system event log)
 * ***************************************************************
 */

/*!
 * Logs a message in the operating system event log.
 * @param type
 *		The type of event to log.
 * @param[in] source
 * 		A null terminated string indicating the source of the system event.
 * @param[in] message
 * 		A null terminated string containing the message to log
 */
extern void log_system_event(enum system_event_type type, const char *source,
		const char *message);

/*!
 * Return the path to the installation directory
 * @param[in, out] install_dir
 * 		the buffer to hold the install directory
 */
extern void get_install_dir(COMMON_PATH install_dir);

/*!
 * Create a new stream connected to a pipe running the given command
 * @note
 * 		Note that the buffer must be fully read for the pipe to be
 *		pclose()'d without undefined behavior.
 * @param[in] cmd
 * 		A string defining the command to be run in the pipe, whose
 * 		output will be stored in the buffer.
 * @param[in] mode
 * 		A string defining the mode of operation for the opened pipe
 * @return
 * 		A pointer to the FILE stream buffer containing the tty output.
 */
extern FILE *common_popen(const char *cmd, const char *mode);

/*!
 * Close a stream opened by popen() and return the status of its child
 * @param[in] p_fstream
 * 		A @c FILE pointer to a file stream
 * @return
 * 		An integer value representing the status of the popen()'d stream
 */
extern int common_pclose(FILE *p_fstream);

/*!
 * load a dynamic library at run time
 * @param lib_path
 * 		path to the library
 */
extern void *dlib_load(const char *lib_path);

/*!
 * close a handle to a library loaded at run time
 * @param handle
 * 		handle to close
 */
extern int dlib_close(void *handle);

/*!
 * Get the appropriate OS specific extension for a dynamic library
 * @param buffer
 * 		buffer to put the extension
 * @param buffer_len
 * 		length of the buffer
 * @return
 * 		return a the buffer pointer
 */
extern char *dlib_suffix(char *buffer, COMMON_SIZE buffer_len);

/*!
 * Find a symbol within dynamically loaded library
 * @param handle
 * @param symbol
 */
extern void *dlib_find_symbol(void *handle, const char *symbol);

/*!
 * Wrapper around the __get_cpuid os call. ESX doesn't support this so it becomes OS dependent
 * @param level
 * @param eax
 * @param ebx
 * @param ecx
 * @param edx
 * @return
 */
extern int get_cpuid(unsigned int level, unsigned int *eax,
		unsigned int *ebx, unsigned int *ecx, unsigned int *edx);

/*!
 * Securely clears the first num bytes of the memory area pointed to by ptr.
 * Compiler optimizations may remove call to memset() and not clear sensitive data from memory.
 * @param ptr
 *	memory area to be cleared
 * @param num
 * 	number of bytes of the memory to be cleared
 */
extern void s_memset(void *ptr, size_t num);

#ifdef __cplusplus
}
#endif

#endif  /* _OS_ADAPTER_H_ */
