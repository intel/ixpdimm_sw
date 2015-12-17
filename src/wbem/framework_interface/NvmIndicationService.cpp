/*
 * NvmIndicationService.cpp
 *
 * Copyright 2015 Intel Corporation All Rights Reserved.
 *
 * INTEL CONFIDENTIAL
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its suppliers
 * or licensors. Title to the Material remains with Intel Corporation or its
 * suppliers and licensors. The Material may contain trade secrets and
 * proprietary and confidential information of Intel Corporation and its
 * suppliers and licensors, and is protected by worldwide copyright and trade
 * secret laws and treaty provisions. No part of the Material may be used,
 * copied, reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way without Intel's prior express written
 * permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be
 * express and approved by Intel in writing.
 *
 * Unless otherwise agreed by Intel in writing, you may not remove or alter this
 * notice or any other notice embedded in Materials by Intel or Intel's
 * suppliers or licensors in any way.
 */

#include "NvmIndicationService.h"

#include <indication/NvmIndicationFactory.h>
#include <intel_cim_framework/Instance.h>
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
