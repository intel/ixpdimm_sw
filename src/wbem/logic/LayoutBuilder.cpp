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
 * Builds the memory allocation layout.
 */

#include "LayoutBuilder.h"
#include <LogEnterExit.h>
#include <utility.h>
#include <exception/NvmExceptionBadRequest.h>
#include "LayoutStepCheckDriverSupportsAppDirect.h"
#include "LayoutStepCheckDriverSupportsStorage.h"
#include "LayoutStepAppDirectSettingsNotRecommended.h"
#include "LayoutStepCheckAsymmetricalPopulation.h"
#include "LayoutStepCheckCurrentVolatileMode.h"
#include "LayoutStepAppDirect.h"
#include "LayoutStepMemory.h"
#include "LayoutStepStorage.h"
#include "LayoutStepReserveDimm.h"

wbem::logic::LayoutBuilder::LayoutBuilder(
		const struct nvm_capabilities &systemCapabilities,
		lib_interface::NvmApi *pApi)
	: m_systemCapabilities(systemCapabilities), m_pLibApi(pApi)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (!m_pLibApi)
	{
		m_pLibApi = lib_interface::NvmApi::getApi();
	}
}

wbem::logic::LayoutBuilder::~LayoutBuilder()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	deleteLayoutSteps();
}

wbem::logic::MemoryAllocationLayout wbem::logic::LayoutBuilder::build(
		const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	populateAllLayoutStepsForRequest(request);

	MemoryAllocationLayout layout;
	initLayoutGoals(request, layout);

	for (std::vector<LayoutStep *>::iterator stepIter = m_layoutSteps.begin();
			stepIter != m_layoutSteps.end(); stepIter++)
	{
		try
		{
			(*stepIter)->execute(request, layout);
		}
		// will be handled by post layout checks
		catch (exception::NvmExceptionBadRequestSize &) {}
	}

	deleteLayoutSteps();

	return layout;
}

void wbem::logic::LayoutBuilder::populateAllLayoutStepsForRequest(const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	deleteLayoutSteps();
	populateWarningGeneratingLayoutSteps();
	populateOrderedLayoutStepsForRequest(request);
}

void wbem::logic::LayoutBuilder::populateWarningGeneratingLayoutSteps()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_layoutSteps.push_back(new LayoutStepCheckDriverSupportsAppDirect(m_systemCapabilities.nvm_features));
	m_layoutSteps.push_back(new LayoutStepAppDirectSettingsNotRecommended(m_systemCapabilities.platform_capabilities));
	m_layoutSteps.push_back(new LayoutStepCheckDriverSupportsStorage(m_systemCapabilities.nvm_features));
	m_layoutSteps.push_back(new LayoutStepCheckAsymmetricalPopulation());
	m_layoutSteps.push_back(new LayoutStepCheckCurrentVolatileMode(m_systemCapabilities.platform_capabilities));
}

void wbem::logic::LayoutBuilder::populateOrderedLayoutStepsForRequest(
		const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Reserve a DIMM before we get started laying out other capacity
	m_layoutSteps.push_back(new LayoutStepReserveDimm());

	// Any capacity marked REMAINING must go last (but before storage)
	LayoutStep *pRemainingStep = NULL;

	LayoutStep *pMemory = new LayoutStepMemory();
	if (pMemory->isRemainingStep(request))
	{
		pRemainingStep = pMemory;
	}
	else
	{
		m_layoutSteps.push_back(pMemory);
	}

	for (size_t i = 0; i < request.appDirectExtents.size(); i++)
	{
		LayoutStep *pAppDirect = new LayoutStepAppDirect(m_systemCapabilities, (int)i, m_pLibApi);
		if (pAppDirect->isRemainingStep(request))
		{
			pRemainingStep = pAppDirect;
		}
		else
		{
			m_layoutSteps.push_back(pAppDirect);
		}
	}

	if (pRemainingStep)
	{
		m_layoutSteps.push_back(pRemainingStep);
	}

	// Anything left after we are done laying out capacity is storage
	m_layoutSteps.push_back(new LayoutStepStorage());
}

void wbem::logic::LayoutBuilder::initLayoutGoals(
		const MemoryAllocationRequest& request, MemoryAllocationLayout &layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct config_goal goal;
	memset(&goal, 0, sizeof (goal));
	for (std::vector<struct Dimm>::const_iterator dimmIter = request.dimms.begin();
			dimmIter != request.dimms.end(); dimmIter++)
	{
		layout.goals[dimmIter->guid] = goal;
	}
}

void wbem::logic::LayoutBuilder::deleteLayoutSteps()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<LayoutStep *>::iterator stepIter = m_layoutSteps.begin();
			stepIter != m_layoutSteps.end(); stepIter++)
	{
		delete *stepIter;
	}

	m_layoutSteps.clear();
}
