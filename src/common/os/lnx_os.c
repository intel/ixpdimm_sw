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
 * This file contains the Linux implementation of the os_adapter.h
 * system call wrappers.
 */


#include <unistd.h>
#include <signal.h>
#include <sys/sendfile.h>
#include <sys/utsname.h>
#include <syslog.h>
#include <pthread.h>
#include <dlfcn.h>
#include <stdio.h>
#include <libgen.h>
#include <cpuid.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>

#include <string/s_str.h>
#include <string/unicode_utilities.h>
#include "os_adapter.h"

#define	LOCALE_DIR	"/usr/share/locale"

/*
 * Return the base path for the language catalog
 */
void get_locale_dir(COMMON_PATH locale_dir)
{
	s_strncpy(locale_dir, COMMON_PATH_LEN, LOCALE_DIR, COMMON_PATH_LEN);
}

char *get_cwd(COMMON_PATH buffer, size_t size)
{
	return getcwd(buffer, size);
}

/*
 * Logs a message in the operating system event log.
 */
void log_system_event(enum system_event_type type, const char *source, const char *message)
{
	openlog(source, LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	int priority;
	switch (type)
	{
		case SYSTEM_EVENT_TYPE_WARNING:
			priority = LOG_WARNING;
			break;
		case SYSTEM_EVENT_TYPE_ERROR:
			priority = LOG_ERR;
			break;
		case SYSTEM_EVENT_TYPE_DEBUG:
			priority = LOG_DEBUG;
			break;
		case SYSTEM_EVENT_TYPE_INFO:
		default:
			priority = LOG_INFO;
			break;
	}
	syslog(priority, "%s", message);
	closelog();
}
/*
 * Blocks for the specified number of msecs.
 */
void nvm_sleep(unsigned long time)
{
	unsigned long time_msec = time % 1000;
	struct timespec ts;
	ts.tv_sec = (time_t)((time - time_msec)/1000);
	ts.tv_nsec = (long)(time_msec * 1000000);
	nanosleep(&ts, NULL);
}

/*
 * Determine if the caller has permission to make changes to the system
 */
int check_admin_permissions()
{
	int rc = COMMON_ERR_INVALIDPERMISSIONS;

	// Running as superuser
	if (geteuid() == 0)
	{
		rc = COMMON_SUCCESS;
	}

	return rc;
}

/*
 * Start a process
 */
int start_process(const char *process_name, unsigned int *p_process_id)
{
	int rc = COMMON_ERR_FAILED;

	pid_t new_process = fork();
	if (new_process == 0)
	{
		// By normal conventions arg0 should point to the program being executed.
		execl(process_name, process_name, NULL);
	}
	else if (new_process > 0)
	{
		rc = COMMON_SUCCESS;
		*p_process_id = new_process;
	}
	return rc;
}

/*
 * Stop a process
 */
int stop_process(unsigned int process_id)
{
	int rc = COMMON_ERR_FAILED;
	if (kill(process_id, SIGKILL) == 0)
	{
		rc = COMMON_SUCCESS;
	}
	return rc;
}

/*
 * Create a thread on the current process
 */
void create_thread(COMMON_UINT64 *p_thread_id, void *(*callback)(void *), void *callback_arg)
{
	pthread_create(
			(pthread_t *)p_thread_id,
			NULL, // default attributes
			callback,
			callback_arg);
}

/*
 * Retrieve the id of the current thread
 */
COMMON_UINT64 get_thread_id()
{
	return pthread_self();
}

/*
 * Initializes a mutex.
 */
int mutex_init(OS_MUTEX *p_mutex, const char *name)
{
	int rc = 0;

	if (p_mutex)
	{
		// set attributes to make mutex reentrant (like windows implementation)
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

		// if named make it cross-process safe
		if (name)
		{
			pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
			// create a shared memory id
			int shmid = shmget(atoi(name), sizeof (pthread_mutex_t), IPC_CREAT | 0666);
			if (shmid != -1)
			{
				// attach to the shared memory
				pthread_mutex_t *p_tmp = (pthread_mutex_t *)shmat(shmid, NULL, 0);
				if (p_tmp)
				{
					memmove(p_mutex, p_tmp, sizeof (pthread_mutex_t));
				}
			}
		}

		// failure when pthread_mutex_init(..) != 0
		rc = (pthread_mutex_init((pthread_mutex_t *)p_mutex, &attr) == 0);
	}
	return rc;
}

/*
 * Locks the given mutex.
 */
int mutex_lock(OS_MUTEX *p_mutex)
{
	int rc = 0;
	if (p_mutex)
	{
		// failure when pthread_mutex_lock(..) != 0
		rc = (pthread_mutex_lock((pthread_mutex_t *)p_mutex) == 0);
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
		// failure when pthread_mutex_unlock(..) != 0
		rc = (pthread_mutex_unlock((pthread_mutex_t *)p_mutex) == 0);
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
		// failure when pthread_mutex_destroy(..) != 0
		rc = (pthread_mutex_destroy((pthread_mutex_t *)p_mutex) == 0);

		// detach the shared memory
		if (name)
		{
			shmdt(p_mutex);
		}
	}
	return rc;
}

/*
 * Initializes a rwlock
 */
int rwlock_init(OS_RWLOCK *p_rwlock)
{
	pthread_rwlock_t *p_handle = (pthread_rwlock_t *)p_rwlock;

	// failure when pthread_rwlock_init(..) != 0
	return ((pthread_rwlock_init(p_handle, NULL)) == 0);
}

/*
 * Applies a shared read-lock to the rwlock
 */
int rwlock_r_lock(OS_RWLOCK *p_rwlock)
{
	pthread_rwlock_t *p_handle = (pthread_rwlock_t *)p_rwlock;

	// failure when pthread_rwlock_rdlock(..) != 0
	return (pthread_rwlock_rdlock(p_handle) == 0);
}

/*
 * Unlocks an shared-read lock
 */
int rwlock_r_unlock(OS_RWLOCK *p_rwlock)
{
	// pthreads implements a single unlock function for
	// shared-read and exclusive-write locks
	return rwlock_w_unlock(p_rwlock);
}

/*
 * Applies an exclusive write-lock to the rwlock
 */
int rwlock_w_lock(OS_RWLOCK *p_rwlock)
{
	pthread_rwlock_t *p_handle = (pthread_rwlock_t *)p_rwlock;

	// failure when pthread_rwlock_wrlock(..) != 0
	return (pthread_rwlock_wrlock(p_handle) == 0);
}

/*
 * Unlocks an exclusive-write lock
 */
int rwlock_w_unlock(OS_RWLOCK *p_rwlock)
{
	pthread_rwlock_t *p_handle = (pthread_rwlock_t *)p_rwlock;

	// failure when pthread_rwlock_unlock(..) != 0
	return (pthread_rwlock_unlock(p_handle) == 0);
}

/*
 * Deletes the rwlock
 */
int rwlock_delete(OS_RWLOCK *p_rwlock)
{
	pthread_rwlock_t *p_handle = (pthread_rwlock_t *)p_rwlock;

	// failure when pthread_rwlock_destroy(..) != 0
	return (pthread_rwlock_destroy(p_handle) == 0);
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
		if (gethostname(name, name_len) != 0)
		{
			return COMMON_ERR_UNKNOWN;
		}
	}
	return rc;
}

/*
 * Retrieve the operating system name.
 */
int get_os_name(char *os_name, const COMMON_SIZE os_name_len)
{
	int rc = COMMON_SUCCESS;
	if (os_name == NULL || os_name_len == 0)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		// get the OS info
		struct utsname name;
		if (uname(&name) == -1)
		{
			rc = COMMON_ERR_UNKNOWN;
		}
		else
		{
			s_strcpy(os_name, name.sysname, os_name_len);
		}
	}
	return rc;
}

/*
 * Retrieve the operating system version as a string.
 */
int get_os_version(char *os_version, const COMMON_SIZE os_version_len)
{
	int rc = COMMON_SUCCESS;
	if (os_version == NULL || os_version_len == 0)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		// get the OS info
		struct utsname name;
		if (uname(&name) == -1)
		{
			rc = COMMON_ERR_UNKNOWN;
		}
		else
		{
			s_strcpy(os_version, name.release, os_version_len);
		}
	}
	return rc;
}

/*
 * Return the path to the installation directory
 */
void get_install_dir(COMMON_PATH install_dir)
{
	s_strcpy(install_dir, __PRODUCT_DATADIR__, COMMON_PATH_LEN);
}

/*
 * Create a new stream connected to a pipe running the given command
 */
FILE *common_popen(const char *cmd, const char *mode)
{
	return popen(cmd, mode);
}

/*
 * Close a stream opened by popen() and return the status of its child
 */
int common_pclose(FILE *p_fstream)
{
	return pclose(p_fstream);
}

/*
 * dynamically load a library
 */
void *dlib_load(const char *lib_path)
{
	return dlopen(lib_path, RTLD_LAZY);
}

/*
 * close a dynamically loaded library
 */
int dlib_close(void *handle)
{
	int rc = 0;
	if (handle)
	{
		rc = dlclose(handle);
	}
	return rc;
}

/*
 * .so for linux
 */
char *dlib_suffix(char *buffer, COMMON_SIZE buffer_len)
{
	s_strncpy(buffer, buffer_len, ".so", s_strnlen(".so", 4));
	return buffer;
}

/*
 * Get the symbol name from a loaded library
 */
void *dlib_find_symbol(void *handle, const char *symbol)
{
	return dlsym(handle, symbol);
}

int get_cpuid(unsigned int level, unsigned int *eax,
		unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
	return __get_cpuid(level, eax, ebx, ecx, edx);
}

void s_memset(void *ptr, size_t num)
{
	volatile unsigned char *p = ptr;
	while (num--) *p++ = 0;
}
