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


#include <LogEnterExit.h>
#include <nvm_management.h>
#include <uid/uid.h>
#include <file_ops/file_ops_adapter.h>
#include <libinvm-cim/ObjectPathBuilder.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <libinvm-cim/ExceptionNotSupported.h>
#include <server/BaseServerFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include "NVDIMMSoftwareInstallationServiceFactory.h"
#include "NVDIMMCollectionFactory.h"

#include <algorithm>
#include <string.h>

#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>
#include <core/device/DeviceHelper.h>

wbem::software::NVDIMMSoftwareInstallationServiceFactory::NVDIMMSoftwareInstallationServiceFactory()
throw (wbem::framework::Exception) : m_UpdateDeviceFw(nvm_update_device_fw),
m_ExamineFwImage(nvm_examine_device_fw),
m_GetManageableDeviceUids(physical_asset::NVDIMMFactory::getManageableDeviceUids)
{ }

wbem::software::NVDIMMSoftwareInstallationServiceFactory::~NVDIMMSoftwareInstallationServiceFactory()
{ }


void wbem::software::NVDIMMSoftwareInstallationServiceFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(SYSTEMNAME_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(NAME_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);

}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::software::NVDIMMSoftwareInstallationServiceFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);



	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);
		checkPath(path);

		// ElementName - "Intel NVM FW Installation Service for" + host name
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			std::string hostName = wbem::server::getHostName();
			framework::Attribute a(std::string("Intel NVM FW Installation Service for ") + hostName, false);
			pInstance->setAttribute(ELEMENTNAME_KEY, a, attributes);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pInstance;
		throw;
	}

	return pInstance;
}

/*
 * Return the object paths for the NVDIMMSoftwareInstallationService class.
 */
wbem::framework::instance_names_t* wbem::software::NVDIMMSoftwareInstallationServiceFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		std::string hostName = wbem::server::getHostName();

		framework::attributes_t keys;
		// SystemCreationClassName - Intel_BaseServer
		keys[SYSTEMCREATIONCLASSNAME_KEY] = framework::Attribute(server::BASESERVER_CREATIONCLASSNAME, true);

		// SystemName - host name
		keys[SYSTEMNAME_KEY] = framework::Attribute(hostName, true);

		// CreationClassName - "<Prefix>_NVDIMMSoftwareInstallationService"
		keys[CREATIONCLASSNAME_KEY] = framework::Attribute(NVDIMMSOFTWAREINSTALLATIONSERVICE_CREATIONCLASSNAME, true);

		// Name - host name
		keys[NAME_KEY] = framework::Attribute(hostName, true);


		framework::ObjectPath path(hostName, NVM_NAMESPACE,
				NVDIMMSOFTWAREINSTALLATIONSERVICE_CREATIONCLASSNAME, keys);
		pNames->push_back(path);
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}
	return pNames;
}



/*
 * entry point for CIMOM to execute an extrinsic method
 */
wbem::framework::UINT32 wbem::software::NVDIMMSoftwareInstallationServiceFactory::executeMethod(
		wbem::framework::UINT32& wbemRc, const std::string method,
		wbem::framework::ObjectPath& object,
		wbem::framework::attributes_t& inParms,
		wbem::framework::attributes_t& outParms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;
	wbemRc = framework::MOF_ERR_SUCCESS;
	// valid InstallOptions for InstallFromURI
	const NVM_UINT16 DEFER = 2; 
	const NVM_UINT16 FORCE = 3;
	const NVM_UINT16 REBOOT = 7;
	const NVM_UINT16 EXAMINE = 32768;

	if (method == NVDIMMSOFTWAREINSTALLATIONSERVICE_INSTALLFROMURI)
	{
		/*
		 * uint32 InstallFromURI(CIM_ConcreateJob REF Job, string URI,
		 * 		CIM_ManagedElement REF Target, uint16 InstallOptions[],
		 * 		string InstallOptionsValues[])
		 *
		 * Returns one of:
		 * 		0 		- Job Completed with No Error
		 * 		2 		- Unspecified Error
		 * 		4 		- Failed
		 * 		5 		- Invalid Parameter
		 * 		4097 	- Unsupported Target Type
		 * 		4099 	- Downgrade/reinstall not supported
		 * 		4100	- Not enough memory
		 * 		4107 	- URI not accessible
		 *
		 */

		std::string uri = inParms[NVDIMMSOFTWAREINSTALLATIONSERVICE_INSTALLFROMURI_PARAM_URI].stringValue();
		std::string target = inParms[NVDIMMSOFTWAREINSTALLATIONSERVICE_INSTALLFROMURI_PARAM_TARGET].stringValue();
		framework::UINT16_LIST installOptions = inParms[NVDIMMSOFTWAREINSTALLATIONSERVICE_INSTALLFROMURI_INSTALLOPTIONS].uint16ListValue();
		// get valid install options
		bool rebootOption =
				std::find(installOptions.begin(), installOptions.end(), REBOOT) != installOptions.end();
		bool deferOption =
				std::find(installOptions.begin(), installOptions.end(), DEFER) != installOptions.end();
		bool forceOption =
				std::find(installOptions.begin(), installOptions.end(), FORCE) != installOptions.end();
		bool examineOption =
				std::find(installOptions.begin(), installOptions.end(), EXAMINE) != installOptions.end();

		// count invalid install options
		int invalidOptions = 0;
		for (size_t i = 0; i < installOptions.size(); i++)
		{
			if (!(installOptions[i] == DEFER ||
					installOptions[i] == REBOOT ||
					installOptions[i] == FORCE ||
					installOptions[i] == EXAMINE))
			{
				invalidOptions++;
			}
		}
		try
		{
			COMMON_PATH absPath;
			if (uri.empty())
			{
				httpRc = framework::CIM_ERR_INVALID_PARAMETER;
				COMMON_LOG_ERROR("URI parameter was missing");
			}
			else if (target.empty())
			{
				httpRc = framework::CIM_ERR_INVALID_PARAMETER;
				COMMON_LOG_ERROR("Target parameter was missing");
			}
			else if (get_absolute_path(uri.c_str(), uri.length() + 1, absPath) != COMMON_SUCCESS)
			{
				httpRc = framework::CIM_ERR_INVALID_PARAMETER;
				COMMON_LOG_ERROR("URI parameter is not valid");
			}
			else if (invalidOptions > 0)
			{
				httpRc = framework::CIM_ERR_INVALID_PARAMETER;
				COMMON_LOG_ERROR_F("Only %d, %d, %d and/or %d are valid InstallOptions",
						DEFER, REBOOT, FORCE, EXAMINE);
			}
			else if (rebootOption && deferOption)
			{
				httpRc = framework::CIM_ERR_INVALID_PARAMETER;
				COMMON_LOG_ERROR_F("%d and %d cannot be used together as InstallOptions",
						DEFER, REBOOT);
			}
			else
			{
				framework::ObjectPathBuilder targetBuilder(target);
				framework::ObjectPath targetObject;
				targetBuilder.Build(&targetObject);

				if (targetObject.getClass() == NVDIMMCOLLECTION_CREATIONCLASSNAME)
				{
					// all devices on system
					if (examineOption)
					{
						std::string version;

						framework::return_codes rc = examineFwImage(absPath, version);
						if (rc == framework::REQUIRES_FORCE)
						{
							wbemRc = SWINSTALLSERVICE_ERR_DOWNGRADE_NOT_SUPPORTED;
						}
						else if (rc != framework::SUCCESS)
						{
							wbemRc = SWINSTALLSERVICE_ERR_NOT_APPLICABLE_TO_TARGET;
						}
					}
					else
					{
						installFromPath(absPath, rebootOption, forceOption);
					}
				}
				else if (targetObject.getClass() == physical_asset::NVDIMM_CREATIONCLASSNAME)
				{
					// single device
					if (examineOption)
					{
						std::string version;
						framework::return_codes rc = examineFwImage(
								targetObject.getKeyValue(TAG_KEY).stringValue(),
								absPath, version);
						if (rc == framework::REQUIRES_FORCE)
						{
							wbemRc = SWINSTALLSERVICE_ERR_DOWNGRADE_NOT_SUPPORTED;
						}
						else if (rc != framework::SUCCESS)
						{
							wbemRc = SWINSTALLSERVICE_ERR_NOT_APPLICABLE_TO_TARGET;
						}
					}
					else
					{
						installFromPath(targetObject.getKeyValue(TAG_KEY).stringValue(),
								absPath, rebootOption, forceOption);
					}
				}
				else
				{
					httpRc = framework::CIM_ERR_INVALID_PARAMETER;
				}
			}
		}
		catch (wbem::framework::ExceptionBadParameter &)
		{
			wbemRc = SWINSTALLSERVICE_ERR_INVALID_PARAMETER;
		}
		catch (wbem::exception::NvmExceptionLibError &e)
		{
			wbemRc = getReturnCodeFromLibException(e);
		}
		catch (wbem::framework::ExceptionNoMemory &)
		{
			wbemRc = SWINSTALLSERVICE_ERR_NOT_ENOUGH_MEMORY;
		}
		catch (wbem::framework::ExceptionNotSupported &)
		{
			wbemRc = SWINSTALLSERVICE_ERR_FAILED;
		}
		catch (wbem::framework::Exception &)
		{
			wbemRc = SWINSTALLSERVICE_ERR_FAILED;
		}
	}
	else
	{
		httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
	}
	return httpRc;
}

wbem::framework::UINT32
wbem::software::NVDIMMSoftwareInstallationServiceFactory::getReturnCodeFromLibException(
		exception::NvmExceptionLibError e)
{
	wbem::framework::UINT32 rc;

	// these are the errors that nvm_update_device_fw can return
	switch(e.getLibError())
	{
	case NVM_ERR_NOMEMORY:
		rc = SWINSTALLSERVICE_ERR_NOT_ENOUGH_MEMORY;
		break;
	case NVM_ERR_NOTMANAGEABLE:
		rc = SWINSTALLSERVICE_ERR_UNSUPPORTED_TARGET_TYPE;
		break;
	case NVM_ERR_BADDEVICE:
	case NVM_ERR_INVALIDPARAMETER:
		rc = SWINSTALLSERVICE_ERR_INVALID_PARAMETER;
		break;
	case NVM_ERR_BADFILE:
		rc = SWINSTALLSERVICE_ERR_URI_NOT_ACCESSIBLE;
		break;
	case NVM_ERR_REQUIRESFORCE:
		rc = SWINSTALLSERVICE_ERR_DOWNGRADE_NOT_SUPPORTED;
		break;
	case NVM_ERR_BADFIRMWARE:
		rc = SWINSTALLSERVICE_ERR_UNSUPPORTED_VERSION_TRANSITION;
		break;
	case NVM_ERR_UNKNOWN:
		rc = SWINSTALLSERVICE_ERR_UNKNOWN;
		break;
	default:
		rc = SWINSTALLSERVICE_ERR_FAILED;
		break;
	}

	return rc;
}

/*
 * for each device uid, call the library to update it's firmware with the path provided
 */
void wbem::software::NVDIMMSoftwareInstallationServiceFactory::installFromPath(
		const std::string& deviceUid,
		const std::string& path, bool activate, bool force) const
throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	COMMON_LOG_DEBUG_F("URI: %s", path.c_str());

	if(path.empty())
	{
		throw framework::ExceptionBadParameter("path");
	}
	if (!core::device::isUidValid(deviceUid))
	{
		throw framework::ExceptionBadParameter("deviceUid");
	}

	int rc;
	NVM_UID uid;
	uid_copy(deviceUid.c_str(), uid);
	// library will check if device is manageable and can update the FW ... if not it will return an error
	if (NVM_SUCCESS !=
			(rc = m_UpdateDeviceFw(uid, path.c_str(), path.length(), activate, force)))
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

void wbem::software::NVDIMMSoftwareInstallationServiceFactory::installFromPath(
		const std::string& path, bool activate, bool force) const
throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<std::string> devices = m_GetManageableDeviceUids();


	for (size_t i = 0; i < devices.size(); i++)
	{
		installFromPath(devices[i], path, activate, force);
	}
}

enum wbem::framework::return_codes wbem::software::NVDIMMSoftwareInstallationServiceFactory::examineFwImage(
		const std::string& path, std::string &version) const
throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::return_codes rc = framework::SUCCESS;
	std::vector<std::string> devices = m_GetManageableDeviceUids();

	for (size_t i = 0; i < devices.size(); i++)
	{
		framework::return_codes tmp_rc;
		if ((tmp_rc = examineFwImage(devices[i], path, version)) != framework::SUCCESS)
		{
			rc = framework::FAIL;
		}
	}
	return rc;
}
enum wbem::framework::return_codes wbem::software::NVDIMMSoftwareInstallationServiceFactory::examineFwImage(
		const std::string& deviceUid, const std::string& path, std::string &version) const
throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::return_codes rc = framework::SUCCESS;
	if (path.empty())
	{
		throw framework::ExceptionBadParameter("path");
	}
	if (!core::device::isUidValid(deviceUid))
	{
		throw framework::ExceptionBadParameter("deviceUid");
	}

	NVM_UID uid;
	uid_copy(deviceUid.c_str(), uid);
	NVM_VERSION fw_version;
	memset(fw_version, 0, NVM_VERSION_LEN);
	int lib_rc;
	if ((lib_rc = m_ExamineFwImage(uid, path.c_str(), path.length(), fw_version, NVM_VERSION_LEN))
			!= NVM_SUCCESS)
	{
		if (lib_rc == NVM_ERR_REQUIRESFORCE)
		{
			rc = framework::REQUIRES_FORCE;
		}
		else
		{
			rc = framework::FAIL;
		}
	}

	version = fw_version;
	return rc;
}
