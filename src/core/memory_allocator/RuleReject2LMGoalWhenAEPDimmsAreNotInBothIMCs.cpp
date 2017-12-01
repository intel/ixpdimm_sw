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
 * Rule that checks if user requested 2LM and not allow the goal to be created if there is no AEP DIMM in one of the IMCs
 */


#include "RuleReject2LMGoalWhenAEPDimmsAreNotInBothIMCs.h"



#include <core/exceptions/NvmExceptionBadRequest.h>
#include <LogEnterExit.h>


core::memory_allocator::RuleReject2LMGoalWhenAEPDimmsAreNotInBothIMCs::RuleReject2LMGoalWhenAEPDimmsAreNotInBothIMCs(
		const std::vector<struct device_discovery> manageableDeviceDiscoveryList)
			: m_manageableDeviceDiscoveryList(manageableDeviceDiscoveryList)

{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::RuleReject2LMGoalWhenAEPDimmsAreNotInBothIMCs::~RuleReject2LMGoalWhenAEPDimmsAreNotInBothIMCs()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

/**
 * Check if each iMC within a socket has at least one AEP DIMM. Do not allow the goal to be created in 2LM otherwise.
 *
 * @param[in] args - args[1]: MemoryAllocationRequest object
 **/
void core::memory_allocator::RuleReject2LMGoalWhenAEPDimmsAreNotInBothIMCs::verify(const MemoryAllocationRequest &request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_BOOL isValidGoal = NVM_FALSE;

	if (0 != request.getMemoryModeCapacityGiB())
	{
		std::list<NVM_UINT16> socketList = getRequestedSockets(request.getDimms());

		for (std::list<NVM_UINT16>::iterator iter = socketList.begin(); iter != socketList.end(); iter++)
		{
			isValidGoal = checkSocketHasAEPDimmOnBothIMCs(*iter);

			if (!isValidGoal)
			{
				throw core::NvmExceptionBadRequestNoAEPInOneOfTheiMCs();
			}
		}
	}
}

/**
 * Check if each iMC has at least one AEP DIMM. Do not allow the goal to be created in 2LM otherwise
 *
 * @param[in] args - args[1]: SocketId
 *
 * @param[out] - True if both iMCs are populated with AEP Dimms otherwise, return false
 **/
NVM_BOOL core::memory_allocator::RuleReject2LMGoalWhenAEPDimmsAreNotInBothIMCs::checkSocketHasAEPDimmOnBothIMCs(NVM_UINT16 socketId)
{
	NVM_BOOL eachIMCHasAtleastOneAEP = NVM_FALSE;
	std::set<NVM_UINT16> uniqueControllerIdsSet;

	for (std::vector<struct device_discovery>::const_iterator iter = m_manageableDeviceDiscoveryList.begin();
			iter != m_manageableDeviceDiscoveryList.end(); iter++)
	{
		if (iter->socket_id == socketId)
		{
			uniqueControllerIdsSet.insert(iter->memory_controller_id);
		}
	}
	eachIMCHasAtleastOneAEP = (uniqueControllerIdsSet.size() == NVM_MAX_IMCS_PER_SOCKET) ? NVM_TRUE : NVM_FALSE;

	return eachIMCHasAtleastOneAEP;
}
