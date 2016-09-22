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

#include "MemoryAllocationGoalCollection.h"
#include <LogEnterExit.h>

namespace core
{
namespace configuration
{

MemoryAllocationGoalCollection::MemoryAllocationGoalCollection()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

MemoryAllocationGoalCollection::MemoryAllocationGoalCollection(
		const MemoryAllocationGoalCollection& other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	*this = other;
}

MemoryAllocationGoalCollection::~MemoryAllocationGoalCollection()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	clear();
}

MemoryAllocationGoalCollection& MemoryAllocationGoalCollection::operator=(
		const MemoryAllocationGoalCollection& other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (this != &other)
	{
		clear();
		for (std::map<std::string, MemoryAllocationGoal *>::const_iterator otherGoal =
				other.m_goals.begin(); otherGoal != other.m_goals.end(); otherGoal++)
		{
			this->m_goals[otherGoal->first] = otherGoal->second->clone();
		}
	}

	return *this;
}

MemoryAllocationGoal& MemoryAllocationGoalCollection::operator[](
		const std::string& deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!hasGoalForDevice(deviceUid))
	{
		throw GoalNotFound();
	}

	return *(m_goals[deviceUid]);
}

bool MemoryAllocationGoalCollection::hasGoalForDevice(const std::string& deviceUid) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool found = false;
	std::map<std::string, MemoryAllocationGoal *>::const_iterator goalIter =
			m_goals.find(deviceUid);
	if (goalIter != m_goals.end())
	{
		found = true;
	}

	return found;
}

size_t MemoryAllocationGoalCollection::size() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_goals.size();
}

void MemoryAllocationGoalCollection::push_back(MemoryAllocationGoal& goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string uid = goal.getDeviceUid();
	safeDelete(&(m_goals[uid]));
	m_goals[uid] = goal.clone();
}

void MemoryAllocationGoalCollection::clear()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::map<std::string, MemoryAllocationGoal *>::iterator goal = m_goals.begin();
			goal != m_goals.end(); goal++)
	{
		MemoryAllocationGoal *pGoal = goal->second;
		safeDelete(&pGoal);
	}

	m_goals.clear();
}

void MemoryAllocationGoalCollection::safeDelete(MemoryAllocationGoal **ppGoal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	delete *ppGoal;
	*ppGoal = NULL;
}

std::vector<std::string> MemoryAllocationGoalCollection::getDeviceUidsForGoals() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<std::string> uids;
	for (std::map<std::string, MemoryAllocationGoal *>::const_iterator goal = m_goals.begin();
			goal != m_goals.end(); goal++)
	{
		uids.push_back(goal->first);
	}

	return uids;
}

} /* namespace configuration */
} /* namespace core */
