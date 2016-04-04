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

#include "LayoutStepPersistentSettingsNotRecommended.h"
#include <LogEnterExit.h>

wbem::logic::LayoutStepPersistentSettingsNotRecommended::LayoutStepPersistentSettingsNotRecommended(
		const struct platform_capabilities &pcap) : m_platformCapabilities(pcap)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

wbem::logic::LayoutStepPersistentSettingsNotRecommended::~LayoutStepPersistentSettingsNotRecommended()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void wbem::logic::LayoutStepPersistentSettingsNotRecommended::execute(
		const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<PersistentExtent>::const_iterator pmIter = request.persistentExtents.begin();
			pmIter != request.persistentExtents.end(); pmIter++)
	{
		// check way + iMC size + channel size
		if (pmIter->byOne ||
			(!pmIter->byOne &&
			!(pmIter->channel == wbem::logic::REQUEST_DEFAULT_INTERLEAVE_FORMAT ||
			  pmIter->imc == wbem::logic::REQUEST_DEFAULT_INTERLEAVE_FORMAT)))
		{
			if (!formatRecommended(*pmIter))
			{
				// only add the warning once
				layout.warnings.push_back(LAYOUT_WARNING_PERSISTENT_SETTINGS_NOT_RECOMMENDED);
				break;
			}
		}
	}
}

bool wbem::logic::LayoutStepPersistentSettingsNotRecommended::formatRecommended(
		const struct wbem::logic::PersistentExtent &pmRequest)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool isRecommended = false;

	for (NVM_UINT16 i  = 0; i < m_platformCapabilities.pm_direct.interleave_formats_count; i++)
	{
		struct interleave_format &format = m_platformCapabilities.pm_direct.interleave_formats[i];
		if (format.ways == INTERLEAVE_WAYS_1)
		{
			 if (pmRequest.byOne && format.recommended)
			 {
				 isRecommended = true;
				 break;
			 }
		}
		else
		{
			if (pmRequest.imc == format.imc &&
				pmRequest.channel == format.channel &&
				format.recommended)
			{
				isRecommended = true;
				break;
			}
		}
	}

	return isRecommended;
}
