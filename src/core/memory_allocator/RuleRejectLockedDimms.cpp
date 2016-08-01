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

#include "RuleRejectLockedDimms.h"

#include <LogEnterExit.h>
#include <uid/uid.h>
#include <core/exceptions/NvmExceptionBadRequest.h>

core::memory_allocator::RuleRejectLockedDimms::RuleRejectLockedDimms(
		const std::vector<struct device_discovery> &manageableDevices) :
		m_manageableDevices(manageableDevices)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::RuleRejectLockedDimms::~RuleRejectLockedDimms()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::RuleRejectLockedDimms::verify(const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<Dimm>::const_iterator iter = request.dimms.begin(); iter != request.dimms.end(); iter++)
	{
		if (isDimmLocked(*iter))
		{
			throw core::NvmExceptionRequestedDimmLocked();
		}
	}
}

bool core::memory_allocator::RuleRejectLockedDimms::isDimmLocked(const core::memory_allocator::Dimm& dimm)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool isLocked = false;

	NVM_UID dimmUid;
	uid_copy(dimm.uid.c_str(), dimmUid);

	for (std::vector<struct device_discovery>::const_iterator iter = m_manageableDevices.begin();
			iter != m_manageableDevices.end(); iter++)
	{
		if (uid_cmp(iter->uid, dimmUid))
		{
			isLocked = isSecurityStateLocked(iter->lock_state);
			break;
		}
	}

	return isLocked;
}

bool core::memory_allocator::RuleRejectLockedDimms::isSecurityStateLocked(const enum lock_state securityState)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool lockedState = false;

	switch (securityState)
	{
	case LOCK_STATE_LOCKED:
	case LOCK_STATE_PASSPHRASE_LIMIT:
	case LOCK_STATE_UNKNOWN:
		lockedState = true;
		break;
	default:
		lockedState = false;
	}

	return lockedState;
}
