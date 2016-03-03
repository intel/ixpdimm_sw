/*
 * Copyright (c) 2015, Intel Corporation
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
 * Rule that checks that no namespaces exist on the dimms requested
 */

#include "RuleNamespacesExist.h"
#include <exception/NvmExceptionLibError.h>
#include <exception/NvmExceptionBadRequest.h>
#include <LogEnterExit.h>
#include <guid/guid.h>
#include <nvm_management.h>
#include <lib_interface/NvmApi.h>

wbem::logic::RuleNamespacesExist::RuleNamespacesExist(
		const struct nvm_capabilities &systemCapabilities) :
		m_systemCapabilities(systemCapabilities)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

wbem::logic::RuleNamespacesExist::~RuleNamespacesExist()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void wbem::logic::RuleNamespacesExist::verify(const MemoryAllocationRequest &request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();

	// check each dimm in the request to every namespace
	for (std::vector<struct Dimm>::const_iterator dimmIter = request.dimms.begin();
			dimmIter != request.dimms.end(); dimmIter++)
	{
		NVM_GUID dimmGuid;
		str_to_guid(dimmIter->guid.c_str(), dimmGuid);
		int nsCount = pApi->getDeviceNamespaceCount(dimmGuid, NAMESPACE_TYPE_UNKNOWN);
		if (nsCount < 0) // error
		{
			// If retrieving NS' is not supported, allow Volatile goal creation on MemoryMode SKU
			if (nsCount == NVM_ERR_NOTSUPPORTED)
			{
				if ((request.volatileCapacity != 0) &&
						(request.persistentExtents.size() == 0) &&
						(!request.storageRemaining))
				{
					if (!m_systemCapabilities.nvm_features.memory_mode)
					{
						throw exception::NvmExceptionVolatileNotSupported();
					}
				}
				else
				{
					throw exception::NvmExceptionLibError(nsCount);
				}
			}
			else
			{
				throw exception::NvmExceptionLibError(nsCount);
			}
		}
		else if (nsCount > 0) // namespaces exist
		{
			COMMON_LOG_ERROR_F("%d namespaces exist on " NVM_DIMM_NAME " %s",
					nsCount, dimmIter->guid.c_str());
			throw exception::NvmExceptionNamespacesExist();
		}
	}
}
