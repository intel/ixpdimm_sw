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


#include <LogEnterExit.h>
#include <nvm_management.h>
#include <uid/uid.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ObjectPathBuilder.h>
#include <physical_asset/NVDIMMFactory.h>
#include <software/NVDIMMCollectionFactory.h>
#include <server/BaseServerFactory.h>
#include "ErasureServiceFactory.h"

#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>
#include <core/device/DeviceHelper.h>
#include "ErasureCapabilitiesFactory.h"


wbem::erasure::ErasureServiceFactory::ErasureServiceFactory()
	throw (wbem::framework::Exception) :
	m_eraseDevice(nvm_erase_device),
	m_GetManageabledeviceUids(physical_asset::NVDIMMFactory::getManageableDeviceUids)
{ }

wbem::erasure::ErasureServiceFactory::~ErasureServiceFactory()
{ }

void wbem::erasure::ErasureServiceFactory::populateAttributeList(framework::attribute_names_t &attributes)
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
wbem::framework::Instance* wbem::erasure::ErasureServiceFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	checkPath(path);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);

		// ElementName - "Erasure Service"
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			framework::Attribute a(ERASURESERVICE_ELEMENTNAME, false);
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
 * Return the object paths for the ErasureService class.
 */
wbem::framework::instance_names_t* wbem::erasure::ErasureServiceFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		std::string hostName = wbem::server::getHostName();

		framework::attributes_t keys;
		keys[SYSTEMCREATIONCLASSNAME_KEY] =
				framework::Attribute(std::string(ERASURESERVICE_SYSTEMCREATIONCLASSNAME), true);

		// SystemName - host name
		keys[SYSTEMNAME_KEY] = framework::Attribute(std::string(hostName), true);

		// CreationClassName - "ErasureService"
		keys[CREATIONCLASSNAME_KEY] = framework::Attribute(ERASURESERVICE_CREATIONCLASSNAME, true);

		// Name - host name
		keys[NAME_KEY] = framework::Attribute(std::string(hostName), true);

		framework::ObjectPath path(hostName, NVM_NAMESPACE,
				ERASURESERVICE_CREATIONCLASSNAME, keys);
		pNames->push_back(path);
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}
	return pNames;
}

wbem::framework::UINT32 wbem::erasure::ErasureServiceFactory::executeMethod(
		wbem::framework::UINT32 &wbemRc,
		const std::string method,
		wbem::framework::ObjectPath &object,
		wbem::framework::attributes_t &inParms,
		wbem::framework::attributes_t &outParms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;
	wbemRc = framework::MOF_ERR_SUCCESS;

	if (method == ERASURESERVICE_ERASEDEVICE)
	{
		// uint32 EraseDevice(CIM_Job REF Job, CIM_ManagedElement REF Element,
		//						string ErasureMethod, string Password)

		// get parameters from inParms
		std::string elementObjectString = inParms[ERASURESERVICE_ERASEDEVICE_ELEMENT].stringValue();
		std::string erasureMethod = inParms[ERASURESERVICE_ERASEDEVICE_ERASUREMETHOD].stringValue();
		std::string password = inParms[ERASURESERVICE_ERASEDEVICE_PASSWORD].stringValue();

		enum eraseType eraseType = getEraseType(erasureMethod);

		if (elementObjectString.empty())
		{
			COMMON_LOG_ERROR_F("%s is required.", ERASURESERVICE_ERASEDEVICE_ELEMENT.c_str());
			httpRc = framework::CIM_ERR_INVALID_PARAMETER;
		}
		else if (erasureMethod.empty())
		{
			COMMON_LOG_ERROR_F("%s is required.", ERASURESERVICE_ERASEDEVICE_ERASUREMETHOD.c_str());
			httpRc = framework::CIM_ERR_INVALID_PARAMETER;
		}
		else if (password.empty())
		{
			COMMON_LOG_ERROR_F("%s is required.", ERASURESERVICE_ERASEDEVICE_PASSWORD.c_str());
                        httpRc = framework::CIM_ERR_INVALID_PARAMETER;
		}
		else if (eraseType == ERASETYPE_UNKNOWN)
		{
			COMMON_LOG_ERROR_F("Erasure Method %s is not supported", erasureMethod.c_str());
			httpRc = framework::CIM_ERR_INVALID_PARAMETER;
		}
		// Note: Password will get checked by eraseDevice
		else
		{
			// Build the object path from the attribute
			framework::ObjectPathBuilder builder(elementObjectString);
			framework::ObjectPath elementObject;
			builder.Build(&elementObject);

			try
			{
				if (elementObject.getClass() == physical_asset::NVDIMM_CREATIONCLASSNAME)
				{
					std::string deviceUidStr = elementObject.getKeyValue(TAG_KEY).stringValue();
					if (!core::device::isUidValid(deviceUidStr))
					{
						throw framework::ExceptionBadParameter("Tag");
					}
					NVM_UID deviceUid;
					uid_copy(deviceUidStr.c_str(), deviceUid);
					eraseDevice(deviceUidStr, password);
				}
				else if (elementObject.getClass() == software::NVDIMMCOLLECTION_CREATIONCLASSNAME)
				{
					eraseDevice(password);
				}
			}
			catch (wbem::exception::NvmExceptionLibError &e)
			{
				wbemRc = getReturnCodeFromLibException(e);
			}
			catch (wbem::framework::ExceptionBadParameter &)
			{
				httpRc = framework::CIM_ERR_INVALID_PARAMETER;
			}
		}
	}
	else if (method == ERASURESERVICE_ERASE)
	{
		// specific "Erase()" return code for "Not Supported"
		// don't use httpRc here because it's a valid method,
		// just not supported by our implementation
		wbemRc = ERASURESERVICE_ERR_NOT_SUPPORTED;
	}
	else
	{
		// all others are unsupported, including "Erase"
		httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
	}

	return httpRc;
}

wbem::framework::UINT32
wbem::erasure::ErasureServiceFactory::getReturnCodeFromLibException(
		const exception::NvmExceptionLibError &e)
{
	wbem::framework::UINT32 rc;

	switch(e.getLibError())
	{
	case NVM_ERR_INVALIDPERMISSIONS:
	case NVM_ERR_BADPASSPHRASE:
		rc = ERASURESERVICE_ERR_PERMISSION_FAILURE;
		break;
	case NVM_ERR_NOTMANAGEABLE:
	case NVM_ERR_SECURITYFROZEN:
	case NVM_ERR_SECURITYDISABLED:
	case NVM_ERR_LIMITPASSPHRASE:
	case NVM_ERR_DEVICEBUSY:
		rc = ERASURESERVICE_ERR_BAD_STATE;
		break;
	default:
		rc = ERASURESERVICE_ERR_FAILED;
		break;
	}

	return rc;
}

wbem::erasure::eraseType wbem::erasure::ErasureServiceFactory::getEraseType(std::string erasureMethod)
{
	wbem::erasure::eraseType result;
	if (erasureMethod == ERASURECAPABILITIES_ERASUREMETHOD_CRYPTO_ERASE)
	{
		result = ERASETYPE_CRYPTO_ERASE;
	}
	else
	{
		result = ERASETYPE_UNKNOWN;
	}

	return result;
}

void wbem::erasure::ErasureServiceFactory::eraseDevice(std::string deviceUid,
		std::string password)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (!core::device::isUidValid(deviceUid))
	{
		throw framework::ExceptionBadParameter("deviceUid");
	}

	NVM_UID uid;
	uid_copy(deviceUid.c_str(), uid);

	int rc = m_eraseDevice(uid, password.c_str(), password.length());
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

void wbem::erasure::ErasureServiceFactory::eraseDevice(std::string password)
throw (framework::Exception)
{
	std::vector<std::string> devices = m_GetManageabledeviceUids();
	for (size_t i = 0; i < devices.size(); i++)
	{
		eraseDevice(devices[i], password);
	}
}
