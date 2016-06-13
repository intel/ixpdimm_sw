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
 * This file contains the provider for the FWEventLog instances
 * that contain FW debug logs.
 */

#include <nvm_management.h>
#include <uid/uid.h>
#include <LogEnterExit.h>
#include <physical_asset/NVDIMMFactory.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include "FWEventLogFactory.h"
#include <server/BaseServerFactory.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>
#include <core/device/DeviceHelper.h>

wbem::support::FWEventLogFactory::FWEventLogFactory()
: m_GetManageabledeviceUids(physical_asset::NVDIMMFactory::getManageableDeviceUids)
{
	m_GetFwLogLevel = nvm_get_fw_log_level;
}

wbem::support::FWEventLogFactory::~FWEventLogFactory()
{
}

void wbem::support::FWEventLogFactory::populateAttributeList(
		framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(MAXNUMBEROFRECORDS_KEY);
	attributes.push_back(CURRENTNUMBEROFRECORDS_KEY);
	attributes.push_back(OVERWRITEPOLICY_KEY);
	attributes.push_back(ENABLEDSTATE_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::support::FWEventLogFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);
		path.checkKey(CREATIONCLASSNAME_KEY, FWEVENTLOG_CREATIONCLASSNAME);

		// extract the UID from the object path
		std::string deviceUidStr = path.getKeyValue(INSTANCEID_KEY).stringValue();
		if (!core::device::isUidValid(deviceUidStr))
		{
			throw framework::ExceptionBadParameter("InstanceId");
		}
		NVM_UID dimmUid;
		uid_copy(deviceUidStr.c_str(), dimmUid);

		int eamrc;
		if (NVM_SUCCESS != (eamrc = wbem::physical_asset::NVDIMMFactory::existsAndIsManageable(deviceUidStr)))
		{
			throw exception::NvmExceptionLibError(eamrc);
		}

		// get dimm discovery info to pull uid
		struct device_discovery dimmdiscovery;
		int rc = nvm_get_device_discovery(dimmUid, &dimmdiscovery);
		if (rc != NVM_SUCCESS)
		{
			// couldn't retrieve any dimm info
			throw exception::NvmExceptionLibError(rc);
		}
		// populate Element Name = FWEVENTLOG_ELEMENTNAME + UID
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			NVM_UID uidStr;
			uid_copy(dimmdiscovery.uid, uidStr);
			framework::Attribute attrElementName(
					FWEVENTLOG_ELEMENTNAME + std::string(uidStr), false);
			pInstance->setAttribute(ELEMENTNAME_KEY, attrElementName, attributes);
		}

		// MaxNumberOfRecords
		if (containsAttribute(MAXNUMBEROFRECORDS_KEY, attributes))
		{
			framework::Attribute a((NVM_UINT64)0, false);
			pInstance->setAttribute(MAXNUMBEROFRECORDS_KEY, a, attributes);
		}
		// CurrentNumberOfRecords Number
		if (containsAttribute(CURRENTNUMBEROFRECORDS_KEY, attributes))
		{
			framework::Attribute a((NVM_UINT64)0, false);
			pInstance->setAttribute(CURRENTNUMBEROFRECORDS_KEY, a, attributes);
		}
		// OverwritePolicy
		if (containsAttribute(OVERWRITEPOLICY_KEY, attributes))
		{
			framework::Attribute a((NVM_UINT16)2, false); // "Wraps When Full"
			pInstance->setAttribute(OVERWRITEPOLICY_KEY, a, attributes);
		}

		// call nvm_get_fw_log_level to get current enabled state
		enum fw_log_level curr_log_level;
		rc = m_GetFwLogLevel(dimmUid, &curr_log_level);
		if (rc != NVM_SUCCESS)
		{
			// couldn't retrieve event log level
			throw exception::NvmExceptionLibError(rc);
		}
		// EnabledState
		if (containsAttribute(ENABLEDSTATE_KEY, attributes))
		{
			int log_level = 0;
			std::string FwLogLevelStr;
			switch (curr_log_level)
			{
				case FW_LOG_LEVEL_DISABLED:
					log_level = CIMFWEVENTLOG_DISABLED;
					FwLogLevelStr = "Disabled";
					break;
				case FW_LOG_LEVEL_ERROR:
					log_level = CIMFWEVENTLOG_ERROR;
					FwLogLevelStr = "Error";
					break;
				case FW_LOG_LEVEL_WARN:
					log_level = CIMFWEVENTLOG_WARNING;
					FwLogLevelStr = "Warning";
					break;
				case FW_LOG_LEVEL_INFO:
					log_level = CIMFWEVENTLOG_INFO;
					FwLogLevelStr = "Info";
					break;
				case FW_LOG_LEVEL_DEBUG:
					log_level = CIMFWEVENTLOG_DEBUG;
					FwLogLevelStr = "Debug";
					break;
				default:
					log_level = CIMFWEVENTLOG_UNKNOWN;
					FwLogLevelStr = "Unknown";
					break;
			}
			framework::Attribute attrFwLogLevel((NVM_UINT16)log_level, FwLogLevelStr, false);
			pInstance->setAttribute(ENABLEDSTATE_KEY, attrFwLogLevel, attributes);
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
 Return the object paths for the FWEventLog class, one per DIMM
 */
wbem::framework::instance_names_t* wbem::support::FWEventLogFactory::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		// get the host server name
		std::string hostName = wbem::server::getHostName();

		std::vector<std::string> devices = wbem::physical_asset::NVDIMMFactory::getManageableDeviceUids();
		// get the list of dimms
		if (devices.size()/*dimmCount*/ > 0)
		{
			for (size_t i = 0; i < devices.size(); i++)
			{
				framework::attributes_t keys;
				framework::Attribute attrTag(devices[i], true);
				keys.insert(std::pair<std::string, framework::Attribute>(
						INSTANCEID_KEY, attrTag));

				// CreationClassName
				framework::Attribute attrCCName(FWEVENTLOG_CREATIONCLASSNAME, true);
				keys.insert(std::pair<std::string, framework::Attribute>(
						CREATIONCLASSNAME_KEY, attrCCName));

				// generate the ObjectPath for the instance
				framework::ObjectPath path(hostName, NVM_NAMESPACE,
						FWEVENTLOG_CREATIONCLASSNAME, keys);
				pNames->push_back(path);
			}
		}
		else
		{
			// should never get here except in SIM
			COMMON_LOG_DEBUG("No Intel NVDIMMs found.");
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}
	return pNames;
}
