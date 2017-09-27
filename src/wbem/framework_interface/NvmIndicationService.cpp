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

#include "NvmIndicationService.h"

#include <indication/NvmIndicationFactory.h>
#include <libinvm-cim/Instance.h>
#include <LogEnterExit.h>
#include <exception/NvmExceptionLibError.h>

namespace wbem
{
namespace framework_interface
{

NvmIndicationService *NvmIndicationService::m_pSingleton = NULL;

void callback(struct event *pEvent)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	COMMON_LOG_DEBUG("Entering callback...");

	std::vector<wbem::framework::Instance *> indications;
	indication::NvmIndicationFactory::createIndications(pEvent, indications);
	COMMON_LOG_DEBUG_F("Number of indications: %llu", indications.size());
	for (size_t i = 0; i < indications.size(); i++)
	{
		COMMON_LOG_DEBUG("Sending indication");
		wbem::framework::CimomAdapter *pAdapter = NvmIndicationService::getSingleton()->getContext();
		pAdapter->sendIndication(*(indications[i]));
		COMMON_LOG_DEBUG("Sent");

		delete indications[i];
	}
}

NvmIndicationService::NvmIndicationService() : IndicationService(),
		m_eventCallbackId(-1) // anything >= 0 is a valid callback ID
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_pApi = lib_interface::NvmApi::getApi();
}

NvmIndicationService::~NvmIndicationService()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void NvmIndicationService::startIndicating(framework::CimomAdapter* pContext)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_eventCallbackId < 0)
	{
		m_pContext = pContext;

		m_eventCallbackId = m_pApi->addEventNotify(EVENT_TYPE_ALL, callback);
		COMMON_LOG_DEBUG_F("m_eventCallbackID: %d", m_eventCallbackId);
		if (m_eventCallbackId < 0)
		{
			throw exception::NvmExceptionLibError(m_eventCallbackId);
		}
	}
}

void NvmIndicationService::stopIndicating()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_eventCallbackId >= 0)
	{
		m_pApi->removeEventNotify(m_eventCallbackId);
		m_eventCallbackId = -1;

		// Caller controls the context
		m_pContext = NULL;
	}
}

NvmIndicationService* NvmIndicationService::getSingleton()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!m_pSingleton)
	{
		m_pSingleton = new NvmIndicationService();
	}

	return m_pSingleton;
}

} /* namespace framework */
} /* namespace wbem */
