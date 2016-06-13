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
 * This file implements a factory for instances of MemoryTopologyView, an internal-only
 * WBEM view class that represents basic topology information about all memory devices.
 * This includes NVM-DIMMs and other devices, such as DRAM.
 */

#include "MemoryTopologyViewFactory.h"
#include <LogEnterExit.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <libinvm-cim/ExceptionBadAttribute.h>
#include <server/BaseServerFactory.h>
#include "NVDIMMFactory.h"
#include <uid/uid.h>
#include <sstream>
#include <libinvm-cim/Types.h>
#include <exception/NvmExceptionLibError.h>
#include <lib_interface/NvmApi.h>

wbem::physical_asset::MemoryTopologyViewFactory::MemoryTopologyViewFactory()
		throw (framework::Exception)
{
}

wbem::physical_asset::MemoryTopologyViewFactory::~MemoryTopologyViewFactory()
{
}

void wbem::physical_asset::MemoryTopologyViewFactory::populateAttributeList(framework::attribute_names_t &attributes)
	throw (framework::Exception)
{
	// Key attributes
	attributes.push_back(PHYSICALID_KEY);

	// Non-key attributes
	attributes.push_back(DIMMID_KEY);
	attributes.push_back(MEMORYTYPE_KEY);
	attributes.push_back(CAPACITY_KEY);
	attributes.push_back(SOCKETID_KEY);
	attributes.push_back(MEMCONTROLLERID_KEY);
	attributes.push_back(CHANNEL_KEY);
	attributes.push_back(CHANNELPOS_KEY);
	attributes.push_back(NODECONTROLLERID_KEY);
	attributes.push_back(DEVICELOCATOR_KEY);
	attributes.push_back(BANKLABEL_KEY);
}

wbem::framework::Instance* wbem::physical_asset::MemoryTopologyViewFactory::getInstance(
		framework::ObjectPath& path, framework::attribute_names_t& attributes)
				throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	checkAttributes(attributes);

	framework::Instance *pInstance = new framework::Instance(path);
	if (!pInstance)
	{
		throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__,
				"couldn't allocate instance of MemoryTopologyView");
	}

	try
	{
		framework::Attribute physicalIdAttr = path.getKeyValue(PHYSICALID_KEY);
		NVM_UINT16 physicalId = getPhysicalIdValue(physicalIdAttr);

		struct memory_topology memTopology;
		getMemoryTopologyForPhysicalId(physicalId, memTopology);

		populateInstanceFromMemoryTopology(*pInstance, attributes, memTopology);
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

NVM_UINT16 wbem::physical_asset::MemoryTopologyViewFactory::getPhysicalIdValue(framework::Attribute& physicalIdAttr)
		throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT16 physicalId = 0;

	switch (physicalIdAttr.getType())
	{
		case framework::UINT8_T:
		case framework::UINT16_T:
		case framework::UINT32_T:
		case framework::UINT64_T:
			physicalId = physicalIdAttr.uintValue();
			break;
		default:
			COMMON_LOG_ERROR_F("PhysicalID attribute had an invalid type: %u",
					physicalIdAttr.getType());
			throw framework::ExceptionBadAttribute(PHYSICALID_KEY.c_str());
	}

	return physicalId;
}

void wbem::physical_asset::MemoryTopologyViewFactory::populateInstanceFromMemoryTopology(framework::Instance& instance,
		framework::attribute_names_t &attributes, const struct memory_topology &memTopology)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	enum memory_type memoryType = memTopology.memory_type;

	// MemoryType - string
	if (containsAttribute(MEMORYTYPE_KEY, attributes))
	{
		framework::Attribute a(memoryTypeToString(memoryType), false);
		instance.setAttribute(MEMORYTYPE_KEY, a);
	}

	// Capacity - uint64
	if (containsAttribute(CAPACITY_KEY, attributes))
	{
		framework::Attribute a(memTopology.raw_capacity, false);
		instance.setAttribute(CAPACITY_KEY, a);
	}

	// DeviceLocator - string
	if (containsAttribute(DEVICELOCATOR_KEY, attributes))
	{
		framework::Attribute a(memTopology.device_locator, false);
		instance.setAttribute(DEVICELOCATOR_KEY, a);
	}

	// BankLabel - string
	if (containsAttribute(BANKLABEL_KEY, attributes))
	{
		framework::Attribute a(memTopology.bank_label, false);
		instance.setAttribute(BANKLABEL_KEY, a);
	}

	// NVM-DIMMs have some special properties that other types of DIMM/device don't have
	if (memoryType == MEMORY_TYPE_NVMDIMM)
	{
		populateNvmDimmInstanceAttributes(instance, attributes, memTopology);
	}
	else
	{
		populateDramDimmInstanceAttributes(instance, attributes, memTopology);
	}
}

void wbem::physical_asset::MemoryTopologyViewFactory::populateNvmDimmInstanceAttributes(
		framework::Instance& instance, framework::attribute_names_t& attributes,
		const struct memory_topology& memTopology)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// NVM-DIMM additional information comes from NFIT
	struct device_discovery device;
	try
	{
		getDeviceDiscoveryForPhysicalId(memTopology.physical_id, device);

		// DimmID - string
		if (containsAttribute(DIMMID_KEY, attributes))
		{
			NVM_UID uidStr;
			uid_copy(device.uid, uidStr);
			std::string dimmId = NVDIMMFactory::uidToDimmIdStr(std::string(uidStr));

			framework::Attribute a(dimmId, false);
			instance.setAttribute(DIMMID_KEY, a);
		}

		// SocketID - string
		if (containsAttribute(SOCKETID_KEY, attributes))
		{
			std::stringstream socketId;
			socketId << device.socket_id;

			framework::Attribute a(socketId.str(), false);
			instance.setAttribute(SOCKETID_KEY, a);
		}

		// MemControllerID - string
		if (containsAttribute(MEMCONTROLLERID_KEY, attributes))
		{
			std::stringstream mcId;
			mcId << device.memory_controller_id;

			framework::Attribute a(mcId.str(), false);
			instance.setAttribute(MEMCONTROLLERID_KEY, a);
		}

		// ChannelID - string
		if (containsAttribute(CHANNEL_KEY, attributes))
		{
			std::stringstream channelId;
			channelId << device.device_handle.parts.mem_channel_id;

			framework::Attribute a(channelId.str(), false);
			instance.setAttribute(CHANNEL_KEY, a);
		}

		// ChannelPos - string
		if (containsAttribute(CHANNELPOS_KEY, attributes))
		{
			std::stringstream channelPos;
			channelPos << device.device_handle.parts.mem_channel_dimm_num;

			framework::Attribute a(channelPos.str(), false);
			instance.setAttribute(CHANNELPOS_KEY, a);
		}

		// NodeControllerID - string
		if (containsAttribute(NODECONTROLLERID_KEY, attributes))
		{
			std::stringstream nodeControllerId;
			nodeControllerId << device.device_handle.parts.node_controller_id;

			framework::Attribute a(nodeControllerId.str(), false);
			instance.setAttribute(NODECONTROLLERID_KEY, a);
		}
	}
	// Failed to get details from driver, treat it like a DRAM DIMM
	catch (framework::Exception &)
	{
		populateDramDimmInstanceAttributes(instance, attributes, memTopology);
	}
}

void wbem::physical_asset::MemoryTopologyViewFactory::populateDramDimmInstanceAttributes(
		framework::Instance& instance, framework::attribute_names_t& attributes,
		const struct memory_topology &memTopology)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// No DimmID for DRAM Dimm
	if (containsAttribute(DIMMID_KEY, attributes))
	{
		std::stringstream dimmId(wbem::NA);
		framework::Attribute a(dimmId.str(), false);
		instance.setAttribute(DIMMID_KEY, a);
	}

	// Specifics about socket/memory controller/channel/etc. position are
	// not available for DRAM

	// SocketID - string
	if (containsAttribute(SOCKETID_KEY, attributes))
	{
		framework::Attribute a(wbem::NA, false);
		instance.setAttribute(SOCKETID_KEY, a);
	}

	// MemControllerID - string
	if (containsAttribute(MEMCONTROLLERID_KEY, attributes))
	{
		framework::Attribute a(wbem::NA, false);
		instance.setAttribute(MEMCONTROLLERID_KEY, a);
	}

	// ChannelID - string
	if (containsAttribute(CHANNEL_KEY, attributes))
	{
		framework::Attribute a(wbem::NA, false);
		instance.setAttribute(CHANNEL_KEY, a);
	}

	// ChannelPos - string
	if (containsAttribute(CHANNELPOS_KEY, attributes))
	{
		framework::Attribute a(wbem::NA, false);
		instance.setAttribute(CHANNELPOS_KEY, a);
	}

	// NodeControllerID - string
	if (containsAttribute(NODECONTROLLERID_KEY, attributes))
	{
		framework::Attribute a(wbem::NA, false);
		instance.setAttribute(NODECONTROLLERID_KEY, a);
	}
}

void wbem::physical_asset::MemoryTopologyViewFactory::getDeviceDiscoveryForPhysicalId(const NVM_UINT16 physicalId,
		struct device_discovery &device) throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();

	std::vector<struct device_discovery> devices;
	pApi->getDevices(devices);

	bool found = false;
	for (size_t i = 0; i < devices.size(); i++)
	{
		if (devices[i].physical_id == physicalId)
		{
			found = true;
			memmove(&device, &(devices[i]), sizeof (device));
			break;
		}
	}

	if (!found)
	{
		std::stringstream msg;
		msg << "Didn't find a device_discovery for physical ID " << physicalId;
		throw framework::Exception(msg.str());
	}
}

std::string wbem::physical_asset::MemoryTopologyViewFactory::memoryTypeToString(const enum memory_type memoryType)
{
	std::string str;
	switch(memoryType)
	{
	case MEMORY_TYPE_DDR4:
		str = MEMORYTOPOLOGYVIEW_MEMORYTYPE_DDR4DRAM;
		break;
	case MEMORY_TYPE_NVMDIMM:
		str = MEMORYTOPOLOGYVIEW_MEMORYTYPE_NVMDIMM;
		break;
	case MEMORY_TYPE_UNKNOWN:
	default:
		str = MEMORYTOPOLOGYVIEW_MEMORYTYPE_UNKNOWN;
		break;
	}

	return str;
}

void wbem::physical_asset::MemoryTopologyViewFactory::getMemoryTopologyForPhysicalId(const NVM_UINT16 physicalId,
		struct memory_topology &memTopology)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();

	std::vector<struct memory_topology> allMemTopology;
	pApi->getMemoryTopology(allMemTopology);

	bool found = false;
	for (size_t i = 0; i < allMemTopology.size(); i++)
	{
		if (allMemTopology[i].physical_id == physicalId)
		{
			found = true;
			memmove(&memTopology, &(allMemTopology[i]), sizeof (memTopology));
			break;
		}
	}

	if (!found)
	{
		throw framework::ExceptionBadAttribute(PHYSICALID_KEY.c_str());
	}
}

wbem::framework::instance_names_t* wbem::physical_asset::MemoryTopologyViewFactory::getInstanceNames()
		throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pInstanceNames = new framework::instance_names_t;
	if (!pInstanceNames)
	{
		throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__,
				"couldn't allocate MemoryTopologyView instance names");
	}

	try
	{
		populateInstanceNames(*pInstanceNames);
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		if (pInstanceNames != NULL)
		{
			delete pInstanceNames;
		}
		throw;
	}

	return pInstanceNames;
}

void wbem::physical_asset::MemoryTopologyViewFactory::populateInstanceNames(framework::instance_names_t& instanceNames)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();

	std::vector<struct memory_topology> memDevices;
	pApi->getMemoryTopology(memDevices);

	std::string hostName = pApi->getHostName();
	for (size_t i = 0; i < memDevices.size(); i++)
	{
		NVM_UINT16 physicalId = memDevices[i].physical_id;

		framework::attributes_t keys;
		keys[PHYSICALID_KEY] = framework::Attribute(physicalId, true);

		framework::ObjectPath instanceName(hostName, NVM_NAMESPACE,
				MEMORYTOPOLOGYVIEW_CREATIONCLASSNAME, keys);
		instanceNames.push_back(instanceName);
	}
}
