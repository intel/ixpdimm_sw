/*
 * Copyright (c) 2015, Intel Corporation
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
 * Lay out the persistent extent.
 */

#include <logic/LayoutStepPersistent.h>
#include <exception/NvmExceptionBadRequest.h>
#include <LogEnterExit.h>
#include <utility.h>
#include "MemoryAllocationUtil.h"

#define	DIMM_LOCATION(iMC, channel) 2 * (channel % CHANNELS_PER_IMC) + iMC
#define	DIMM_POPULATED(map, dimmIndex) ((map >> dimmIndex) & 0b1)

wbem::logic::LayoutStepPersistent::LayoutStepPersistent(
		const struct nvm_capabilities &cap,
		const int pmExtentIndex,
		lib_interface::NvmApi *pApi)
	: m_systemCap(cap), m_pmExtentIndex(pmExtentIndex), m_pLibApi(pApi)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!m_pLibApi)
	{
		m_pLibApi = lib_interface::NvmApi::getApi();
	}
}

wbem::logic::LayoutStepPersistent::~LayoutStepPersistent()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

bool wbem::logic::LayoutStepPersistent::isRemainingStep(const MemoryAllocationRequest &request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool isRemaining = false;
	if (request.persistentExtents.size() > m_pmExtentIndex)
	{
		isRemaining = request.persistentExtents[m_pmExtentIndex].capacity == REQUEST_REMAINING_CAPACITY;
	}
	return isRemaining;
}

void wbem::logic::LayoutStepPersistent::execute(const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (request.persistentExtents.size() > m_pmExtentIndex)
	{
		std::vector<Dimm> dimmsToLayout;
		for (std::vector<Dimm>::const_iterator dimmIter = request.dimms.begin();
				dimmIter != request.dimms.end(); dimmIter++)
		{
			if (layout.reserveDimmGuid != dimmIter->guid)
			{
				dimmsToLayout.push_back(*dimmIter);
				m_dimmExistingSets[dimmIter->guid] =
						layout.goals[dimmIter->guid].persistent_count;
			}
		}

		NVM_UINT64 bytesToAllocate = getRequestedCapacityBytes(
				request.persistentExtents[m_pmExtentIndex].capacity, request, layout);
		if (bytesToAllocate)
		{
			NVM_UINT64 bytesRemaining = bytesToAllocate;
			while (bytesRemaining)
			{
				try
				{
					if (request.persistentExtents[m_pmExtentIndex].byOne)
					{
						bytesRemaining = layoutByOnePm(bytesRemaining, dimmsToLayout, request, layout);
					}
					else
					{
						bytesRemaining = layoutInterleavedPm(bytesRemaining, dimmsToLayout, request, layout);
					}
				}
				catch (exception::NvmExceptionBadRequestSize &e)
				{
					// out of capacity, clean up and pass along the exception
					layout.persistentCapacities.push_back(bytesToConfigGoalSize(bytesToAllocate - bytesRemaining));
					throw e;
				}
			}
			layout.persistentCapacities.push_back(bytesToConfigGoalSize(bytesToAllocate - bytesRemaining));
		}
	}
}

NVM_UINT64 wbem::logic::LayoutStepPersistent::layoutByOnePm(
		const NVM_UINT64 &bytesToAllocate,
		const std::vector<Dimm> &dimms,
		const wbem::logic::MemoryAllocationRequest &request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// iterate over each dimm and create a x1 iset on each one
	NVM_UINT64 bytesAllocated = 0;
	std::vector<Dimm> dimmsIncluded;
	NVM_UINT64 bytesPerDimm = getLargestPerDimmSymmetricalBytes(
			dimms,
			layout.goals,
			bytesToAllocate,
			dimmsIncluded);


	for (std::vector<Dimm>::const_iterator dimmIter = dimmsIncluded.begin();
			dimmIter != dimmsIncluded.end(); dimmIter++)
	{
		std::vector<Dimm> byOneDimmList;
		byOneDimmList.push_back(*dimmIter);
		bytesAllocated += layoutInterleaveSet(bytesPerDimm, byOneDimmList, request, layout);
	}

	// unable to map anything
	if (bytesAllocated == 0)
	{
		throw exception::NvmExceptionBadRequestSize();
	}
	return (bytesToAllocate - bytesAllocated);
}

NVM_UINT64 wbem::logic::LayoutStepPersistent::layoutInterleavedPm(
		const NVM_UINT64 &bytesToAllocate,
		const std::vector<wbem::logic::Dimm> &dimms,
		const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytesAllocated = 0;
	// create a map of socket to dimm lists
	std::map<NVM_UINT16, std::vector<Dimm> >sockets;
	for (std::vector<Dimm>::const_iterator dimmIter = dimms.begin();
						dimmIter != dimms.end(); dimmIter++)
	{
		sockets[dimmIter->socket].push_back(*dimmIter);
	}
	NVM_UINT64 dimmCount = removeEmptySockets(sockets, bytesToAllocate, request, layout);

	if (sockets.size())
	{
		NVM_UINT64 bytesPerDimm = bytesToAllocate / dimmCount;
		for (std::map<NVM_UINT16, std::vector<Dimm> >::iterator socketIter = sockets.begin();
			socketIter != sockets.end(); socketIter++)
		{
			NVM_UINT64 bytesPerSocket = bytesPerDimm * socketIter->second.size();
			bytesAllocated += layoutInterleavedPmAcrossSocket(bytesPerSocket, socketIter->second, request, layout);
		}
	}

	// unable to map anything
	if (bytesAllocated == 0)
	{
		throw exception::NvmExceptionBadRequestSize();
	}

	return (bytesToAllocate - bytesAllocated);
}

NVM_UINT64 wbem::logic::LayoutStepPersistent::layoutInterleavedPmAcrossSocket(
		const NVM_UINT64 &bytesPerSocket,
		const std::vector<wbem::logic::Dimm> &dimms,
		const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> remainingDimms = getDimmsWithCapacity(dimms, layout);
	NVM_UINT64 bytesAllocated = 0;

	// layout evenly across dimms in socket
	NVM_UINT64 maxBytesPerDimm = bytesPerSocket / remainingDimms.size();
	while (bytesAllocated < bytesPerSocket)
	{
		std::vector<Dimm> dimmsIncluded;
		NVM_UINT64 bytesPerDimm = getBestInterleaveBytesPerDimm(
				remainingDimms, layout.goals, maxBytesPerDimm*remainingDimms.size(), request, dimmsIncluded);
		if (bytesPerDimm > maxBytesPerDimm)
		{
			bytesPerDimm = maxBytesPerDimm;
		}
		bytesAllocated += layoutInterleaveSet(bytesPerDimm, dimmsIncluded, request, layout);

		remainingDimms = getRemainingDimms(remainingDimms, dimmsIncluded);
		if (remainingDimms.size() == 0)
		{
			break;
		}
	}
	return bytesAllocated;
}

NVM_UINT64 wbem::logic::LayoutStepPersistent::layoutInterleaveSet(
		const NVM_UINT64 &bytesPerDimm,
		const std::vector<wbem::logic::Dimm> &dimms,
		const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytesAllocated = 0;
	MemoryAllocationUtil util(m_pLibApi);
	NVM_UINT16 setId = util.getNextAvailableInterleaveSetId(layout);
	enum interleave_ways way = getInterleaveWay(dimms.size());
	enum interleave_size channelSize = getInterleaveChannelSize(request, dimms.size());
	enum interleave_size imcSize = getInterleaveImcSize(request, dimms.size());
	for (std::vector<Dimm>::const_iterator dimmIter = dimms.begin();
					dimmIter != dimms.end(); dimmIter++)
	{
		struct pm_attributes settings;
		memset(&settings, 0, sizeof (settings));
		settings.interleave.ways = way;
		settings.interleave.channel = channelSize;
		settings.interleave.imc = imcSize;
		for (size_t i = 0; i < dimms.size(); i++)
		{
			str_to_guid(dimms[i].guid.c_str(), settings.dimms[i]);
		}
		addInterleaveSetToGoal(dimmIter->guid, layout.goals[dimmIter->guid], bytesPerDimm, setId, settings);
		bytesAllocated += bytesPerDimm;
	}
	return bytesAllocated;
}

NVM_UINT64 wbem::logic::LayoutStepPersistent::removeEmptySockets(
		std::map<NVM_UINT16, std::vector<Dimm> > &sockets,
		const NVM_UINT64 &bytesToAllocate,
		const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 dimmCount = 0;
	std::vector<NVM_UINT16> emptySockets;
	for (std::map<NVM_UINT16, std::vector<Dimm> >::iterator socketIter = sockets.begin();
		socketIter != sockets.end(); socketIter++)
	{
		try
		{
			std::vector<Dimm> dimmsIncluded;
			NVM_UINT64 bytesPerDimm = getLargestPerDimmSymmetricalBytes(
					socketIter->second, layout.goals, bytesToAllocate, dimmsIncluded);
			if (bytesPerDimm == 0)
			{
				emptySockets.push_back(socketIter->first);
			}
			else
			{
				dimmCount += dimmsIncluded.size();
			}
		}
		catch (exception::NvmExceptionBadRequestSize &)
		{
			emptySockets.push_back(socketIter->first);
		}
	}
	for (std::vector<NVM_UINT16>::const_iterator eraseIter = emptySockets.begin();
			eraseIter != emptySockets.end(); eraseIter++)
	{
		sockets.erase(*eraseIter);
	}

	return dimmCount;
}

std::vector<wbem::logic::Dimm> wbem::logic::LayoutStepPersistent::getDimmsWithCapacity(
		const std::vector<Dimm> &dimms,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> dimmsWithCapacity;
	for (std::vector<Dimm>::const_iterator dimmIter = dimms.begin();
			dimmIter != dimms.end(); dimmIter++)
	{
		if (getDimmUnallocatedBytes(dimmIter->capacity, layout.goals[dimmIter->guid]) > 0)
		{
			dimmsWithCapacity.push_back(*dimmIter);
		}
	}
	return dimmsWithCapacity;
}

bool wbem::logic::LayoutStepPersistent::interleaveSetsMatch(
		const struct pm_attributes &oldSet, const struct pm_attributes &newSet)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool match = false;
	if (oldSet.interleave.ways == newSet.interleave.ways &&
		oldSet.mirrored == newSet.mirrored)
	{
		// only check everything else if not x1
		if ((oldSet.interleave.ways == INTERLEAVE_WAYS_1) ||
			(oldSet.interleave.channel == newSet.interleave.channel &&
			oldSet.interleave.imc == newSet.interleave.imc &&
			cmp_bytes((const unsigned char *)oldSet.dimms,
					(const unsigned char *)newSet.dimms,
					COMMON_GUID_LEN * NVM_MAX_DEVICES_PER_POOL)))
		{
			match = true;
		}
	}

	return match;
}

bool wbem::logic::LayoutStepPersistent::interleaveSetFromPreviousExtent(
		const std::string &dimmGuid,
		struct config_goal &goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return (goal.persistent_count <= m_dimmExistingSets[dimmGuid]);
}

void wbem::logic::LayoutStepPersistent::addInterleaveSetToGoal(
		const std::string &dimmGuid,
		struct config_goal &goal, const NVM_UINT64 &sizeBytes,
		const NVM_UINT16 &setId, const struct pm_attributes &settings)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// extend existing
	bool extended = false;
	if (!interleaveSetFromPreviousExtent(dimmGuid, goal))
	{
		if (goal.persistent_count == 1 &&
				interleaveSetsMatch(goal.persistent_1_settings, settings))
		{
			goal.persistent_1_size += bytesToConfigGoalSize(sizeBytes);
			extended = true;
		}
		else if (goal.persistent_count == 2 &&
				interleaveSetsMatch(goal.persistent_2_settings, settings))
		{
			goal.persistent_2_size += bytesToConfigGoalSize(sizeBytes);
			extended = true;
		}
	}
	// or create a new one
	if (!extended)
	{
		goal.persistent_count++;
		if (goal.persistent_count == 1)
		{
			goal.persistent_1_size = bytesToConfigGoalSize(sizeBytes);
			goal.persistent_1_set_id = setId;
			memmove(&goal.persistent_1_settings, &settings, sizeof (settings));
		}
		else
		{
			goal.persistent_2_size = bytesToConfigGoalSize(sizeBytes);
			goal.persistent_2_set_id = setId;
			memmove(&goal.persistent_2_settings, &settings, sizeof (settings));
		}
	}
}

NVM_UINT64 wbem::logic::LayoutStepPersistent::getRequestedCapacityBytes(
		const NVM_UINT64 &requestedCapacity,
		const struct MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 capacityBytes = 0;
	if (requestedCapacity != REQUEST_REMAINING_CAPACITY)
	{
		capacityBytes = configGoalSizeToBytes(requestedCapacity);
	}
	else
	{
		capacityBytes = getRemainingPersistentBytesFromDimms(request, layout);
	}

	return capacityBytes;
}

NVM_UINT64 wbem::logic::LayoutStepPersistent::getDimmUnallocatedPersistentBytes(
		const struct Dimm &dimm, const struct config_goal &goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytes = 0;
	if (goal.persistent_count < MAX_PERSISTENT_EXTENTS)
	{
		bytes = getDimmUnallocatedBytes(dimm.capacity, goal);
	}

	return bytes;
}

NVM_UINT64 wbem::logic::LayoutStepPersistent::getRemainingPersistentBytesFromDimms(
		const MemoryAllocationRequest &request,
		MemoryAllocationLayout &layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytes = 0;
	for (std::vector<struct Dimm>::const_iterator dimmIter = request.dimms.begin();
			dimmIter != request.dimms.end(); dimmIter++)
	{
		if (layout.reserveDimmGuid != dimmIter->guid)
		{
			bytes += getDimmUnallocatedPersistentBytes(*dimmIter, layout.goals[dimmIter->guid]);
		}
	}

	// no remaining persistent capacity left on any dimms
	if (bytes == 0)
	{
		throw exception::NvmExceptionBadRequestSize();
	}
	return bytes;
}

int wbem::logic::LayoutStepPersistent::getDimmPopulationMap(
		const std::vector<wbem::logic::Dimm> &requestedDimms,
		std::map<std::string, struct config_goal> &goals)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int map = 0;
	for (std::vector<struct Dimm>::const_iterator dimmIter = requestedDimms.begin();
				dimmIter != requestedDimms.end(); dimmIter++)
	{
		if (getDimmUnallocatedBytes(dimmIter->capacity, goals[dimmIter->guid]) > 0)
		{
			int dimmLocation = DIMM_LOCATION(dimmIter->memoryController, dimmIter->channel);
			map += (1 << dimmLocation);
		}
	}
	return map;
}

bool wbem::logic::LayoutStepPersistent::dimmPopulationMatchesInterleaveSet(
		const int &dimmMap, const int &interleaveSet)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool match = true;
	for (int i = 0; i < DIMMS_PER_SOCKET; i++)
	{
		if (DIMM_POPULATED(interleaveSet, i) && !DIMM_POPULATED(dimmMap, i))
		{
			match = false;
			break;
		}
	}

	return match;
}

std::vector<wbem::logic::Dimm> wbem::logic::LayoutStepPersistent::getDimmsMatchingInterleaveSet(
		const int &interleaveSet,
		const std::vector<wbem::logic::Dimm> &requestedDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> dimms;
	for (std::vector<struct Dimm>::const_iterator dimmIter = requestedDimms.begin();
					dimmIter != requestedDimms.end(); dimmIter++)
	{
		int dimmLocation = DIMM_LOCATION(dimmIter->memoryController, dimmIter->channel);
		if (DIMM_POPULATED(interleaveSet, dimmLocation))
		{
			dimms.push_back(*dimmIter);
		}
	}
	return dimms;
}

NVM_UINT64 wbem::logic::LayoutStepPersistent::getBestInterleaveBytesPerDimm(
		const std::vector<wbem::logic::Dimm> &requestedDimms,
		std::map<std::string, struct config_goal> &goals,
		const NVM_UINT64 &requestedBytes,
		const wbem::logic::MemoryAllocationRequest &request,
		std::vector<wbem::logic::Dimm> &dimmsIncluded)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytesPerDimm = 0;
	int dimmPopulationMap = getDimmPopulationMap(requestedDimms, goals);

	size_t setCount = (sizeof (INTERLEAVE_SETS) / sizeof (int));
	for (size_t i = 0; i < setCount; i++)
	{
		if (dimmPopulationMatchesInterleaveSet(dimmPopulationMap, INTERLEAVE_SETS[i]))
		{
			std::vector<Dimm> dimms = getDimmsMatchingInterleaveSet(INTERLEAVE_SETS[i], requestedDimms);
			if (canMapInterleavedCapacity(dimms, goals, request))
			{
				bytesPerDimm = getLargestPerDimmSymmetricalBytes(
					dimms,
					goals,
					requestedBytes,
					dimmsIncluded);
				break;
			}
		}
	}

	// no match found
	if (bytesPerDimm == 0)
	{
		throw exception::NvmExceptionBadRequestSize();
	}

	return bytesPerDimm;
}

bool wbem::logic::LayoutStepPersistent::canMapInterleavedCapacity(
		const std::vector<wbem::logic::Dimm> &dimms,
		std::map<std::string, struct config_goal> &goals,
		const wbem::logic::MemoryAllocationRequest &request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool setExtendable = true;
	// new set
	struct pm_attributes newSet;
	memset(&newSet, 0, sizeof (newSet));
	newSet.interleave.ways = request.persistentExtents[m_pmExtentIndex].byOne ?
			INTERLEAVE_WAYS_1 : getInterleaveWay(dimms.size());
	newSet.interleave.channel = getInterleaveChannelSize(request, dimms.size());
	newSet.interleave.imc = getInterleaveImcSize(request, dimms.size());
	int dimmIndex = 0;
	for (std::vector<Dimm>::const_iterator dimmIter = dimms.begin();
				dimmIter != dimms.end(); dimmIter++)
	{
		str_to_guid(dimmIter->guid.c_str(), newSet.dimms[dimmIndex++]);
	}

	for (std::vector<Dimm>::const_iterator dimmIter = dimms.begin();
			dimmIter != dimms.end(); dimmIter++)
	{
		if (goals[dimmIter->guid].persistent_count == MAX_PERSISTENT_EXTENTS)
		{
			if (interleaveSetFromPreviousExtent(dimmIter->guid, goals[dimmIter->guid]))
			{
				setExtendable = false;
				break;
			}
			if (!interleaveSetsMatch(goals[dimmIter->guid].persistent_2_settings, newSet))
			{
				setExtendable = false;
				break;
			}
		}
	}
	return setExtendable;
}

bool wbem::logic::LayoutStepPersistent::isValidWay(const size_t &numDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool isValid = true;
	try
	{
		getInterleaveWay(numDimms);
	}
	catch (exception::NvmExceptionPersistentSettingsNotSupported &)
	{
		isValid = false;
	}
	return isValid;
}

// TODO: platform specific
enum interleave_ways wbem::logic::LayoutStepPersistent::getInterleaveWay(
		const size_t &numDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	enum interleave_ways ways;
	switch (numDimms)
	{
		case 1:
			ways = INTERLEAVE_WAYS_1;
			break;
		case 2:
			ways = INTERLEAVE_WAYS_2;
			break;
		case 3:
			ways = INTERLEAVE_WAYS_3;
			break;
		case 4:
			ways = INTERLEAVE_WAYS_4;
			break;
		case 6:
			ways = INTERLEAVE_WAYS_6;
			break;
		default:
			throw exception::NvmExceptionPersistentSettingsNotSupported();
			break;
	}
	return ways;
}

enum interleave_size wbem::logic::LayoutStepPersistent::getInterleaveChannelSize(
		const MemoryAllocationRequest &request, const size_t &numDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	enum interleave_size channelSize =
			(enum interleave_size)request.persistentExtents[m_pmExtentIndex].channel;
	if (request.persistentExtents[m_pmExtentIndex].channel == REQUEST_DEFAULT_INTERLEAVE_FORMAT)
	{
		// lookup recommended
		for (int i = 0; i < m_systemCap.platform_capabilities.pm_direct.interleave_formats_count; i++)
		{
			if (m_systemCap.platform_capabilities.pm_direct.interleave_formats[i].ways == numDimms)
			{
				channelSize = m_systemCap.platform_capabilities.pm_direct.interleave_formats[i].channel;
				if (m_systemCap.platform_capabilities.pm_direct.interleave_formats[i].recommended)
				{
					// found a recommended size, stop looking
					break;
				}
			}
		}
	}

	return channelSize;
}

enum interleave_size wbem::logic::LayoutStepPersistent::getInterleaveImcSize(
		const MemoryAllocationRequest &request, const size_t &numDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	enum interleave_size imcSize =
			(enum interleave_size)request.persistentExtents[m_pmExtentIndex].imc;
	if (request.persistentExtents[m_pmExtentIndex].imc == REQUEST_DEFAULT_INTERLEAVE_FORMAT)
	{
		// lookup recommended
		for (int i = 0; i < m_systemCap.platform_capabilities.pm_direct.interleave_formats_count; i++)
		{
			if (m_systemCap.platform_capabilities.pm_direct.interleave_formats[i].ways == numDimms)
			{
				imcSize = m_systemCap.platform_capabilities.pm_direct.interleave_formats[i].imc;
				if (m_systemCap.platform_capabilities.pm_direct.interleave_formats[i].recommended)
				{
					// found a recommended size, stop looking
					break;
				}
			}
		}
	}

	return imcSize;
}

std::vector<wbem::logic::Dimm> wbem::logic::LayoutStepPersistent::getRemainingDimms(
		const std::vector<Dimm> &dimms, const std::vector<Dimm> &dimmsIncluded)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> remainingDimms;
	for (std::vector<Dimm>::const_iterator allIter = dimms.begin();
			allIter != dimms.end(); allIter++)
	{
		bool included = false;
		for (std::vector<Dimm>::const_iterator includedIter = dimmsIncluded.begin();
				includedIter != dimmsIncluded.end(); includedIter++)
		{
			if (allIter->guid == includedIter->guid)
			{
				included = true;
				break;
			}
		}
		if (!included)
		{
			remainingDimms.push_back(*allIter);
		}
	}
	return remainingDimms;
}
