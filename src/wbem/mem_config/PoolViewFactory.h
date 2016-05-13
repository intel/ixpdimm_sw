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
 * This file contains the provider for the PoolView instances. PoolView
 * is an internal view class for the Show -pool CLI command.
 */

#ifndef	_WBEM__INTEL_POOLVIEW_FACTORY_H_
#define	_WBEM__INTEL_POOLVIEW_FACTORY_H_

#include <string>

#include <framework_interface/NvmInstanceFactory.h>


namespace wbem
{
	namespace mem_config
	{
		static const std::string INTEL_POOLVIEW_CREATIONCLASSNAME = "Intel_PoolView"; //!< Creation Class Name static

		static const std::string POOLTYPE_VOLATILE = "Volatile";
		static const std::string POOLTYPE_MIRRORED = "MirroredAppDirect";
		static const std::string POOLTYPE_APPDIRECT = "AppDirect";
		static const std::string POOLTYPE_UNKNOWN = "Unknown";

		static const std::string POOLENCRYPTION_ENCRYPTED = "Encrypted";
		static const std::string POOLENCRYPTION_NOTENCRYPTED = "NotEncrypted";
		static const std::string POOLENCRYPTION_MIXED = "Mixed";

		static const std::string POOL_HEALTH_STR_UNKNOWN = "Unknown";
		static const std::string POOL_HEALTH_STR_NORMAL = "Healthy";
		static const std::string POOL_HEALTH_STR_DEGRADED = "Degraded";
		static const std::string POOL_HEALTH_STR_FAILED = "Failed";

/*!
 * Provider Factory for Intel_PoolView
 */
		class NVM_API PoolViewFactory : public framework_interface::NvmInstanceFactory
		{
		public:

			/*!
			 * Initialize a new Intel_PoolView.
			 */
			PoolViewFactory() throw (framework::Exception);

			/*!
			 * Clean up the PoolViewFactory
			 */
			~PoolViewFactory();

			/*!
			 * Implementation of the standard CIM method to retrieve a specific instance
			 * @param[in] path
			 * 		The object path of the instance to retrieve.
			 * @param[in] attributes
			 * 		The attributes to retrieve.
			 * @throw Exception if unable to retrieve the host information.
			 * @todo Should throw an exception if the object path doesn't match
			 * the results of getHostName.
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

			/*
			 * Helper function to retrieve a list of pools
			 */
			static std::vector<struct pool> getPoolList(bool pmOnly=false) throw (framework::Exception);

			/*
			 * Helper function to retrieve a specific pool.
			 */
			static struct pool *getPool(const std::string &poolUidStr) throw (wbem::framework::Exception);

			static wbem::framework::STR_LIST getAppDirectSettings(const struct pool * pPool);

		private:
			std::vector<struct namespace_details> m_nsCache; // cache for namespace_details to avoid repeated library calls
			void populateAttributeList(framework::attribute_names_t &attributes)
					throw (framework::Exception);

			/*
			 * get the type string for a pool
			 */
			std::string getPoolType(struct pool *pPool);

			/*
			 * Get if the encryption of a pool is encrypted, not, or both
			 */
			std::string getEncryptionCapable(pool *pPool);

			/*
			 * Count number of namespaces on a pool
			 */
			NVM_UINT32 countNamespaces(const struct pool *pPool, namespace_type const type);

			/*
			 * Helper function to get a MB string from bytes.
			 */
			std::string getMbString(const NVM_UINT64 bytes);

			std::string getString(const NVM_UINT64 value);

			/*
			 * initialize namespace cache
			 */
			void lazyInitNs();

			std::string getEncryptionEnabled(const struct pool * pPool);

			std::string getInterleaveSetFormatStr(const struct interleave_format &format);

			std::string poolHealthToStr(const enum pool_health &health);

			std::string getEraseCapable(pool *pPool);
		};

	} // mem_config
} // wbem
#endif  // _WBEM__INTEL_POOLVIEW_FACTORY_H_
