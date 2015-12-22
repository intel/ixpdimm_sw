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
 * Implementation of MemoryAllocator
 */

#include "MemoryAllocator.h"
#include <common_types.h>
#include "RuleNoDimms.h"
#include "RuleProvisionCapacityNotSupported.h"
#include "RulePersistentNotSupported.h"
#include "RuleMirroredPersistentNotSupported.h"
#include "RuleVolatileCapacityNotSupported.h"
#include "RuleStorageCapacityNotSupported.h"
#include "RuleNamespacesExist.h"
#include "RuleDimmHasConfigGoal.h"
#include "RulePartialSocketConfigured.h"
#include "RuleDimmListInvalid.h"
#include "RuleTooManyPersistentExtents.h"
#include "RuleTooManyRemaining.h"
#include "RuleReserveDimmPropertyInvalid.h"
#include "RuleRejectLockedDimms.h"
#include <LogEnterExit.h>
#include "LayoutBuilder.h"
#include <exception/NvmExceptionLibError.h>
#include <intel_cim_framework/ExceptionNoMemory.h>
#include <nvm_management.h>
#include <guid/guid.h>
#include <lib_interface/NvmApi.h>
#include <utility.h>
#include "PostLayoutAddressDecoderLimitCheck.h"
#include "PostLayoutRequestDeviationCheck.h"

wbem::logic::MemoryAllocator::MemoryAllocator(const struct nvm_capabilities &systemCapabilities,
		const std::vector<struct device_discovery> &manageableDevices,
		const std::vector<struct pool> &pools,
		const NVM_UINT16 socketCount,
		lib_interface::NvmApi *pApi) :
		m_systemCapabilities(systemCapabilities),
		m_manageableDevices(manageableDevices),
		m_pools(pools),
		m_socketCount(socketCount),
		m_pLibApi(pApi)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!m_pLibApi)
	{
		m_pLibApi = lib_interface::NvmApi::getApi();
	}
	populateRequestRules();
	populatePostLayoutChecks();
}

wbem::logic::MemoryAllocator* wbem::logic::MemoryAllocator::getNewMemoryAllocator()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	lib_interface::NvmApi *pApi = wbem::lib_interface::NvmApi::getApi();

	struct nvm_capabilities systemCapabilities;
	memset (&systemCapabilities, 0, sizeof (systemCapabilities));
	int rc = pApi->getNvmCapabilities(&systemCapabilities);
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}

	std::vector<struct device_discovery> manageableDevices;
	pApi->getManageableDimms(manageableDevices);

	std::vector<struct pool> pools;
	pApi->getPools(pools);

	int numSockets = pApi->getSocketCount();
	if (rc < 0)
	{
		throw exception::NvmExceptionLibError(rc);
	}

	wbem::logic::MemoryAllocator *pAllocator = new wbem::logic::MemoryAllocator(
			systemCapabilities,
			manageableDevices,
			pools,
			numSockets,
			pApi);
	if (!pAllocator)
	{
		throw wbem::framework::ExceptionNoMemory(__FILE__, __FUNCTION__,
				"Couldn't create MemoryAllocator");
	}

	return pAllocator;
}

wbem::logic::MemoryAllocator::~MemoryAllocator()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	deleteRequestRules();
	deleteLayoutRules();
}

wbem::logic::MemoryAllocationLayout wbem::logic::MemoryAllocator::layout(
		const struct MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	validateRequest(request);

	LayoutBuilder builder(m_systemCapabilities);
	wbem::logic::MemoryAllocationLayout layout = builder.build(request);

	validateLayout(request, layout);

	return layout;
}

void wbem::logic::MemoryAllocator::populateRequestRules()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Note that order matters
	m_requestRules.push_back(new RuleProvisionCapacityNotSupported(m_systemCapabilities));
	m_requestRules.push_back(new RuleNoDimms());
	m_requestRules.push_back(new RuleTooManyPersistentExtents());
	m_requestRules.push_back(new RuleTooManyRemaining());
	m_requestRules.push_back(new RuleReserveDimmPropertyInvalid());
	m_requestRules.push_back(new RuleDimmListInvalid(m_manageableDevices));
	m_requestRules.push_back(new RuleVolatileCapacityNotSupported(m_systemCapabilities));
	m_requestRules.push_back(new RulePersistentNotSupported(m_systemCapabilities));
	m_requestRules.push_back(new RuleMirroredPersistentNotSupported);
	m_requestRules.push_back(new RuleStorageCapacityNotSupported(m_systemCapabilities));
	m_requestRules.push_back(new RuleDimmHasConfigGoal);
	m_requestRules.push_back(new RuleNamespacesExist);
	m_requestRules.push_back(new RuleRejectLockedDimms(m_manageableDevices));
	m_requestRules.push_back(new RulePartialSocketConfigured(m_manageableDevices));
}

void wbem::logic::MemoryAllocator::populatePostLayoutChecks()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_postLayoutChecks.push_back(new PostLayoutAddressDecoderLimitCheck(
			m_manageableDevices,
			m_pools,
			m_socketCount));
	m_postLayoutChecks.push_back(new PostLayoutRequestDeviationCheck());
}

void wbem::logic::MemoryAllocator::validateLayout(
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

void wbem::logic::MemoryAllocator::validateRequest(const struct MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<RequestRule *>::const_iterator ruleIter = m_requestRules.begin();
			ruleIter != m_requestRules.end(); ruleIter++)
	{
		(*ruleIter)->verify(request);
	}
}

void wbem::logic::MemoryAllocator::deleteRequestRules()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<RequestRule *>::iterator ruleIter = m_requestRules.begin();
			ruleIter != m_requestRules.end(); ruleIter++)
	{
		delete *ruleIter; // value of iter == (RequestRule *)
	}
}

void wbem::logic::MemoryAllocator::deleteLayoutRules()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<PostLayoutCheck *>::iterator ruleIter = m_postLayoutChecks.begin();
			ruleIter != m_postLayoutChecks.end(); ruleIter++)
	{
		delete *ruleIter; // value of iter == (LayoutRule *)
	}
}

void wbem::logic::MemoryAllocator::allocate(struct MemoryAllocationLayout &layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::map<std::string, struct config_goal>::iterator goalIter = layout.goals.begin();
			goalIter != layout.goals.end(); goalIter++)
	{
		NVM_GUID guid;
		str_to_guid((*goalIter).first.c_str(), guid);
		int rc = m_pLibApi->createConfigGoal(guid, &((*goalIter).second));
		if (rc != NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(rc);
		}
	}
}

NVM_UINT64 wbem::logic::MemoryAllocator::getTotalCapacitiesOfRequestedDimmsinB(
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

