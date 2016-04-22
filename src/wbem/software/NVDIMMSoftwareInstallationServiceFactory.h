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
 * This file contains the provider for the NVDIMMSoftwareInstallationService instance
 * which provides functionality to update the FW on an NVM DIMM.
 */


#ifndef	_WBEM_SOFTWARE_NVDIMMSOFTWAREINSTALLATIONSERVICE_FACTORY_H_
#define	_WBEM_SOFTWARE_NVDIMMSOFTWAREINSTALLATIONSERVICE_FACTORY_H_

#include <string>
#include <vector>

#include <framework_interface/NvmInstanceFactory.h>
#include <exception/NvmExceptionLibError.h>


namespace wbem
{
namespace software
{
	static const std::string NVDIMMSOFTWAREINSTALLATIONSERVICE_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "NVDIMMSoftwareInstallationService"; //!< Creation Class Name static
	static const std::string NVDIMMSOFTWAREINSTALLATIONSERVICE_INSTALLFROMURI = "InstallFromURI"; //!< Method name
	static const std::string NVDIMMSOFTWAREINSTALLATIONSERVICE_INSTALLFROMURI_PARAM_URI = "URI"; //!< InstallFromURI param
	static const std::string NVDIMMSOFTWAREINSTALLATIONSERVICE_INSTALLFROMURI_PARAM_TARGET = "Target"; //!< InstallFromURI param
	static const std::string NVDIMMSOFTWAREINSTALLATIONSERVICE_INSTALLFROMURI_INSTALLOPTIONS = "InstallOptions"; //!< InstallFromURI param

	static const NVM_UINT32 SWINSTALLSERVICE_ERR_UNKNOWN = 2;
	static const NVM_UINT32 SWINSTALLSERVICE_ERR_FAILED = 4;
	static const NVM_UINT32 SWINSTALLSERVICE_ERR_INVALID_PARAMETER = 5;
	static const NVM_UINT32 SWINSTALLSERVICE_ERR_UNSUPPORTED_TARGET_TYPE = 4097;
	static const NVM_UINT32 SWINSTALLSERVICE_ERR_DOWNGRADE_NOT_SUPPORTED = 4099;
	static const NVM_UINT32 SWINSTALLSERVICE_ERR_NOT_ENOUGH_MEMORY = 4100;
	static const NVM_UINT32 SWINSTALLSERVICE_ERR_UNSUPPORTED_VERSION_TRANSITION = 4102;
	static const NVM_UINT32 SWINSTALLSERVICE_ERR_NOT_APPLICABLE_TO_TARGET = 4106;
	static const NVM_UINT32 SWINSTALLSERVICE_ERR_URI_NOT_ACCESSIBLE = 4107;
/*!
 * Provider Factory for NVDIMMSoftwareInstallationService
 */
class NVM_API NVDIMMSoftwareInstallationServiceFactory : public framework_interface::NvmInstanceFactory
{
	public:
		/*!
		 * Initialize a new NVDIMMSoftwareInstallationService.
		 */
		NVDIMMSoftwareInstallationServiceFactory() throw (framework::Exception);

		/*!
		 * Clean up the NVDIMMSoftwareInstallationServiceFactory
		 */
		~NVDIMMSoftwareInstallationServiceFactory();

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

		wbem::framework::UINT32 executeMethod(
			wbem::framework::UINT32 &wbem_return,
			const std::string method,
			wbem::framework::ObjectPath &object,
			wbem::framework::attributes_t &inParms,
			wbem::framework::attributes_t &outParms);

		/*!
		 * install firmware onto a device
		 * @param deviceUid
		 * 		List of uids to install the firmware onto
		 * @param path
		 * 		path to where the firmware file is.
		 * @param activate
		 * 		If true, the firmware will become active without a reboot
		 * @param force
		 * 		If true, the firmware will be loaded even if the minor version is less then
		 * 		the current FW version.
		 */
		void installFromPath(const std::string &deviceUid, const std::string &path,
				bool activate = false, bool force = false) const throw (framework::Exception);

		/*!
		 * install firmware onto all devices within the system
		 * @param path
		 * 		path to where the firmware file is.
		 * @param activate
		 * 		If true, the firmware will become active without a reboot
		 * @param force
		 * 		If true, the firmware will be loaded even if the minor version is less then
		 * 		the current FW version.
		 */
		void installFromPath(const std::string &path, bool activate = false,
				bool force = false) const throw (framework::Exception);

		/*!
		 * Examine a FW image and determine if it is valid for a given device
		 * @param deviceUid
		 * 		Device to examine FW image for
		 * @param path
		 * 		Path to FW image
		 * @param version
		 * 		Contains the version of the FW image after it has been examined
		 * @return
		 * 		Returns the success if the
		 */
		enum wbem::framework::return_codes examineFwImage(const std::string &deviceUid,
				const std::string &path, std::string &version) const
		throw (framework::Exception);

		/*!
		 * Examine a FW image and determine if it is valid for all devices.
		 * If any don't succeed the method returns failed.
		 * @param path
		 * 		Path to FW image
		 * @param version
		 * 		Contains the version of the FW image after it has been examined
		 * @return
		 * 		Returns the success if the
		 */
		enum wbem::framework::return_codes examineFwImage(const std::string &path,
				std::string &version) const
		throw (framework::Exception);

		/*!
		 * API indirection
		 * @param device_uid
		 * @param path
		 * @param path_len
		 * @param activate
		 * @param force
		 * @return
		 */
		int (*m_UpdateDeviceFw)(const NVM_UID device_uid, const NVM_PATH path,
				const NVM_SIZE path_len, NVM_BOOL activate, NVM_BOOL force);

		/*!
		 * API for examine FW image
		 * @param device_uid
		 * @param path
		 * @param path_len
		 * @param image_version
		 * @param image_version_len
		 * @return
		 */
		int (*m_ExamineFwImage)(const NVM_UID device_uid,
				const NVM_PATH path, const NVM_SIZE path_len,
				NVM_VERSION image_version, const NVM_SIZE image_version_len);

		/*!
		 * Removes direct dependency on static method
		 * @return
		 */
		std::vector<std::string> (*m_GetManageableDeviceUids)();

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		/*
		 * convert NvmExceptionLibError to extrinsic return code
		 */
		wbem::framework::UINT32 getReturnCodeFromLibException(exception::NvmExceptionLibError e);
};

} // software
} // wbem
#endif  // _WBEM_SOFTWARE_NVDIMMSOFTWAREINSTALLATIONSERVICE_FACTORY_H_
