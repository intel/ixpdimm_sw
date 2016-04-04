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
 * This file contains the NvmMonitor entry point from the CMPI provider.
 */

#include "cmpiMonitor.h"
#include <LogEnterExit.h>
#include <monitor/NvmMonitorBase.h>
#include <signal.h>
#include <unistd.h>
#include <vector>

void *worker(void *arg); // background worker
bool timeToQuit(unsigned long timeoutSeconds); // determine when to quit thread
void signalHandler(int sigNum); // handle interupt signals

bool keepRunning = false; // used to signal when to stop threads
bool isRunning = false; // used to track when running
size_t g_threadCount = 0; // number of monitor threads
pthread_t *g_pThreads = NULL; // array of monitor threads
std::vector<monitor::NvmMonitorBase *> g_monitors;

/*
 * Start the monitor threads
 */
void cmpiMonitor::Init()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// only want one instance running, even if multiple instances of the provider
	// are initialized
	if (isRunning == false)
	{
		signal(SIGINT, signalHandler); // Ctrl-C
		signal(SIGUSR1, signalHandler); // Sent from SFCB

		keepRunning = true;
		monitor::NvmMonitorBase::getMonitors(g_monitors);

		g_threadCount = g_monitors.size();

		g_pThreads = new pthread_t[g_threadCount];// (pthread_t *)malloc(sizeof (pthread_t) * g_threadCount);
		for (size_t t = 0; t < g_threadCount; t++)
		{
			pthread_create(g_pThreads + t, NULL, &worker, (void *) (g_monitors[t]));
		}
		isRunning = true;
	}
}

/*
 * Cleanup the monitor threads
 */
void cmpiMonitor::Cleanup()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (isRunning)
	{
		// signal threads to end
		keepRunning = false;
		// make sure each thread finishes before exiting
		for (size_t t = 0; t < g_threadCount; t++)
		{
			int i;
			pthread_join(g_pThreads[t], (void **)&i);
		}

		if (g_pThreads)
		{
			delete g_pThreads;
			g_pThreads = NULL;
		}

		// clean up
		monitor::NvmMonitorBase::deleteMonitors(g_monitors);

		isRunning = false;
		log_gather();
	}
}

/*
 * -----------------------------------------------------------------------------
 * "Private" Helper functions
 * -----------------------------------------------------------------------------
 *
 */
/*
 * Catch signals
 */
void signalHandler(int sigNum)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// signal it's time to quit
	cmpiMonitor::Cleanup();
}

/*
 * Wait until the timeout elapses or the process is signaled to quit
 */
bool timeToQuit(unsigned long timeoutSeconds)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
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
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
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

