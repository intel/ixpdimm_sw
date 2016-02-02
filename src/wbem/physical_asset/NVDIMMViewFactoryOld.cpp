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

#include <exception/NvmExceptionLibError.h>
#include <intel_cim_framework/ExceptionBadParameter.h>
#include <server/BaseServerFactory.h>
#include <physical_asset/NVDIMMViewFactoryOld.h>
#include <physical_asset/NVDIMMFactory.h>
#include <LogEnterExit.h>
#include <string/s_str.h>
#include <sstream>
#include <string.h>
#include <guid/guid.h>
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>

wbem::physical_asset::NVDIMMViewFactoryOld::NVDIMMViewFactoryOld()
			throw (wbem::framework::Exception)
			: m_GetFwLogLevel(nvm_get_fw_log_level),
			  m_injectDeviceError(nvm_inject_device_error),
			  m_clearInjectedDeviceError(nvm_clear_injected_device_error)
{

}

wbem::physical_asset::NVDIMMViewFactoryOld::~NVDIMMViewFactoryOld()
{
}

void wbem::physical_asset::NVDIMMViewFactoryOld::populateAttributeList(
	framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(DIMMGUID_KEY);

	// add non-key NVDIMM attributes
	attributes.push_back(DIMMID_KEY);
	attributes.push_back(DIMMHANDLE_KEY);
	attributes.push_back(PHYSICALID_KEY);
	attributes.push_back(MANUFACTURER_KEY);
	attributes.push_back(MANUFACTURERID_KEY);
	attributes.push_back(MODEL_KEY);
	attributes.push_back(CAPACITY_KEY);
	attributes.push_back(VENDORID_KEY);
	attributes.push_back(DEVICEID_KEY);
	attributes.push_back(REVISIONID_KEY);
	attributes.push_back(SOCKETID_KEY);
	attributes.push_back(MEMORYTYPE_KEY);
	attributes.push_back(SERIALNUMBER_KEY);
	attributes.push_back(LOCKSTATE_KEY);
	attributes.push_back(MANAGEABILITYSTATE_KEY);
	attributes.push_back(FORMFACTOR_KEY);
	attributes.push_back(DATAWIDTH_KEY);
	attributes.push_back(TOTALWIDTH_KEY);
	attributes.push_back(SPEED_KEY);
	attributes.push_back(VOLATILECAPACITY_KEY);
	attributes.push_back(PERSISTENTCAPACITY_KEY);
	attributes.push_back(PARTNUMBER_KEY);
	attributes.push_back(BANKLABEL_KEY);
	attributes.push_back(HEALTHSTATE_KEY);
	attributes.push_back(COMMUNICATIONSTATUS_KEY);
	attributes.push_back(ISNEW_KEY);
	attributes.push_back(POWERMANAGEMENTENABLED_KEY);
	attributes.push_back(POWERLIMIT_KEY);
	attributes.push_back(PEAKPOWERBUDGET_KEY);
	attributes.push_back(AVGPOWERBUDGET_KEY);
	attributes.push_back(DIESPARINGENABLED_KEY);
	attributes.push_back(DIESPARINGLEVEL_KEY);
	attributes.push_back(LASTSHUTDOWNSTATUS_KEY);
	attributes.push_back(DIESPARESUSED_KEY);
	attributes.push_back(FIRSTFASTREFRESH_KEY);
	attributes.push_back(CHANNEL_KEY);
	attributes.push_back(CHANNELPOS_KEY);
	attributes.push_back(CONFIGURATIONSTATUS_KEY);
	attributes.push_back(SECURITYCAPABILITIES_KEY);
	attributes.push_back(LASTSHUTDOWNTIME_KEY);
	attributes.push_back(DIESPARINGCAPABLE_KEY);
	attributes.push_back(MEMORYTYPECAPABILITIES_KEY);
	attributes.push_back(FWLOGLEVEL_KEY);

	// Add View Attributes
	attributes.push_back(FWLOGLEVEL_KEY);
	attributes.push_back(FWAPIVERSION_KEY);
	attributes.push_back(FWVERSION_KEY);
	attributes.push_back(UNCONFIGUREDCAPACITY_KEY);
	attributes.push_back(INACCESSIBLECAPACITY_KEY);
	attributes.push_back(RESERVEDCAPACITY_KEY);
	attributes.push_back(INTERFACEFORMATCODE_KEY);
	attributes.push_back(DEVICELOCATOR_KEY);
	attributes.push_back(ACTIONREQUIRED_KEY);
	attributes.push_back(ACTIONREQUIREDEVENTS_KEY);
	attributes.push_back(MEMORYMODESSUPPORTED_KEY);
	attributes.push_back(MIXEDSKU_KEY);
	attributes.push_back(SKUVIOLATION_KEY);
	attributes.push_back(MEMCONTROLLERID_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::physical_asset::NVDIMMViewFactoryOld::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		checkAttributes(attributes);
		// extract the GUID from the object path
		framework::Attribute guidAttr = path.getKeyValue(DIMMGUID_KEY);

		// -1 because the length of the string does not include the null terminator
		if (guidAttr.stringValue().length() != NVM_GUIDSTR_LEN - 1)
		{
			throw framework::ExceptionBadParameter(guidAttr.stringValue().c_str());
		}

		NVM_GUID dimmGuid;
		str_to_guid((char*)guidAttr.stringValue().c_str(), dimmGuid);

		// get dimm info
		struct device_details dimm;
		memset(&dimm, 0, sizeof(dimm));
		int rc = nvm_get_device_details(dimmGuid, &dimm);
		// try to get device discovery if device is not manageable
		if (rc == NVM_ERR_NOTMANAGEABLE)
		{
			int tmpRc = nvm_get_device_discovery(dimmGuid, &dimm.discovery);
			if (tmpRc != NVM_SUCCESS)
			{
				// couldn't retrieve any dimm info, punt
				throw exception::NvmExceptionLibError(rc);
			}
		}
		// any other error besides not manageable, punt
		else if (rc != NVM_SUCCESS)
		{
			// couldn't retrieve the dimm info for some other reason than not being manageable
			throw exception::NvmExceptionLibError(rc);
		}

		NVDIMMFactory::fillInNVDIMMInstance(dimm, attributes, pInstance);

		// DimmID = handle or guid depending on user selection
		if (containsAttribute(DIMMID_KEY, attributes))
		{
			framework::Attribute attrDimmId = guidToDimmIdAttribute(guidAttr.stringValue());
			pInstance->setAttribute(DIMMID_KEY, attrDimmId, attributes);
		}
		// DimmHandle = NFIT Handle
		if (containsAttribute(DIMMHANDLE_KEY, attributes))
		{
			framework::Attribute attrDimmHandle(dimm.discovery.device_handle.handle, false);
			pInstance->setAttribute(DIMMHANDLE_KEY, attrDimmHandle, attributes);
		}
		// FWLogLevel
		if (containsAttribute(FWLOGLEVEL_KEY, attributes))
		{
			enum fw_log_level curr_log_level;
			rc = m_GetFwLogLevel(dimmGuid, &curr_log_level);

			if (rc != NVM_SUCCESS)
			{
				// couldn't retrieve event log level
				curr_log_level = FW_LOG_LEVEL_UNKNOWN;
			}

			std::string FwLogLevelStr;
			switch (curr_log_level)
			{
				case FW_LOG_LEVEL_DISABLED:
					FwLogLevelStr = "Disabled";
					break;
				case FW_LOG_LEVEL_ERROR:
					FwLogLevelStr = "Error";
					break;
				case FW_LOG_LEVEL_WARN:
					FwLogLevelStr = "Warning";
					break;
				case FW_LOG_LEVEL_INFO:
					FwLogLevelStr = "Info";
					break;
				case FW_LOG_LEVEL_DEBUG:
					FwLogLevelStr = "Debug";
					break;
				default:
					FwLogLevelStr = "Unknown";
					break;

			}
			framework::Attribute attrFwLogLevel((NVM_UINT32)curr_log_level, FwLogLevelStr, false);
			pInstance->setAttribute(FWLOGLEVEL_KEY, attrFwLogLevel, attributes);
		}
		// Device Locator
		if (containsAttribute(DEVICELOCATOR_KEY, attributes))
		{
			framework::Attribute attrDeviceLocator(dimm.device_locator, false);
			pInstance->setAttribute(DEVICELOCATOR_KEY, attrDeviceLocator, attributes);
		}
		// InterfaceFormatCode
		if (containsAttribute(INTERFACEFORMATCODE_KEY, attributes))
		{
			framework::Attribute attrIfc(dimm.discovery.interface_format_code, false);
			pInstance->setAttribute(INTERFACEFORMATCODE_KEY, attrIfc, attributes);
		}
		// FW API Version
		if (containsAttribute(FWAPIVERSION_KEY, attributes))
		{
			framework::Attribute attrFwApiVersion(dimm.discovery.fw_api_version, false);
			pInstance->setAttribute(FWAPIVERSION_KEY, attrFwApiVersion, attributes);
		}
		// FW Version
		if (containsAttribute(FWVERSION_KEY, attributes))
		{
			framework::Attribute attrFwVersion(dimm.discovery.fw_revision, false);
			pInstance->setAttribute(FWVERSION_KEY, attrFwVersion, attributes);
		}

		if (containsAttribute(UNCONFIGUREDCAPACITY_KEY, attributes))
		{
			framework::Attribute attrUnconfiguredCapacity(dimm.capacities.unconfigured_capacity, false);
			pInstance->setAttribute(UNCONFIGUREDCAPACITY_KEY, attrUnconfiguredCapacity, attributes);
		}

		if (containsAttribute(INACCESSIBLECAPACITY_KEY, attributes))
		{
			framework::Attribute attrInaccessibleCapacity(dimm.capacities.inaccessible_capacity, false);
			pInstance->setAttribute(INACCESSIBLECAPACITY_KEY, attrInaccessibleCapacity, attributes);
		}

		if (containsAttribute(RESERVEDCAPACITY_KEY, attributes))
		{
			framework::Attribute attrReservedCapacity(dimm.capacities.reserved_capacity, false);
			pInstance->setAttribute(RESERVEDCAPACITY_KEY, attrReservedCapacity, attributes);
		}

		if (containsAttribute(ACTIONREQUIRED_KEY, attributes) ||
						containsAttribute(ACTIONREQUIREDEVENTS_KEY, attributes))
		{
			struct event_filter filter;
			memset(&filter, 0, sizeof (filter));
			filter.filter_mask = NVM_FILTER_ON_AR | NVM_FILTER_ON_GUID;
			filter.action_required = true;
			memmove(filter.guid, dimmGuid, NVM_GUID_LEN);
			int eventCount = nvm_get_event_count(&filter);
			if (eventCount < 0)				{
				COMMON_LOG_ERROR_F("Failed to retrieve events for namespace %s, error %d",
						(char*)guidAttr.stringValue().c_str(), eventCount);
				throw exception::NvmExceptionLibError(eventCount);
			}

			// ActionRequired = true if any unacknowledged action required events for this namespace
			if (containsAttribute(ACTIONREQUIRED_KEY, attributes))
			{
				framework::Attribute a(eventCount > 0 ? true : false, false);
				pInstance->setAttribute(ACTIONREQUIRED_KEY, a, attributes);
			}

			// ActionRequiredEvents = list of action required events ids and messages
			if (containsAttribute(ACTIONREQUIREDEVENTS_KEY, attributes))
			{
				framework::STR_LIST arEventList;
				if (eventCount > 0)
				{
					// get the events
					struct event events[eventCount];
					eventCount = nvm_get_events(&filter, events, eventCount);
					if (eventCount < 0)
					{
						COMMON_LOG_ERROR_F("Failed to retrieve events for namespace %s, error %d",
								(char*)guidAttr.stringValue().c_str(), eventCount);
						throw exception::NvmExceptionLibError(eventCount);
					}

					for (int i = 0; i < eventCount; i++)
					{
						std::stringstream eventMsg;
						eventMsg << "Event " << events[i].event_id;
						char msg[NVM_EVENT_MSG_LEN + (3 * NVM_EVENT_ARG_LEN)];
						s_snprintf(msg, (NVM_EVENT_MSG_LEN + (3 * NVM_EVENT_ARG_LEN)),
								events[i].message,
								events[i].args[0],
								events[i].args[1],
								events[i].args[2]);
						eventMsg << " - " << msg;
						arEventList.push_back(eventMsg.str());
					}
				}
				framework::Attribute a(arEventList, false);
				pInstance->setAttribute(ACTIONREQUIREDEVENTS_KEY, a, attributes);
			}
		}

		// MemoryModesSupported
		if (containsAttribute(MEMORYMODESSUPPORTED_KEY, attributes))
		{
			framework::Attribute a(getMemoryModesSupported(dimm), false);
			pInstance->setAttribute(MEMORYMODESSUPPORTED_KEY, a, attributes);
		}
		// MixedSKU
		if (containsAttribute(MIXEDSKU_KEY, attributes))
		{
			framework::Attribute a((bool)dimm.status.mixed_sku, false);
			pInstance->setAttribute(MIXEDSKU_KEY, a, attributes);
		}
		// SKUViolation
		if (containsAttribute(SKUVIOLATION_KEY, attributes))
		{
			framework::Attribute a((bool)dimm.status.sku_violation, false);
			pInstance->setAttribute(SKUVIOLATION_KEY, a, attributes);
		}
		// MemControllerID
		if (containsAttribute(MEMCONTROLLERID_KEY, attributes))
		{
			framework::Attribute attrMemCtrl(dimm.discovery.memory_controller_id, false);
			pInstance->setAttribute(MEMCONTROLLERID_KEY, attrMemCtrl, attributes);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		if (pInstance != NULL)
		{
			delete pInstance;
		}
		throw;
	}

	return pInstance;
}

std::string wbem::physical_asset::NVDIMMViewFactoryOld::getMemoryModesSupported(const struct device_details &dimm)
{
	std::stringstream memoryModes;

	framework::UINT16_LIST memoryModeList;
	NVDIMMFactory::buildMemoryTypeCapabilitiesFromDeviceCapabilities(dimm.discovery.device_capabilities, memoryModeList);

	for (size_t i = 0; i < memoryModeList.size(); i++)
	{
		if (i > 0)
		{
			memoryModes << ", ";
		}

		memoryModes << getMemoryModeString(memoryModeList[i]);
	}

	return memoryModes.str();
}

std::string wbem::physical_asset::NVDIMMViewFactoryOld::getMemoryModeString(const framework::UINT16 mode)
{
	std::string result;

	switch (mode)
	{
	case NVDIMM_MEMORYTYPECAPABILITIES_MEMORYMODE:
		result = TR("2LM");
		break;
	case NVDIMM_MEMORYTYPECAPABILITIES_STORAGEMODE:
		result = TR("Storage");
		break;
	case NVDIMM_MEMORYTYPECAPABILITIES_APPDIRECTMODE:
		result = TR("AppDirect");
		break;
	default:
		result = TR("Unknown");
	}

	return result;
}

/*
 * Return an object path for each NVDIMM in the system
 */
wbem::framework::instance_names_t* wbem::physical_asset::NVDIMMViewFactoryOld::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		// get the host server name and array of devices
		std::string hostName = wbem::server::getHostName();
		wbem::physical_asset::devices_t devices = wbem::physical_asset::NVDIMMFactory::getAllDevices();

		// create an object path for each dimm
		wbem::physical_asset::devices_t::const_iterator iter = devices.begin();
		for (; iter != devices.end(); iter++)
		{
			framework::attributes_t keys;

			// Tag = DIMM GUID
			NVM_GUID_STR guidStr;
			guid_to_str(iter->guid, guidStr);
			framework::Attribute attrDimmID(guidStr, true);
			keys.insert(std::pair<std::string, framework::Attribute>(
					DIMMGUID_KEY, attrDimmID));

			// generate the ObjectPath for the instance
			framework::ObjectPath path(hostName, NVM_NAMESPACE,
					NVDIMMVIEW_CREATIONCLASSNAMEOLD, keys);
			pNames->push_back(path);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}

	return pNames;
}

/*
 * Attempt to convert a handle to a GUID. Return false if the
 * handle is not found.
 * TODO: there is a potential performance improvement to be had here
 * by caching the dimm list rather than retrieving it every time.
 */
bool wbem::physical_asset::NVDIMMViewFactoryOld::handleToGuid(
		const NVM_UINT32 &handle, std::string &dimmGuid)
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool validHandle = false;

	wbem::physical_asset::devices_t devices = wbem::physical_asset::NVDIMMFactory::getAllDevices();

	physical_asset::devices_t::const_iterator iter = devices.begin();
	// find the matching handle
	for (; iter != devices.end(); iter++)
	{
		if ((*iter).device_handle.handle == handle)
		{
			validHandle = true;
			NVM_GUID_STR guidStr;
			guid_to_str((*iter).guid, guidStr);
			dimmGuid = guidStr;
			break;
		}
	}

	return validHandle;
}

/*
 * Attempt to convert a GUID to a handle. Throws an
 * exeption if not found.
 */
void wbem::physical_asset::NVDIMMViewFactoryOld::guidToHandle(
		const std::string& guidStr,  NVM_UINT32& handle)
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_GUID guid;
	handle = 0;

	str_to_guid(guidStr.c_str(), guid);
	struct device_discovery device;
	int rc;
	if ((rc = nvm_get_device_discovery(guid, &device)) == NVM_SUCCESS)
	{
		handle = device.device_handle.handle;
	}
	else
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

/*!
 * Utility method to convert a Dimm GUID to an ID attribute based on the db setting
 */
wbem::framework::Attribute wbem::physical_asset::NVDIMMViewFactoryOld::guidToDimmIdAttribute(
		const std::string &dimmGuid)
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Attribute dimmIdAttr;
	// look up the database setting for the preferred DimmID output
	// default to using dimm handle
	bool useHandle = true;
	char value[CONFIG_VALUE_LEN];
	if (get_config_value(SQL_KEY_CLI_DIMM_ID, value) == COMMON_SUCCESS)
	{
		// switch to guid
		if (s_strncmpi("GUID", value, strlen("GUID")) == 0)
		{
			useHandle = false;
		}
	}

	// convert GUID to handle
	if (useHandle)
	{
		NVM_UINT32 handle;
		guidToHandle(dimmGuid.c_str(), handle);
		framework::Attribute attrHandle(handle, false);
		dimmIdAttr = attrHandle;
	}
	// use GUID
	else
	{
		framework::Attribute attrGuid(dimmGuid, false);
		dimmIdAttr = attrGuid;
	}
	return dimmIdAttr;
}

/*!
 * Utility method to convert a Dimm GUID to a string based on the db setting
 */
std::string wbem::physical_asset::NVDIMMViewFactoryOld::guidToDimmIdStr(const std::string &dimmGuid)
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::framework::Attribute attribute = guidToDimmIdAttribute(dimmGuid);
	return attribute.asStr();
}

void wbem::physical_asset::NVDIMMViewFactoryOld::injectPoisonError(const std::string &dimmGuid,
		const NVM_UINT64 dpa)
throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct device_error error;
	memset(&error, 0, sizeof (error));
	error.type = ERROR_TYPE_POISON;
	error.error_injection_parameter.dpa = dpa;
	injectError(dimmGuid, &error);
}

void wbem::physical_asset::NVDIMMViewFactoryOld::clearPoisonError(const std::string &dimmGuid,
		const NVM_UINT64 dpa)
throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct device_error error;
	memset(&error, 0, sizeof (error));
	error.type = ERROR_TYPE_POISON;
	error.error_injection_parameter.dpa = dpa;
	clearError(dimmGuid, &error);
}

void wbem::physical_asset::NVDIMMViewFactoryOld::clearAllErrors(const std::string &dimmGuid)
throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct device_error error;
	memset(&error, 0, sizeof (error));
	error.type = ERROR_TYPE_CLEAR_ALL;
	clearError(dimmGuid, &error);
}

void wbem::physical_asset::NVDIMMViewFactoryOld::injectTemperatureError(
		const std::string &dimmGuid,
		const NVM_REAL32 temperature)
throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct device_error error;
	memset(&error, 0, sizeof (error));
	error.type = ERROR_TYPE_TEMPERATURE;
	error.error_injection_parameter.temperature = nvm_encode_temperature(temperature);
	injectError(dimmGuid, &error);
}

void wbem::physical_asset::NVDIMMViewFactoryOld::clearError(const std::string &dimmGuid,
		struct device_error *p_error)
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// check for valid dimm guid
	// note - .length() doesn't include NULL terminator
	if (dimmGuid.empty() || dimmGuid.length() != NVM_GUIDSTR_LEN - 1)
	{
		COMMON_LOG_ERROR("Invalid dimm guid");
		throw wbem::framework::ExceptionBadParameter(wbem::DEVICEID_KEY.c_str());
	}
	NVM_GUID guid;
	str_to_guid(dimmGuid.c_str(), guid);

	int rc = m_clearInjectedDeviceError(guid, p_error);
	if (rc != NVM_SUCCESS)
	{
		throw wbem::exception::NvmExceptionLibError(rc);
	}
}

void wbem::physical_asset::NVDIMMViewFactoryOld::injectError(const std::string &dimmGuid,
		struct device_error *p_error)
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// check for valid dimm guid
	// note - .length() doesn't include NULL terminator
	if (dimmGuid.empty() || dimmGuid.length() != NVM_GUIDSTR_LEN - 1)
	{
		COMMON_LOG_ERROR("Invalid dimm guid");
		throw wbem::framework::ExceptionBadParameter(wbem::DEVICEID_KEY.c_str());
	}
	NVM_GUID guid;
	str_to_guid(dimmGuid.c_str(), guid);

	int rc = m_injectDeviceError(guid, p_error);
	if (rc != NVM_SUCCESS)
	{
		throw wbem::exception::NvmExceptionLibError(rc);
	}
}

