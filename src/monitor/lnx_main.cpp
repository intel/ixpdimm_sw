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
 * This file contains the entry point for the NvmMonitor service on Linux.
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include<pthread.h>
#include <time.h>
#include<signal.h>

#include "NvmMonitorBase.h"

#define PID_FILE_NAME "/var/run/ixpdimm-monitor.pid"

int setupDaemon();
void signalHandler(int sigNum);
void *worker(void *arg);

// flag set when a signal is received to exit the service
bool keepRunning = true;

int main(int argc, char **argv)
{
	int rc = EXIT_SUCCESS;

	// Register interupt signals with the signalHandler
	signal(SIGINT, signalHandler);

	if (argc == 2)
	{
		std::string argOne = argv[1];
		if (argOne == "-d")
		{
			rc = setupDaemon();
		}
	}

	if (rc == EXIT_SUCCESS)
	{
		std::vector<monitor::NvmMonitorBase *> monitors;
		monitor::NvmMonitorBase::getMonitors(monitors);

		size_t threadCount = monitors.size();

		pthread_t threads[monitors.size()];
		for (size_t t = 0; t < threadCount; t++)
		{
			pthread_create(threads + t, NULL, &worker, (void *) (monitors[t]));
		}

		while (keepRunning)
		{
			// Keep the process running until signaled to close
			sleep(1);
		}

		// make sure each thread finishes before exiting
		for (size_t t = 0; t < threadCount; t++)
		{
			int i;
			pthread_join(threads[t], (void **)&i);
		}

		// clean up
		monitor::NvmMonitorBase::deleteMonitors(monitors);
	}

	return rc;
}

/*
 * Catch signals
 */
void signalHandler(int sigNum)
{
	// signal it's time to quit
	keepRunning = false;
}

/*
 * Wait until the timeout elapses or the process is signaled to quit
 */
bool timeToQuit(unsigned long timeoutSeconds)
{
	time_t beg = time(NULL);
	time_t cur;
	unsigned long elapsedSeconds = 0;
	while (keepRunning && elapsedSeconds < timeoutSeconds)
	{
		cur = time(NULL);
		elapsedSeconds = (unsigned long)difftime(cur, beg);
		sleep(1);
	}

	return !keepRunning;
}

/*
 * worker thread. Loops until it's time to quit, calling the monitor function.
 */
void *worker(void *arg)
{
	if (arg != NULL)
	{
		monitor::NvmMonitorBase *callback = (monitor::NvmMonitorBase *)arg;
		callback->init();

		while (!timeToQuit(callback->getIntervalSeconds()))
		{
			callback->monitor();
		}

		callback->cleanup();
	}
	return NULL;
}

/*
 * Writes the PID to a given file
 */
int write_pid_file(const char *pid_file, pid_t pid)
{
	int rc = EXIT_SUCCESS;

	int fd;

	fd = open(pid_file, O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC | O_TRUNC, S_IRUSR | S_IWUSR);

	if (fd < 0)
	{
		rc = EXIT_FAILURE;
	}
	else
	{
		dprintf(fd, "%ld", (long)pid);
		close(fd);
	}

	return rc;
}

/*
 * Setup the process as a background daemon
 */
int setupDaemon()
{
	int rc = EXIT_SUCCESS;

	/* Our process ID and Session ID */
	pid_t pid, sid;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0)
	{
		rc = EXIT_FAILURE;
	}
	else
	{
		/* If we got a good PID, then
		   we can exit the parent process. */
		if (pid > 0)
		{
			rc = write_pid_file(PID_FILE_NAME, pid);
			_exit(EXIT_SUCCESS);
		}
		else
		{
			/* Change the file mode mask */
			umask(0);

			/* Open any logs here */

			/* Create a new SID for the child process */
			sid = setsid();
			if (sid < 0)
			{
				/* Log the failure */
				rc = EXIT_FAILURE;
			}
			else
			{
				/* Change the current working directory */
				if ((chdir("/")) < 0)
				{
					/* Log the failure */
					rc = EXIT_FAILURE;
				}
				else
				{
					/* Close out the standard file descriptors */
					close(STDIN_FILENO);
					close(STDOUT_FILENO);
					close(STDERR_FILENO);
				}
			}
		}
	}
	return rc;
}
