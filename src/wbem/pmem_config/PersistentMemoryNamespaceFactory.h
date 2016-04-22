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
 * This file contains the provider for the PeristentMemoryNamespace instances.
 */

#ifndef	_WBEM_PMEMCONFIG_PERSISTENTMEMORYNAMESPACE_FACTORY_H_
#define	_WBEM_PMEMCONFIG_PERSISTENTMEMORYNAMESPACE_FACTORY_H_

#include <string>

#include <framework_interface/NvmInstanceFactory.h>
#include <exception/NvmExceptionLibError.h>


namespace wbem
{
namespace pmem_config
{
	static const std::string PMNS_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "PersistentMemoryNamespace";

	// supported extrinsic methods
	static const std::string PM_NAMESPACE_REQUESTSTATECHANGE = "RequestStateChange";

	// parameters of extrinsic methods
	static const std::string PM_NAMESPACE_STATE = "RequestedState";

	// Error constants
	static const NVM_UINT32 PM_NAMESPACE_ERR_FAILED = 5;

	// constants used in RequestStateChange
	static const NVM_UINT32 PM_NAMESPACE_ENABLE_STATE_UNKNOWN = 0;
	static const NVM_UINT32 PM_NAMESPACE_ENABLE_STATE_ENABLED = 2; // enabled
	static const NVM_UINT32 PM_NAMESPACE_ENABLE_STATE_DISABLED = 3; // disabled

	// constants for operational status
	static const NVM_UINT16 PM_NAMESPACE_OPSTATUS_INSERVICE = 11; // In service value
	static const NVM_UINT16 PM_NAMESPACE_OPSTATUS_STOPPED = 10; // Stopped value
	static const NVM_UINT16 PM_NAMESPACE_OPSTATUS_UNKNOWN = 0; // Unknown value

	static const NVM_UINT32 PM_NAMESPACE_NOT_SUPPORTED = 1;
	static const NVM_UINT32 PM_NAMESPACE_UNKNOWN = 2;
	static const NVM_UINT32 PM_NAMESPACE_FAILED = 4;
	static const NVM_UINT32 PM_NAMESPACE_INVALID_PARAMETER = 5;
	static const NVM_UINT32 PM_NAMESPACE_IN_USE = 6;
	static const NVM_UINT32 PM_NAMESPACE_INVALID_STATE_TRANSITION = 4097;

/*!
 * Provider Factory for Intel_PersistentMemoryNamespace
 */
class NVM_API PersistentMemoryNamespaceFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new PersistentMemoryNamespaceFactory.
		 */
		PersistentMemoryNamespaceFactory() throw (framework::Exception);

		/*!
		 * Clean up the PersistentMemoryNamespaceFactory
		 */
		~PersistentMemoryNamespaceFactory();

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

		/*!
		 * Test to see if to instances are associated.
		 * @param[in] associationClass
		 * 		Pointer to the association class
		 * @param[in] pAntInstance
		 * 		Pointer to the antecedent instance
		 * @param pDepInstance
		 * 		Pointer to the dependent instance
		 * @return true if there is a antecedent/dependent relationship.
		 */
		bool isAssociated(const std::string &associationClass, framework::Instance* pAntInstance,
			 framework::Instance* pDepInstance);

		/*!
		 * Entry point for CIM extrinsic methods
		 */
		wbem::framework::UINT32 executeMethod(
				wbem::framework::UINT32 &wbem_return,
				const std::string method,
				wbem::framework::ObjectPath &object,
				wbem::framework::attributes_t &inParms,
				wbem::framework::attributes_t &outParms);

		/*!
		 * Interface to the library API nvm_modify_namespace function.
		 * This pointer allows for dependency injection and decouples the dependency on the API
		 */
		int (*m_modifyNamespaceEnabled)(const NVM_UID namespace_uid, const enum namespace_enable_state enabled);

		/*!
		 * modify a namespace
		 */
		virtual void modifyNamespace(std::string namespaceUidStr, const NVM_UINT16 stateValue);

		/*!
		 * create the object path give the namespace UID
		 */
		static void createPathFromUid(const NVM_UID nsUid,
				framework::ObjectPath &path);
		static void createPathFromUid(const std::string &nsUid,
				framework::ObjectPath &path);

		/*
		 * Return true if the device driver is capable of making the modification to the enable state
		 */
		virtual bool isModifyNamespaceEnabledSupported(const enum namespace_enable_state enabled);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
				throw (framework::Exception);

		/*
		 * Helper fn to retrieve namespace details from a namespace instance
		 */
		int namespaceDetailsFromInstance(wbem::framework::Instance *pNsInstance,
				struct namespace_details *pNsDetails);

		wbem::framework::Attribute getOperationalStatusAttr(const struct namespace_details &ns);

		/*
		 * convert NvmExceptionLibError to extrinsic return code
		 */
		wbem::framework::UINT32 getReturnCodeFromLibException(exception::NvmExceptionLibError e);
}; // class

} // pmem_config
} // wbem
#endif  // _WBEM_PMEMCONFIG_PERSISTENTMEMORYNAMESPACE_FACTORY_H_
