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
 * This file contains the provider for the SupportDataService instance
 * that provides the capability to capture and export support data.
 */

#ifndef	_WBEM_SUPPORT_SUPPORTDATASERVICE_FACTORY_H_
#define	_WBEM_SUPPORT_SUPPORTDATASERVICE_FACTORY_H_

#include <string>
#include <nvm_management.h>
#include <server/BaseServerFactory.h>
#include <framework_interface/NvmInstanceFactory.h>


namespace wbem
{
namespace support
{
	static const std::string SUPPORTDATASERVICE_SYSTEMCREATIONCLASSNAME = wbem::server::BASESERVER_CREATIONCLASSNAME; //!< System Creation ClassName static
	static const std::string SUPPORTDATASERVICE_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "SupportDataService"; //!< Creation ClassName static
	static const std::string SUPPORTDATASERVICE_NAME = "NVDIMM Support Service"; //!< Name static

	static const std::string SUPPORTDATASERVICE_CREATE = "Create"; //!< extrinsic method name
	static const std::string SUPPORTDATASERVICE_EXPORTTOURI = "ExportToUri"; //!< extrinsic method name
	static const std::string SUPPORTDATASERVICE_EXPORTURI = "ExportUri"; //!< method param
	static const std::string SUPPORTDATASERVICE_OMD_OBJPATH = "OpaqueManagementData"; //!< method param
	static const std::string SUPPORTDATASERVICE_ELEMNAME = "ElementName"; //!< method param

	static const std::string SUPPORTDATASERVICE_CLEAR = "Clear"; //!< extrinsic method name

	static const NVM_UINT32 SUPPORTDATASERVICE_ERR_UNKNOWN = 2;

/*!
 * Models the physical aspects of an Intel NVDIMM
 */
class NVM_API SupportDataServiceFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new SupportDataServiceFactory.
		 */
		SupportDataServiceFactory() throw (framework::Exception);

		/*!
		 * Clean up the SupportDataServiceFactory
		 */
		~SupportDataServiceFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the DIMM info
		 * @return The NVDIMM instance.
		 */
		framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * NVDIMM object paths.
		 * @throw Exception if unable to retrieve the DIMM list
		 * @throw Exception if unable to retrieve the server name
		 * @return The object paths for each NVDIMM.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

		// Extrinsic methods
		/*!
		 * Implementation of the standard CIM method to capture system support information
		 * @param elementName name associated with the support snapshot created by this call
		 * @param opaqueManagementData object path of the single OpaqueManagementData instance
		 * @throw Exception if unable to save the support data
		 */
		void create(const std::string elementName, const framework::ObjectPath& opaqueManagementData)
		const throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to export accumulated system support information
		 * @param exportUri file to hold exported support data
		 * @throw Exception if unable to save the support data
		 */
		void exportToUri(const std::string exportUri) const throw (framework::Exception);


		/*!
		 * Implementation of the vendor specific clean extrinsic method
		 */
		void clear() throw (framework::Exception);

		/*!
		 * Provider for nvm_purge_state_data
		 * @return
		 */
		int (*m_PurgeStateData)();

		/*!
		 * Provider for nvm_save_state
		 * @param name Name given to the snapshot
		 * @param name_len length of name
		 */
		int (*m_SaveStateProvider)(const char *name, const NVM_SIZE name_len);

		/*!
		 * Provider for nvm_gather_support
		 * @param support_file snapshot will be saved to this file
		 * @param support_file_len length of support_file name
		 */
		int (*m_GatherSupportProvider)(const NVM_PATH support_file, const NVM_SIZE support_file_len);

		wbem::framework::UINT32 executeMethod(
					wbem::framework::UINT32 &wbem_return,
					const std::string method,
					wbem::framework::ObjectPath &object,
					wbem::framework::attributes_t &inParms,
					wbem::framework::attributes_t &outParms);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);
};

} // support
} // wbem
#endif  // #ifndef _WBEM_SUPPORT_SUPPORTDATASERVICE_FACTORY_H_
