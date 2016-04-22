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
 * This file contains the provider for the NamespaceView instances which is
 * an internal only view class for the show namespace CLI command.
 */

#ifndef	_WBEM_PMEMCONFIG_NAMESPACEVIEW_FACTORY_H_
#define	_WBEM_PMEMCONFIG_NAMESPACEVIEW_FACTORY_H_

#include <string>
#include <nvm_management.h>
#include <framework_interface/NvmInstanceFactory.h>

namespace wbem
{
namespace pmem_config
{
	static const std::string NAMESPACEVIEW_CREATIONCLASSNAME = "Intel_NamespaceView";

	// namespace access type values
	static const NVM_UINT16 NS_ACCESSTYPE_READONLY = 2;
	static const NVM_UINT16 NS_ACCESSTYPE_READWRITE = 4;

	// namespace optimize values
	static const NVM_UINT16 NS_OPTIMIZE_NONE = 2;
	static const NVM_UINT16 NS_OPTIMIZE_PERFORMANCE = 3;
	static const NVM_UINT16 NS_OPTIMIZE_SIZE = 4;
	static const NVM_UINT16 NS_OPTIMIZE_COPYONWRITE = 5;

	// namespace type strings
	static const std::string NS_TYPE_STR_UNKNOWN = "Unknown";
	static const std::string NS_TYPE_STR_STORAGE = "Storage";
	static const std::string NS_TYPE_STR_APPDIRECT = "AppDirect";

	// namespace health strings
	static const std::string NS_HEALTH_STR_UNKNOWN = "Unknown";
	static const std::string NS_HEALTH_STR_NORMAL = "Healthy";
	static const std::string NS_HEALTH_STR_WARN = "Warning";
	static const std::string NS_HEALTH_STR_ERR = "Critical";
	static const std::string NS_HEALTH_STR_BROKENMIRROR = "BrokenMirror";

	// namespace security values 
	static const NVM_UINT16 NS_SECURITY_ENCRYPTION_OFF = 0;
	static const NVM_UINT16 NS_SECURITY_ENCRYPTION_ON = 1;
	static const NVM_UINT16 NS_SECURITY_ERASE = 2;

	// namespace memory page allocation strings
	static const std::string NS_MEMORY_PAGE_ALLOCATION_STR_NONE = "None";
	static const std::string NS_MEMORY_PAGE_ALLOCATION_STR_DRAM = "DRAM";
	static const std::string NS_MEMORY_PAGE_ALLOCATION_STR_APP_DIRECT = "AppDirect";

/*!
 * Provider Factory for Intel_NamespaceView
 */
class NVM_API NamespaceViewFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new NamespaceViewFactory.
		 */
		NamespaceViewFactory() throw (framework::Exception);

		/*!
		 * Clean up the NamespaceViewFactory.
		 */
		~NamespaceViewFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the namespace details.
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
		 * Helper function to retrieve a list of namespace Uids
		 */
		static std::vector<std::string> getNamespaceUidList()
			throw (framework::Exception);

		/*
		 * Helper function to retrieve details about a specific namespace.
		 */
		static struct namespace_details getNamespaceDetails(const std::string &nsId)
			throw (framework::Exception);

		/*
		 * Helper function to convert namespace health to a string
		 */
		static std::string namespaceHealthToStr(const enum namespace_health &health);

		/*
		 * Helper function to convert namespace enable state to a string
		 */
		static std::string namespaceEnableStateToStr(
				const enum namespace_enable_state &state);

		/*
		 * Helper function to convert namespace type to a string
		 */
		static std::string namespaceTypeToStr(const enum namespace_type &type);

		/*
		 * Helper function to convert namespace btt to an optimize string
		 */
		static std::string namespaceOptimizeToStr(const NVM_BOOL &btt);

		/*
		 * Helper function to convert namespace btt to an optimize value
		 */
		static NVM_UINT16 namespaceOptimizeToValue(const NVM_BOOL &btt);

		/*
 		 * Helper function to convert namespace security attributes to a security value
		 */
		static framework::UINT16_LIST namespaceSecurityToValue(struct namespace_security_features security);

		/*
		 * Helper function to convert namespace memory page allocation attribute to a memory page allocation string
		 */
		static std::string namespaceMemoryPageAllocationToStr(const enum namespace_memory_page_allocation allocation);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
		throw (framework::Exception);
}; // class

} // pmem_config
} // wbem
#endif  // _WBEM_PMEMCONFIG_NAMESPACEVIEW_FACTORY_H_
