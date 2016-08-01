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
 * Add a layout warning if BIOS doesn't recommend the specified interleave format.
 */

#include "LayoutStepAppDirectSettingsNotRecommended.h"

#include <LogEnterExit.h>

core::memory_allocator::LayoutStepAppDirectSettingsNotRecommended::LayoutStepAppDirectSettingsNotRecommended(
		const struct platform_capabilities &pcap) : m_platformCapabilities(pcap)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::LayoutStepAppDirectSettingsNotRecommended::~LayoutStepAppDirectSettingsNotRecommended()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::LayoutStepAppDirectSettingsNotRecommended::execute(
		const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<AppDirectExtent>::const_iterator appDirectIter = request.appDirectExtents.begin();
			appDirectIter != request.appDirectExtents.end(); appDirectIter++)
	{
		// check way + iMC size + channel size
		if (appDirectIter->byOne ||
			(!appDirectIter->byOne &&
			!(appDirectIter->channel == core::memory_allocator::REQUEST_DEFAULT_INTERLEAVE_FORMAT ||
			  appDirectIter->imc == core::memory_allocator::REQUEST_DEFAULT_INTERLEAVE_FORMAT)))
		{
			if (!formatRecommended(*appDirectIter))
			{
				// only add the warning once
				layout.warnings.push_back(LAYOUT_WARNING_APP_DIRECT_SETTINGS_NOT_RECOMMENDED);
				break;
			}
		}
	}
}

bool core::memory_allocator::LayoutStepAppDirectSettingsNotRecommended::formatRecommended(
		const struct core::memory_allocator::AppDirectExtent &appDirectRequest)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool isRecommended = false;

	for (NVM_UINT16 i  = 0; i < m_platformCapabilities.app_direct_mode.interleave_formats_count; i++)
	{
		struct interleave_format &format = m_platformCapabilities.app_direct_mode.interleave_formats[i];
		if (format.ways == INTERLEAVE_WAYS_1)
		{
			 if (appDirectRequest.byOne && format.recommended)
			 {
				 isRecommended = true;
				 break;
			 }
		}
		else
		{
			if (appDirectRequest.imc == format.imc &&
				appDirectRequest.channel == format.channel &&
				format.recommended)
			{
				isRecommended = true;
				break;
			}
		}
	}

	return isRecommended;
}
