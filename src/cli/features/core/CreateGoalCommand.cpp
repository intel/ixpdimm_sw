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

#include <libinvm-cli/CliFrameworkTypes.h>
#include "CreateGoalCommand.h"
#include "NamespaceFeature.h"
#include "FieldSupportFeature.h"

namespace cli
{
namespace nvmcli
{

framework::ResultBase *CreateGoalCommand::Parser::parse(
	const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	m_parsedCommand = parsedCommand;

	parseTargetDimm();
	parseTargetSocket();
	parsePropertyMemoryMode();
	parsePropertyReserveDimm();
	parsePropertyPmType();
	parseOptionForce();
	parseOptionUnits();

	return m_pResult;
}

int CreateGoalCommand::Parser::getMemoryMode()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_memoryModeValue;
}

bool CreateGoalCommand::Parser::isPmTypeAppDirect()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return framework::stringsIEqual(m_pmType, PMTYPE_VALUE_APPDIRECT);
}

bool CreateGoalCommand::Parser::isPmTypeAppDirectNotInterleaved()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return framework::stringsIEqual(m_pmType, PMTYPE_VALUE_APPDIRECTNOTINTERLEAVED);
}

bool CreateGoalCommand::Parser::isPmTypeAppDirectStorage()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return framework::stringsIEqual(m_pmType, PMTYPE_VALUE_STORAGE);
}

bool CreateGoalCommand::Parser::isReserveDimmNone()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return framework::stringsIEqual(m_reserveDimmType, PMTYPE_VALUE_NONE);
}

bool CreateGoalCommand::Parser::isReserveDimmAppDirect()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return framework::stringsIEqual(m_reserveDimmType, PMTYPE_VALUE_APPDIRECTNOTINTERLEAVED);
}

bool CreateGoalCommand::Parser::isReserveDimmStorage()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return framework::stringsIEqual(m_reserveDimmType, PMTYPE_VALUE_STORAGE);
}

bool CreateGoalCommand::Parser::isForce()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_isForce;
}

std::string CreateGoalCommand::Parser::getUnits()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_units;
}

std::vector<std::string> CreateGoalCommand::Parser::getDimms()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_dimms;
}

std::vector<NVM_UINT16> CreateGoalCommand::Parser::getSockets()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_sockets;
}

CreateGoalCommand::Parser::Parser() : m_pResult(NULL),
	m_memoryModeValue(0),
	m_pmType(PMTYPE_VALUE_APPDIRECT),
	m_reserveDimmType(PMTYPE_VALUE_NONE),
	m_isForce(false) { }

framework::ResultBase *CreateGoalCommand::ShowGoalAdapter::showCurrentGoal(
	const std::string &units) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ParsedCommand showGoal;
	if (!units.empty())
	{
		showGoal.options[framework::OPTION_UNITS.name] = units;
	}
	// TODO: Remove dependency on NamespaceFeature (Do as part of US16523)
	NamespaceFeature showGoalCmd;
	return showGoalCmd.run(NamespaceFeature::SHOW_CONFIG_GOAL, showGoal);
}

CreateGoalCommand::NoChangeResult::NoChangeResult()
	: SimpleResult(cli::nvmcli::CREATE_CONFIG_GOAL_MSG +
				   cli::framework::UNCHANGED_MSG) { }

CreateGoalCommand::CreateGoalCommand(
	core::memory_allocator::MemoryAllocator &allocator,
	core::memory_allocator::MemoryAllocationRequestBuilder &requestBuilder,
	UserPrompt &prompt,
	const ShowGoalAdapter &showGoalAdapter)
	: m_allocator(allocator), m_requestBuilder(requestBuilder), m_prompt(prompt),
	m_showGoalAdapter(showGoalAdapter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

bool CreateGoalCommand::UserPrompt::promptUserConfirmationForLayout(
	const core::memory_allocator::MemoryAllocationLayout &layout,
	const std::string capacityUnits)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// TODO: Remove dependency on NamespaceFeature (Do as part of US16523)
	std::string promptStr = NamespaceFeature::getPromptStringForLayout(layout, capacityUnits);

	return m_prompt.prompt(promptStr);
}

CreateGoalCommand::UserPrompt::UserPrompt(const framework::YesNoPrompt &prompt)
	: m_prompt(prompt)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

framework::CommandSpec CreateGoalCommand::getCommandSpec(int id)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::CommandSpec result(id, "Create Memory Allocation Goal", framework::VERB_CREATE);
	result.help = TR(
		"Create a memory allocation goal on one or more "
		NVM_DIMM_NAME
		"s. This operation stores the specified goal on the "
		NVM_DIMM_NAME
		"(s) for the BIOS to read on the next reboot in order to map the "
		NVM_DIMM_NAME
		" capacity into the system address space.");
	result.addOption(framework::OPTION_FORCE)
		.helpText(TR("Reconfiguring "
			NVM_DIMM_NAME
			"s is a destructive operation which requires confirmation from the user. "
			"This option suppresses the confirmation."));
	result.addOption(framework::OPTION_UNITS)
		.abbreviation("-u")
		.isValueRequired(true)
		.valueText("B|MB|MiB|GB|GiB|TB|TiB");
	result.addTarget(TARGET_DIMM).isValueRequired(true);
	result.addTarget(TARGET_GOAL_R).isValueAccepted(false);
	result.addTarget(TARGET_SOCKET)
		.isValueRequired(true)
		.helpText(TR("Create a memory allocation goal on the "
			NVM_DIMM_NAME
			"s on specific sockets by supplying the socket target and one or more comma-separated "
			"socket identifiers. The default is to configure all manageable "
			NVM_DIMM_NAME
			"s on all sockets."));

	result.addProperty(MEMORYMODE_NAME)
		.isRequired(false)
		.isValueRequired(true)
		.helpText(TR("Percentage of the total capacity to use in Memory Mode (0-100)."));

	result.addProperty("PersistentMemoryType")
		.isRequired(false)
		.isValueRequired(true)
		.valueText("AppDirect|AppDirectNotInterleaved|Storage")
		.helpText(TR("If MemoryMode is not 100%, the type of persistent memory to create."));

	result.addProperty("ReserveDimm")
		.isRequired(false)
		.isValueRequired(true)
		.valueText("None|AppDirectNotInterleaved|Storage")
		.helpText(TR("Reserve one "
			NVM_DIMM_NAME
			" across the specified target "
			"for a different purpose"));
	return result;
}

framework::ResultBase *CreateGoalCommand::execute(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	m_pResult = m_parser.parse(parsedCommand);

	if (m_pResult == NULL)
	{
		try
		{
			setupRequestBuilder();
			const core::memory_allocator::MemoryAllocationRequest &request = m_requestBuilder.build();
			core::memory_allocator::MemoryAllocationLayout layout = m_allocator.layout(request);

			if (userReallyLikesThisLayout(layout, m_parser.getUnits()))
			{
				m_allocator.allocate(layout);
				m_pResult = m_showGoalAdapter.showCurrentGoal(m_parser.getUnits());
			}
			else
			{
				m_pResult = new NoChangeResult();
			}
		}
		catch (wbem::framework::Exception &e)
		{
			m_pResult = NvmExceptionToResult(e);
		}
		catch (std::exception &e)
		{
			m_pResult = CoreExceptionToResult(e);
		}
	}

	return m_pResult;
}

bool CreateGoalCommand::userReallyLikesThisLayout(
	const core::memory_allocator::MemoryAllocationLayout &layout,
	const std::string capacityUnits)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_parser.isForce() || m_prompt.promptUserConfirmationForLayout(layout, capacityUnits);
}

void CreateGoalCommand::setupRequestBuilder()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	m_requestBuilder.setMemoryModePercentage(m_parser.getMemoryMode());

	if (m_parser.isReserveDimmStorage())
	{
		m_requestBuilder.reserveDimmForStorage();
	}
	else if (m_parser.isReserveDimmNone())
	{
		m_requestBuilder.noReservedDimm();
	}
	else if (m_parser.isReserveDimmAppDirect())
	{
		m_requestBuilder.reserveDimmForNonInterleavedAppDirect();
	}

	if (m_parser.isPmTypeAppDirect())
	{
		m_requestBuilder.setPersistentTypeAppDirectInterleaved();

	}
	else if (m_parser.isPmTypeAppDirectNotInterleaved())
	{
		m_requestBuilder.setPersistentTypeAppDirectNonInterleaved();
	}
	else if (m_parser.isPmTypeAppDirectStorage())
	{
		m_requestBuilder.setPersistentTypeStorage();
	}

	if (m_parser.getDimms().size() > 0u)
	{
		m_requestBuilder.addDimmIds(m_parser.getDimms());
	}
	if (m_parser.getSockets().size() > 0u)
	{
		m_requestBuilder.addSocketIds(m_parser.getSockets());
	}
}

bool CreateGoalCommand::Parser::hasError()
{
	return m_pResult != NULL;
}

void CreateGoalCommand::Parser::parseOptionForce()
{
	if (!hasError())
	{
		framework::Parser::getOptionValue(m_parsedCommand, framework::OPTION_FORCE.name,
			&m_isForce);
	}
}

void CreateGoalCommand::Parser::parseOptionUnits()
{
	if (!hasError())
	{
		std::vector<std::string> validUnits;
		validUnits.push_back(PREFERENCE_SIZE_B);
		validUnits.push_back(PREFERENCE_SIZE_MIB);
		validUnits.push_back(PREFERENCE_SIZE_MB);
		validUnits.push_back(PREFERENCE_SIZE_GIB);
		validUnits.push_back(PREFERENCE_SIZE_GB);
		validUnits.push_back(PREFERENCE_SIZE_TIB);
		validUnits.push_back(PREFERENCE_SIZE_TB);

		bool hasUnits = false;
		m_units = framework::Parser::getOptionValue(m_parsedCommand,
			framework::OPTION_UNITS.name, &hasUnits);
		if (hasUnits)
		{
			if (std::find(validUnits.begin(), validUnits.end(), m_units) == validUnits.end())
			{
				m_pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_OPTION,
					framework::OPTION_UNITS.name, m_units);
			}
		}
	}
}

void CreateGoalCommand::Parser::parseTargetDimm()
{
	if (!hasError())
	{
		m_dimms = framework::Parser::getTargetValues(m_parsedCommand, TARGET_DIMM.name);
	}
}

void CreateGoalCommand::Parser::parseTargetSocket()
{
	if (!hasError())
	{
		const std::vector<std::string> &socketStrings = framework::Parser::getTargetValues(
			m_parsedCommand, TARGET_SOCKET.name);

		for (size_t i = 0; i < socketStrings.size() && !m_pResult; i++)
		{
			int socketId;
			if (stringToInt(socketStrings[i], &socketId))
			{
				m_sockets.push_back(socketId);
			}
			else
			{
				m_pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_TARGET, TARGET_SOCKET.name, socketStrings[i]);
			}
		}
	}
}

void CreateGoalCommand::Parser::parsePropertyMemoryMode()
{
	if (!hasError())
	{
		bool memoryModeExists = false;
		std::string memoryMode =
			framework::Parser::getPropertyValue(m_parsedCommand, MEMORYMODE_NAME,
				&memoryModeExists);

		if (memoryModeExists)
		{
			if (!stringToInt(memoryMode, &m_memoryModeValue)
				|| m_memoryModeValue > 100
				|| m_memoryModeValue < 0)
			{
				m_pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, MEMORYMODE_NAME, memoryMode);
				m_memoryModeValue = 0;
			}
		}
	}
}

void CreateGoalCommand::Parser::parsePropertyPmType()
{
	if (!hasError())
	{
		bool pmTypeExists = false;
		std::string pmType =
			framework::Parser::getPropertyValue(m_parsedCommand, PMTYPE_NAME,
				&pmTypeExists);

		if (pmTypeExists)
		{
			m_pmType = pmType;

			if (!isPmTypeAppDirect() &&
				!isPmTypeAppDirectStorage() &&
				!isPmTypeAppDirectNotInterleaved())
			{
				m_pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, PMTYPE_NAME, pmType);
			}
		}
	}
}

void CreateGoalCommand::Parser::parsePropertyReserveDimm()
{
	if (!hasError())
	{
		bool reserveDimmExists;
		std::string reserveDimm =
			framework::Parser::getPropertyValue(m_parsedCommand, RESERVEDIMM_NAME,
				&reserveDimmExists);

		if (reserveDimmExists)
		{
			m_reserveDimmType = reserveDimm;
			if (!isReserveDimmAppDirect() &&
				!isReserveDimmNone() &&
				!isReserveDimmStorage())
			{
				m_pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, RESERVEDIMM_NAME, reserveDimm);
			}
		}
	}
}

}
}
