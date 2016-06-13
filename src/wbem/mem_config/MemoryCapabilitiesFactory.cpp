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
 * This file contains the provider for the MemoryCapabilities instances.
 */


#include <algorithm>
#include <nvm_management.h>
#include <LogEnterExit.h>

#include <libinvm-cim/Types.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include "MemoryCapabilitiesFactory.h"
#include <server/BaseServerFactory.h>
#include <server/SystemCapabilitiesFactory.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>
#include "InterleaveSet.h"

wbem::mem_config::MemoryCapabilitiesFactory::MemoryCapabilitiesFactory()
{
}

wbem::mem_config::MemoryCapabilitiesFactory::~MemoryCapabilitiesFactory()
{
}

void wbem::mem_config::MemoryCapabilitiesFactory::populateAttributeList(
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(MEMORYMODES_KEY);
	attributes.push_back(REPLICATIONSUPPORT_KEY);
	attributes.push_back(RELIABILITYSUPPORT_KEY);
	attributes.push_back(ALIGNMENT_KEY);
	attributes.push_back(CHANNELINTERLEAVESUPPORT_KEY);
	attributes.push_back(CHANNELINTERLEAVEWAYSUPPORT_KEY);
	attributes.push_back(MEMORYCONTROLLERINTERLEAVESUPPORT_KEY);
	attributes.push_back(MEMORYMODEALIGNMENT_KEY);
	attributes.push_back(APPDIRECTALIGNMENT_KEY);
	attributes.push_back(PLATFORMCONFIGSUPPORTED_KEY);
	attributes.push_back(PLATFORMRUNTIMESUPPORTED_KEY);
	attributes.push_back(CURRENTVOLATILEMODE_KEY);
	attributes.push_back(CURRENTAPPDIRECTMODE_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::mem_config::MemoryCapabilitiesFactory::getInstance(
		framework::ObjectPath &path,
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Verify ObjectPath ...
	std::string hostName = wbem::server::getHostName();

	// Verify InstanceID
	framework::Attribute attribute = path.getKeyValue(INSTANCEID_KEY);
	if (attribute.stringValue() != (hostName + MEMORYCAPABILITIES_INSTANCEID))
	{
		throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
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
			// ElementName - hostname + " NVM Capabilities"
			if (containsAttribute(ELEMENTNAME_KEY, attributes))
			{
				framework::Attribute a((hostName + MEMORYCAPABILITIES_ELEMENTNAME), false);
				pInstance->setAttribute(ELEMENTNAME_KEY, a, attributes);
			}

			// MemoryModes
			if (containsAttribute(MEMORYMODES_KEY, attributes))
			{
				framework::Attribute a(getMemoryModes(nvmCaps), false);
				pInstance->setAttribute(MEMORYMODES_KEY, a, attributes);
			}

			// ReplicationSupport
			if (containsAttribute(REPLICATIONSUPPORT_KEY, attributes))
			{
				framework::Attribute a(getReplicationSupport(nvmCaps), false);
				pInstance->setAttribute(REPLICATIONSUPPORT_KEY, a, attributes);
			}

			// ReliabilitySupport
			if (containsAttribute(RELIABILITYSUPPORT_KEY, attributes))
			{
				framework::Attribute a(getReliabilitySupport(nvmCaps), false);
				pInstance->setAttribute(RELIABILITYSUPPORT_KEY, a, attributes);
			}

			// Alignment - BIOS reported alignment requirement
			if (containsAttribute(ALIGNMENT_KEY, attributes))
			{
				framework::Attribute a((NVM_UINT16)(nvmCaps.platform_capabilities.app_direct_mode.interleave_alignment_size), false);
				pInstance->setAttribute(ALIGNMENT_KEY, a, attributes);
			}

			// Interleave sizes, Channel way values and Memory Controller Interleave sizes
			framework::UINT16_LIST channelSizeList;	// BIOS reported memory channel interleave sizes
			framework::UINT16_LIST waysList; // BIOS supported channel interleave ways
			framework::UINT16_LIST imcSizeList; // BIOS reported iMC interleave sizes
			if (nvmCaps.nvm_features.app_direct_mode)
			{
				for (int i = 0; i < nvmCaps.platform_capabilities.app_direct_mode.interleave_formats_count; i++)
				{
					NVM_UINT16 channelSize = (NVM_UINT16) wbem::mem_config::InterleaveSet::getExponentFromInterleaveSize(
							nvmCaps.platform_capabilities.app_direct_mode.interleave_formats[i].channel);
					// only add unique values
					if (std::find(channelSizeList.begin(), channelSizeList.end(), channelSize) == channelSizeList.end())
					{
						channelSizeList.push_back(channelSize);
					}

					NVM_UINT16 way = nvmCaps.platform_capabilities.app_direct_mode.interleave_formats[i].ways;
					// only add unique values
					if (std::find(imcSizeList.begin(), imcSizeList.end(), way) == imcSizeList.end())
					{
						waysList.push_back(way);
					}

					NVM_UINT16 imcSize = (NVM_UINT16) wbem::mem_config::InterleaveSet::getExponentFromInterleaveSize(
							nvmCaps.platform_capabilities.app_direct_mode.interleave_formats[i].imc);
					// only add unique values
					if (std::find(imcSizeList.begin(), imcSizeList.end(), imcSize) == imcSizeList.end())
					{
						imcSizeList.push_back(imcSize);
					}

				}
			}

			if (containsAttribute(CHANNELINTERLEAVESUPPORT_KEY, attributes))
			{
				framework::Attribute a(channelSizeList, false);
				pInstance->setAttribute(CHANNELINTERLEAVESUPPORT_KEY, a, attributes);
			}

			if (containsAttribute(CHANNELINTERLEAVEWAYSUPPORT_KEY, attributes))
			{
				framework::Attribute b(waysList, false);
				pInstance->setAttribute(CHANNELINTERLEAVEWAYSUPPORT_KEY, b, attributes);
			}

			if (containsAttribute(MEMORYCONTROLLERINTERLEAVESUPPORT_KEY, attributes))
			{
				framework::Attribute c(imcSizeList, false);
				pInstance->setAttribute(MEMORYCONTROLLERINTERLEAVESUPPORT_KEY, c, attributes);
			}


			// TwoLevelMemoryAlignment - BIOS reported alignment requirement in bytes
			if (containsAttribute(MEMORYMODEALIGNMENT_KEY, attributes))
			{
				framework::Attribute a((NVM_UINT16)nvmCaps.platform_capabilities.memory_mode.interleave_alignment_size, false);
				pInstance->setAttribute(MEMORYMODEALIGNMENT_KEY, a, attributes);
			}

			// AppDirectAlignment - BIOS reported alignment requirement in bytes
			if (containsAttribute(APPDIRECTALIGNMENT_KEY, attributes))
			{
				framework::Attribute a((NVM_UINT16)(nvmCaps.platform_capabilities.app_direct_mode.interleave_alignment_size), false);
				pInstance->setAttribute(APPDIRECTALIGNMENT_KEY, a, attributes);
			}

			// PlatformConfigSupported - boolean
			// If true, the platform level configuration of NVM-DIMMs can be modified.
			// If false, any supported changes must be done through BIOS setup
			if (containsAttribute(PLATFORMCONFIGSUPPORTED_KEY, attributes))
			{
				framework::Attribute a((framework::BOOLEAN)nvmCaps.nvm_features.modify_device_capacity, false);
				pInstance->setAttribute(PLATFORMCONFIGSUPPORTED_KEY, a, attributes);
			}

			// PlatformRuntimeSupported - boolean
			// If true, the platform supports a runtime interface for validating management configuration of NVM-DIMMs
			if (containsAttribute(PLATFORMRUNTIMESUPPORTED_KEY, attributes))
			{
				framework::Attribute a((framework::BOOLEAN)nvmCaps.platform_capabilities.bios_runtime_support, false);
				pInstance->setAttribute(PLATFORMRUNTIMESUPPORTED_KEY, a, attributes);
			}

			// CurrentVolatileMode - volatile memory mode currently selected by the BIOS
			if (containsAttribute(CURRENTVOLATILEMODE_KEY, attributes))
			{
				framework::Attribute a(translateVolatileMode(nvmCaps), false);
				pInstance->setAttribute(CURRENTVOLATILEMODE_KEY, a, attributes);
			}

			// CurrentAppDirectMode - App Direct mode currently selected by the BIOS
			if (containsAttribute(CURRENTAPPDIRECTMODE_KEY, attributes))
			{
				framework::Attribute a(translateAppDirectMode(nvmCaps), false);
				pInstance->setAttribute(CURRENTAPPDIRECTMODE_KEY, a, attributes);
			}

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
wbem::framework::instance_names_t* wbem::mem_config::MemoryCapabilitiesFactory::getInstanceNames()
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// get the host server name
	std::string hostName = wbem::server::getHostName();
	framework::instance_names_t *pNames = new framework::instance_names_t();
	framework::attributes_t keys;

	std::string instance_id = hostName + MEMORYCAPABILITIES_INSTANCEID;
	keys[INSTANCEID_KEY] = framework::Attribute(instance_id, true);

	// generate the ObjectPath for the instance (only one per server)
	framework::ObjectPath path(hostName, NVM_NAMESPACE,
			MEMORYCAPABILITIES_CREATIONCLASSNAME, keys);
	pNames->push_back(path);

	return pNames;
}

wbem::framework::UINT16_LIST wbem::mem_config::MemoryCapabilitiesFactory::getMemoryModes(
		const struct nvm_capabilities &nvmCaps)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::UINT16_LIST modes;
	if (nvmCaps.platform_capabilities.one_lm_mode.supported)
	{
		modes.push_back(MEMORYMODE_1LM);
	}
	if (nvmCaps.nvm_features.memory_mode)
	{
		modes.push_back(MEMORYMODE_MEMORY);
	}
	if (nvmCaps.nvm_features.storage_mode)
	{
		modes.push_back(MEMORYMODE_STORAGE);
	}
	if (nvmCaps.nvm_features.app_direct_mode)
	{
		modes.push_back(MEMORYMODE_APP_DIRECT);
	}

	return modes;
}

wbem::framework::UINT16_LIST wbem::mem_config::MemoryCapabilitiesFactory::getReplicationSupport(
		const struct nvm_capabilities &nvmCaps)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::UINT16_LIST replicationList;
	if (nvmCaps.nvm_features.app_direct_mode &&
			nvmCaps.platform_capabilities.memory_mirror_supported)
	{
		replicationList.push_back(REPLICATION_LOCAL);
	}
	else
	{
		replicationList.push_back(REPLICATION_NONE);
	}
	return replicationList;
}

wbem::framework::UINT16_LIST wbem::mem_config::MemoryCapabilitiesFactory::getReliabilitySupport(
		const struct nvm_capabilities &nvmCaps)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::UINT16_LIST reliabilityList;
	if (nvmCaps.nvm_features.app_direct_mode)
	{
		if (nvmCaps.platform_capabilities.memory_spare_supported)
		{
			reliabilityList.push_back(RELIABILITY_DIMMSPARING);
		}
		if (nvmCaps.platform_capabilities.memory_migration_supported)
		{
			reliabilityList.push_back(RELIABILITY_MEMORYMIGRATION);
		}
	}
	return reliabilityList;
}

/*
 * Convert library enumeration for current volatile mode to wbem value
 */
wbem::framework::UINT16 wbem::mem_config::MemoryCapabilitiesFactory::translateVolatileMode(const struct nvm_capabilities &nvmCaps)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::UINT16 wbemVolatileMode;

	// if memory mode is not supported, default to 1lm
	enum volatile_mode mode = nvmCaps.platform_capabilities.current_volatile_mode;
	if (!nvmCaps.nvm_features.memory_mode &&
			mode == VOLATILE_MODE_MEMORY) // bios support but DIMM SKU does not support
	{
		mode = VOLATILE_MODE_1LM;
	}

	switch (mode)
	{
		case VOLATILE_MODE_1LM:
			wbemVolatileMode = MEMORYMODE_1LM;
			break;
		case VOLATILE_MODE_MEMORY:
			wbemVolatileMode = MEMORYMODE_MEMORY;
			break;
		case VOLATILE_MODE_AUTO:
			wbemVolatileMode = MEMORYMODE_AUTO;
			break;
		default:
			wbemVolatileMode = MEMORYMODE_UNKNOWN;
			break;
	}
	return wbemVolatileMode;
}

/*
 * Convert library enumeration for current App Direct mode to wbem value
 */
wbem::framework::UINT16 wbem::mem_config::MemoryCapabilitiesFactory::translateAppDirectMode(
		const struct nvm_capabilities &nvmCaps)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::UINT16 wbemAppDirectMode;

	// if app direct is not supported, default to disabled
	enum app_direct_mode mode = nvmCaps.platform_capabilities.current_app_direct_mode;
	if (!nvmCaps.nvm_features.app_direct_mode &&
			mode == APP_DIRECT_MODE_ENABLED) // bios support but DIMM SKU does not support
	{
		mode = APP_DIRECT_MODE_DISABLED;
	}

	switch (mode)
	{
		case APP_DIRECT_MODE_DISABLED:
			wbemAppDirectMode = MEMORYMODE_APP_DIRECT_DISABLED;
			break;
		case APP_DIRECT_MODE_ENABLED:
			wbemAppDirectMode = MEMORYMODE_APP_DIRECT;
			break;
		default:
			wbemAppDirectMode = MEMORYMODE_UNKNOWN;
			break;
	}
	return wbemAppDirectMode;
}

/*
 * Static method to get the recommended IMC/channel interleave sizes for App Direct memory.
 */
bool wbem::mem_config::MemoryCapabilitiesFactory::getRecommendedInterleaveSizes(
		interleave_size &imcSize,
		interleave_size &channelSize)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool result = false;

	try
	{
		struct nvm_capabilities nvmcaps;
		int rc = nvm_get_nvm_capabilities(&nvmcaps);
		if (rc == NVM_SUCCESS)
		{
			// Ignore all Memory Mode formats - we don't let users select volatile interleave
			// Add all App Direct formats but only if supported.
			if (nvmcaps.platform_capabilities.app_direct_mode.interleave_formats_count > 0)
			{
				for (NVM_UINT8 i = 0; i < nvmcaps.platform_capabilities.app_direct_mode.interleave_formats_count; i++)
				{
					if (nvmcaps.platform_capabilities.app_direct_mode.interleave_formats[i].recommended)
					{
						// take the IMC and channel sizes
						imcSize = nvmcaps.platform_capabilities.app_direct_mode.interleave_formats[i].imc;
						channelSize = nvmcaps.platform_capabilities.app_direct_mode.interleave_formats[i].channel;
						result = true;
						break;
					}
				}
			}

		}
	}
	catch (framework::Exception &e)
	{
		COMMON_LOG_ERROR_F("Exception caught trying to get recommended interleave sizes: %s",
				e.what());
	}

	return result;
}
