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
 * This file contains the provider for the NVDIMMView instances which
 * is an internal only NVM DIMM view used by the CLI.
 */

#ifndef	_WBEM_PHYSICALASSET_NVDIMM_VIEW_FACTORY_H_
#define	_WBEM_PHYSICALASSET_NVDIMM_VIEW_FACTORY_H_

#include <string>
#include <nvm_management.h>
#include <framework_interface/NvmInstanceFactory.h>


namespace wbem
{
namespace physical_asset
{
static const std::string NVDIMMVIEW_CREATIONCLASSNAMEOLD = std::string(NVM_WBEM_PREFIX) + "NVDIMMView"; //!< Creation ClassName static

// TODO: This will be totally replaced with the WBEM/Logic split (US13017). It is in part replaced, but there
// are several dependencies still.
/*!
 * Provides a view of an Intel NVDIMM
 */
class NVM_API NVDIMMViewFactoryOld : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new NVDIMMViewFactory.
		 */
		NVDIMMViewFactoryOld() throw (wbem::framework::Exception);

		/*!
		 * Clean up the NVDIMMViewFactory
		 */
		~NVDIMMViewFactoryOld();

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

		/*!
		 * Provider for nvm_get_fw_log_level
		 * @param[out,in] p_log_level
		 * 		A buffer for the log_level, allocated by the caller.
		 * @return Returns one of the following @link #return_code return_codes: @endlink @n
		 * 		#NVM_SUCCESS @n
		 * 		#NVM_ERR_NOTSUPPORTED @n
		 * 		#NVM_ERR_NOMEMORY @n
		 * 		#NVM_ERR_INVALIDPARAMETER @n
		 */
		int (*m_GetFwLogLevel)(const NVM_GUID device_guid, enum fw_log_level *p_log_level);

		/*!
		 * Utility method to convert a device physical id to a guid
		 */
		static bool handleToGuid(const NVM_UINT32 &handle, std::string &dimmGuid)
			throw (framework::Exception);

		/*!
		 * Utility method to convert a guid to a handle
		 */
		static void guidToHandle(const std::string &dimmGuid, NVM_UINT32 &handle)
			throw (framework::Exception);

		/*!
		 * Utility method to convert a Dimm GUID to an ID attribute based on the db setting
		 */
		static framework::Attribute guidToDimmIdAttribute(const std::string &dimmGuid)
			throw (framework::Exception);

		/*!
		 * Utility method to convert a Dimm GUID to an string based on the db setting
		 */
		static std::string guidToDimmIdStr(const std::string &dimmGuid)
			throw (wbem::framework::Exception);

		/*!
		 * API indirection for nvm_inject_device_error
		 * @param device_guid
		 * @param passphrase
		 * @param passphrase_len
		 * @return
		 */
		int (*m_injectDeviceError)(const NVM_GUID device_guid,
				const struct device_error *p_error);

		/*!
		 * API indirection for nvm_clear_injected_device_error
		 * @param device_guid
		 * @param passphrase
		 * @param passphrase_len
		 * @return
		 */
		int (*m_clearInjectedDeviceError)(const NVM_GUID device_guid,
				const struct device_error *p_error);

		void injectTemperatureError(const std::string &dimmGuid,
			const NVM_REAL32 temperature)
			throw (framework::Exception);

		void injectPoisonError(const std::string &dimmGuid,
			const NVM_UINT64 dpa)
			throw (framework::Exception);

		void clearPoisonError(const std::string &dimmGuid,
			const NVM_UINT64 dpa)
			throw (framework::Exception);

		void clearAllErrors(const std::string &dimmGuid)
			throw (framework::Exception);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		void injectError(const std::string &dimmGuid,
				struct device_error *p_error)
			throw (wbem::framework::Exception);

		void clearError(const std::string &dimmGuid,
				struct device_error *p_error)
			throw (wbem::framework::Exception);

		std::string getMemoryModesSupported(const struct device_details &dimm);

		std::string getMemoryModeString(const framework::UINT16 mode);

};

} // physical_asset
} // wbem
#endif  // #ifndef _WBEM_PHYSICALASSET_NVDIMM_VIEW_FACTORY_H_
