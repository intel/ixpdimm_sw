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

#include "ShowGoalCommand.h"
#include <LogEnterExit.h>
#include <libinvm-i18n/libIntel_i18n.h>
#include <libinvm-cli/CliFrameworkTypes.h>
#include "CommandParts.h"
#include "framework/CliHelper.h"
#include "ShowCommandPropertyUtilities.h"
#include "ShowCommandUtilities.h"
#include "WbemToCli_utilities.h"

namespace cli
{
namespace nvmcli
{
const std::string ShowGoalCommand::XML_ROOT = "ConfigGoal";

const std::string ShowGoalCommand::DIMMID = "DimmID";
const std::string ShowGoalCommand::SOCKETID = "SocketID";
const std::string ShowGoalCommand::MEMORYSIZE = "MemorySize";
const std::string ShowGoalCommand::APPDIRECT1SIZE = "AppDirect1Size";
const std::string ShowGoalCommand::APPDIRECT1INDEX = "AppDirect1Index";
const std::string ShowGoalCommand::APPDIRECT1SETTINGS = "AppDirect1Settings";
const std::string ShowGoalCommand::APPDIRECT2SIZE = "AppDirect2Size";
const std::string ShowGoalCommand::APPDIRECT2INDEX = "AppDirect2Index";
const std::string ShowGoalCommand::APPDIRECT2SETTINGS = "AppDirect2Settings";
const std::string ShowGoalCommand::STATUS = "Status";
const std::string ShowGoalCommand::ACTIONREQUIRED = "ActionRequired";
const std::string ShowGoalCommand::ACTIONREQUIREDEVENTS = "ActionRequiredEvents";

const std::string ShowGoalCommand::ResultBuilder::NOT_APPLICABLE = "N/A";

std::string ShowGoalCommand::ResultBuilder::m_capacityUnits = "";

ShowGoalCommand::ShowGoalCommand(core::device::DeviceService &deviceService,
		core::configuration::MemoryAllocationGoalService &goalService) :
				CommandBase(),
				m_deviceService(deviceService),
				m_goalService(goalService)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

ShowGoalCommand::~ShowGoalCommand()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

framework::CommandSpec ShowGoalCommand::getCommandSpec(const int commandId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::CommandSpec spec(commandId,
			TR("Show Memory Allocation Goal"),
			framework::VERB_SHOW,
			TR("Show the memory allocation goal on one or more " NVM_DIMM_NAME "s. "
					"Once the goal is successfully applied by the BIOS, it is no "
					"longer displayed."));

	spec.addOption(framework::OPTION_ALL);
	spec.addOption(framework::OPTION_DISPLAY);
	spec.addOption(framework::OPTION_UNITS)
		.abbreviation("-u")
		.isValueRequired(true)
		.valueText("B|MB|MiB|GB|GiB|TB|TiB");

	spec.addTarget(TARGET_DIMM)
			.isValueRequired(true);
	spec.addTarget(TARGET_SOCKET)
			.isValueRequired(true);
	spec.addTarget(TARGET_GOAL_R)
			.isValueAccepted(false);

	return spec;
}

framework::ResultBase* cli::nvmcli::ShowGoalCommand::execute(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	initWithParsedCommand(parsedCommand);
	setBuilderOutputOptions();

	if (unitsOptionIsValid())
	{
		try
		{
			populateAllDevices();
			if (dimmIdsAreValid() && socketIdsAreValid())
			{
				filterDevices();
				populateGoalsForFilteredDevices();

				m_resultBuilder.setGoals(m_goals);
				m_pResult = m_resultBuilder.buildResult();
			}
		}
		catch (core::LibraryException &e)
		{
			m_pResult = cli::nvmcli::CoreExceptionToResult(e);
		}
	}

	return m_pResult;
}

void ShowGoalCommand::initWithParsedCommand(const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_parsedCommand = parsedCommand;
	m_displayOptions = framework::DisplayOptions(m_parsedCommand.options);
	m_unitsOption = framework::UnitsOption(m_parsedCommand.options);
	m_dimmIds = framework::CliHelper::splitCommaSeperatedString(
			m_parsedCommand.targets[TARGET_DIMM.name]);
	m_socketIds = framework::CliHelper::splitCommaSeperatedString(
			m_parsedCommand.targets[TARGET_SOCKET.name]);
}

void ShowGoalCommand::setBuilderOutputOptions()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_resultBuilder.setDisplayOptions(m_displayOptions);

	if (m_displayOptions.isDefault())
	{
		m_resultBuilder.setOutputTypeTable();
	}
	else
	{
		m_resultBuilder.setOutputTypeText();
	}

	m_resultBuilder.setCapacityUnits(m_unitsOption.getCapacityUnits());
}

bool ShowGoalCommand::unitsOptionIsValid()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_pResult = ShowCommandUtilities::getInvalidUnitsOptionResult(m_unitsOption);

	return m_pResult == NULL;
}

void ShowGoalCommand::populateAllDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_devices = m_deviceService.getAllDevices();
}

bool ShowGoalCommand::dimmIdsAreValid()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_pResult = ShowCommandUtilities::getInvalidDimmIdResult(m_dimmIds, m_devices);

	return m_pResult == NULL;
}

bool ShowGoalCommand::socketIdsAreValid()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_pResult = ShowCommandUtilities::getInvalidSocketIdResult(m_socketIds, m_devices);

	return m_pResult == NULL;
}

void ShowGoalCommand::filterDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	ShowCommandUtilities::filterDevicesOnDimmIds(m_devices, m_dimmIds);
	ShowCommandUtilities::filterDevicesOnSocketIds(m_devices, m_socketIds);
}

void ShowGoalCommand::populateGoalsForFilteredDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (size_t i = 0; i < m_devices.size(); i++)
	{
		if (m_devices[i].isManageable())
		{
			populateGoalForDevice(m_devices[i]);
		}
	}
}

void ShowGoalCommand::populateGoalForDevice(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	try
	{
		core::Result<core::configuration::MemoryAllocationGoal> goal =
				m_goalService.getGoalForDevice(device.getUid());
		m_goals.push_back(goal.getValue());
	}
	catch (core::configuration::MemoryAllocationGoalService::NoGoalOnDevice &)
	{
		// Ignore
	}
}

ShowGoalCommand::ResultBuilder::ResultBuilder() :
		m_outputType(framework::ResultBase::OUTPUT_TEXTTABLE),
		m_pResult(NULL)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	setCapacityUnits("");

	m_props.addCustom(ShowGoalCommand::DIMMID, getDimmId).setIsRequired();

	m_props.addUint16(ShowGoalCommand::SOCKETID, &core::configuration::MemoryAllocationGoal::getDeviceSocketId).setIsDefault();
	m_props.addUint64(ShowGoalCommand::MEMORYSIZE, &core::configuration::MemoryAllocationGoal::getMemorySizeInBytes, convertBytesToUnits).setIsDefault();
	m_props.addUint64(ShowGoalCommand::APPDIRECT1SIZE, &core::configuration::MemoryAllocationGoal::getAppDirect1SizeInBytes, convertBytesToUnits).setIsDefault();
	m_props.addUint64(ShowGoalCommand::APPDIRECT2SIZE, &core::configuration::MemoryAllocationGoal::getAppDirect2SizeInBytes, convertBytesToUnits).setIsDefault();
	m_props.addBool(ShowGoalCommand::ACTIONREQUIRED, &core::configuration::MemoryAllocationGoal::isActionRequired).setIsDefault();

	m_props.addCustom(ShowGoalCommand::APPDIRECT1INDEX, getAppDirect1Index);
	m_props.addCustom(ShowGoalCommand::APPDIRECT1SETTINGS, getAppDirect1Settings);
	m_props.addCustom(ShowGoalCommand::APPDIRECT2INDEX, getAppDirect2Index);
	m_props.addCustom(ShowGoalCommand::APPDIRECT2SETTINGS, getAppDirect2Settings);
	m_props.addOther(ShowGoalCommand::STATUS, &core::configuration::MemoryAllocationGoal::getStatus, formatConfigGoalStatus);
	m_props.addCustom(ShowGoalCommand::ACTIONREQUIREDEVENTS, getActionRequiredEvents);
}

ShowGoalCommand::ResultBuilder::~ResultBuilder()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void ShowGoalCommand::ResultBuilder::setGoals(
		const core::configuration::MemoryAllocationGoalCollection &collection)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_goals = collection;
}

void ShowGoalCommand::ResultBuilder::setDisplayOptions(const framework::DisplayOptions &options)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_options = options;
}

void ShowGoalCommand::ResultBuilder::setCapacityUnits(const std::string& units)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_capacityUnits = units;
}

void ShowGoalCommand::ResultBuilder::setOutputTypeText()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_outputType = framework::ResultBase::OUTPUT_TEXT;
}

void ShowGoalCommand::ResultBuilder::setOutputTypeTable()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_outputType = framework::ResultBase::OUTPUT_TEXTTABLE;
}

framework::ResultBase* ShowGoalCommand::ResultBuilder::buildResult()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (displayOptionsAreValid())
	{
		createGoalResult();
	}

	return m_pResult;
}

bool ShowGoalCommand::ResultBuilder::displayOptionsAreValid()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_pResult = ShowCommandPropertyUtilities<core::configuration::MemoryAllocationGoal>::
			getInvalidDisplayOptionResult(m_options, m_props);

	return m_pResult == NULL;
}

void ShowGoalCommand::ResultBuilder::createGoalResult()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::ObjectListResult *pListResult = new framework::ObjectListResult();
	pListResult->setRoot(ShowGoalCommand::XML_ROOT);
	pListResult->setOutputType(m_outputType);

	std::vector<std::string> goalUids = m_goals.getDeviceUidsForGoals();
	for (std::vector<std::string>::iterator dimmUid = goalUids.begin();
			dimmUid != goalUids.end(); dimmUid++)
	{
		addGoalToListResult(m_goals[*dimmUid], *pListResult);
	}

	m_pResult = pListResult;
}

void ShowGoalCommand::ResultBuilder::addGoalToListResult(core::configuration::MemoryAllocationGoal& goal,
		framework::ObjectListResult& listResult)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!goalHasBeenApplied(goal))
	{
		framework::PropertyListResult goalResult =
				getPropertyListResultForGoal(goal);

		listResult.insert(ShowGoalCommand::XML_ROOT, goalResult);
	}
}

bool ShowGoalCommand::ResultBuilder::goalHasBeenApplied(core::configuration::MemoryAllocationGoal& goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (goal.getStatus() == CONFIG_GOAL_STATUS_SUCCESS);
}

framework::PropertyListResult ShowGoalCommand::ResultBuilder::getPropertyListResultForGoal(
		core::configuration::MemoryAllocationGoal& goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::PropertyListResult goalResult;
	for (size_t i = 0; i < m_props.size(); i++)
	{
		framework::IPropertyDefinition<core::configuration::MemoryAllocationGoal> &p = m_props[i];
		if (propertyShouldBeDisplayed(p))
		{
			goalResult.insert(p.getName(), p.getValue(goal));
		}
	}

	return goalResult;
}

bool ShowGoalCommand::ResultBuilder::propertyShouldBeDisplayed(
		framework::IPropertyDefinition<core::configuration::MemoryAllocationGoal>& property)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return ShowCommandPropertyUtilities<core::configuration::MemoryAllocationGoal>::
			isPropertyDisplayed(property, m_options);
}

std::string ShowGoalCommand::ResultBuilder::getDimmId(core::configuration::MemoryAllocationGoal& goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return ShowCommandUtilities::getDimmIdFromDeviceUidAndHandle(goal.getDeviceUid(),
			goal.getDeviceHandle());
}

std::string ShowGoalCommand::ResultBuilder::convertBytesToUnits(NVM_UINT64 bytes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string sizeWithUnits = convertCapacityFormat(bytes, m_capacityUnits);

	return sizeWithUnits;
}

std::string ShowGoalCommand::ResultBuilder::formatConfigGoalStatus(config_goal_status status)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string statusStr = TR("Unknown");

	switch (status)
	{
	case CONFIG_GOAL_STATUS_NEW:
		statusStr = TR("New");
		break;
	case CONFIG_GOAL_STATUS_ERR_BADREQUEST:
		statusStr = TR("Failed - Bad request");
		break;
	case CONFIG_GOAL_STATUS_ERR_INSUFFICIENTRESOURCES:
		statusStr = TR("Failed - Not enough resources");
		break;
	case CONFIG_GOAL_STATUS_ERR_FW:
		statusStr = TR("Failed - Firmware error");
		break;
	case CONFIG_GOAL_STATUS_ERR_UNKNOWN:
		statusStr = TR("Failed - Unknown");
		break;
	default:
		break;
	}

	return statusStr;
}

std::string ShowGoalCommand::ResultBuilder::getAppDirect1Index(
		core::configuration::MemoryAllocationGoal& goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream appDirectIndex;
	if (goal.hasAppDirect1())
	{
		appDirectIndex << goal.getAppDirect1Id();
	}
	else
	{
		appDirectIndex << NOT_APPLICABLE;
	}

	return appDirectIndex.str();
}

std::string ShowGoalCommand::ResultBuilder::getAppDirect1Settings(core::configuration::MemoryAllocationGoal &goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string appDirectSettings;
	if (goal.hasAppDirect1())
	{
		appDirectSettings = getFormattedAppDirectSettings(goal.getAppDirect1InterleaveWay(),
				goal.getAppDirect1MemoryControllerInterleave(), goal.getAppDirect1ChannelInterleave());
	}
	else
	{
		appDirectSettings = NOT_APPLICABLE;
	}

	return appDirectSettings;
}

std::string ShowGoalCommand::ResultBuilder::getFormattedAppDirectSettings(const NVM_UINT16 way,
		const interleave_size imcInterleave, const interleave_size channelInterleave)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream appDirectSettings;

	appDirectSettings << "x" << way;
	if (way > 1) // don't need interleave sizes unless multiple devices are interleaved
	{
		appDirectSettings << " - ";
		appDirectSettings << getFormattedInterleaveSize(imcInterleave);
		appDirectSettings << " iMC x ";
		appDirectSettings << getFormattedInterleaveSize(channelInterleave);
		appDirectSettings << " Channel";
	}

	return appDirectSettings.str();
}

std::string ShowGoalCommand::ResultBuilder::getFormattedInterleaveSize(const interleave_size interleaveSize)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string formattedSize = TR("Unknown");

	switch (interleaveSize)
	{
	case INTERLEAVE_SIZE_64B:
		formattedSize = "64B";
		break;
	case INTERLEAVE_SIZE_128B:
		formattedSize = "128B";
		break;
	case INTERLEAVE_SIZE_256B:
		formattedSize = "256B";
		break;
	case INTERLEAVE_SIZE_4KB:
		formattedSize = "4KB";
		break;
	case INTERLEAVE_SIZE_1GB:
		formattedSize = "1GB";
		break;
	default:
		break;
	}

	return formattedSize;
}

std::string ShowGoalCommand::ResultBuilder::getAppDirect2Index(
		core::configuration::MemoryAllocationGoal& goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream appDirectIndex;
	if (goal.hasAppDirect2())
	{
		appDirectIndex << goal.getAppDirect2Id();
	}
	else
	{
		appDirectIndex << NOT_APPLICABLE;
	}

	return appDirectIndex.str();
}

std::string ShowGoalCommand::ResultBuilder::getAppDirect2Settings(core::configuration::MemoryAllocationGoal &goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string appDirectSettings;
	if (goal.hasAppDirect2())
	{
		appDirectSettings = getFormattedAppDirectSettings(goal.getAppDirect2InterleaveWay(),
				goal.getAppDirect2MemoryControllerInterleave(), goal.getAppDirect2ChannelInterleave());
	}
	else
	{
		appDirectSettings = NOT_APPLICABLE;
	}

	return appDirectSettings;
}

std::string ShowGoalCommand::ResultBuilder::getActionRequiredEvents(core::configuration::MemoryAllocationGoal &goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string formattedEventList = NOT_APPLICABLE;

	std::vector<event> events = goal.getActionRequiredEvents();
	if (!events.empty())
	{
		formattedEventList = ShowCommandUtilities::getFormattedEventList(events);
	}

	return formattedEventList;
}

} /* namespace nvmcli */
} /* namespace cli */
