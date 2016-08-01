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
 * Rule to determine if app direct memory request is supported
 */

#include "RuleAppDirectNotSupported.h"

#include <LogEnterExit.h>
#include <core/exceptions/NvmExceptionBadRequest.h>

core::memory_allocator::RuleAppDirectNotSupported::RuleAppDirectNotSupported(
		const struct nvm_capabilities &cap) : m_systemCap(cap)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::RuleAppDirectNotSupported::~RuleAppDirectNotSupported()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::RuleAppDirectNotSupported::verify(const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (request.appDirectExtents.size() > 0)
	{
		verifyAppDirectSupported();
		verifyAppDirectSettingsSupported(request);
	}
}

void core::memory_allocator::RuleAppDirectNotSupported::verifyAppDirectSupported()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!m_systemCap.platform_capabilities.app_direct_mode.supported)
	{
		throw core::NvmExceptionRequestNotSupported();
	}
}

void core::memory_allocator::RuleAppDirectNotSupported::verifyAppDirectSettingsSupported(
		const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<AppDirectExtent>::const_iterator iter = request.appDirectExtents.begin();
				iter != request.appDirectExtents.end(); iter++)
	{
		// check way + iMC size + channel size
		if (!formatSupported(*iter))
		{
			throw core::NvmExceptionAppDirectSettingsNotSupported();
		}
	}
}

bool core::memory_allocator::RuleAppDirectNotSupported::formatSupported(
		const struct core::memory_allocator::AppDirectExtent &adRequest)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool supported = false;
	for (NVM_UINT16 i  = 0; i < m_systemCap.platform_capabilities.app_direct_mode.interleave_formats_count; i++)
	{
		struct interleave_format &format = m_systemCap.platform_capabilities.app_direct_mode.interleave_formats[i];
		if (adRequest.byOne)
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
			if (adRequest.imc == core::memory_allocator::REQUEST_DEFAULT_INTERLEAVE_FORMAT ||
				adRequest.channel == core::memory_allocator::REQUEST_DEFAULT_INTERLEAVE_FORMAT)
			{
				supported = true;
				break;
			}

			if (adRequest.imc == format.imc &&
				adRequest.channel == format.channel)
			{
				supported = true;
				break;
			}
		}
	}
	return supported;
}
