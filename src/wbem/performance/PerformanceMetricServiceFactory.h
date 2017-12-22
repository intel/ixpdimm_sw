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

/*
 * This file contains the provider for the PerformanceMetricService instances
 * which define an NVM DIMM performance metric service.
 */

#ifndef _WBEM_PERFORMANCE_METRIC_SERVICE_FACTORY_H_
#define _WBEM_PERFORMANCE_METRIC_SERVICE_FACTORY_H_

#include <framework_interface/NvmInstanceFactory.h>

namespace wbem
{
namespace performance
{

static const std::string PERFORMANCEMETRICSERVICE_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "PerformanceMetricService";

static const std::string PERFORMANCEMETRICSERVICE_NAME = "NVDIMM Performance Metric Service for ";

/*!
 * Provider Factory for PerformanceMetricService
 */
class NVM_CIM_API PerformanceMetricServiceFactory : public framework_interface::NvmInstanceFactory
{
public:

	PerformanceMetricServiceFactory() throw (framework::Exception);

	~PerformanceMetricServiceFactory();

	framework::instance_names_t* getInstanceNames() throw (framework::Exception);

	framework::Instance* getInstance(framework::ObjectPath &path,
		framework::attribute_names_t &attributes) throw (framework::Exception);

	framework::Instance* modifyInstance(framework::ObjectPath &path, framework::attributes_t &attributes)
		throw (framework::Exception);

private:

	void validateObjectPath(framework::ObjectPath &path, std::string hostName) throw (wbem::framework::Exception);
	void populateAttributeList(framework::attribute_names_t &attributes) throw (framework::Exception);
};

}
}

#endif /* _WBEM_PERFORMANCE_METRIC_SERVICE_FACTORY_H_ */
