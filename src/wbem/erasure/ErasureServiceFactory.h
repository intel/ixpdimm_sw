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
 * This file contains the CIM provider for the ErasureService class.
 */


#ifndef	_WBEM_ERASURE_ERASURESERVICE_FACTORY_H_
#define	_WBEM_ERASURE_ERASURESERVICE_FACTORY_H_

#include <string>
#include <server/BaseServerFactory.h>
#include <framework_interface/NvmInstanceFactory.h>
#include <exception/NvmExceptionLibError.h>


namespace wbem
{
namespace erasure
{
	static const std::string ERASURESERVICE_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "ErasureService"; //!< Creation Class Name static
	static const std::string ERASURESERVICE_ELEMENTNAME = "Erasure Service"; //!< Element name
	static const std::string ERASURESERVICE_SYSTEMCREATIONCLASSNAME = wbem::server::BASESERVER_CREATIONCLASSNAME; //!< System Creation class
	static const std::string ERASURESERVICE_ERASEDEVICE = "EraseDevice"; //!< EraseDevice Method
	static const std::string ERASURESERVICE_ERASE = "Erase"; //!< Erase Method
	static const std::string ERASURESERVICE_ERASEDEVICE_ELEMENT = "Element"; //!< EraseDevice Method param
	static const std::string ERASURESERVICE_ERASEDEVICE_ERASUREMETHOD = "ErasureMethod"; //!< EraseDevice Method param
	static const std::string ERASURESERVICE_ERASEDEVICE_PASSWORD = "Password"; //!< EraseDevice Method param

	/*
	 * WBEM version of API erase_type so can be exposed to CLI. Keep aligned with API
	 * erase_type values to make conversion easier.
	 */
	enum eraseType
	{
		ERASETYPE_UNKNOWN = 99,
		ERASETYPE_CRYPTO_ERASE = 2
	};

	static const NVM_UINT32 ERASURESERVICE_ERR_NOT_SUPPORTED = 1;
	static const NVM_UINT32 ERASURESERVICE_ERR_FAILED = 4;
	static const NVM_UINT32 ERASURESERVICE_ERR_PERMISSION_FAILURE = 32768;
	static const NVM_UINT32 ERASURESERVICE_ERR_BAD_STATE = 32769;

/*!
 * Provider Factory for ErasureService
 */
class NVM_API ErasureServiceFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new ErasureService.
		 */
		ErasureServiceFactory() throw (framework::Exception);

		/*!
		 * Clean up the ErasureServiceFactory
		 */
		~ErasureServiceFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the host information.
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

		wbem::framework::UINT32 executeMethod(
			wbem::framework::UINT32 &wbem_return,
			const std::string method,
			wbem::framework::ObjectPath &object,
			wbem::framework::attributes_t &inParms,
			wbem::framework::attributes_t &outParms);


		/*!
		 * Secure erase device
		 * @param deviceUid
		 * 		UID of the device to erase
		 * @param password
		 * 		Password to the device
		 */
		void eraseDevice(std::string deviceUid, std::string password)
				throw (framework::Exception);

		/*!
		 * Secure erase all devices on system
		 * @param password
		 * 		Password to the device
		 */
		void eraseDevice(std::string password)
				throw (framework::Exception);

		/*!
		 * API indirection
		 * @param device_uid
		 * @param passphrase
		 * @param passphrase_len
		 * @return
		 */
		int (*m_eraseDevice)(const NVM_UID device_uid,
				const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len);

		/*!
		 * Function pointer that represents a dependency. Will be assigned default function in
		 * constructor, but can be changed for testing purposes.
		 * @return
		 */
		std::vector<std::string> (*m_GetManageabledeviceUids)();

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		wbem::erasure::eraseType getEraseType(std::string erasureMethod);

		/*
		 * convert NvmExceptionLibError to extrinsic return code
		 */
		wbem::framework::UINT32 getReturnCodeFromLibException(const exception::NvmExceptionLibError &e);
};

} // erasure
} // wbem
#endif  // _WBEM_ERASURE_ERASURESERVICE_FACTORY_H_
