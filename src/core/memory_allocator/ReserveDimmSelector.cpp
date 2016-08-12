/*
 * INTEL CONFIDENTIAL
 *
 * Copyright 2016 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to the
 * source code ("Material") are owned by Intel Corporation or its suppliers or
 * licensors. Title to the Material remains with Intel Corporation or its
 * suppliers and licensors. The Material may contain trade secrets and proprietary
 * and confidential information of Intel Corporation and its suppliers and licensors,
 * and is protected by worldwide copyright and trade secret laws and treaty
 * provisions. No part of the Material may be used, copied, reproduced, modified,
 * published, uploaded, posted, transmitted, distributed, or disclosed in any way
 * without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be express
 * and approved by Intel in writing.
 *
 * Unless otherwise agreed by Intel in writing, you may not remove or alter this
 * notice or any other notice embedded in Materials by Intel or Intel's suppliers
 * or licensors in any way.
 */


#include "ReserveDimmSelector.h"
#include <LogEnterExit.h>

namespace core
{
namespace memory_allocator
{

ReserveDimmSelector::ReserveDimmSelector(const std::vector<Dimm> &dimms) :
		m_sockets(), m_selectedDimmUid()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	sortDimmsBySocket(dimms);
}

void ReserveDimmSelector::sortDimmsBySocket(const std::vector<Dimm> &dimms)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<Dimm>::const_iterator dimmIter = dimms.begin();
			dimmIter != dimms.end(); dimmIter++)
	{
		NVM_UINT16 socket = dimmIter->socket;
		m_sockets[socket].push_back(*dimmIter);
	}
}

ReserveDimmSelector::~ReserveDimmSelector()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

std::string ReserveDimmSelector::getReservedDimm()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	if (m_sockets.empty())
	{
		throw NoDimmsException();
	}

	selectDimmToReserve();
	return m_selectedDimmUid;
}

bool ReserveDimmSelector::dimmSelected() const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return !m_selectedDimmUid.empty();
}

void ReserveDimmSelector::selectDimm(const Dimm& dimm)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	m_selectedDimmUid = dimm.uid;
}

void ReserveDimmSelector::selectDimmToReserve()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	trySelectDimmAloneOnMemoryController();
	trySelectDimmWithoutPartnerOnOtherMemoryController();
	trySelectDifferentSizedDimm();
	trySelectSmallestSizedDimm();
	trySelectFirstDimm();
}

void ReserveDimmSelector::trySelectDimmAloneOnMemoryController()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	for (std::map<NVM_UINT16, std::vector<Dimm> >::iterator socketIter = m_sockets.begin();
			!dimmSelected() && socketIter != m_sockets.end();
			socketIter++)
	{
		selectDimmAloneOnMemoryControllerFromSocket(socketIter->second);
	}
}

void ReserveDimmSelector::selectDimmAloneOnMemoryControllerFromSocket(
		const std::vector<Dimm>& socketDimms)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	std::map<NVM_UINT16, std::vector<Dimm> > imcDimms =
			getDimmsSortedByMemoryController(socketDimms);

	std::vector<Dimm> isolatedDimms = getDimmsAloneOnTheirImc(imcDimms);

	if (isolatedDimms.size() == 1 && atLeastOneImcFullyPopulated(imcDimms))
	{
		selectDimm(isolatedDimms.front());
	}
}

std::map<NVM_UINT16, std::vector<Dimm> > ReserveDimmSelector::getDimmsSortedByMemoryController(
		const std::vector<Dimm> &dimms)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	std::map<NVM_UINT16, std::vector<Dimm> > imcDimms;
	for (std::vector<Dimm>::const_iterator dimmIter = dimms.begin();
			dimmIter != dimms.end(); dimmIter++)
	{
		imcDimms[dimmIter->memoryController].push_back(*dimmIter);
	}

	return imcDimms;
}

std::vector<Dimm> ReserveDimmSelector::getDimmsAloneOnTheirImc(
		std::map<NVM_UINT16, std::vector<Dimm> > sortedByImc)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> isolatedDimms;
	for (std::map<NVM_UINT16, std::vector<Dimm> >::const_iterator imc = sortedByImc.begin();
			imc != sortedByImc.end(); imc++)
	{
		if (imc->second.size() == 1)
		{
			isolatedDimms.push_back(imc->second.front());
		}
	}

	return isolatedDimms;
}

bool ReserveDimmSelector::atLeastOneImcFullyPopulated(
		std::map<NVM_UINT16, std::vector<Dimm> > sortedByImc)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	bool found = false;
	for (std::map<NVM_UINT16, std::vector<Dimm> >::const_iterator imc = sortedByImc.begin();
			imc != sortedByImc.end(); imc++)
	{
		if (imc->second.size() == CHANNELS_PER_IMC)
		{
			found = true;
			break;
		}
	}

	return found;
}

void ReserveDimmSelector::trySelectDimmWithoutPartnerOnOtherMemoryController()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	for (std::map<NVM_UINT16, std::vector<Dimm> >::iterator socketIter = m_sockets.begin();
			!dimmSelected() && socketIter != m_sockets.end();
			socketIter++)
	{
		selectDimmWithoutPartnerOnOtherMemoryControllerFromSocket(socketIter->second);
	}
}

void ReserveDimmSelector::selectDimmWithoutPartnerOnOtherMemoryControllerFromSocket(
		const std::vector<Dimm>& socketDimms)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	std::map<NVM_UINT16, std::vector<Dimm> > channelPartnerDimms =
			getDimmsSortedByChannelPartnership(socketDimms);

	std::vector<Dimm> unpartneredDimms = getUnpartneredDimms(channelPartnerDimms);
	if (!unpartneredDimms.empty())
	{
		selectDimm(unpartneredDimms.front());
	}
}

std::map<NVM_UINT16, std::vector<Dimm> > ReserveDimmSelector::getDimmsSortedByChannelPartnership(
		const std::vector<Dimm>& dimms)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	std::map<NVM_UINT16, std::vector<Dimm> > partneredDimms;
	for (std::vector<Dimm>::const_iterator dimmIter = dimms.begin();
			dimmIter != dimms.end(); dimmIter++)
	{
		NVM_UINT16 channelIndex = dimmIter->channel % CHANNELS_PER_IMC;
		partneredDimms[channelIndex].push_back(*dimmIter);
	}

	return partneredDimms;
}

std::vector<Dimm> ReserveDimmSelector::getUnpartneredDimms(
		std::map<NVM_UINT16, std::vector<Dimm> > channelPartners)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> unpartneredDimms;
	for (std::map<NVM_UINT16, std::vector<Dimm> >::iterator channelIter = channelPartners.begin();
			channelIter != channelPartners.end(); channelIter++)
	{
		std::vector<Dimm> &partners = channelIter->second;
		if (partners.size() == 1)
		{
			unpartneredDimms.push_back(partners.front());
		}
	}

	return unpartneredDimms;
}

void ReserveDimmSelector::trySelectDifferentSizedDimm()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	for (std::map<NVM_UINT16, std::vector<Dimm> >::iterator socketIter = m_sockets.begin();
			!dimmSelected() && socketIter != m_sockets.end();
			socketIter++)
	{
		selectDifferentSizedDimmFromSocket(socketIter->second);
	}
}

void ReserveDimmSelector::selectDifferentSizedDimmFromSocket(const std::vector<Dimm>& socketDimms)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	std::map<NVM_UINT64, std::vector<Dimm> > dimmsByCapacity =
			getDimmsSortedByCapacity(socketDimms);

	std::vector<Dimm> differentSizeDimms = getUniquelySizedDimms(dimmsByCapacity);
	if (differentSizeDimms.size() == 1)
	{
		selectDimm(differentSizeDimms.front());
	}
}

std::map<NVM_UINT64, std::vector<Dimm> > ReserveDimmSelector::getDimmsSortedByCapacity(
		const std::vector<Dimm>& dimms)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	std::map<NVM_UINT64, std::vector<Dimm> > dimmsByCapacity;
	for (std::vector<Dimm>::const_iterator dimmIter = dimms.begin();
			dimmIter != dimms.end(); dimmIter++)
	{
		dimmsByCapacity[dimmIter->capacity].push_back(*dimmIter);
	}

	return dimmsByCapacity;
}

std::vector<Dimm> ReserveDimmSelector::getUniquelySizedDimms(
		std::map<NVM_UINT64, std::vector<Dimm> > dimmsByCapacity)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> differentSizeDimms;
	for (std::map<NVM_UINT64, std::vector<Dimm> >::iterator capacityGroup = dimmsByCapacity.begin();
			capacityGroup != dimmsByCapacity.end(); capacityGroup++)
	{
		std::vector<Dimm> &dimmsWithSameCapacity = capacityGroup->second;
		if (dimmsWithSameCapacity.size() == 1)
		{
			differentSizeDimms.push_back(dimmsWithSameCapacity.front());
		}
	}

	return differentSizeDimms;
}

void ReserveDimmSelector::trySelectSmallestSizedDimm()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	for (std::map<NVM_UINT16, std::vector<Dimm> >::iterator socketIter = m_sockets.begin();
			!dimmSelected() && socketIter != m_sockets.end();
			socketIter++)
	{
		selectSmallestSizedDimmFromSocket(socketIter->second);
	}
}

void ReserveDimmSelector::selectSmallestSizedDimmFromSocket(const std::vector<Dimm>& socketDimms)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	std::map<NVM_UINT64, std::vector<Dimm> > dimmsByCapacity =
				getDimmsSortedByCapacity(socketDimms);

	// At least two different capacities on the socket
	if (dimmsByCapacity.size() > 1)
	{
		const std::vector<Dimm> &smallestDimms = getSmallestCapacityDimms(dimmsByCapacity);
		selectDimm(smallestDimms.front());
	}
}

std::vector<Dimm> ReserveDimmSelector::getSmallestCapacityDimms(
		const std::map<NVM_UINT64, std::vector<Dimm> >& sortedByCapacity)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	// Sorted - smallest capacity will be first key
	return sortedByCapacity.begin()->second;
}

void ReserveDimmSelector::trySelectFirstDimm()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	if (!dimmSelected())
	{
		selectFirstDimm();
	}
}

void ReserveDimmSelector::selectFirstDimm()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	const std::vector<Dimm> &firstSocketDimms = m_sockets.begin()->second;
	selectDimm(firstSocketDimms.front());
}

} /* namespace memory_allocator */
} /* namespace core */
