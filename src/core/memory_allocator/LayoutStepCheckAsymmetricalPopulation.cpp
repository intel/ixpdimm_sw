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
 * Add a warning if DIMMs are not optimally populated across each socket.
 */

#include "LayoutStepCheckAsymmetricalPopulation.h"

#include <LogEnterExit.h>

core::memory_allocator::LayoutStepCheckAsymmetricalPopulation::LayoutStepCheckAsymmetricalPopulation()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::LayoutStepCheckAsymmetricalPopulation::~LayoutStepCheckAsymmetricalPopulation()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::LayoutStepCheckAsymmetricalPopulation::execute(const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (request.memoryCapacity != 0 || request.appDirectExtents.size() > 0)
	{
		std::map<NVM_UINT16, std::vector<Dimm> > socketDimmsMap;
		for (std::vector<Dimm>::const_iterator dimmIter = request.dimms.begin();
				dimmIter != request.dimms.end(); dimmIter++)
		{
			socketDimmsMap[dimmIter->socket].push_back(*dimmIter);
		}

		for (std::map<NVM_UINT16, std::vector<Dimm> >::const_iterator socketIter = socketDimmsMap.begin();
				socketIter != socketDimmsMap.end(); socketIter++)
		{
			if (socketHasAsymmetricalSizedDimms(socketIter->second) ||
				socketHasAsymmetricalDimmPopulation(socketIter->second))
			{
				layout.warnings.push_back(LAYOUT_WARNING_NONOPTIMAL_POPULATION);
				break;
			}
		}
	}
}

bool core::memory_allocator::LayoutStepCheckAsymmetricalPopulation::socketHasAsymmetricalSizedDimms(
		const std::vector<Dimm> &dimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool asymmetrical = false;

	NVM_UINT64 size = 0;
	for (std::vector<Dimm>::const_iterator dimmIter = dimms.begin();
			dimmIter != dimms.end(); dimmIter++)
	{
		if (size == 0)
		{
			size = dimmIter->capacity;
		}
		else
		{
			if (dimmIter->capacity != size)
			{
				asymmetrical = true;
				break;
			}
		}
	}

	return asymmetrical;
}

bool core::memory_allocator::LayoutStepCheckAsymmetricalPopulation::socketHasAsymmetricalDimmPopulation(
		const std::vector<Dimm> &dimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool asymmetrical = false;

	std::map<NVM_UINT16, std::vector<Dimm> > channelMap;
	for (std::vector<Dimm>::const_iterator dimmIter = dimms.begin();
			dimmIter != dimms.end(); dimmIter++)
	{
		channelMap[(dimmIter->channel % core::memory_allocator::CHANNELS_PER_IMC)].push_back(*dimmIter);
	}

	for (std::map<NVM_UINT16, std::vector<Dimm> >::const_iterator channelIter = channelMap.begin();
			channelIter != channelMap.end(); channelIter++)
	{
		if (channelIter->second.size() == 1)
		{
			asymmetrical = true;
			break;
		}
	}

	return asymmetrical;
}

