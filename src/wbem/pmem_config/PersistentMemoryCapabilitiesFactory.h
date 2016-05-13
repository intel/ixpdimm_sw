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
 * This file contains the provider for the PeristentMemoryCapabilities instances.
 */

#ifndef	_WBEM_PMEMCONFIG_PERSISTENTMEMORYCAPABILITIES_FACTORY_H_
#define	_WBEM_PMEMCONFIG_PERSISTENTMEMORYCAPABILITIES_FACTORY_H_

#include <string>

#include <framework_interface/NvmInstanceFactory.h>
#include <exception/NvmExceptionLibError.h>


namespace wbem
{
namespace pmem_config
{
	static const std::string PMCAP_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "PersistentMemoryCapabilities";
	static const std::string PMCAP_ELEMENTNAME = "Pool Capabilities for: ";
	static const NVM_UINT16 PMCAP_REPLICATION_LOCAL = 3;
	static const NVM_UINT16 PMCAP_MEMORYARCHITECTURE_NUMA = 3;
	static const NVM_UINT16 PMCAP_ACCESSGRANULARITY_BLOCK = 1;
	static const NVM_UINT16 PMCAP_ACCESSGRANULARITY_BYTE = 2;
	static const NVM_UINT16 PMCAP_SECURITYFEATURES_ENCRYPTION = 3;
	static const NVM_UINT16 PMCAP_SECURITYFEATURES_CRYPTO = 4;
	static const std::string PMCAP_GETBLOCKSIZES = "GetSupportedBlockSizes"; //!< extrinsic method name
	static const std::string PMCAP_BLOCKSIZES_PARAMNAME = "BlockSizes"; //!< extrinsic method out param

	static const NVM_UINT32 PMCAP_ERR_NOT_SUPPORTED = 1;
	static const NVM_UINT32 PMCAP_ERR_UNKNOWN = 2;
	static const NVM_UINT32 PMCAP_ERR_FAILED = 4;
	static const NVM_UINT32 PMCAP_ERR_INVALID_PARAMETER = 5;
	static const NVM_UINT32 PMCAP_ERR_INSUFFICIENT_RESOURCES = 4097;

/*!
 * Provider Factory for Intel_PersistentMemoryNamespace
 */
class NVM_API PersistentMemoryCapabilitiesFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new PersistentMemoryCapabilitiesFactory.
		 */
		PersistentMemoryCapabilitiesFactory() throw (framework::Exception);

		/*!
		 * Clean up the PersistentMemoryCapabilitiesFactory
		 */
		~PersistentMemoryCapabilitiesFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the pool information.
		 * @return The instance.
		 */
		framework::Instance* getInstance(framework::ObjectPath &path,
				framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * object paths.
		 * @return The object path.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM extrinsic method
		 */
		wbem::framework::UINT32 executeMethod(
					wbem::framework::UINT32 &wbem_return,
					const std::string method,
					wbem::framework::ObjectPath &object,
					wbem::framework::attributes_t &inParms,
					wbem::framework::attributes_t &outParms);


		/*!
		 * Determine the security features of the dimms in the pool
		 */
		framework::UINT16_LIST getPoolSecurityFeatures(struct pool *pPool) throw (framework::Exception);

		framework::UINT64 getMaxNamespacesPerPool(struct pool *pPool, NVM_UINT64 min_namespace_size)
		throw (framework::Exception);

		/*!
		 * Get the supported block sizes
		 */
		virtual void getSupportedBlockSizes(framework::UINT64_LIST &list);

		/*!
		 * Get memory page allocation capability
		 */
		virtual NVM_BOOL getMemoryPageAllocationCapability();

		/*!
		 * Retrieve platform NVM capabilities
		 */
		static struct nvm_capabilities getNvmCapabilities() throw (wbem::framework::Exception);

		/*
		 * Retrieve pool struct given an object path
		 */
		static struct pool *getPool(wbem::framework::ObjectPath &object) throw (wbem::framework::Exception);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
				throw (framework::Exception);

		/*
		 * convert NvmExceptionLibError to extrinsic return code
		 */
		wbem::framework::UINT32 getReturnCodeFromLibException(exception::NvmExceptionLibError e);
}; // class

} // pmem_config
} // wbem
#endif  // _WBEM_PMEMCONFIG_PERSISTENTMEMORYCAPABILITIES_FACTORY_H_
