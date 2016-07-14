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
 * This file contains the provider for the SystemCapabilities instance
 * which is an internal only view class for the CLI that combines
 * MemoryCapabilities and PersistentMemoryCapabilities.
 */

#include <sstream>
#include <algorithm>
#include <nvm_management.h>

#include <LogEnterExit.h>

#include <libinvm-cim/Types.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include "SystemCapabilitiesFactory.h"
#include <server/BaseServerFactory.h>
#include <mem_config/InterleaveSet.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>

wbem::server::SystemCapabilitiesFactory::SystemCapabilitiesFactory()
{
}

wbem::server::SystemCapabilitiesFactory::~SystemCapabilitiesFactory()
{
}

void wbem::server::SystemCapabilitiesFactory::populateAttributeList(
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(SYSTEMNAME_KEY);

	// add non-key attributes
	attributes.push_back(PLATFORMCONFIGSUPPORTED_KEY);
	attributes.push_back(ALIGNMENT_KEY);
	attributes.push_back(MEMORYMODESSUPPORTED_KEY);
	attributes.push_back(CURRENTVOLATILEMODE_KEY);
	attributes.push_back(CURRENTAPPDIRECTMODE_KEY);
	attributes.push_back(SUPPORTEDAPP_DIRECT_SETTINGS_KEY);
	attributes.push_back(RECOMMENDEDAPP_DIRECT_SETTINGS_KEY);
	attributes.push_back(MINNAMESPACESIZE_KEY);
	attributes.push_back(BLOCKSIZES_KEY);
	attributes.push_back(APP_DIRECT_MEMORY_MIRROR_SUPPORT_KEY);
	attributes.push_back(DIMMSPARESUPPORT_KEY);
	attributes.push_back(APP_DIRECT_MEMORY_MIGRATION_SUPPORT_KEY);
	attributes.push_back(RENAMENAMESPACESUPPORT_KEY);
	attributes.push_back(ENABLENAMESPACESUPPORT_KEY);
	attributes.push_back(DISABLENAMESPACESUPPORT_KEY);
	attributes.push_back(GROWAPPDIRECTNAMESPACESUPPORT_KEY);
	attributes.push_back(SHRINKAPPDIRECTNAMESPACESUPPORT_KEY);
	attributes.push_back(GROWSTORAGENAMESPACESUPPORT_KEY);
	attributes.push_back(SHRINKSTORAGENAMESPACESUPPORT_KEY);
	attributes.push_back(INITIATESCRUBSUPPORT_KEY);
	attributes.push_back(MEMORYPAGEALLOCATIONCAPABLE_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::server::SystemCapabilitiesFactory::getInstance(
		framework::ObjectPath &path,
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Verify host in object path
	std::string hostName = wbem::server::getHostName();
	framework::Attribute attribute = path.getKeyValue(SYSTEMNAME_KEY);
	if (attribute.stringValue() != hostName)
	{
		throw framework::ExceptionBadParameter(SYSTEMNAME_KEY.c_str());
	}

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		checkAttributes(attributes);

		struct nvm_capabilities nvmCaps;
		int rc = nvm_get_nvm_capabilities(&nvmCaps);
		if (rc == NVM_SUCCESS)
		{
			// PlatformConfigSupported - boolean
			// If true, the platform level configuration of NVM-DIMMs can be modified.
			// If false, any supported changes must be done through BIOS setup
			if (containsAttribute(PLATFORMCONFIGSUPPORTED_KEY, attributes))
			{
				framework::Attribute attr((framework::UINT16)nvmCaps.nvm_features.modify_device_capacity, false);
				pInstance->setAttribute(PLATFORMCONFIGSUPPORTED_KEY, attr, attributes);
			}

			// Alignment - BIOS reported alignment requirement
			if (containsAttribute(ALIGNMENT_KEY, attributes))
			{
				framework::Attribute attr(getAppDirectAlignment(nvmCaps), false);
				pInstance->setAttribute(ALIGNMENT_KEY, attr, attributes);
			}

			// MemoryModes
			if (containsAttribute(MEMORYMODESSUPPORTED_KEY, attributes))
			{
				framework::Attribute attr(getSupportedMemoryModes(nvmCaps), false);
				pInstance->setAttribute(MEMORYMODESSUPPORTED_KEY, attr, attributes);
			}

			// CurrentVolatileMode - volatile memory mode currently selected by the BIOS
			if (containsAttribute(CURRENTVOLATILEMODE_KEY, attributes))
			{
				framework::Attribute attr(getCurrentVolatileMode(nvmCaps), false);
				pInstance->setAttribute(CURRENTVOLATILEMODE_KEY, attr, attributes);
			}

			// CurrentAppDirectMode - App Direct mode currently selected by the BIOS
			if (containsAttribute(CURRENTAPPDIRECTMODE_KEY, attributes))
			{
				framework::Attribute attr(getCurrentAppDirectMode(nvmCaps), false);
				pInstance->setAttribute(CURRENTAPPDIRECTMODE_KEY, attr, attributes);
			}

			if (containsAttribute(SUPPORTEDAPP_DIRECT_SETTINGS_KEY, attributes))
			{
				framework::Attribute attr(getSupportedSettings(nvmCaps), false);
				pInstance->setAttribute(SUPPORTEDAPP_DIRECT_SETTINGS_KEY, attr, attributes);
			}

			if (containsAttribute(RECOMMENDEDAPP_DIRECT_SETTINGS_KEY, attributes))
			{
				framework::Attribute attr(getRecommendedSettings(nvmCaps), false);
				pInstance->setAttribute(RECOMMENDEDAPP_DIRECT_SETTINGS_KEY, attr, attributes);
			}

			if (containsAttribute(MINNAMESPACESIZE_KEY, attributes))
			{
				framework::UINT32 minSize = 0;
				if (nvmCaps.nvm_features.create_namespace)
				{
					minSize = nvmCaps.sw_capabilities.min_namespace_size;
				}
				framework::Attribute attr(minSize, false);
				pInstance->setAttribute(MINNAMESPACESIZE_KEY, attr, attributes);
			}
			if (containsAttribute(BLOCKSIZES_KEY, attributes))
			{
				framework::UINT32_LIST blockSizeList;
				if (nvmCaps.nvm_features.create_namespace)
				{
					for (NVM_UINT32 i = 0; i < nvmCaps.sw_capabilities.block_size_count; i++)
					{
						blockSizeList.push_back(nvmCaps.sw_capabilities.block_sizes[i]);
					}
				}
				framework::Attribute attr(blockSizeList, false);
				pInstance->setAttribute(BLOCKSIZES_KEY, attr, attributes);
			}
			if (containsAttribute(APP_DIRECT_MEMORY_MIRROR_SUPPORT_KEY, attributes))
			{
				framework::UINT16 mirror = 0;
				if (nvmCaps.nvm_features.app_direct_mode)
				{
					mirror = nvmCaps.platform_capabilities.memory_mirror_supported;
				}
				framework::Attribute attr(mirror, false);
				pInstance->setAttribute(APP_DIRECT_MEMORY_MIRROR_SUPPORT_KEY, attr, attributes);
			}
			if (containsAttribute(DIMMSPARESUPPORT_KEY, attributes))
			{
				framework::UINT16 spare = 0;
				if (nvmCaps.nvm_features.app_direct_mode)
				{
					spare = nvmCaps.platform_capabilities.memory_spare_supported;
				}
				framework::Attribute attr(spare, false);
				pInstance->setAttribute(DIMMSPARESUPPORT_KEY, attr, attributes);
			}
			if (containsAttribute(APP_DIRECT_MEMORY_MIGRATION_SUPPORT_KEY, attributes))
			{
				framework::UINT16 migration = 0;
				if (nvmCaps.nvm_features.app_direct_mode)
				{
					migration = nvmCaps.platform_capabilities.memory_migration_supported;
				}
				framework::Attribute attr(migration, false);
				pInstance->setAttribute(APP_DIRECT_MEMORY_MIGRATION_SUPPORT_KEY, attr, attributes);
			}

			addCapabilitySupportedAttribute(pInstance, attributes,
					RENAMENAMESPACESUPPORT_KEY,
					(bool)nvmCaps.nvm_features.rename_namespace);
			addCapabilitySupportedAttribute(pInstance, attributes,
					ENABLENAMESPACESUPPORT_KEY,
					(bool)nvmCaps.nvm_features.enable_namespace);
			addCapabilitySupportedAttribute(pInstance, attributes,
					DISABLENAMESPACESUPPORT_KEY,
					(bool)nvmCaps.nvm_features.disable_namespace);
			addCapabilitySupportedAttribute(pInstance, attributes,
					GROWAPPDIRECTNAMESPACESUPPORT_KEY,
					nvmCaps.nvm_features.grow_namespace && nvmCaps.nvm_features.app_direct_mode);
			addCapabilitySupportedAttribute(pInstance, attributes,
					SHRINKAPPDIRECTNAMESPACESUPPORT_KEY,
					nvmCaps.nvm_features.shrink_namespace && nvmCaps.nvm_features.app_direct_mode);
			addCapabilitySupportedAttribute(pInstance, attributes,
					GROWSTORAGENAMESPACESUPPORT_KEY,
					nvmCaps.nvm_features.grow_namespace && nvmCaps.nvm_features.storage_mode);
			addCapabilitySupportedAttribute(pInstance, attributes,
					SHRINKSTORAGENAMESPACESUPPORT_KEY,
					nvmCaps.nvm_features.shrink_namespace && nvmCaps.nvm_features.app_direct_mode);
			addCapabilitySupportedAttribute(pInstance, attributes,
					INITIATESCRUBSUPPORT_KEY,
					(bool)nvmCaps.nvm_features.start_address_scrub);
			addCapabilitySupportedAttribute(pInstance, attributes,
					MEMORYPAGEALLOCATIONCAPABLE_KEY,
					(bool)nvmCaps.sw_capabilities.namespace_memory_page_allocation_capable);
		}
		else
		{
			throw exception::NvmExceptionLibError(rc);
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
 * Return the object paths for the MemoryCapabilities Class.
 */
wbem::framework::instance_names_t* wbem::server::SystemCapabilitiesFactory::getInstanceNames()
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// get the host server name
	std::string hostName = wbem::server::getHostName();
	framework::instance_names_t *pNames = new framework::instance_names_t();
	framework::attributes_t keys;

	keys[SYSTEMNAME_KEY] = framework::Attribute(hostName, true);

	// generate the ObjectPath for the instance (only one per server)
	framework::ObjectPath path(hostName, NVM_NAMESPACE,
			SYSTEMCAPABILITIES_CREATIONCLASSNAME, keys);
	pNames->push_back(path);

	return pNames;
}

wbem::framework::STR_LIST wbem::server::SystemCapabilitiesFactory::getSupportedMemoryModes(
		const struct nvm_capabilities &nvmCaps)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::STR_LIST modes;
	if (nvmCaps.platform_capabilities.one_lm_mode.supported)
	{
		modes.push_back(MEMORYMODE_1LM_STR);
	}
	if (nvmCaps.nvm_features.memory_mode)
	{
		modes.push_back(MEMORYMODE_MEMORY_STR);
	}
	if (nvmCaps.nvm_features.storage_mode)
	{
		modes.push_back(MEMORYMODE_STORAGE_STR);
	}
	if (nvmCaps.nvm_features.app_direct_mode)
	{
		modes.push_back(MEMORYMODE_APP_DIRECT_STR);
	}

	return modes;
}

std::string wbem::server::SystemCapabilitiesFactory::getCurrentVolatileMode(
		const struct nvm_capabilities &nvmCaps)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string returnStr;

	// if memory mode is not supported, default to 1lm
	enum volatile_mode mode = nvmCaps.platform_capabilities.current_volatile_mode;
	if (!nvmCaps.nvm_features.memory_mode &&
			mode == VOLATILE_MODE_MEMORY) // bios support but DIMM SKU does not support
	{
		mode = VOLATILE_MODE_1LM;
	}

	// convert mode to a string
	switch (mode)
	{
		case VOLATILE_MODE_1LM:
			returnStr = MEMORYMODE_1LM_STR;
			break;
		case VOLATILE_MODE_MEMORY:
			returnStr = MEMORYMODE_MEMORY_STR;
			break;
		case VOLATILE_MODE_AUTO:
			returnStr = MEMORYMODE_AUTO_STR;
			break;
		case VOLATILE_MODE_UNKNOWN:
		default:
			returnStr = MEMORYMODE_UNKNOWN_STR;
			break;
	}
	return returnStr;
}

std::string wbem::server::SystemCapabilitiesFactory::getCurrentAppDirectMode(
		const struct nvm_capabilities &nvmCaps)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// if app direct is not supported, default to disabled
	enum app_direct_mode mode = nvmCaps.platform_capabilities.current_app_direct_mode;
	if (!nvmCaps.nvm_features.app_direct_mode &&
			mode == APP_DIRECT_MODE_ENABLED) // bios support but DIMM SKU does not support
	{
		mode = APP_DIRECT_MODE_DISABLED;
	}

	std::string returnStr;
	switch (mode)
	{
		case APP_DIRECT_MODE_DISABLED:
			returnStr = MEMORYMODE_APP_DIRECT_DISABLED_STR;
			break;
		case APP_DIRECT_MODE_ENABLED:
			returnStr = MEMORYMODE_APP_DIRECT_STR;
			break;
		case APP_DIRECT_MODE_UNKNOWN:
		default:
			returnStr = MEMORYMODE_UNKNOWN_STR;
			break;
	}
	return returnStr;
}

std::string wbem::server::SystemCapabilitiesFactory::getInterleaveSetFormatStr(
		const struct interleave_format &format, bool mirrorSupported)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::stringstream formatStr;
	// output format
	formatStr << wbem::mem_config::InterleaveSet::getInterleaveFormatString(&format);
	// input format
	formatStr << " (";
	formatStr << wbem::mem_config::InterleaveSet::getInterleaveFormatInputString(&format, mirrorSupported);
	formatStr << ")";
	return formatStr.str();
}

void wbem::server::SystemCapabilitiesFactory::addFormatStringIfNotInList(wbem::framework::STR_LIST &list,
		const struct interleave_format &format, bool mirrorSupported)
{
	std::string formatStr = getInterleaveSetFormatStr(format, mirrorSupported);
	if (std::find(list.begin(), list.end(), formatStr) == list.end())
	{
		list.push_back(formatStr);
	}
}

wbem::framework::STR_LIST wbem::server::SystemCapabilitiesFactory::getSupportedSettings(
		const struct nvm_capabilities &nvmCaps)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::STR_LIST supportedSettings;

	if (nvmCaps.nvm_features.app_direct_mode)
	{
		bool mirrorSupported = nvmCaps.platform_capabilities.memory_mirror_supported;
		for (NVM_UINT16 i = 0; i < nvmCaps.platform_capabilities.app_direct_mode.interleave_formats_count; i++)
		{
			const struct interleave_format &format =
					nvmCaps.platform_capabilities.app_direct_mode.interleave_formats[i];

			addFormatStringIfNotInList(supportedSettings, format, mirrorSupported);
		}
	}
	return supportedSettings;
}

wbem::framework::STR_LIST wbem::server::SystemCapabilitiesFactory::getRecommendedSettings(
		const struct nvm_capabilities &nvmCaps)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::STR_LIST recommendedSettings;

	if (nvmCaps.nvm_features.app_direct_mode)
	{
		bool mirrorSupported = nvmCaps.platform_capabilities.memory_mirror_supported;
		for (NVM_UINT16 i = 0; i < nvmCaps.platform_capabilities.app_direct_mode.interleave_formats_count; i++)
		{
			const struct interleave_format &format =
					nvmCaps.platform_capabilities.app_direct_mode.interleave_formats[i];
			if (format.recommended)
			{
				addFormatStringIfNotInList(recommendedSettings, format, mirrorSupported);
			}
		}
	}
	return recommendedSettings;
}

wbem::framework::UINT64 wbem::server::SystemCapabilitiesFactory::getAppDirectAlignment(
		const struct nvm_capabilities &nvmCaps)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	NVM_UINT64 alignment = 0;
	if (nvmCaps.nvm_features.app_direct_mode)
	{
		alignment = (NVM_UINT64)1 << nvmCaps.platform_capabilities.app_direct_mode.interleave_alignment_size; // 2^n
	}
	return alignment;
}

void wbem::server::SystemCapabilitiesFactory::addCapabilitySupportedAttribute(
		framework::Instance *pInstance,
		framework::attribute_names_t attributes,
		std::string key, bool value)
{
	if (containsAttribute(key, attributes))
	{
		pInstance->setAttribute(key, framework::Attribute((framework::UINT16)value, false));
	}
}
