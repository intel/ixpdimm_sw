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

#include <LogEnterExit.h>
#include <cli/features/core/WbemToCli_utilities.h>
#include <libinvm-cli/CliFrameworkTypes.h>
#include <cli/features/core/CommandParts.h>
#include <persistence/config_settings.h>
#include <persistence/lib_persistence.h>
#include <cli/features/core/framework/CliHelper.h>

#include "ShowTopologyCommand.h"

namespace cli
{
namespace nvmcli
{
ShowTopologyCommand::ShowTopologyCommand(core::device::TopologyService &service)
	: m_service(service), m_pResult(NULL)
{
	m_props.addOther("MemoryType", &core::device::Topology::getMemoryType, &convertMemoryType).setIsDefault();
	m_props.addUint64("Capacity", &core::device::Topology::getRawCapacity, &convertCapacity).setIsDefault();
	m_props.addCustom("DimmId", getDimmId).setIsRequired();
	m_props.addUint16("PhysicalID", &core::device::Topology::getPhysicalID).setIsDefault();
	m_props.addStr("DeviceLocator", &core::device::Topology::getDeviceLocator).setIsDefault();
	m_props.addUint16("SocketID", &core::device::Topology::getSocketId);
	m_props.addCustom("MemControllerID", getMemoryControllerId);
	m_props.addCustom("ChannelID", getChannelId);
	m_props.addCustom("ChannelPos", getChannelPosition);
	m_props.addCustom("NodeControllerID", getNodeControllerId);
	m_props.addStr("BankLabel", &core::device::Topology::getBankLabel);
}


framework::ResultBase *ShowTopologyCommand::execute(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_parsedCommand = parsedCommand;
	m_dimmIds = framework::CliHelper::splitCommaSeperatedString(m_parsedCommand.targets[TARGET_DIMM.name]);
	m_socketIds = framework::CliHelper::splitCommaSeperatedString(m_parsedCommand.targets[TARGET_SOCKET.name]);
	m_displayOptions = framework::DisplayOptions(m_parsedCommand.options);

	if (displayOptionsAreValid())
	{
		try
		{
			m_topologies = m_service.getAllTopologies();
			if (dimmIdsAreValid())
			{
				filterTopologiesOnDimmIds();
				if (socketIdsAreValid())
				{
					filterTopologiesOnSocketIds();
					createResults();
				}
			}
		}
		catch (core::LibraryException &e)
		{
			int libRc = e.getErrorCode();
			if (libRc == NVM_ERR_NOMEMORY)
			{
				m_pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_OUTOFMEMORY, NOMEMORY_ERROR_STR);
			}
			else if (libRc == NVM_ERR_NOTSUPPORTED)
			{
				m_pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_NOTSUPPORTED, NOTSUPPORTED_ERROR_STR);
			}
			else
			{
				// return the library message
				m_pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN, e.what());
			}
		}
	}

	return m_pResult;
}

std::string ShowTopologyCommand::convertMemoryType(memory_type type)
{
	std::map<NVM_UINT64, std::string> map;
	map[MEMORY_TYPE_UNKNOWN] = TR("Unknown");
	map[MEMORY_TYPE_DDR4] = TR("DDR4");
	map[MEMORY_TYPE_NVMDIMM] = TR("NVM-DIMM");
	return map[type];
}

std::string ShowTopologyCommand::convertCapacity(NVM_UINT64 value)
{
        return convertCapacityFormat(value);
}

void ShowTopologyCommand::filterTopologiesOnDimmIds()
{
        if (m_dimmIds.size() > 0)
        {
                for (size_t i = m_topologies.size(); i > 0; i--)
                {
			core::device::Topology &topology = m_topologies[i - 1];

                        std::string deviceHandle = uint64ToString(topology.getDeviceHandle());

                        if (!m_dimmIds.contains(topology.getUid()) &&
				 !m_dimmIds.contains(deviceHandle))
                        {
                                m_topologies.removeAt(i - 1);
                        }
                }
        }
}

void ShowTopologyCommand::filterTopologiesOnSocketIds()
{
        if (m_socketIds.size() > 0)
        {
                for (size_t i = m_topologies.size(); i > 0; i--)
                {
			core::device::Topology &topology = m_topologies[i - 1];

                        std::string socket_id = uint64ToString(topology.getSocketId());

                        if (!m_socketIds.contains(socket_id))
                        {
                                m_topologies.removeAt(i - 1);
                        }
                }
        }
}

bool ShowTopologyCommand::dimmIdsAreValid()
{
        std::string badDimmId = getFirstBadDimmId(m_topologies);
        if (!badDimmId.empty())
        {
                m_pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
                        getInvalidDimmIdErrorString(badDimmId));
        }

        return m_pResult == NULL;
}
bool ShowTopologyCommand::socketIdsAreValid()
{
        std::string badSocketId = getFirstBadSocketId(m_topologies);
        if (!badSocketId.empty())
        {
                m_pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_TARGET,
                        TARGET_SOCKET.name, badSocketId);
        }

        return m_pResult == NULL;
}

std::string ShowTopologyCommand::getFirstBadSocketId(core::device::TopologyCollection &topologies) const
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

        std::string badsocketId = "";
        for (size_t i = 0; i < m_socketIds.size() && badsocketId.empty(); i++)
        {
                bool socketIdFound = false;
                for (size_t j = 0; j < topologies.size() && !socketIdFound; j++)
                {
                        if (m_socketIds[i] == uint64ToString(topologies[j].getSocketId()))
                        {
                                socketIdFound = true;
                        }
                }
                if (!socketIdFound)
                {
                        badsocketId = m_socketIds[i];
                }
        }
        return badsocketId;
}

std::string ShowTopologyCommand::getFirstBadDimmId(core::device::TopologyCollection &topologies) const
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

        std::string badDimmId = "";

        for (size_t i = 0; i < m_dimmIds.size() && badDimmId.empty(); i++)
        {
                bool dimmIdFound = false;
                for (size_t j = 0; j < topologies.size() && !dimmIdFound; j++)
                {
                        if (framework::stringsIEqual(m_dimmIds[i], topologies[j].getUid()) ||
                                m_dimmIds[i] == uint64ToString(topologies[j].getDeviceHandle()))
                        {
                                dimmIdFound = true;
                        }
                }
                if (!dimmIdFound)
                {
                        badDimmId = m_dimmIds[i];
                }
        }
        return badDimmId;
}

void ShowTopologyCommand::createResults()
{
        framework::ObjectListResult *pList = new framework::ObjectListResult();
        pList->setRoot("DimmTopology");
        m_pResult = pList;

        for (size_t i = 0; i < m_topologies.size(); i++)
        {
		framework::PropertyListResult value;
		for (size_t j = 0; j < m_props.size(); j++)
	        {
                        framework::IPropertyDefinition<core::device::Topology> &p = m_props[j];
			if (isPropertyDisplayed(p))
			{
				value.insert(p.getName(), p.getValue(m_topologies[i]));
			}
		}

                pList->insert("DimmTopology", value);
	}

        m_pResult->setOutputType(
                m_displayOptions.isDefault() ?
                framework::ResultBase::OUTPUT_TEXTTABLE :
                framework::ResultBase::OUTPUT_TEXT);
}

bool ShowTopologyCommand::isPropertyDisplayed(
        framework::IPropertyDefinition<core::device::Topology> &p)
{
        return p.isRequired() ||
                   m_displayOptions.isAll() ||
                   (p.isDefault() && m_displayOptions.isDefault()) ||
                   m_displayOptions.contains(p.getName());
}

bool ShowTopologyCommand::displayOptionsAreValid()
{
        std::string invalidDisplay;
        const std::vector<std::string> &display = m_displayOptions.getDisplay();
        for (size_t i = 0; i < display.size() && invalidDisplay.empty(); i++)
        {
                if (!m_props.contains(display[i]))
                {
                        invalidDisplay = display[i];
                }
        }

        if (!invalidDisplay.empty())
        {
                m_pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_OPTION,
                        framework::OPTION_DISPLAY.name, invalidDisplay);
        }
        return m_pResult == NULL;
}

std::string ShowTopologyCommand::getDimmId(core::device::Topology &topology)
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

        std::stringstream result;
        bool useHandle = true;
        char value[CONFIG_VALUE_LEN];
        if (get_config_value(SQL_KEY_CLI_DIMM_ID, value) == COMMON_SUCCESS)
        {
		// switch to uid
                if (s_strncmpi("UID", value, strlen("UID")) == 0)
                {
                        useHandle = false;
                }
        }

	if (topology.getMemoryType() == MEMORY_TYPE_NVMDIMM)
	{
		if (useHandle)
		{
			result << topology.getDeviceHandle();
	        }
		else
	        {
			result << topology.getUid();
	        }
	}
	else if (topology.getMemoryType() == MEMORY_TYPE_DDR4)
	{
		result << "N/A";
	}

        return result.str();
}

std::string ShowTopologyCommand::getChannelId(core::device::Topology &topology)
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::stringstream result;

	if (topology.getMemoryType() == MEMORY_TYPE_NVMDIMM)
	{
		result << topology.getChannelId();
	}
	else if (topology.getMemoryType() == MEMORY_TYPE_DDR4)
	{
		result << "N/A";
	}

	return result.str();
}
std::string ShowTopologyCommand::getChannelPosition(core::device::Topology &topology)
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::stringstream result;

	if (topology.getMemoryType() == MEMORY_TYPE_NVMDIMM)
        {
                result << topology.getChannelPosition();
        }
        else if (topology.getMemoryType() == MEMORY_TYPE_DDR4)
        {
                result << "N/A";
        }

        return result.str();
}
std::string ShowTopologyCommand::getMemoryControllerId(core::device::Topology &topology)
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::stringstream result;

	if (topology.getMemoryType() == MEMORY_TYPE_NVMDIMM)
        {
                result << topology.getMemoryControllerId();
        }
        else if (topology.getMemoryType() == MEMORY_TYPE_DDR4)
        {
                result << "N/A";
        }

        return result.str();
}
std::string ShowTopologyCommand::getNodeControllerId(core::device::Topology &topology)
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::stringstream result;

	if (topology.getMemoryType() == MEMORY_TYPE_NVMDIMM)
        {
                result << topology.getNodeControllerId();
        }
        else if (topology.getMemoryType() == MEMORY_TYPE_DDR4)
        {
                result << "N/A";
        }

        return result.str();
}
}
}
