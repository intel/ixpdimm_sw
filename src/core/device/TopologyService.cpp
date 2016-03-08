/*
 * Copyright (c) 2016, Intel Corporation
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

#include "TopologyService.h"
#include <core/NvmApi.h>
#include <core/exceptions/NoMemoryException.h>


core::device::TopologyService *core::device::TopologyService::m_pSingleton =
		new core::device::TopologyService();

core::device::TopologyService &core::device::TopologyService::getService()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (!m_pSingleton)
	{
		throw NoMemoryException();
	}

	return *m_pSingleton;
}

core::device::TopologyCollection core::device::TopologyService::getAllTopologies()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	TopologyCollection result;

	int tc = m_pApi.getMemoryTopologyCount();
	if (tc < 0)
	{
		throw LibraryException(tc);
	}

	int dc = m_pApi.getDeviceCount();
	if (dc < 0)
        {
                throw LibraryException(dc);
        }

	device_discovery devices[dc];
	memory_topology mem_topology[tc];

	int topology_count = tc;
	tc = m_pApi.getMemoryTopology(mem_topology, topology_count);
	if (tc < 0)
	{
		throw LibraryException(tc);
	}

	int devices_count = dc;
	dc = m_pApi.getDevices(devices, devices_count);
	if (dc < 0)
        {
                throw LibraryException(dc);
        }

	for (int i = 0; i < topology_count; i++)
	{
		device_discovery device;
		for (int j = 0; j < devices_count; j++)
		{
			if (devices[j].physical_id == mem_topology[i].physical_id)
			{
				device = devices[j];
				break;
			}
		}
		Topology topology(mem_topology[i], device);
		result.push_back(topology);
	}

	return result;
}
