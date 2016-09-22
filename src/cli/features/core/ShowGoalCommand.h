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

#ifndef SHOWGOALCOMMAND_H_
#define SHOWGOALCOMMAND_H_

#include <nvm_types.h>

#include <string>
#include <exception>
#include <libinvm-cli/ObjectListResult.h>
#include <libinvm-cli/PropertyListResult.h>
#include "framework/PropertyDefinitionList.h"
#include "framework/PropertyDefinitionBase.h"
#include <core/configuration/MemoryAllocationGoal.h>
#include <core/configuration/MemoryAllocationGoalCollection.h>

#include <cli/features/core/framework/CommandBase.h>
#include <libinvm-cli/CommandSpec.h>
#include <core/configuration/MemoryAllocationGoalService.h>
#include <core/device/DeviceService.h>
#include <core/StringList.h>

namespace cli
{
namespace nvmcli
{



class NVM_API ShowGoalCommand : public framework::CommandBase
{
	public:
		static const std::string XML_ROOT;

		static const std::string DIMMID;
		static const std::string SOCKETID;
		static const std::string MEMORYSIZE;
		static const std::string APPDIRECT1SIZE;
		static const std::string APPDIRECT1INDEX;
		static const std::string APPDIRECT1SETTINGS;
		static const std::string APPDIRECT2SIZE;
		static const std::string APPDIRECT2INDEX;
		static const std::string APPDIRECT2SETTINGS;
		static const std::string STORAGESIZE;
		static const std::string STATUS;
		static const std::string ACTIONREQUIRED;
		static const std::string ACTIONREQUIREDEVENTS;

		ShowGoalCommand(core::device::DeviceService &deviceService = core::device::DeviceService::getService(),
				core::configuration::MemoryAllocationGoalService &goalService =
						core::configuration::MemoryAllocationGoalService::getService());
		virtual ~ShowGoalCommand();

		static framework::CommandSpec getCommandSpec(const int commandId);

		// Caller is expected to manage returned memory
		virtual framework::ResultBase *execute(const framework::ParsedCommand &parsedCommand);

		class NVM_API ResultBuilder
		{
			public:

				class DimmNotFound : public std::exception {};

				ResultBuilder();
				virtual ~ResultBuilder();

				virtual void setGoals(const core::configuration::MemoryAllocationGoalCollection &collection);
				virtual void setDisplayOptions(const framework::DisplayOptions &options);
				virtual void setCapacityUnits(const std::string &units);
				virtual void setOutputTypeText();
				virtual void setOutputTypeTable();

				virtual framework::ResultBase *buildResult();

			private:
				static const std::string NOT_APPLICABLE;

				framework::PropertyDefinitionList<core::configuration::MemoryAllocationGoal> m_props;
				framework::DisplayOptions m_options;
				framework::ResultBase::outputTypes m_outputType;
				core::configuration::MemoryAllocationGoalCollection m_goals;

				// Not creating multiple instances of a command at a time, so this should be safe
				static std::string m_capacityUnits;

				framework::ResultBase *m_pResult;

				static std::string getDimmId(core::configuration::MemoryAllocationGoal &goal);
				static std::string convertBytesToUnits(NVM_UINT64 bytes);
				static std::string formatConfigGoalStatus(config_goal_status status);
				static std::string getAppDirect1Index(core::configuration::MemoryAllocationGoal &goal);
				static std::string getAppDirect1Settings(core::configuration::MemoryAllocationGoal &goal);
				static std::string getFormattedAppDirectSettings(const NVM_UINT16 way,
						const interleave_size imcInterleave, const interleave_size channelInterleave);
				static std::string getFormattedInterleaveSize(const interleave_size interleaveSize);
				static std::string getAppDirect2Index(core::configuration::MemoryAllocationGoal &goal);
				static std::string getAppDirect2Settings(core::configuration::MemoryAllocationGoal &goal);
				static std::string getActionRequiredEvents(core::configuration::MemoryAllocationGoal &goal);

				bool displayOptionsAreValid();

				void createGoalResult();
				void addGoalToListResult(core::configuration::MemoryAllocationGoal &goal, framework::ObjectListResult &listResult);
				bool goalHasBeenApplied(core::configuration::MemoryAllocationGoal &goal);
				framework::PropertyListResult getPropertyListResultForGoal(
						core::configuration::MemoryAllocationGoal &goal);
				bool propertyShouldBeDisplayed(
						framework::IPropertyDefinition<core::configuration::MemoryAllocationGoal> &property);
		};

	private:
		core::device::DeviceService &m_deviceService;
		core::configuration::MemoryAllocationGoalService &m_goalService;
		ResultBuilder m_resultBuilder;

		core::StringList m_dimmIds;
		core::StringList m_socketIds;

		core::device::DeviceCollection m_devices;
		core::configuration::MemoryAllocationGoalCollection m_goals;

		void initWithParsedCommand(const framework::ParsedCommand &parsedCommand);
		void setBuilderOutputOptions();

		bool unitsOptionIsValid();

		void populateAllDevices();
		bool dimmIdsAreValid();
		bool socketIdsAreValid();
		void filterDevices();
		void populateGoalsForFilteredDevices();
		void populateGoalForDevice(core::device::Device &device);
};

} /* namespace nvmcli */
} /* namespace cli */

#endif /* SHOWGOALCOMMAND_H_ */
