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
 * Rule that checks that no config goals exist on the dimms requested
 */

#include "RuleDimmHasConfigGoal.h"

#include <LogEnterExit.h>
#include <uid/uid.h>
#include <core/exceptions/LibraryException.h>
#include <core/exceptions/NvmExceptionBadRequest.h>

core::memory_allocator::RuleDimmHasConfigGoal::RuleDimmHasConfigGoal(core::NvmLibrary &nvmLib) :
	m_nvmLib(nvmLib)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::RuleDimmHasConfigGoal::~RuleDimmHasConfigGoal()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::RuleDimmHasConfigGoal::verify(const MemoryAllocationRequest &request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<struct Dimm>::const_iterator dimmIter = request.dimms.begin();
				dimmIter != request.dimms.end(); dimmIter++)
	{
		if (dimmHasUnappliedGoal(dimmIter->uid))
		{
			throw core::NvmExceptionDimmHasConfigGoal();
		}
	}
}

bool core::memory_allocator::RuleDimmHasConfigGoal::dimmHasUnappliedGoal(const std::string& dimmUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool hasGoal = false;

	try
	{
		struct config_goal goal = m_nvmLib.getConfigGoal(dimmUid);
		if (goal.status != CONFIG_GOAL_STATUS_SUCCESS)
		{
			hasGoal = true;
		}
	}
	catch (core::LibraryException &e)
	{
		// Goal not existing is OK
		if (e.getErrorCode() != NVM_ERR_NOTFOUND)
		{
			throw;
		}
	}

	return hasGoal;
}
