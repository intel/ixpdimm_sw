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
 * This file contains the definitino of the NvmMonitor service.
 */

#include <vector>
#include <persistence/lib_persistence.h>
#include <sstream>
#include <os/os_adapter.h>
#include <persistence/logging.h>
#include <nvm_types.h>

#ifndef _MONITOR_NVMMONITOR_H_
#define _MONITOR_NVMMONITOR_H_

namespace monitor
{
	static const std::string MONITOR_INTERVAL_SUFFIX_KEY = "_MONITOR_INTERVAL_SECONDS";
	static const std::string MONITOR_ENABLED_SUFFIX_KEY = "_MONITOR_ENABLED";
	static const size_t DEFAULT_INTERVAL_SECONDS = 60;
	static const bool DEFAULT_MONITOR_ENABLED = true;

	/*
	 * Base class for NvmMonitors.
	 * Note: Any class inheriting from NvmMonitorBase should add the appropriate
	 * entries to the apss.dat DB by adding to the create_default_config(char *path)
	 * function in lib_persistence.c. Also see MONITOR_INTERVAL_SUFFIX_KEY,
	 * MONITOR_ENABLED_SUFFIX_KEY, and the NvmMonitorBase constructor for reference.
	 */
	class NVM_API NvmMonitorBase
	{

	public:
		/*
		 * Main Constructor
		 */
		NvmMonitorBase(std::string const &name);

		virtual ~NvmMonitorBase();

		virtual void init() {}
		virtual void monitor() = 0;
		virtual void cleanup() {}

		std::string const & getName() const;

		size_t getIntervalSeconds() const;

		bool isEnabled() const;

		/*
		 * Static function to get the collection of enabled monitors
		 */
		static void getMonitors(std::vector<NvmMonitorBase *> &monitors);
		static void deleteMonitors(std::vector<NvmMonitorBase *> &monitors);

	protected:
		std::string m_name;
		size_t m_intervalSeconds;
		bool m_enabled;

	};



};

#endif /* _MONITOR_NVMMONITOR_H_ */
