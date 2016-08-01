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
 * Implementation of MemoryAllocator
 */

#include "MemoryAllocator.h"
#include <LogEnterExit.h>
#include <core/device/DeviceService.h>
#include <core/exceptions/NoMemoryException.h>
#include <core/exceptions/NvmExceptionBadRequest.h>

#include "LayoutBuilder.h"
#include "RuleNoDimms.h"
#include "RuleProvisionCapacityNotSupported.h"
#include "RuleStorageCapacityNotSupported.h"
#include "RuleMemoryModeCapacityNotSupported.h"
#include "RuleAppDirectNotSupported.h"
#include "RuleMirroredAppDirectNotSupported.h"
#include "RuleTooManyAppDirectExtents.h"
#include "RuleTooManyRemaining.h"
#include "RuleDimmHasConfigGoal.h"
#include "RuleNamespacesExist.h"
#include "RulePartialSocketConfigured.h"
#include "RuleReserveDimmPropertyInvalid.h"
#include "RuleDimmListInvalid.h"
#include "RuleRejectLockedDimms.h"
#include "PostLayoutAddressDecoderLimitCheck.h"
#include "PostLayoutRequestDeviationCheck.h"

core::memory_allocator::MemoryAllocator::MemoryAllocator(const struct nvm_capabilities &systemCapabilities,
		const std::vector<struct device_discovery> &manageableDevices,
		const std::vector<struct pool> &pools,
		const NVM_UINT16 socketCount,
		core::NvmLibrary &nvmLib) :
		m_systemCapabilities(systemCapabilities),
		m_manageableDevices(manageableDevices),
		m_pools(pools),
		m_socketCount(socketCount),
		m_nvmLib(nvmLib)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	populateRequestRules();
	populatePostLayoutChecks();
}

core::memory_allocator::MemoryAllocator* core::memory_allocator::MemoryAllocator::getNewMemoryAllocator()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	core::NvmLibrary &nvmLib = core::NvmLibrary::getNvmLibrary();
	core::device::DeviceService devService(nvmLib);

	struct nvm_capabilities systemCapabilities = nvmLib.getNvmCapabilities();
	std::vector<struct pool> pools = nvmLib.getPools();
	int numSockets = nvmLib.getSocketCount();

	std::vector<std::string> manageableUids = devService.getManageableUids();
	std::vector<struct device_discovery> manageableDevices;
	for (std::vector<std::string>::iterator uidIter = manageableUids.begin();
			uidIter != manageableUids.end(); uidIter++)
	{
		manageableDevices.push_back(nvmLib.getDeviceDiscovery(*uidIter));
	}

	core::memory_allocator::MemoryAllocator *pAllocator = new core::memory_allocator::MemoryAllocator(
			systemCapabilities,
			manageableDevices,
			pools,
			numSockets,
			nvmLib);
	if (!pAllocator)
	{
		COMMON_LOG_ERROR("Couldn't create MemoryAllocator");
		throw core::NoMemoryException();
	}

	return pAllocator;
}

core::memory_allocator::MemoryAllocator::~MemoryAllocator()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	deleteRequestRules();
	deleteLayoutRules();
}

core::memory_allocator::MemoryAllocationLayout core::memory_allocator::MemoryAllocator::layout(
		const struct MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	validateRequest(request);

	LayoutBuilder builder(m_systemCapabilities, m_nvmLib);
	core::memory_allocator::MemoryAllocationLayout layout = builder.build(request);

	validateLayout(request, layout);

	return layout;
}

void core::memory_allocator::MemoryAllocator::populateRequestRules()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Note that order matters
	m_requestRules.push_back(new RuleProvisionCapacityNotSupported(m_systemCapabilities));
	m_requestRules.push_back(new RuleNoDimms());
	m_requestRules.push_back(new RuleTooManyAppDirectExtents());
	m_requestRules.push_back(new RuleTooManyRemaining());
	m_requestRules.push_back(new RuleReserveDimmPropertyInvalid());
	m_requestRules.push_back(new RuleDimmListInvalid(m_manageableDevices));
	m_requestRules.push_back(new RuleMemoryModeCapacityNotSupported(m_systemCapabilities));
	m_requestRules.push_back(new RuleAppDirectNotSupported(m_systemCapabilities));
	m_requestRules.push_back(new RuleMirroredAppDirectNotSupported);
	m_requestRules.push_back(new RuleStorageCapacityNotSupported(m_systemCapabilities));
	m_requestRules.push_back(new RuleDimmHasConfigGoal(m_nvmLib));
	m_requestRules.push_back(new RuleNamespacesExist(m_nvmLib));
	m_requestRules.push_back(new RuleRejectLockedDimms(m_manageableDevices));
	m_requestRules.push_back(new RulePartialSocketConfigured(m_manageableDevices, m_nvmLib));
}

void core::memory_allocator::MemoryAllocator::populatePostLayoutChecks()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_postLayoutChecks.push_back(new PostLayoutAddressDecoderLimitCheck(
			m_manageableDevices,
			m_pools,
			m_socketCount));
	m_postLayoutChecks.push_back(new PostLayoutRequestDeviationCheck());
}

void core::memory_allocator::MemoryAllocator::validateLayout(
		const struct MemoryAllocationRequest& request,
		const MemoryAllocationLayout layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<PostLayoutCheck *>::const_iterator ruleIter = m_postLayoutChecks.begin();
			ruleIter != m_postLayoutChecks.end(); ruleIter++)
	{
		(*ruleIter)->verify(request, layout);
	}
}

void core::memory_allocator::MemoryAllocator::validateRequest(const struct MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<RequestRule *>::const_iterator ruleIter = m_requestRules.begin();
			ruleIter != m_requestRules.end(); ruleIter++)
	{
		(*ruleIter)->verify(request);
	}
}

void core::memory_allocator::MemoryAllocator::deleteRequestRules()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<RequestRule *>::iterator ruleIter = m_requestRules.begin();
			ruleIter != m_requestRules.end(); ruleIter++)
	{
		delete *ruleIter; // value of iter == (RequestRule *)
	}
}

void core::memory_allocator::MemoryAllocator::deleteLayoutRules()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<PostLayoutCheck *>::iterator ruleIter = m_postLayoutChecks.begin();
			ruleIter != m_postLayoutChecks.end(); ruleIter++)
	{
		delete *ruleIter; // value of iter == (LayoutRule *)
	}
}

void core::memory_allocator::MemoryAllocator::allocate(struct MemoryAllocationLayout &layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool atLeastOneRequestSucceeded = false;

	for (std::map<std::string, struct config_goal>::iterator goalIter = layout.goals.begin();
			goalIter != layout.goals.end(); goalIter++)
	{
		try
		{
			std::string uid = goalIter->first;
			m_nvmLib.createConfigGoal(uid, (*goalIter).second);

			atLeastOneRequestSucceeded = true;
		}
		catch (core::LibraryException &e)
		{
			COMMON_LOG_ERROR_F("creating config goal failed with rc = %d",
					e.getErrorCode());
			if (atLeastOneRequestSucceeded)
			{
				throw core::NvmExceptionPartialResultsCouldNotBeUndone();
			}
			else
			{
				throw;
			}
		}
	}
}

NVM_UINT64 core::memory_allocator::MemoryAllocator::getTotalCapacitiesOfRequestedDimmsinB(
		const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	NVM_UINT64 totalDimmCapacitiesInBytes = 0;

	std::vector<Dimm>::const_iterator requestIter = request.dimms.begin();
	for (; requestIter != request.dimms.end(); requestIter++)
	{
		totalDimmCapacitiesInBytes += requestIter->capacity;
	}

	return totalDimmCapacitiesInBytes;
}

