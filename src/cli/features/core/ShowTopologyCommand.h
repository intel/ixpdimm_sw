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
#ifndef CR_MGMT_SHOWTOPOLOGYCOMMAND_H
#define CR_MGMT_SHOWTOPOLOGYCOMMAND_H

#include "framework/PropertyDefinitionBase.h"
#include "framework/PropertyDefinitionList.h"
#include "framework/DisplayOptions.h"
#include <libinvm-cli/CliFrameworkTypes.h>
#include <libinvm-cli/ResultBase.h>
#include <libinvm-cli/PropertyListResult.h>
#include <lib/nvm_types.h>
#include <core/device/TopologyService.h>
#include <core/device/DeviceService.h>
#include <core/StringList.h>
#include <cli/features/core/framework/CommandBase.h>

namespace cli
{
namespace nvmcli
{

class NVM_API ShowTopologyCommand : framework::CommandBase
{
public:
	ShowTopologyCommand(
			core::device::TopologyService &service = core::device::TopologyService::getService());

	framework::ResultBase *execute(const framework::ParsedCommand &parsedCommand);

private:
	core::device::TopologyService &m_service;

	framework::ParsedCommand m_parsedCommand;

	bool dimmIdsAreValid();
        void filterTopologiesOnDimmIds();
        bool socketIdsAreValid();
        void filterTopologiesOnSocketIds();
        void createResults();
        bool displayOptionsAreValid();
        bool dimmIdIsFiltered(NVM_UINT8 device_id);

	NVM_UINT8 getMatchingDevice(core::device::Topology &topology);
	std::string getFirstBadDimmId(core::device::TopologyCollection &topology) const;
	std::string getFirstBadSocketId(core::device::TopologyCollection &topology) const;

        static std::string getDimmId(core::device::Topology &);
        static std::string getChannelId(core::device::Topology &);
        static std::string getChannelPosition(core::device::Topology &);
        static std::string getMemoryControllerId(core::device::Topology &);
        static std::string getNodeControllerId(core::device::Topology &);
	static std::string convertCapacity(NVM_UINT64 capacity);
	static std::string convertMemoryType(memory_type type);


	framework::ResultBase *m_pResult;
	framework::PropertyDefinitionList<core::device::Topology> m_props;
	core::StringList m_dimmIds;
	core::StringList m_socketIds;
	framework::DisplayOptions m_displayOptions;
	core::device::TopologyCollection m_topologies;
	bool isPropertyDisplayed(framework::IPropertyDefinition<core::device::Topology> &p);
};

}
}

#endif //CR_MGMT_SHOWDEVICECOMMAND_H
