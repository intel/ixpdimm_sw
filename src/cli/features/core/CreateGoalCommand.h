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
#ifndef CR_MGMT_CREATEGOALCOMMAND_H
#define CR_MGMT_CREATEGOALCOMMAND_H

#include <cli/features/core/framework/CommandBase.h>
#include <libinvm-cli/CommandSpec.h>
#include <libinvm-cli/ErrorResult.h>
#include <libinvm-cli/SyntaxErrorResult.h>
#include <libinvm-cli/CliFrameworkTypes.h>
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include <libinvm-cli/Parser.h>
#include <core/memory_allocator/MemoryAllocator.h>
#include <core/memory_allocator/MemoryAllocationRequestBuilder.h>
#include <cli/features/core/framework/YesNoPrompt.h>
#include "CommandParts.h"
#include "WbemToCli_utilities.h"

namespace cli
{
namespace nvmcli
{
static const std::string MEMORYMODE_NAME = "MemoryMode";
static const std::string PMTYPE_NAME = "PersistentMemoryType";
static const std::string RESERVEDIMM_NAME = "ReserveDimm";

static const std::string PMTYPE_VALUE_APPDIRECT = "AppDirect";
static const std::string PMTYPE_VALUE_APPDIRECTNOTINTERLEAVED = "AppDirectNotInterleaved";
static const std::string PMTYPE_VALUE_NONE = "None";
static const std::string PMTYPE_VALUE_STORAGE = "Storage";

class NVM_API CreateGoalCommand : public framework::CommandBase
{
public:
	class NVM_API Parser
	{
	public:

		Parser();
		framework::ResultBase *parse(const framework::ParsedCommand &parsedCommand);
		int getMemoryMode();
		bool isPmTypeAppDirect();
		bool isPmTypeAppDirectNotInterleaved();
		bool isPmTypeAppDirectStorage();
		bool isReserveDimmNone();
		bool isReserveDimmAppDirect();
		bool isReserveDimmStorage();
		bool isForce();

		std::string getUnits();
		std::vector<std::string> getDimms();
		std::vector<NVM_UINT16> getSockets();

	private:
		framework::ResultBase *m_pResult;
		int m_memoryModeValue;
		std::string m_pmType;
		std::string m_reserveDimmType;
		bool m_isForce;
		std::string m_units;
		std::vector<std::string> m_dimms;
		std::vector<COMMON_UINT16> m_sockets;
		framework::ParsedCommand m_parsedCommand;

		bool hasError();

		void parseOptionForce();

		void parseOptionUnits();

		void parseTargetDimm();

		void parseTargetSocket();

		void parsePropertyMemoryMode();

		void parsePropertyPmType();

		void parsePropertyReserveDimm();
	};

	class NVM_API ShowGoalAdapter
	{
	public:
		virtual framework::ResultBase *showCurrentGoal(const std::string &units) const;
	};

	class NVM_API NoChangeResult : public framework::SimpleResult
	{
	public:
		NoChangeResult();
	};

	class NVM_API UserPrompt
	{
	public:
		UserPrompt(const framework::YesNoPrompt &prompt);

		virtual ~UserPrompt() { }

		virtual bool promptUserConfirmationForLayout(
			const core::memory_allocator::MemoryAllocationLayout &layout,
			const std::string capacityUnits);

	private:
		const framework::YesNoPrompt &m_prompt;
	};

	CreateGoalCommand(
		core::memory_allocator::MemoryAllocator &allocator,
		core::memory_allocator::MemoryAllocationRequestBuilder &requestBuilder,
		UserPrompt &prompt,
		const ShowGoalAdapter &showGoalAdapter);

	static framework::CommandSpec getCommandSpec(int id);

	virtual ~CreateGoalCommand() { }

	virtual framework::ResultBase *execute(const framework::ParsedCommand &parsedCommand);

	void setupRequestBuilder();

private:
	Parser m_parser;
	core::memory_allocator::MemoryAllocator &m_allocator;
	core::memory_allocator::MemoryAllocationRequestBuilder &m_requestBuilder;
	UserPrompt &m_prompt;
	const ShowGoalAdapter &m_showGoalAdapter;
	bool userReallyLikesThisLayout(const core::memory_allocator::MemoryAllocationLayout &layout,
		const std::string capacityUnits);
};

}
}
#endif //CR_MGMT_CREATEGOALCOMMAND_H
