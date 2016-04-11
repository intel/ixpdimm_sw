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
 * This file contains the provider for the MemoryResources instances which
 * represents system NVM capacity in total regardless of how or whether
 * it is accessible to the host operating system.
 */

#ifndef	_WBEM_MEMCONFIG_MEMORYRESOURCES_FACTORY_H_
#define	_WBEM_MEMCONFIG_MEMORYRESOURCES_FACTORY_H_

#include <string>

#include <framework_interface/NvmInstanceFactory.h>
#include <core/system/SystemService.h>

namespace wbem
{
namespace mem_config
{
	static const std::string MEMORYRESOURCES_ALLOCATIONUNITS_VAL = "bytes"; //!< Allocation Units Static Value
	static const std::string MEMORYRESOURCES_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "MemoryResources"; //!< CreationClassName
	static const std::string MEMORYRESOURCES_INSTANCEID = "NVM Pool"; //!< InstanceID Static String
	static const std::string MEMORYRESOURCES_POOLID = "NVMPool1"; //!< PoolId Static String
	static const std::string MEMORYRESOURCES_ELEMENTNAME = "Platform NVM Primordial Pool"; //!< ElementName Static String
	static const std::string MEMORYRESOURCES_RESOURCETYPE = "Multi-mode memory"; //!< Resource Type Static String
	static const NVM_UINT16 MEMORYRESOURCES_RESOURCETYPE_VAL = 34; //!< Resource Type Static Value

/*!
 * The SNIA base server profile models the host server.
 */
class NVM_API MemoryResourcesFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new MemoryResourcesFactory.
		 */
		MemoryResourcesFactory() throw (framework::Exception);

		/*!
		 * Clean up the MemoryResourcesFactory
		 */
		~MemoryResourcesFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the dimm capacity information.
		 * @return The instance.
		 */
		framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * host server object paths.
		 * @remarks There is only one class per server so return list
		 * should only contain one item.
		 * @return The object path of the instance.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		/*
		 * Helper function to get capacity allocated from pool
		 */
		NVM_UINT64 getCapacityAllocatedFromPool();

		void toInstance(core::system::SystemMemoryResources &memoryResourcesInfo,
				wbem::framework::Instance &instance, wbem::framework::attribute_names_t attributes);
};

} // mem_config
} // wbem
#endif  // #ifndef _WBEM_MEMCONFIG_MEMORYRESOURCES_FACTORY_H_
