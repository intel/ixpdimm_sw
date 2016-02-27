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

	int rc = m_pApi.getMemoryTopologyCount();

	if (rc < 0)
	{
		throw LibraryException(rc);
	}

	int count = rc;
	memory_topology mem_topology[count];

	rc = m_pApi.getMemoryTopology(mem_topology, count);
	if (rc < 0)
	{
		throw LibraryException(rc);
	}

	for (int i = 0; i < count; i++)
	{
		Topology topology(mem_topology[i]);
		result.push_back(topology);
	}

	return result;
}
