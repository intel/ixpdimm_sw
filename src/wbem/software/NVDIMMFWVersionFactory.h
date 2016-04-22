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
 * This file contains the provider for the NVDIMMFWVersion instances
 * which represents the version of the FW on an NVM DIMM.
 */

#ifndef	_WBEM_SOFTWARE_NVDIMMFWVERSION_FACTORY_H_
#define	_WBEM_SOFTWARE_NVDIMMFWVERSION_FACTORY_H_

#include <string>
#include <framework_interface/NvmInstanceFactory.h>
#include <core/device/Device.h>
#include <core/device/DeviceFirmwareInfo.h>

namespace wbem
{
namespace software
{
	static const std::string NVDIMMFWVERSION_CREATIONCLASSNAME =
			std::string(NVM_WBEM_PREFIX) + "NVDIMMFWVersion"; //!< Creation Class Name static
	static const NVM_UINT16 NVDIMMFWVERSION_CLASSIFICATIONS_FW = 10;

	static const std::string NVDIMMFWVERSION_INSTANCEID_PREFIX = "NVDIMMFW";

	static const std::string NVMDIMMFWVERSION_FWTYPE_UNKNOWN_STR = "Unknown";
	static const std::string NVMDIMMFWVERSION_FWTYPE_PRODUCTION_STR = "Production";
	static const std::string NVMDIMMFWVERSION_FWTYPE_DFX_STR = "DFx";
	static const std::string NVMDIMMFWVERSION_FWTYPE_DEBUG_STR = "Debug";

	static const NVM_UINT16 NVMDIMMFWVERSION_FWTYPE_UNKNOWN = 0;
	static const NVM_UINT16 NVMDIMMFWVERSION_FWTYPE_PRODUCTION = 1;
	static const NVM_UINT16 NVMDIMMFWVERSION_FWTYPE_DFX = 2;
	static const NVM_UINT16 NVMDIMMFWVERSION_FWTYPE_DEBUG = 3;

	static const std::string NVMDIMMFWVERSION_DELIMITER = "-";

/*!
 * Provider Factory for NVDIMMFWVersion
 */
class NVM_API NVDIMMFWVersionFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new NVDIMMFWVersion.
		 */
		NVDIMMFWVersionFactory() throw (framework::Exception);

		/*!
		 * Clean up the NVDIMMFWVersionFactory
		 */
		~NVDIMMFWVersionFactory();

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

		static void addFirmwareInstanceNamesForDevice(framework::instance_names_t &instanceNames,
				const std::string &hostName,
				const struct device_discovery &device);

		static void addFirmwareInstanceNamesForDeviceFromFwInfo(
				framework::instance_names_t &instanceNames,
				const std::string &hostName,
				core::device::Device &device,
				const core::device::DeviceFirmwareInfo &fwInfo);

		static std::string getInstanceId(const std::string &fwVersion,
				const std::string &fwApiVersion,
				const enum device_fw_type fwType,
				const std::string &commitId = "",
				const std::string &build_configuration = "");

		static framework::ObjectPath getActiveFirmwareInstanceName(const std::string &hostName,
				core::device::Device &device,
				const core::device::DeviceFirmwareInfo &fwInfo);

		static framework::ObjectPath getStagedFirmwareInstanceName(const std::string &hostName,
				core::device::Device &device,
				const core::device::DeviceFirmwareInfo &fwInfo);

		static std::string translateFwType(const NVM_UINT16 fw_type);

	protected:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		/*
		 * Parse instanceId string into FW version, FW API version, FW type commit ID
		 * and build configuration
		 */
		static void parseInstanceId(std::string instanceId, std::string &fwVersion,
				std::string &fwApiVersion, NVM_UINT16 &fwType, std::string &commitId,
				std::string &build_configuration);

		static framework::ObjectPath getInstanceName(const std::string &hostName,
				const std::string instanceId);
};

} // software
} // wbem
#endif  // _WBEM_SOFTWARE_NVDIMMFWVERSION_FACTORY_H_
