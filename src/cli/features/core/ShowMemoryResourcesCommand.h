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
#ifndef CR_MGMT_SHOWMEMORYRESOURCESCOMMAND_H
#define CR_MGMT_SHOWMEMORYRESOURCESCOMMAND_H

#include <libinvm-cli/CliFrameworkTypes.h>
#include <libinvm-cli/ResultBase.h>
#include "framework/PropertyDefinitionList.h"
#include <cli/features/core/framework/CommandBase.h>

#include <core/system/SystemService.h>

namespace cli
{
namespace nvmcli
{

class NVM_API ShowMemoryResourcesCommand : framework::CommandBase
{
public:
	ShowMemoryResourcesCommand(
			core::system::SystemService &service = core::system::SystemService::getService());

	framework::ResultBase *execute(const framework::ParsedCommand &parsedCommand);

private:
	core::system::SystemService &m_service;

	framework::PropertyDefinitionList<core::system::SystemMemoryResources> m_props;

	core::system::SystemMemoryResources m_memoryResourcesInfo;

	void createResults();
	bool displayOptionsAreValid();
	bool isPropertyDisplayed(framework::IPropertyDefinition<core::system::SystemMemoryResources> &p);
	static std::string convertCapacity(NVM_UINT64 capacity);
};

}
}

#endif //CR_MGMT_SHOWMEMORYRESOURCESCOMMAND_H
