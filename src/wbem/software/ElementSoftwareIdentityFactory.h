/*
 * Copyright (c) 2016, Intel Corporation
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
 * ElementSoftwareIdentityFactory associations
 */

#ifndef ELEMENTSOFTWAREIDENTITYFACTORY_H_
#define ELEMENTSOFTWAREIDENTITYFACTORY_H_

#include <libinvm-cim/AssociationFactory.h>
#include <lib_interface/NvmApi.h>
#include <core/device/DeviceService.h>
#include <core/device/DeviceFirmwareService.h>
#include <core/system/SystemService.h>
#include <NvmStrings.h>
#include <nvm_types.h>

namespace wbem
{
namespace software
{

static const std::string ELEMENTSOFTWAREIDENTITY_CREATIONCLASSNAME =
		std::string(NVM_WBEM_PREFIX) + "ElementSoftwareIdentity";

static const framework::UINT16 ELEMENTSOFTWAREIDENTITY_ELEMENTSOFTWARESTATUS_CURRENT = 2;
static const framework::UINT16 ELEMENTSOFTWAREIDENTITY_ELEMENTSOFTWARESTATUS_NEXT = 3;
static const framework::UINT16 ELEMENTSOFTWAREIDENTITY_ELEMENTSOFTWARESTATUS_FALLBACK = 4;

class NVM_API ElementSoftwareIdentityFactory : public framework::AssociationFactory
{
	public:
		ElementSoftwareIdentityFactory(framework::Instance *pInstance = NULL,
				const std::string &resultClassName = "",
				const std::string &roleName = "",
				const std::string &resultRoleName = "",
				core::device::DeviceService &deviceService = core::device::DeviceService::getService(),
				core::device::DeviceFirmwareService &deviceFwService = core::device::DeviceFirmwareService::getService(),
				core::system::SystemService &systemService = core::system::SystemService::getService());
		virtual ~ElementSoftwareIdentityFactory();

		virtual framework::Instance* getInstance(framework::ObjectPath &path,
				framework::attribute_names_t &attributes)
			throw (framework::Exception);
		virtual framework::instance_names_t* getInstanceNames()
			throw (framework::Exception);

	protected:
		core::device::DeviceService &m_deviceService;
		core::device::DeviceFirmwareService &m_deviceFirmwareService;
		core::system::SystemService &m_systemService;

		virtual void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);
		virtual void initClassMap();
		virtual void initAssociationTable();

		// Associations filter on instance/result/role
		bool filterIncludesAssociationsForClasses(const std::string &antecedentClass, const std::string &dependentClass);
		bool instanceCouldHaveAssociation(const std::string &antecedentClass, const std::string &dependentClass);
		bool instanceMatchesClass(const std::string &className);
		bool instanceMatchesObjectPath(const framework::ObjectPath &path);
		bool resultFilterMatchesAssociation(const std::string &antecedentClass, const std::string &dependentClass);
		bool resultClassMatchesClass(const std::string &className);
		bool resultRoleMatchesRoleName(const std::string &className);
		bool roleMatchesAssociation(const std::string &antecedentClass, const std::string &dependentClass);
		bool associationObjectPathMatchesFilter(const framework::ObjectPath &associationPath);

		void validateObjectPath(const framework::ObjectPath &path, bool &addActiveVersionStatus,
				bool &addStagedVersionStatus);
		void validateFwVersionToDeviceRefPaths(
				const framework::ObjectPath& antecedent, const framework::ObjectPath& dependent,
				bool &addActiveVersionStatus, bool &addStagedVersionStatus);
		void validateFwVersionToDeviceCollectionRefPaths(
				const framework::ObjectPath& antecedent, const framework::ObjectPath& dependent,
				bool &addActiveVersionStatus, bool &addStagedVersionStatus);
		bool isAssociationBetweenClasses(const framework::ObjectPath &antecedent, const framework::ObjectPath &dependent,
				const std::string &antecedentClass, const std::string &dependentClass);
		core::device::Device getDeviceForObjectPath(const framework::ObjectPath &devicePath);
		core::device::DeviceFirmwareInfo getFirmwareInfoForDevice(const std::string &deviceUid);
		framework::ObjectPath getAttributeObjectPathFromInstancePath(
				const framework::ObjectPath &instancePath,
				const std::string &attributeKey);
		bool isActiveFwVersion(const std::string fwVersionInstanceId,
				core::device::Device &device,
				core::device::DeviceFirmwareInfo &fwInfo);
		bool isStagedFwVersion(const std::string fwVersionInstanceId,
				core::device::Device &device,
				core::device::DeviceFirmwareInfo &fwInfo);
		framework::UINT16_LIST getElementSoftwareStatus(const bool isActiveVersion,
				const bool isStagedVersion);
		void getAllDeviceFwInfo(core::device::DeviceCollection &devices,
				core::device::DeviceFirmwareInfoCollection &fwInfoForDevices);
		bool allDeviceFirmwareVersionsMatch(core::device::DeviceCollection &devices,
				core::device::DeviceFirmwareInfoCollection &deviceFwInfo);

		void addInstanceNamesForDevice(framework::instance_names_t &instanceNames,
				const std::string &hostName, core::device::Device &device,
				const core::device::DeviceFirmwareInfo &fwInfo);
		void addInstanceNameForDeviceCollection(framework::instance_names_t &instanceNames,
				const std::string &hostName, core::device::DeviceCollection &devices,
				core::device::DeviceFirmwareInfoCollection &deviceFwInfo);
};

} /* namespace software */
} /* namespace wbem */

#endif /* ELEMENTSOFTWAREIDENTITYFACTORY_H_ */
