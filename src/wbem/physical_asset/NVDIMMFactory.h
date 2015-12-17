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
 * This file contains the provider for the NVDIMM instances which
 * model the physical aspects of an NVM DIMM.
 */


#ifndef	_WBEM_PHYSICALASSET_NVDIMM_FACTORY_H_
#define	_WBEM_PHYSICALASSET_NVDIMM_FACTORY_H_

#include <string>
#include <nvm_management.h>
#include <framework_interface/NvmInstanceFactory.h>


namespace wbem
{
namespace physical_asset
{
	static const std::string NVDIMM_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "NVDIMM"; //!< Creation ClassName static
	static const std::string NVDIMM_ELEMENTNAME_prefix = "Intel NVDIMM "; //!< Element Name = prefix + GUID
	static const int NVDIMM_COMMUNICATION_OK = 2; //!< Communication status value for OK
	static const int NVDIMM_COMMUNICATION_NOCONTACT = 4; //!< Communication status value for no contact
	static const std::string NVDIMM_SETPASSPHRASE = "SetPassphrase"; //!< extrinsic method name
	static const std::string NVDIMM_REMOVEPASSPHRASE = "RemovePassphrase"; //!< extrinsic method name
	static const std::string NVDIMM_UNLOCK = "Unlock"; //!< extrinsic method name
	static const std::string NVDIMM_SETPASSPHRASE_NEWPASSPHRASE = "NewPassphrase"; //!< method param
	static const std::string NVDIMM_SETPASSPHRASE_CURRENTPASSPHRASE = "CurrentPassphrase"; //!< method param
	static const NVM_UINT16 DEVICE_HEALTH_UNMANAGEABLE = 65534; //!< Additional health state for unmanageable dimms

	static const NVM_UINT32 SECURITY_PASSPHRASE = 0; 
	static const NVM_UINT32 SECURITY_UNLOCK= 1;
	static const NVM_UINT32 SECURITY_ERASE = 2;

	static const NVM_UINT16 DEVICE_LAST_SHUTDOWN_STATUS_UKNOWN = 0;
	static const NVM_UINT16 DEVICE_LAST_SHUTDOWN_STATUS_FW_FLUSH_COMPLETE = 1;
	static const NVM_UINT16 DEVICE_LAST_SHUTDOWN_STATUS_PM_ADR_COMMAND = 2;
	static const NVM_UINT16 DEVICE_LAST_SHUTDOWN_STATUS_PM_S3 = 3;
	static const NVM_UINT16 DEVICE_LAST_SHUTDOWN_STATUS_PM_S5 = 4;
	static const NVM_UINT16 DEVICE_LAST_SHUTDOWN_STATUS_DDRT_POWER_FAIL = 5;
	static const NVM_UINT16 DEVICE_LAST_SHUTDOWN_STATUS_PMIC_12V_POWER_FAIL = 6;
	static const NVM_UINT16 DEVICE_LAST_SHUTDOWN_STATUS_PM_WARM_RESET = 7;
	static const NVM_UINT16 DEVICE_LAST_SHUTDOWN_STATUS_THERMAL_SHUTDOWN = 8;

	static const NVM_UINT16 NVDIMM_MEMORYTYPECAPABILITIES_MEMORYMODE = 0;
	static const NVM_UINT16 NVDIMM_MEMORYTYPECAPABILITIES_STORAGEMODE = 1;
	static const NVM_UINT16 NVDIMM_MEMORYTYPECAPABILITIES_APPDIRECTMODE = 2;

	static const NVM_UINT16 NVDIMM_OPSTATUS_UNKNOWN = 0;
	static const NVM_UINT16 NVDIMM_OPSTATUS_OK = 2;
	static const NVM_UINT16 NVDIMM_OPSTATUS_DEGRADED = 3;
	static const NVM_UINT16 NVDIMM_OPSTATUS_ERROR = 6;
	static const NVM_UINT16 NVDIMM_OPSTATUS_NONRECOVERABLEERROR = 7;
	static const NVM_UINT16 NVDIMM_OPSTATUS_MIXEDSKU = 32768;
	static const NVM_UINT16 NVDIMM_OPSTATUS_SKUVIOLATION = 32769;

	typedef std::vector<struct device_discovery> devices_t;

/*!
 * Models the physical aspects of an Intel NVDIMM
 */
class NVM_API NVDIMMFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new NVDIMMFactory.
		 */
		NVDIMMFactory() throw (framework::Exception);

		/*!
		 * Clean up the NVDIMMFactory
		 */
		~NVDIMMFactory();


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
		 * Helper method to retrieve NVDIMM instance attributes
		 * @param[in] dimm
		 * 		The device details fetched using the API.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @param[out] pInstance
		 * 		pointer to the instance
		 * @throw Exception if unable to retrieve the DIMM info
		 */
		static void fillInNVDIMMInstance(struct device_details &dimm,
			framework::attribute_names_t &attributes,
			framework::Instance *pInstance) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * NVDIMM object paths.
		 * @throw Exception if unable to retrieve the DIMM list
		 * @throw Exception if unable to retrieve the server name
		 * @return The object paths for each NVDIMM.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

		/*!
		 * Standard CIM method to modify an existing instance.
		 * @param[in] path
		 * 		The object path of the instance to modify.
		 * @param[in] attributes
		 * 		The attributes to modify.
		 * @throw Exception if not implemented.
		 * @return The updated instance.
		 */
		framework::Instance* modifyInstance(framework::ObjectPath &path,
				framework::attributes_t &attributes) throw (framework::Exception);


		wbem::framework::UINT32 executeMethod(
					wbem::framework::UINT32 &wbem_return,
					const std::string method,
					wbem::framework::ObjectPath &object,
					wbem::framework::attributes_t &inParms,
					wbem::framework::attributes_t &outParms);


		// Extrinsic Methods
		/*!
		 * C++ entry point for setting a passphrase on a device
		 * @param deviceGuid
		 * @param newPassphrase
		 * @param currentPassphrase
		 */
		void setPassphrase(std::string deviceGuid, std::string newPassphrase, std::string currentPassphrase);

		/*!
		 * C++ entry point for removing a passphrase on a device
		 * @param deviceGuid
		 * @param currentPassphrase
		 */
		void removePassphrase(std::string deviceGuid, std::string currentPassphrase);

		/*!
		 * C++ entry point for unlocking a device
		 * @param deviceGuid
		 * @param currentPassphrase
		 */
		void unlock(std::string deviceGuid, std::string currentPassphrase);

		/*!
		 * Get all manageable device guids on system
		 * @return
		 */
		static std::vector<std::string> getManageableDeviceGuids();

		/*!
		 * Utility method to check if a device is valid and manageable
		 * @param dimmGuid
		 * 		Device to validate
		 * @return
		 * 		If the device exists and is manageable return NVM_SUCCESS.
		 * 		If device does not exist, return NVM_ERR_BADDEVICE.
		 * 		If device is not manageable, return NVM_ERR_NOTMANAGEABLE.
		 */
		static int existsAndIsManageable(const std::string &dimmGuid);

		/*!
		 * Interface to the library API. This pointer allows for dependency injection and decouples the dependency on the API
		 * @param device_guid
		 * @param old_passphrase
		 * @param old_passphrase_len
		 * @param new_passphrase
		 * @param new_passphrase_len
		 * @return
		 */
		int (*m_SetPassphrase)(const NVM_GUID device_guid,
				const NVM_PASSPHRASE old_passphrase, const NVM_SIZE old_passphrase_len,
				const NVM_PASSPHRASE new_passphrase, const NVM_SIZE new_passphrase_len);
		/*!
		 * Interface to the library API. This pointer allows for dependency injection and decouples the dependency on the API
		 * @param device_guid
		 * @param passphrase
		 * @param passphrase_len
		 * @return
		 */
		int (*m_RemovePassphrase)(const NVM_GUID device_guid,
				const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len);
		/*!
		 * Interface to the library API. This pointer allows for dependency injection and decouples the dependency on the API
		 * @param device_guid
		 * @param passphrase
		 * @param passphrase_len
		 * @return
		 */
		int (*m_UnlockDevice)(const NVM_GUID device_guid,
				const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len);

		/*
		 * Helper to convert device config status to a string
		 */
		static std::string deviceConfigStatusToStr(int configState);

		/*
		 * Helper function to populate an objectpath with appropriate keys
		 */
		static void createPathFromGuid(const NVM_GUID guid, framework::ObjectPath &path);
		static void createPathFromGuid(const std::string guid, framework::ObjectPath &path);

		/*
		 * Helper function to convert from struct device_capabilities to a list of memory modes.
		 */
		static void buildMemoryTypeCapabilitiesFromDeviceCapabilities(const struct device_capabilities &capabilities,
				framework::UINT16_LIST &memoryTypeCapabilities);

		static devices_t getAllDevices();

		static devices_t getManageableDevices();

		/*
		 * Helper function to convert memory type enum to a string
		 */
		static std::string memoryTypetoStr(enum memory_type memoryType);

		/*
		 * Helper function to convert device status to operational status list
		 */
		static wbem::framework::UINT16_LIST deviceStatusToOpStatus(const struct device_status *p_status);

		/*
		 * Return true if the modified attribute is allowed to be modified
		 */
		static bool attributeIsModifiable(framework::attribute_names_t modifiableAttributes,
				std::string attributeThatWasModified);

		/*
		 * Return true is the new attributes does not equal the old attribute
		 */
		static bool attributeHasChanged(framework::Attribute oldAttr, framework::Attribute newAttr);

		/*
		 * Helper function to check if the client is attempting to modify an unmodifyable attribute
		 */
		static void checkAttributesAreModifiable(framework::Instance *pInstance, framework::attributes_t &attributes,
				framework::attribute_names_t modifyableAttributes);
private:

		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);
		static void constructLastShutDownStatuses(NVM_UINT8 lastShutdownState,
			wbem::framework::UINT16_LIST &shutdownStatuses);

		/*
		 * Helper to convert an integer to a fw_log_level enum.
		 */
		enum fw_log_level convertToLogLevelEnum(NVM_UINT16 logLevel);


};

} // physical_asset
} // wbem
#endif  // #ifndef _WBEM_PHYSICALASSET_NVDIMM_FACTORY_H_
