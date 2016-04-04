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
 * Rule to determine if persistent memory request is supported
 */

#include "RulePersistentNotSupported.h"
#include <exception/NvmExceptionBadRequest.h>
#include <LogEnterExit.h>

wbem::logic::RulePersistentNotSupported::RulePersistentNotSupported(
		const struct nvm_capabilities &cap) : m_systemCap(cap)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

wbem::logic::RulePersistentNotSupported::~RulePersistentNotSupported()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void wbem::logic::RulePersistentNotSupported::verify(const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (request.persistentExtents.size() > 0)
	{
		verifyAppDirectSupported();
		verifyPersistentSettingsSupported(request);
	}
}

void wbem::logic::RulePersistentNotSupported::verifyAppDirectSupported()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!m_systemCap.platform_capabilities.pm_direct.supported)
	{
		throw exception::NvmExceptionRequestNotSupported();
	}
}

void wbem::logic::RulePersistentNotSupported::verifyPersistentSettingsSupported(
		const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<PersistentExtent>::const_iterator iter = request.persistentExtents.begin();
				iter != request.persistentExtents.end(); iter++)
	{
		// check way + iMC size + channel size
		if (!formatSupported(*iter))
		{
			throw exception::NvmExceptionPersistentSettingsNotSupported();
		}
	}
}

bool wbem::logic::RulePersistentNotSupported::formatSupported(
		const struct wbem::logic::PersistentExtent &pmRequest)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool supported = false;
	for (NVM_UINT16 i  = 0; i < m_systemCap.platform_capabilities.pm_direct.interleave_formats_count; i++)
	{
		struct interleave_format &format = m_systemCap.platform_capabilities.pm_direct.interleave_formats[i];
		if (pmRequest.byOne)
		{
			if (format.ways == INTERLEAVE_WAYS_1)
			{
				supported = true;
				break;
			}
		}
		else
		{
			// if either is set to default then both are
			if (pmRequest.imc == wbem::logic::REQUEST_DEFAULT_INTERLEAVE_FORMAT ||
				pmRequest.channel == wbem::logic::REQUEST_DEFAULT_INTERLEAVE_FORMAT)
			{
				supported = true;
				break;
			}

			if (pmRequest.imc == format.imc &&
				pmRequest.channel == format.channel)
			{
				supported = true;
				break;
			}
		}
	}
	return supported;
}
