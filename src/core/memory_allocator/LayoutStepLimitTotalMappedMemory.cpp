/*
 * Copyright (c) 2017 Intel Corporation
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
 * Reduce laid out capacity based on SKU mapped memory limit
 */

#include "LayoutStepLimitTotalMappedMemory.h"
#include <LogEnterExit.h>
#include <core/memory_allocator/LayoutStepAppDirect.h>
#include <core/memory_allocator/LayoutStepMemory.h>
#include <math.h>


core::memory_allocator::LayoutStepLimitTotalMappedMemory::LayoutStepLimitTotalMappedMemory(const std::vector<struct device_details> deviceDetailsList,
		MemoryAllocationUtil &util) :
		m_newTotalMappedSizeOnSocketInGib(0), m_limitOnSocketInGib(0), m_exceededSocketLimitByInGib(0), m_memAllocUtil(util),
	 m_DeviceDetailsList(deviceDetailsList), m_curSocketIdDeviceDetailsMap(), m_reqSocketIdConfigGoalMap(), m_curSocketInfoFromTyep6Map()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::LayoutStepLimitTotalMappedMemory::~LayoutStepLimitTotalMappedMemory()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::execute(
	const MemoryAllocationRequest& request, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool needLayoutChange = false;
	std::vector<Dimm> reqDimms = request.getDimms();

	initCurrentDeviceDetailsMap(request); // initialize a map of socketId and devicedetails of all the manageable devices in the system
	initReqConfigGoalMap(request, layout, reqDimms);	 // this is a map of socketId and the requested dimms config_goal that is calculated based on the request
	initPcatType6info(); // this is a map of socketId and socket info from socket struct

	for (std::map<NVM_UINT16, struct socket>::iterator socketInfo = m_curSocketInfoFromTyep6Map.begin();
			socketInfo != m_curSocketInfoFromTyep6Map.end(); socketInfo++)
	{
		initSocketDimms(socketInfo->first, reqDimms);
		calculateTotalMappedCapacityPerSocket(socketInfo->first, m_curSocketIdDeviceDetailsMap[socketInfo->first],
							socketInfo->second, m_reqSocketIdConfigGoalMap[socketInfo->first], reqDimms);
		if (mappedSizeExceedsLimit())
		{
			needLayoutChange = true;
			initializeExceedsLimit();
			shrinkLayoutGoals(layout);
		}
	}

	if (needLayoutChange)
	{
		shrinkLayoutCapacities(request, layout);
		layout.warnings.push_back(LAYOUT_WARNING_SKU_MAPPED_MEMORY_LIMITED);
	}

	layout.remainingCapacity = B_TO_GiB(getRemainingBytesFromRequestedDimms(request, layout));
}

/*Capacity skuing rules
 * -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | Previous configuration	|	Requested configuration (User requests WHOLE SOCKET)					|	Requested configuration(User requests only the unconfigured dimms)
   |
   |	1 LM + AD			|			1 LM + AD														|			1 LM + AD
   |								TotalNewMappedCapacity = CurrentSPA - CurrentAD + RequestedTotalAD			TotalNewMappedCapacity = CurrentSPA - currentADOfRequestedDimms+ requestedTotalAD
   |						|																			|
   |
 * |--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |						|																			|
 * |							2 LM + AD																				2LM + AD (combination of AD and volatile could be  requested example MEMORYMODE=50)
 * |	1 LM + AD 			|	TotalNewMappedCapacity = RequestedVolatileCapacity + RequestedAD		|
   |																												TotalNewMappedCapacity = RequestedVolatileCapacity + RequestedAD + currentAD
 * |						|																			|
 * |						|																			|
 * |------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * 							|																			|
 * |							1 LM + AD																				1LM + AD
 * |	2LM + AD			|	TotalNewMappedCapacity =  RequestedAD + DDR4Capacity					|
   |																												TotalNewMappedCapacity =  RequestedAD + currentAD + DDR4Capacity
 * |						|																			|
 * |-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * */
void core::memory_allocator::LayoutStepLimitTotalMappedMemory::calculateTotalMappedCapacityPerSocket
					(NVM_UINT16 socketId, std::vector<struct device_details> curDeviceDetailsList,
							struct socket sktType6Info, std::vector<struct config_goal> reqConfigGoalList, std::vector<Dimm> reqDimms)
{
	NVM_UINT64 totalReqSizeInGib = 0;
	NVM_UINT64 totalCurrSizeInB = 0;
	m_newTotalMappedSizeOnSocketInGib = 0; // re-initialize for each socket
	m_limitOnSocketInGib = B_TO_GiB(sktType6Info.mapped_memory_limit);

	bool isUserReqWholeSocket = (curDeviceDetailsList.size() == reqConfigGoalList.size()) ? true : false;
	bool isCurrentConfigurationMemMode = false;
	bool isReqConfigurationMemMode = false;

	for (std::vector<struct config_goal>::iterator reqGoal = reqConfigGoalList.begin();
				reqGoal != reqConfigGoalList.end(); reqGoal++)
	{
		// requested configuration memory mode?
		if (reqGoal->memory_size > 0)
		{
			isReqConfigurationMemMode = true;
		}

		// calculate TotalRequestedMappedMemory based on the requested dimms alone
		totalReqSizeInGib += reqGoal->memory_size;
		totalReqSizeInGib += reqGoal->app_direct_1_size;
		totalReqSizeInGib += reqGoal->app_direct_2_size;
	}

	// currently in memory mode?
	for (std::vector<struct device_details>::iterator curDeviceDetail = curDeviceDetailsList.begin();
			curDeviceDetail != curDeviceDetailsList.end(); curDeviceDetail++)
	{
		if (curDeviceDetail->capacities.memory_capacity > 0)
		{
			isCurrentConfigurationMemMode = true;
			// the user might have requested goal for one dimm instead of whole socket, consider the dimms in current config as well for isReqmemMode
			if (!isReqConfigurationMemMode && !isUserReqWholeSocket &&
					curDeviceDetail->status.is_configured)
			{
				isReqConfigurationMemMode = true;
				break;
			}
		}
	}

	if ((isCurrentConfigurationMemMode) && (0 == sktType6Info.total_2lm_ddr_cache_memory))
	{
		COMMON_LOG_ERROR_F("Cached memory capacity is 0 when in 2LM for socketId %d", socketId);
		throw core::LibraryException(NVM_ERR_BADPCAT);
	}

	// 1 LM + AD -> 1 LM + AD
	if (!isCurrentConfigurationMemMode && !isReqConfigurationMemMode)
	{
		totalCurrSizeInB += sktType6Info.total_mapped_memory; // get the ddr4 capacity from totalSPA and subtract out all the appdirect capacity

		for (std::vector<device_details>::iterator curDeviceDetail = curDeviceDetailsList.begin();
					curDeviceDetail != curDeviceDetailsList.end(); curDeviceDetail++)
		{
			if (isUserReqWholeSocket)
			{
				totalCurrSizeInB -= curDeviceDetail->capacities.app_direct_capacity;
			}
			else if (!curDeviceDetail->status.is_configured)
			{
				// addressing the corner case where a new dimm could be configured as AD
				totalCurrSizeInB -= curDeviceDetail->capacities.app_direct_capacity;
			}
		}
	}
	else if (isCurrentConfigurationMemMode && !isReqConfigurationMemMode) // 2LM + AD -> 1LM + AD
	{
		// cached memory will now become mapped memory
		totalCurrSizeInB += sktType6Info.total_2lm_ddr_cache_memory;

		if (!isUserReqWholeSocket)
		{
			for (std::vector<struct device_details>::iterator curDeviceDetail = curDeviceDetailsList.begin();
					curDeviceDetail !=  curDeviceDetailsList.end(); curDeviceDetail++)
			{
				if (curDeviceDetail->status.is_configured)
				{
					totalCurrSizeInB += curDeviceDetail->capacities.app_direct_capacity;
				}
			}
		}
	}
	else if (isReqConfigurationMemMode && !isUserReqWholeSocket) // 2LM + AD -> 2LM +AD or 1LM +AD -> 2LM + AD
	{
		for (std::vector<device_details>::iterator curDeviceDetail = curDeviceDetailsList.begin();
				curDeviceDetail != curDeviceDetailsList.end();
				curDeviceDetail++)
		{
			if (curDeviceDetail->status.is_configured)
			{
				totalCurrSizeInB += curDeviceDetail->capacities.app_direct_capacity;
				totalCurrSizeInB += curDeviceDetail->capacities.memory_capacity; // this will be 0 if currently in 1 LM + AD
			}
		}
	}

	m_newTotalMappedSizeOnSocketInGib = B_TO_GiB(totalCurrSizeInB) + totalReqSizeInGib;
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkLayoutGoals(
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	shrinkAppDirect2(layout);

	shrinkAppDirect1(layout);

	shrinkMemory(layout);
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkAppDirect2(
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	shrinkAD2(m_socketDimms, m_exceededSocketLimitByInGib, layout);
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkAppDirect1(
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	shrinkAD1(m_socketDimms, m_exceededSocketLimitByInGib, layout);
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkMemory(
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_exceededSocketLimitByInGib > 0)
	{
		std::vector<core::memory_allocator::Dimm> memDimms = get2LMDimms(m_socketDimms, layout);
		if (!memDimms.empty())
		{
			// If limit exceeds 2LM, eliminate all 2LM, else shrink evenly on all 2LM DIMMs
			NVM_UINT64 total2LMapacity = getTotal2LMCapacity(memDimms, layout);
			if (m_exceededSocketLimitByInGib >= total2LMapacity)
			{
				killAllCapacityByType(memDimms, layout, CAPACITY_TYPE_2LM);

				m_exceededSocketLimitByInGib =
					m_exceededSocketLimitByInGib - total2LMapacity;
			}
			else
			{
				NVM_UINT64 reduceBy = calculateCapacityToShrinkPerDimm(m_exceededSocketLimitByInGib, memDimms.size());

				for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
						memDimms.begin(); dimm != memDimms.end(); dimm++)
				{
					if (m_exceededSocketLimitByInGib > 0)
					{
						if (layout.goals.find(dimm->uid) != layout.goals.end())
						{
							config_goal& goal = layout.goals[dimm->uid];

							shrinkSizePerDimm(reduceBy, goal.memory_size);
						}
					}
				}
			}
		}
	}
}

bool core::memory_allocator::LayoutStepLimitTotalMappedMemory::mappedSizeExceedsLimit()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_newTotalMappedSizeOnSocketInGib > m_limitOnSocketInGib;
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::initializeExceedsLimit()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_exceededSocketLimitByInGib = m_newTotalMappedSizeOnSocketInGib - m_limitOnSocketInGib;
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkLayoutCapacities(
		const MemoryAllocationRequest& request,	MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 memoryCapacity = 0,  appDirectCapacity = 0, totalCapacity = 0;
	std::vector<core::memory_allocator::Dimm> dimms = request.getDimms();
	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			dimms.begin(); dimm != dimms.end(); dimm++)
	{
		memoryCapacity += layout.goals[dimm->uid].memory_size;
		appDirectCapacity +=
			layout.goals[dimm->uid].app_direct_1_size + layout.goals[dimm->uid].app_direct_2_size;
		totalCapacity += B_TO_GiB(dimm->capacityBytes);
	}

	layout.memoryCapacity = memoryCapacity;
	layout.appDirectCapacity = appDirectCapacity;
}

NVM_UINT64 core::memory_allocator::LayoutStepLimitTotalMappedMemory::getTotal2LMCapacity(
		const std::vector<core::memory_allocator::Dimm>& dimms,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 total2LMCapacity = 0;

	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			dimms.begin(); dimm != dimms.end(); dimm++)
	{
		total2LMCapacity += layout.goals[dimm->uid].memory_size;
	}

	return total2LMCapacity;
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkSizePerDimm(
		NVM_UINT64 reduceBy, NVM_UINT64 &size)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	shrinkSize(m_exceededSocketLimitByInGib, reduceBy, size);
}


// Initialize m_currDeviceDetailsMap - a map of socketId and device_details of all the dimms with  that socket Id
void core::memory_allocator::LayoutStepLimitTotalMappedMemory::initCurrentDeviceDetailsMap
			(const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<device_details>::iterator deviceDetails = m_DeviceDetailsList.begin();
			deviceDetails != m_DeviceDetailsList.end(); deviceDetails++)
	{
		m_curSocketIdDeviceDetailsMap[deviceDetails->discovery.socket_id].push_back(*deviceDetails);

	}
}

// layout.goals does not have socketId which is important for the limit calc so create a map of socketId and config_goals
void core::memory_allocator::LayoutStepLimitTotalMappedMemory::initReqConfigGoalMap
			(const MemoryAllocationRequest& request, const MemoryAllocationLayout& layout, const std::vector<Dimm> reqDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<Dimm>::const_iterator reqDimm = reqDimms.begin(); reqDimm != reqDimms.end(); reqDimm++)
	{
		for (std::map<std::string, struct config_goal>::const_iterator goal = layout.goals.begin();
				goal != layout.goals.end(); goal++)
		{
			if (uid_cmp(goal->first.c_str(), reqDimm->uid.c_str()))
			{
				m_reqSocketIdConfigGoalMap[reqDimm->socket].push_back(goal->second);
			}
		}
	}
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::initPcatType6info()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	for (std::map<NVM_UINT16, std::vector<struct config_goal> >::iterator goal = m_reqSocketIdConfigGoalMap.begin(); goal != m_reqSocketIdConfigGoalMap.end(); goal++)
	{
		struct socket skt = m_memAllocUtil.getSocket(goal->first);
		if (skt.is_capacity_skuing_supported)
		{
			m_curSocketInfoFromTyep6Map[goal->first] = skt;
		}
	}

}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::initSocketDimms(NVM_UINT16 socketId, std::vector<Dimm> reqDimms)
{
	m_socketDimms.clear();
	for (std::vector<Dimm>::const_iterator reqDimm = reqDimms.begin(); reqDimm != reqDimms.end(); reqDimm++)
	{
		if (reqDimm->socket == socketId)
		{
			m_socketDimms.push_back(*reqDimm);
		}
	}
}
