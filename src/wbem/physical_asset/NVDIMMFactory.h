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
 * This file contains the provider for the NVDIMM instances which
 * model the physical aspects of an NVM DIMM.
 */


#ifndef    _WBEM_PHYSICALASSET_NVDIMM_FACTORY_H_
#define    _WBEM_PHYSICALASSET_NVDIMM_FACTORY_H_

#include <string>
#include <nvm_management.h>
#include <framework_interface/NvmInstanceFactory.h>
#include <exception/NvmExceptionLibError.h>
#include <core/device/DeviceService.h>
#include <core/system/SystemService.h>


namespace wbem
{
namespace physical_asset
{
static const std::string NVDIMM_CREATIONCLASSNAME =
	std::string(NVM_WBEM_PREFIX) + "NVDIMM"; //!< Creation ClassName static
static const std::string NVDIMM_ELEMENTNAME_PREFIX = "Intel NVDIMM "; //!< Element Name = prefix + UID
static const int NVDIMM_COMMUNICATION_OK = 2; //!< Communication status value for OK
static const int NVDIMM_COMMUNICATION_NOCONTACT = 4; //!< Communication status value for no contact
static const std::string NVDIMM_SETPASSPHRASE = "SetPassphrase"; //!< extrinsic method name
static const std::string NVDIMM_REMOVEPASSPHRASE = "RemovePassphrase"; //!< extrinsic method name
static const std::string NVDIMM_UNLOCK = "Unlock"; //!< extrinsic method name
static const std::string NVDIMM_SETPASSPHRASE_NEWPASSPHRASE = "NewPassphrase"; //!< method param
static const std::string NVDIMM_SETPASSPHRASE_CURRENTPASSPHRASE = "CurrentPassphrase"; //!< method param
static const NVM_UINT16 DEVICE_HEALTH_UNMANAGEABLE = 65534; //!< Additional health state for unmanageable dimms

static const NVM_UINT32 SECURITY_PASSPHRASE = 0;
static const NVM_UINT32 SECURITY_UNLOCK = 1;
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

static const NVM_UINT32 NVDIMM_ERR_UNKNOWN = 1;
static const NVM_UINT32 NVDIMM_ERR_FAILED = 2;
static const NVM_UINT32 NVDIMM_ERR_NOT_ALLOWED = 3;
static const NVM_UINT32 NVDIMM_ERR_NOT_SUPPORTED = 4;
static const NVM_UINT32 NVDIMM_ERR_INVALID_PARAMETER = 5;

static const framework::UINT16 UNKNOWN = 1;
static const framework::UINT16 NOT_APPLICABLE = 2;
static const framework::UINT16 REMOVABLE_WHEN_OFF = 3;
static const framework::UINT16 REMOVABLE_WHEN_ON_OR_OFF = 4;

typedef std::vector<struct device_discovery> devices_t;

/*!
 * Models the physical aspects of an Intel NVDIMM
 */
class NVM_API NVDIMMFactory : public framework_interface::NvmInstanceFactory
{
public:
	NVDIMMFactory(
		core::device::DeviceService &deviceService = core::device::DeviceService::getService(),
		core::system::SystemService &systemService = core::system::SystemService::getService()
	);
	~NVDIMMFactory();

	virtual framework::instances_t *getInstances(framework::attribute_names_t &attributes);
	framework::Instance *getInstance(framework::ObjectPath &path,
		framework::attribute_names_t &attributes);

	framework::instance_names_t *getInstanceNames();

	framework::Instance *modifyInstance(framework::ObjectPath &path,
		framework::attributes_t &attributes) ;

	wbem::framework::UINT32 executeMethod(
		wbem::framework::UINT32 &wbem_return,
		const std::string method,
		wbem::framework::ObjectPath &object,
		wbem::framework::attributes_t &inParms,
		wbem::framework::attributes_t &outParms);


	// Extrinsic Methods
	void setPassphrase(std::string deviceUid, std::string newPassphrase,
		std::string currentPassphrase);

	void removePassphrase(std::string deviceUid, std::string currentPassphrase);

	void unlock(std::string deviceUid, std::string currentPassphrase);

	void injectTemperatureError(const std::string &dimmUid,
			const NVM_REAL32 temperature);

	void injectPoisonError(const std::string &dimmUid,
			const NVM_UINT64 dpa);

	void injectSoftwareTrigger(const std::string &dimmUid,
			const NVM_UINT16 error);

	void clearPoisonError(const std::string &dimmUid,
			const NVM_UINT64 dpa);

	void clearTemperatureError(const std::string &dimmUid);

	void clearSoftwareTrigger(const std::string &dimmUid,
				const NVM_UINT16 error);

	// Helper functions
	static void uidToHandle(const std::string &dimmUid, NVM_UINT32 &handle);

	static std::vector<std::string> getManageableDeviceUids();

	static int existsAndIsManageable(const std::string &dimmUid);

	static devices_t getAllDevices();

	static devices_t getManageableDevices();

	static framework::Attribute uidToDimmIdAttribute(const std::string &dimmUid);

	static std::string uidToDimmIdStr(const std::string &dimmUid);

	void createPathFromUid(const NVM_UID uid, framework::ObjectPath &path);

	void createPathFromUid(const std::string uid, framework::ObjectPath &path,
			std::string hostname = "");

	static wbem::framework::UINT16_LIST deviceStatusToOpStatus(
			core::device::Device &device);

	static void toInstance(core::device::Device &nvdimm, framework::Instance &instance,
			wbem::framework::attribute_names_t attributes);

	// API Interfaces
	int (*m_SetPassphrase)(const NVM_UID device_uid,
			const NVM_PASSPHRASE old_passphrase, const NVM_SIZE old_passphrase_len,
			const NVM_PASSPHRASE new_passphrase, const NVM_SIZE new_passphrase_len);

	int (*m_RemovePassphrase)(const NVM_UID device_uid,
			const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len);

	int (*m_UnlockDevice)(const NVM_UID device_uid,
			const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len);

	int (*m_GetFwLogLevel)(const NVM_UID device_uid, enum fw_log_level *p_log_level);

	int (*m_injectDeviceError)(const NVM_UID device_uid,
			const struct device_error *p_error);

	int (*m_clearInjectedDeviceError)(const NVM_UID device_uid,
			const struct device_error *p_error);

private:
	core::device::DeviceService &m_deviceService;
	core::system::SystemService &m_systemService;

	void populateAttributeList(framework::attribute_names_t &attributes);

	/*
	 * Helper to convert an integer to a fw_log_level enum.
	 */
	enum fw_log_level convertToLogLevelEnum(NVM_UINT16 logLevel);

	/*
	 * convert NvmExceptionLibError to extrinsic return code
	 */
	wbem::framework::UINT32 getReturnCodeFromLibException(const exception::NvmExceptionLibError &e);

	static std::string getMemoryModeString(core::device::Device &nvdimm);

	void injectError(const std::string &dimmUid,
			struct device_error *p_error);

	void clearError(const std::string &dimmUid,
			struct device_error *p_error);

};

} // physical_asset
} // wbem
#endif  // #ifndef _WBEM_PHYSICALASSET_NVDIMM_FACTORY_H_
