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

#include "ElementSoftwareIdentityFactory.h"
#include <LogEnterExit.h>
#include <libinvm-cim/ObjectPathBuilder.h>
#include <libinvm-cim/ExceptionBadAttribute.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <exception/NvmExceptionLibError.h>
#include <core/exceptions/NoMemoryException.h>
#include <uid/uid.h>

#include <software/NVDIMMFWVersionFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include <software/NVDIMMCollectionFactory.h>
#include <support/DiagnosticIdentityFactory.h>
#include <support/NVDIMMDiagnosticFactory.h>

namespace wbem
{
namespace software
{

ElementSoftwareIdentityFactory::ElementSoftwareIdentityFactory(
		framework::Instance *pInstance,
		const std::string &resultClassName,
		const std::string &roleName,
		const std::string &resultRoleName,
		core::device::DeviceService &deviceService,
		core::device::DeviceFirmwareService &deviceFwService,
		core::system::SystemService &systemService) :
		framework::AssociationFactory(pInstance,
				ELEMENTSOFTWAREIDENTITY_CREATIONCLASSNAME,
				resultClassName, roleName, resultRoleName),
				m_deviceService(deviceService),
				m_deviceFirmwareService(deviceFwService),
				m_systemService(systemService)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	setCimNamespace(wbem::NVM_NAMESPACE);

	initClassMap();
	initAssociationTable();
}

ElementSoftwareIdentityFactory::~ElementSoftwareIdentityFactory()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

framework::Instance* ElementSoftwareIdentityFactory::getInstance(framework::ObjectPath& path,
		framework::attribute_names_t& attributes) throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);

		// Make sure the instance exists, and if it needs ElementSoftwareStatus
		bool addActiveVersionStatus = false;
		bool addStagedVersionStatus = false;
		validateObjectPath(path, addActiveVersionStatus, addStagedVersionStatus);

		// ElementSoftwareStatus
		if (containsAttribute(wbem::ELEMENTSOFTWARESTATUS_KEY, attributes))
		{
			framework::UINT16_LIST statusList = getElementSoftwareStatus(
					addActiveVersionStatus, addStagedVersionStatus);
			framework::Attribute statusAttr(statusList, false);
			pInstance->setAttribute(wbem::ELEMENTSOFTWARESTATUS_KEY, statusAttr);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pInstance;
		throw;
	}

	// This is an instance of an association
	markInstanceAttributesAsAssociationRefs(*pInstance);

	return pInstance;
}

void ElementSoftwareIdentityFactory::validateObjectPath(
		const framework::ObjectPath& path, bool& activeVersion,
		bool& stagedVersion)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::ObjectPath antecedentPath = getAttributeObjectPathFromInstancePath(path,
			ANTECEDENT_KEY);
	framework::ObjectPath dependentPath = getAttributeObjectPathFromInstancePath(path,
			wbem::DEPENDENT_KEY);

	if (isAssociationBetweenClasses(antecedentPath, dependentPath,
			NVDIMMFWVERSION_CREATIONCLASSNAME, physical_asset::NVDIMM_CREATIONCLASSNAME))
	{
		validateFwVersionToDeviceRefPaths(antecedentPath, dependentPath,
				activeVersion, stagedVersion);
	}
	else if (isAssociationBetweenClasses(antecedentPath, dependentPath,
			NVDIMMFWVERSION_CREATIONCLASSNAME, NVDIMMCOLLECTION_CREATIONCLASSNAME))
	{
		validateFwVersionToDeviceCollectionRefPaths(antecedentPath, dependentPath,
				activeVersion, stagedVersion);
	}
	else
	{
		// None of the other ElementSoftwareIdentity associations deal with the firmware
		activeVersion = false;
		stagedVersion = false;
	}
}

bool ElementSoftwareIdentityFactory::isAssociationBetweenClasses(
		const framework::ObjectPath& antecedent, const framework::ObjectPath& dependent,
		const std::string& antecedentClass, const std::string& dependentClass)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool result = (antecedent.getClass() == antecedentClass) &&
			(dependent.getClass() == dependentClass);

	return result;
}

void ElementSoftwareIdentityFactory::validateFwVersionToDeviceRefPaths(
		const framework::ObjectPath& antecedent, const framework::ObjectPath& dependent,
		bool& addActiveVersionStatus, bool& addStagedVersionStatus)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	try
	{
		// Need to make sure this path corresponds to a real FW version on a real DIMM
		core::device::Device device = getDeviceForObjectPath(dependent);
		core::device::DeviceFirmwareInfo fwInfo = getFirmwareInfoForDevice(device.getUid());

		std::string fwInstanceId = antecedent.getKeyValue(INSTANCEID_KEY).stringValue();
		addActiveVersionStatus = isActiveFwVersion(fwInstanceId, device, fwInfo);
		addStagedVersionStatus = isStagedFwVersion(fwInstanceId, device, fwInfo);
	}
	catch (core::LibraryException &e)
	{
		throw exception::NvmExceptionLibError(e.getErrorCode());
	}

	if (!addActiveVersionStatus && !addStagedVersionStatus)
	{
		throw framework::ExceptionBadAttribute(ANTECEDENT_KEY.c_str());
	}
}

void ElementSoftwareIdentityFactory::validateFwVersionToDeviceCollectionRefPaths(
		const framework::ObjectPath& antecedent, const framework::ObjectPath& dependent,
		bool& addActiveVersionStatus, bool& addStagedVersionStatus)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Need all device info to determine if any association exists
	core::device::DeviceCollection devices;
	core::device::DeviceFirmwareInfoCollection deviceFwInfo;
	getAllDeviceFwInfo(devices, deviceFwInfo);

	wbem::framework::instance_names_t realCollectionAssociations;
	addInstanceNameForDeviceCollection(realCollectionAssociations, m_systemService.getHostName(),
			devices, deviceFwInfo);

	if (realCollectionAssociations.size() == 0)
	{
		// No instance!
		throw framework::ExceptionBadAttribute(wbem::DEPENDENT_KEY.c_str());
	}

	framework::ObjectPath realAntecedentPath = getAttributeObjectPathFromInstancePath(
			realCollectionAssociations[0], ANTECEDENT_KEY);
	framework::ObjectPath realDependentPath = getAttributeObjectPathFromInstancePath(
			realCollectionAssociations[0], wbem::DEPENDENT_KEY);
	if (antecedent.asString(true) != realAntecedentPath.asString(true))
	{
		throw framework::ExceptionBadAttribute(wbem::ANTECEDENT_KEY.c_str());
	}

	if (dependent.asString(true) != realDependentPath.asString(true))
	{
		throw framework::ExceptionBadAttribute(wbem::DEPENDENT_KEY.c_str());
	}

	// This association is to the active version only
	addActiveVersionStatus = true;
	addStagedVersionStatus = false;
}

core::device::Device ElementSoftwareIdentityFactory::getDeviceForObjectPath(
		const framework::ObjectPath& path)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string uidStr = path.getKeyValue(wbem::TAG_KEY).stringValue();
	core::Result<core::device::Device> deviceResult = m_deviceService.getDevice(uidStr);
	return deviceResult.getValue();
}

core::device::DeviceFirmwareInfo ElementSoftwareIdentityFactory::getFirmwareInfoForDevice(
		const std::string& deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Start with an empty FW info by default, because this is what we want if
	// the DIMM is unmanageable
	struct device_fw_info blankFwInfo;
	memset(&blankFwInfo, 0, sizeof (blankFwInfo));
	core::device::DeviceFirmwareInfo emptyFwInfo(deviceUid, blankFwInfo);
	core::Result<core::device::DeviceFirmwareInfo> fwInfoResult(emptyFwInfo);
	try
	{
		fwInfoResult = m_deviceFirmwareService.getFirmwareInfo(deviceUid);
	}
	catch (core::LibraryException &e)
	{
		// Unmanageable is valid instance - but everything should be zeroed out
		if (e.getErrorCode() != NVM_ERR_NOTMANAGEABLE)
		{
			throw;
		}
	}

	core::device::DeviceFirmwareInfo resultFwInfo = fwInfoResult.getValue();
	return resultFwInfo;
}

framework::ObjectPath ElementSoftwareIdentityFactory::getAttributeObjectPathFromInstancePath(
		const framework::ObjectPath& instancePath,
		const std::string& attributeKey)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Attribute pathAttr = instancePath.getKeyValue(attributeKey);
	framework::ObjectPathBuilder pathBuilder(pathAttr.stringValue());
	framework::ObjectPath path;
	if (!pathBuilder.Build(&path))
	{
		throw framework::ExceptionBadAttribute(attributeKey.c_str());
	}

	return path;
}

bool ElementSoftwareIdentityFactory::isActiveFwVersion(
		const std::string fwVersionInstanceId,
		core::device::Device &device,
		core::device::DeviceFirmwareInfo &fwInfo)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool match = false;

	std::string activeInstanceId = NVDIMMFWVersionFactory::getInstanceId(device.getFwRevision(),
			device.getFwApiVersion(), fwInfo.getActiveType(), fwInfo.getActiveCommitId(), fwInfo.getActiveBuildConfiguration());
	if (fwVersionInstanceId == activeInstanceId)
	{
		match = true;
	}

	return match;
}

bool ElementSoftwareIdentityFactory::isStagedFwVersion(
		const std::string fwVersionInstanceId,
		core::device::Device &device,
		core::device::DeviceFirmwareInfo &fwInfo)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool match = false;

	std::string stagedInstanceId = NVDIMMFWVersionFactory::getInstanceId(
			fwInfo.getStagedRevision(), device.getFwApiVersion(), fwInfo.getStagedType());
	if (fwVersionInstanceId == stagedInstanceId)
	{
		match = true;
	}

	return match;
}

framework::UINT16_LIST ElementSoftwareIdentityFactory::getElementSoftwareStatus(
		const bool isActiveVersion, const bool isStagedVersion)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::UINT16_LIST statusList;

	if (isActiveVersion)
	{
		statusList.push_back(ELEMENTSOFTWAREIDENTITY_ELEMENTSOFTWARESTATUS_CURRENT);
		statusList.push_back(ELEMENTSOFTWAREIDENTITY_ELEMENTSOFTWARESTATUS_FALLBACK);
	}

	if (isStagedVersion)
	{
		statusList.push_back(ELEMENTSOFTWAREIDENTITY_ELEMENTSOFTWARESTATUS_NEXT);
	}

	return statusList;
}

void ElementSoftwareIdentityFactory::getAllDeviceFwInfo(
		core::device::DeviceCollection& devices,
		core::device::DeviceFirmwareInfoCollection& fwInfoForDevice)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	devices = m_deviceService.getAllDevices();

	for (size_t i = 0; i < devices.size(); i++)
	{
		core::device::DeviceFirmwareInfo fwInfo =
				getFirmwareInfoForDevice(devices[i].getUid());

		fwInfoForDevice.push_back(fwInfo);
	}
}

framework::instance_names_t* ElementSoftwareIdentityFactory::getInstanceNames() throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string hostName = m_systemService.getHostName();

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		bool filterMatchesFwVersionToNvdimm = filterIncludesAssociationsForClasses(
				NVDIMMFWVERSION_CREATIONCLASSNAME,
				physical_asset::NVDIMM_CREATIONCLASSNAME);
		bool filterMatchesFwVersionToCollection = filterIncludesAssociationsForClasses(
				NVDIMMFWVERSION_CREATIONCLASSNAME,
				NVDIMMCOLLECTION_CREATIONCLASSNAME);

		// Need device and FW info for a few of these different associations
		core::device::DeviceCollection devices;
		core::device::DeviceFirmwareInfoCollection deviceFwInfo;
		if (filterMatchesFwVersionToNvdimm || filterMatchesFwVersionToCollection)
		{
			getAllDeviceFwInfo(devices, deviceFwInfo);
		}

		// NVDIMMFWVersion -> NVDIMM
		if (filterMatchesFwVersionToNvdimm)
		{
			for (size_t i = 0; i < devices.size(); i++)
			{
				std::string uidStr = devices[i].getUid();
				addInstanceNamesForDevice(*pNames, hostName, devices[i], deviceFwInfo[uidStr]);
			}
		}

		// NVDIMMFWVersion -> NVDIMMCollection
		if (filterMatchesFwVersionToCollection)
		{
			addInstanceNameForDeviceCollection(*pNames, hostName, devices, deviceFwInfo);
		}

		// DiagnosticIdentity -> NVDIMMDiagnostic - use superclass logic
		wbem::framework::instance_names_t *pAssocTableNames = framework::AssociationFactory::getInstanceNames();
		pNames->insert(pNames->end(), pAssocTableNames->begin(), pAssocTableNames->end());
		delete pAssocTableNames;

	}
	catch (framework::Exception &)
	{
		delete pNames;
		throw;
	}
	catch (core::NoMemoryException)
	{
		delete pNames;
		throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__, "Could not allocate memory");
	}

	return pNames;
}

bool ElementSoftwareIdentityFactory::filterIncludesAssociationsForClasses(
		const std::string &antecedentClass, const std::string &dependentClass)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool result = false;
	if (instanceCouldHaveAssociation(antecedentClass, dependentClass) &&
			resultFilterMatchesAssociation(antecedentClass, dependentClass) &&
			roleMatchesAssociation(antecedentClass, dependentClass))
	{
		result = true;
	}

	return result;
}

bool ElementSoftwareIdentityFactory::instanceCouldHaveAssociation(
		const std::string& antecedentClass, const std::string& dependentClass)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (instanceMatchesClass(antecedentClass) ||
			instanceMatchesClass(dependentClass));
}

bool ElementSoftwareIdentityFactory::instanceMatchesClass(const std::string& className)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// NULL matches all
	return (m_pInstance == NULL || m_pInstance->getClass() == className);
}

bool ElementSoftwareIdentityFactory::resultFilterMatchesAssociation(
		const std::string& antecedentClass, const std::string& dependentClass)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool result = false;

	if ((resultClassMatchesClass(antecedentClass) && resultRoleMatchesRoleName(wbem::ANTECEDENT_KEY) &&
			instanceMatchesClass(dependentClass)) ||
		(resultClassMatchesClass(dependentClass) && resultRoleMatchesRoleName(wbem::DEPENDENT_KEY) &&
			instanceMatchesClass(antecedentClass)))
	{
		result = true;
	}

	return result;
}

bool ElementSoftwareIdentityFactory::resultClassMatchesClass(const std::string& className)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (m_resultClassName.empty() || (m_resultClassName == className));
}

bool ElementSoftwareIdentityFactory::resultRoleMatchesRoleName(const std::string& roleName)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (m_resultRoleName.empty() || (m_resultRoleName == roleName));
}

bool ElementSoftwareIdentityFactory::roleMatchesAssociation(const std::string& antecedentClass,
		const std::string& dependentClass)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool result = false;
	if (m_roleName.empty() ||
			((m_roleName == wbem::ANTECEDENT_KEY) && instanceMatchesClass(antecedentClass)) ||
			((m_roleName == wbem::DEPENDENT_KEY) && instanceMatchesClass(dependentClass)))
	{
		result = true;
	}

	return result;
}

void ElementSoftwareIdentityFactory::addInstanceNamesForDevice(framework::instance_names_t& instanceNames,
		const std::string& hostName, core::device::Device& device,
		const core::device::DeviceFirmwareInfo &fwInfo)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::ObjectPath deviceInstanceName;
	physical_asset::NVDIMMFactory nvdimmFactory;
	nvdimmFactory.createPathFromUid(device.getUid(), deviceInstanceName,
			m_systemService.getHostName());

	framework::instance_names_t fwVersionInstances;
	NVDIMMFWVersionFactory::addFirmwareInstanceNamesForDeviceFromFwInfo(fwVersionInstances,
			hostName, device, fwInfo);

	for (framework::instance_names_t::const_iterator fwNameIter = fwVersionInstances.begin();
			fwNameIter != fwVersionInstances.end(); fwNameIter++)
	{
		framework::attributes_t keys;

		framework::Attribute antecedent(fwNameIter->asString(true), true);
		antecedent.setIsAssociationClassInstance(true);
		keys[wbem::ANTECEDENT_KEY] = antecedent;

		framework::Attribute dependent(deviceInstanceName.asString(true), true);
		dependent.setIsAssociationClassInstance(true);
		keys[wbem::DEPENDENT_KEY] = dependent;

		framework::ObjectPath associationPath(hostName, m_cimNamespace,
				ELEMENTSOFTWAREIDENTITY_CREATIONCLASSNAME, keys);

		if (associationObjectPathMatchesFilter(associationPath))
		{
			instanceNames.push_back(associationPath);
		}
	}
}

bool ElementSoftwareIdentityFactory::associationObjectPathMatchesFilter(
		const framework::ObjectPath& associationPath)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Attribute antecedent = associationPath.getKeyValue(wbem::ANTECEDENT_KEY);
	framework::ObjectPath antecedentPath;
	framework::ObjectPathBuilder antecedentPathBuilder(antecedent.stringValue());
	antecedentPathBuilder.Build(&antecedentPath);

	framework::Attribute dependent = associationPath.getKeyValue(wbem::DEPENDENT_KEY);
	framework::ObjectPath dependentPath;
	framework::ObjectPathBuilder dependentPathBuilder(dependent.stringValue());
	dependentPathBuilder.Build(&dependentPath);

	return (instanceMatchesObjectPath(antecedentPath) || instanceMatchesObjectPath(dependentPath));
}

bool ElementSoftwareIdentityFactory::instanceMatchesObjectPath(const framework::ObjectPath& path)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// NULL instance always matches
	bool result = true;
	if (m_pInstance)
	{
		framework::ObjectPath instancePath = m_pInstance->getObjectPath();
		result = (instancePath.asString(true) == path.asString(true));
	}

	return result;
}

bool ElementSoftwareIdentityFactory::allDeviceFirmwareVersionsMatch(
		core::device::DeviceCollection &devices,
		core::device::DeviceFirmwareInfoCollection &deviceFwInfo)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string firstUidStr = devices[0].getUid();
	framework::ObjectPath firstFwObjectPath = NVDIMMFWVersionFactory::getActiveFirmwareInstanceName(
			"", devices[0], deviceFwInfo[firstUidStr]);

	bool fwMatches = true;
	for (size_t i = 0; i < devices.size(); i++)
	{
		std::string uidStr = devices[i].getUid();
		framework::ObjectPath deviceFwObjectPath = NVDIMMFWVersionFactory::getActiveFirmwareInstanceName("",
			devices[i], deviceFwInfo[uidStr]);
		if (deviceFwObjectPath.asString(true) != firstFwObjectPath.asString(true))
		{
			fwMatches = false;
			break;
		}
	}

	return fwMatches;
}

void ElementSoftwareIdentityFactory::addInstanceNameForDeviceCollection(
		framework::instance_names_t& instanceNames, const std::string& hostName,
		core::device::DeviceCollection &devices,
		core::device::DeviceFirmwareInfoCollection &deviceFwInfo)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if ((devices.size() > 0) && allDeviceFirmwareVersionsMatch(devices, deviceFwInfo))
	{
		// If all versions are the same, it doesn't matter which one we pick
		std::string uidStr = devices[0].getUid();
		framework::ObjectPath fwObjectPath = NVDIMMFWVersionFactory::getActiveFirmwareInstanceName(
				hostName, devices[0], deviceFwInfo[uidStr]);
		framework::ObjectPath collectionObjectPath = NVDIMMCollectionFactory::getObjectPath(hostName);

		framework::attributes_t keys;

		framework::Attribute antecedent(fwObjectPath.asString(true), true);
		antecedent.setIsAssociationClassInstance(true);
		keys[wbem::ANTECEDENT_KEY] = antecedent;

		framework::Attribute dependent(collectionObjectPath.asString(true), true);
		dependent.setIsAssociationClassInstance(true);
		keys[wbem::DEPENDENT_KEY] = dependent;

		framework::ObjectPath associationPath(hostName, m_cimNamespace,
				ELEMENTSOFTWAREIDENTITY_CREATIONCLASSNAME, keys);
		if (associationObjectPathMatchesFilter(associationPath))
		{
			instanceNames.push_back(associationPath);
		}
	}
}

void ElementSoftwareIdentityFactory::populateAttributeList(framework::attribute_names_t& attributes)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Keys - defined in class map
	framework::AssociationFactory::populateAttributeList(attributes);

	// other attributes
	attributes.push_back(ELEMENTSOFTWARESTATUS_KEY);
}

void ElementSoftwareIdentityFactory::initClassMap()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	addClassToMap(ELEMENTSOFTWAREIDENTITY_CREATIONCLASSNAME,
		wbem::ANTECEDENT_KEY, wbem::DEPENDENT_KEY);
}

void ElementSoftwareIdentityFactory::initAssociationTable()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Fall through to superclass logic
	// DiagnosticIdentity -> NVDIMMDiagnostic
	addAssociationToTable(ELEMENTSOFTWAREIDENTITY_CREATIONCLASSNAME,
		framework::ASSOCIATIONTYPE_COMPLEX,
		support::DIAGNOSTICIDENTITY_CREATIONCLASSNAME,
		support::NVDIMMDIAGNOSTIC_CREATIONCLASSNAME);
}

} /* namespace software */
} /* namespace wbem */
