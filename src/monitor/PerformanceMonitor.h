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
 * This file contains the definition of the performance monitoring class
 * of the NvmMonitor service which periodically polls and stores performance metrics
 * for each manageable NVM-DIMM in the system.
 */

#include "NvmMonitorBase.h"
#include <nvm_management.h>
#include <persistence/schema.h>

#ifndef _MONITOR_PERFORMANCEMONITOR_H_
#define _MONITOR_PERFORMANCEMONITOR_H_


namespace monitor
{
	static const std::string PERFORMANCE_MONITOR_NAME = "PERFORMANCE";
	static const std::string PERFORMANCE_TABLE_NAME = "performance";

	/*
	 * Monitor class to periodically poll and store performance metrics for
	 * each manageable NVM-DIMM in the system.
	 */
	class PerformanceMonitor : public NvmMonitorBase
	{
		public:
			PerformanceMonitor();
			virtual ~PerformanceMonitor();
			virtual void monitor();

		private:
			std::vector<std::string> getDimmList();
			bool storeDimmPerformanceData(const std::string &dimmUidStr, struct device_performance &performance);
			void trimPerformanceData();
			PersistentStore *m_pStore;
	};
}

#endif /* _MONITOR_PERFORMANCEMONITOR_H_ */
