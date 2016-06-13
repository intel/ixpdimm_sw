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
 * This file contains the provider for the PersistentMemoryPool instances
 * which represent NVM-DIMM hosted persistent capacity with a given set of QoS attributes.
 */

#include <libinvm-cim/Exception.h>
#include <common_types.h>
#include <framework_interface/NvmInstanceFactory.h>
#include <exception/NvmExceptionLibError.h>

#ifndef _WBEM_PMEMCONFIG_PERSISTENTMEMORYPOOL_FACTORY_H_
#define _WBEM_PMEMCONFIG_PERSISTENTMEMORYPOOL_FACTORY_H_

namespace wbem
{
namespace pmem_config
{
	static const std::string PERSISTENTMEMORYPOOL_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "PersistentMemoryPool"; //!< CreationClassName
	static const std::string PERSISTENTMEMORYPOOL_ELEMENTNAME = "NVMDIMM Available Capacity Pool"; //!< ElementName static string
	static const std::string PERSISTENTMEMORYPOOL_POOLID = "NVMDIMM Available Capacity"; //! PoolID static string
	static const std::string PERSISTENTMEMORYPOOL_GETSUPPORTEDSIZERANGE = "GetSupportedSizeRange"; //! Method string
	static const std::string PERSISTENTMEMORYPOOL_GOAL = "Goal"; //!< GetSupportedSizeRange Method param
	static const std::string PERSISTENTMEMORYPOOL_MIN_NS_SIZE = "MinimumNamespaceSize"; //!< GetSupportedSizeRange Method param
	static const std::string PERSISTENTMEMORYPOOL_MAX_NS_SIZE = "MaximumNamespaceSize"; //!< GetSupportedSizeRange Method param
	static const std::string PERSISTENTMEMORYPOOL_NS_SIZE_DIVISOR = "NamespaceSizeDivisor"; //!< GetSupportedSizeRange Method param
	static const NVM_UINT16 STORAGEVOLUME_ELEMENT_TYPE_VAL = 3; //!< Resource Type Static Value
	static const std::string PERSISTENTMEMORYPOOL_ALLOCATIONUNITS = "bytes"; //! AllocationUnits static string
	static const NVM_UINT16 PERSISTENTMEMORYPOOL_RESOURCETYPE = 35; //! ResourceType other type "Non-Volatile Memory"

	static const NVM_UINT32 PERSISTENTMEMORYPOOL_ERR_NOT_SUPPORTED = 1;
	static const NVM_UINT32 PERSISTENTMEMORYPOOL_ERR_UNKNOWN = 2;
	static const NVM_UINT32 PERSISTENTMEMORYPOOL_ERR_FAILED = 4;
	static const NVM_UINT32 PERSISTENTMEMORYPOOL_ERR_INVALID_PARAMETER = 5;
	static const NVM_UINT32 PERSISTENTMEMORYPOOL_ERR_INSUFFICIENT_RESOURCES = 4097;

	class NVM_API PersistentMemoryPoolFactory: public wbem::framework_interface::NvmInstanceFactory
	{
	public:
		/*!
		 * Construct the PersistentMemoryPoolFactory
		 */
		PersistentMemoryPoolFactory() throw (wbem::framework::Exception);

		/*!
		 * Clean up the PersistentMemoryPoolFactory
		 */
		virtual ~PersistentMemoryPoolFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the PM pool information.
		 * @return The instance.
		 */
		framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * persistent memory pool object paths.
		 * @remarks There may be any number of pools per NVM-DIMM and platform.
		 * @return The object paths of the instances.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

		/*!
		 * Implementation of interface to execute/invoke an extrinsic WBEM method.
		 * @param wbem_return
		 * @param[in] method
		 * 		Extrinsic method return (as defined in MOF).
		 * @param[in] object
		 * 		The object path to the instance supporting the method.
		 * @param[in] inParms
		 * 		STL map of in parameters.
		 * @param[in] outParms
		 * 		STL map of out parameters.
		 * @remarks Caller must delete all inParms and outParms members and maps.
		 * @return HTTP return code (see CIM Operations over HTTP)
		 */
		wbem::framework::UINT32 executeMethod(wbem::framework::UINT32 &wbem_return,
			const std::string method,
			wbem::framework::ObjectPath &object,
			wbem::framework::attributes_t &inParms,
			wbem::framework::attributes_t &outParms);

		/*!
		 * Gets the available capacity for creating a namespace.
		 * @param pool_uid
		 * 		The pool identifier.
		 */
		struct possible_namespace_ranges getSupportedSizeRange(const std::string &poolUid);

		/*!
		 * Gets the available capacities for creating a namespace.
		 * @param pool_uid
		 * 		The pool identifier.
		 */
		virtual void getSupportedSizeRange(const std::string &poolUid,
				COMMON_UINT64 &largestPossibleAdNs,
				COMMON_UINT64 &smallestPossibleAdNs,
				COMMON_UINT64 &adIncrement,
				COMMON_UINT64 &largestPossibleStorageNs,
				COMMON_UINT64 &smallestPossibleStorageNs,
				COMMON_UINT64 &storageIncrement);

		/*!
		 * Provider for extern int nvm_get_available_persistent_size_range
		 * @param[in] pool_id
		 * 		UUID of the pool getting ranges for
		 * @param[in,out] p_range
		 * 		Structure that will contain the ranges
		 */
		int (*m_GetAvailablePersistentSizeRange)(const NVM_UID pool_uid,
				struct possible_namespace_ranges *p_range);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);
	};

} // pmem_config
} // wbem

#endif /* _WBEM_PMEMCONFIG_PERSISTENTMEMORYPOOL_FACTORY_H_ */
