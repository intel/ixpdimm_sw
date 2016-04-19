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
 * This file contains the implementation of the NvmMonitor service.
 */


#include <string>
#include <LogEnterExit.h>
#include "NvmMonitorBase.h"
#include "PerformanceMonitor.h"
#include "EventMonitor.h"

/*
 * Constructor
 * param:  name - used to look up monitor configurations in the config database.
 * 		If config keys aren't find, then default values are used
 */
monitor::NvmMonitorBase::NvmMonitorBase(std::string const &name)
		: m_name(name)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// get values from Database
	std::string intervalKey = m_name + MONITOR_INTERVAL_SUFFIX_KEY;
	std::string enabledKey = m_name + MONITOR_ENABLED_SUFFIX_KEY;

	m_intervalSeconds = DEFAULT_INTERVAL_SECONDS;
	m_enabled = DEFAULT_MONITOR_ENABLED;

	int configResult;
	if (get_bounded_config_value_int(intervalKey.c_str(), &configResult) == COMMON_SUCCESS)
	{
		m_intervalSeconds = (size_t) configResult;
	}

	if (get_config_value_int(enabledKey.c_str(), &configResult) == COMMON_SUCCESS)
	{
		m_enabled = configResult != 0;
	}
}

monitor::NvmMonitorBase::~NvmMonitorBase()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void monitor::NvmMonitorBase::getMonitors(std::vector<monitor::NvmMonitorBase *> &monitors)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	EventMonitor *event = new EventMonitor();
	if (event && event->isEnabled())
	{
		monitors.push_back(event);
	}
	else if (event)
	{
		delete event;
	}

	PerformanceMonitor *performance = new PerformanceMonitor();
	if (performance && performance->isEnabled())
	{
		monitors.push_back(performance);
	}
	else
	{
		delete performance;
	}
}

/*
 * free the memory allocated for the monitors
 */
void monitor::NvmMonitorBase::deleteMonitors(std::vector<NvmMonitorBase *> &monitors)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	for(size_t m = 0; m < monitors.size(); m++)
	{
		delete monitors[m];
	}
}

/*
 * Getters for private fields
 */

std::string const &monitor::NvmMonitorBase::getName() const
{
	return m_name;
}

size_t monitor::NvmMonitorBase::getIntervalSeconds() const
{
	return m_intervalSeconds;
}

bool monitor::NvmMonitorBase::isEnabled() const
{
	return m_enabled;
}
