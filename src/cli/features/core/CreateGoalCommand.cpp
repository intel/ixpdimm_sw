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
#include "ShowGoalCommand.h"
#include "FieldSupportFeature.h"
#include "ShowCommandUtilities.h"

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
	parsePropertyPmType();
	parsePropertyReserved();
	parsePropertyConfig();
	parseOptionForce();
	parseOptionUnits();

	return m_pResult;
}

int CreateGoalCommand::Parser::getMemoryMode()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_memoryModeValue;
}

NVM_UINT64 CreateGoalCommand::Parser::getReserved()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_reservedValue;
}

std::string CreateGoalCommand::Parser::getConfig()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_configValue;
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
	m_reservedValue(0),
	m_pmType(PMTYPE_VALUE_APPDIRECT),
	m_configValue(""),
	m_memoryModeExists(false),
	m_pmTypeExists(false),
	m_reserveStorageExists(false),
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
	ShowGoalCommand showGoalCmd;
	return showGoalCmd.execute(showGoal);
}

framework::ResultBase* CreateGoalCommand::ShowGoalAdapter::showGoalForLayout(
		const core::memory_allocator::MemoryAllocationLayout& layout,
		const std::string& units) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	cli::framework::ResultBase *pDisplayGoal = NULL;
	try
	{
		ShowGoalCommand::ResultBuilder showLayoutBuilder;
		showLayoutBuilder.setOutputTypeTable();
		showLayoutBuilder.setDisplayOptions(getLayoutGoalDisplayOptions());
		showLayoutBuilder.setCapacityUnits(units);

		core::configuration::MemoryAllocationGoalService &goalService =
				core::configuration::MemoryAllocationGoalService::getService();
		core::configuration::MemoryAllocationGoalCollection layoutGoals =
				goalService.getGoalsFromMemoryAllocationLayout(layout);
		showLayoutBuilder.setGoals(layoutGoals);

		pDisplayGoal = showLayoutBuilder.buildResult();
	}
	catch (std::exception &)
	{
		delete pDisplayGoal;
		throw;
	}

	return pDisplayGoal;
}

framework::DisplayOptions CreateGoalCommand::ShowGoalAdapter::getLayoutGoalDisplayOptions() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	core::StringList displayProperties = getLayoutGoalDisplayProperties();
	std::stringstream displayStr;
	for (core::StringList::const_iterator prop = displayProperties.begin();
			prop != displayProperties.end(); prop++)
	{
		if (prop != displayProperties.begin())
		{
			displayStr << ",";
		}
		displayStr << *prop;
	}
	framework::StringMap optionMap;
	optionMap[framework::OPTION_DISPLAY.name] = displayStr.str();

	framework::DisplayOptions options(optionMap);
	return options;
}

core::StringList CreateGoalCommand::ShowGoalAdapter::getLayoutGoalDisplayProperties() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	core::StringList displayProperties;
	displayProperties.push_back(ShowGoalCommand::SOCKETID);
	displayProperties.push_back(ShowGoalCommand::DIMMID);
	displayProperties.push_back(ShowGoalCommand::MEMORYSIZE);
	displayProperties.push_back(ShowGoalCommand::APPDIRECT1SIZE);
	displayProperties.push_back(ShowGoalCommand::APPDIRECT2SIZE);

	return displayProperties;
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

	std::string promptStr = getPromptStringForLayout(layout, capacityUnits);

	return m_prompt.prompt(promptStr);
}

std::string CreateGoalCommand::UserPrompt::getPromptStringForLayout(
		const core::memory_allocator::MemoryAllocationLayout& layout,
		const std::string capacityUnits)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream promptStr;

	promptStr << CREATE_GOAL_CONFIRMATION_PREFIX << std::endl << std::endl;
	promptStr << getLayoutGoalForConfirmation(layout, capacityUnits) << std::endl << std::endl;
	promptStr << getLayoutWarningsForConfirmation(layout);
	promptStr << CREATE_GOAL_CONFIRMATION_SUFFIX;

	return promptStr.str();
}

std::string CreateGoalCommand::UserPrompt::getLayoutGoalForConfirmation(
		const core::memory_allocator::MemoryAllocationLayout& layout,
		const std::string capacityUnits)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string layoutGoalString;

	framework::ResultBase *pGoalResult = m_showGoalAdapter.showGoalForLayout(layout, capacityUnits);
	layoutGoalString = pGoalResult->output();
	delete pGoalResult;

	return layoutGoalString;
}

std::string CreateGoalCommand::UserPrompt::getLayoutWarningsForConfirmation(
		const core::memory_allocator::MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream layoutWarnings;
	int warningsAdded = 0;
	for (std::vector<enum core::memory_allocator::LayoutWarningCode>::const_iterator warningIter =
		layout.warnings.begin(); warningIter != layout.warnings.end(); warningIter++)
	{
		std::string warningStr = getStringForLayoutWarning(*warningIter);
		if (!warningStr.empty())
		{
			warningsAdded++;
			layoutWarnings << warningStr << std::endl;
		}
	}

	if (warningsAdded > 0)
	{
		layoutWarnings << std::endl;
	}

	return layoutWarnings.str();
}

std::string CreateGoalCommand::UserPrompt::getStringForLayoutWarning(
		enum core::memory_allocator::LayoutWarningCode warningCode)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string warningStr;

	switch (warningCode)
	{
	case core::memory_allocator::LAYOUT_WARNING_APP_DIRECT_NOT_SUPPORTED_BY_DRIVER:
		warningStr = CREATE_GOAL_APP_DIRECT_NOT_SUPPORTED_BY_DRIVER_WARNING;
		break;
	case core::memory_allocator::LAYOUT_WARNING_STORAGE_NOT_SUPPORTED_BY_DRIVER:
		warningStr = CREATE_GOAL_STORAGE_ONLY_NOT_SUPPORTED_BY_DRIVER_WARNING;
		break;
	case core::memory_allocator::LAYOUT_WARNING_NONOPTIMAL_POPULATION:
		warningStr = CREATE_GOAL_NON_OPTIMAL_DIMM_POPULATION_WARNING;
		break;
	case core::memory_allocator::LAYOUT_WARNING_REQUESTED_MEMORY_MODE_NOT_USABLE:
		warningStr = CREATE_GOAL_REQUESTED_MEMORY_MODE_NOT_USABLE_WARNING;
		break;
	case core::memory_allocator::LAYOUT_WARNING_GOAL_ADJUSTED_MORE_THAN_10PERCENT:
		warningStr = CREATE_GOAL_ADJUSTED_MORE_THAN_10PERCENT_WARNING;
		break;
	case core::memory_allocator::LAYOUT_WARNING_SKU_MAPPED_MEMORY_LIMITED:
		warningStr = CREATE_GOAL_SKU_MAPPED_MEMORY_LIMITED_WARNING;
		break;
	default:
		COMMON_LOG_ERROR_F("Unrecognized layout warning code: %d", warningCode);
		warningStr = "";
	}

	return warningStr;
}

CreateGoalCommand::UserPrompt::UserPrompt(const framework::YesNoPrompt &prompt,
		const ShowGoalAdapter &showGoalAdapter)
	: m_prompt(prompt), m_showGoalAdapter(showGoalAdapter)
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
		.valueText("0|%")
		.helpText(TR("Percentage of the total capacity to use in Memory Mode (0-100)."));

	result.addProperty("PersistentMemoryType")
		.isRequired(false)
		.isValueRequired(true)
		.valueText("AppDirect|AppDirectNotInterleaved")
		.helpText(TR("If MemoryMode is not 100%, the type of persistent memory to create."));

	result.addProperty(RESERVED_NAME)
		.isRequired(false)
		.isValueRequired(true)
		.valueText("0|%")
		.helpText(TR("Reserve a percentage (0-100) of the requested AEP DIMM capacity"
			" that will not be mapped into the system physical address space."));

	result.addProperty(CONFIG_NAME)
			.isRequired(false)
			.isValueRequired(true)
			.valueText("MM|AD|MM+AD")
			.helpText(TR("Create a memory allocation goal which utilizes all of the "
					"specified AEP DIMMs in one of pre-defined configurations."));
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

	std::string configString = m_parser.getConfig();
	if (configString.empty())
	{
		m_requestBuilder.setMemoryModePercentage(m_parser.getMemoryMode());
		m_requestBuilder.setReservedPercentage(m_parser.getReserved());
	}
	else
	{
		if (configString.compare(CREATE_GOAL_CONFIG_MEMORY_MODE) == 0)
		{
			m_requestBuilder.setMemoryModePercentage(100);
		}
		else if (configString.compare(CREATE_GOAL_CONFIG_MEMORY_APPDIRECT_MODE) == 0)
		{
			m_requestBuilder.setMemoryModePercentage(25);
		}
	}
	if (m_parser.isPmTypeAppDirect())
	{
		m_requestBuilder.setPersistentTypeAppDirectInterleaved();
	}
	else if (m_parser.isPmTypeAppDirectNotInterleaved())
	{
		m_requestBuilder.setPersistentTypeAppDirectNonInterleaved();
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
		std::string memoryMode =
			framework::Parser::getPropertyValue(m_parsedCommand, MEMORYMODE_NAME,
				&m_memoryModeExists);

		if (m_memoryModeExists)
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
		std::string pmType =
			framework::Parser::getPropertyValue(m_parsedCommand, PMTYPE_NAME,
				&m_pmTypeExists);

		if (m_pmTypeExists)
		{
			m_pmType = pmType;

			if (!isPmTypeAppDirect() &&
				!isPmTypeAppDirectNotInterleaved())
			{
				m_pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, PMTYPE_NAME, pmType);
			}
		}
	}
}

void CreateGoalCommand::Parser::parsePropertyReserved()
{
	if (!hasError())
	{
		std::string reserveStorageString =
				framework::Parser::getPropertyValue(m_parsedCommand, RESERVED_NAME,
						&m_reserveStorageExists);

		if (m_reserveStorageExists)
		{
			if (!stringToInt(reserveStorageString, &m_reservedValue)
					|| m_reservedValue > 100
					|| m_reservedValue < 0
					|| m_reservedValue + m_memoryModeValue > 100)
			{
				m_pResult = new framework::SyntaxErrorBadValueResult(
						framework::TOKENTYPE_PROPERTY, RESERVED_NAME, reserveStorageString);
				m_reservedValue = 0;
			}
		}
	}
}

void CreateGoalCommand::Parser::parsePropertyConfig()
{
	if (!hasError())
	{
		bool configExists = false;
		m_configValue =
				framework::Parser::getPropertyValue(m_parsedCommand, CONFIG_NAME,
						&configExists);
		std::vector<std::string> validConfigs;
		validConfigs.push_back(CREATE_GOAL_CONFIG_MEMORY_MODE);
		validConfigs.push_back(CREATE_GOAL_CONFIG_APPDIRECT_MODE);
		validConfigs.push_back(CREATE_GOAL_CONFIG_MEMORY_APPDIRECT_MODE);

		if (configExists)
		{
			if (std::find(validConfigs.begin(), validConfigs.end(), m_configValue) == validConfigs.end())
			{
				m_pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
						CONFIG_NAME, m_configValue);
			}
			else if (m_memoryModeExists || m_reserveStorageExists || m_pmTypeExists)
			{
				m_pResult = new framework::SyntaxErrorResult(TRS(CREATE_GOAL_CONFIG_CANNOT_BE_COMBINED_ERROR));
			}
		}

	}
}

}
}
