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
 * This file contains the NVMCLI simulator related commands.
 */

#include "SimulatorFeature.h"

#include "WbemToCli_utilities.h"
#include "CommandParts.h"

#include <libinvm-cli/SyntaxErrorResult.h>
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include <libinvm-cli/SyntaxErrorMissingValueResult.h>
#include <libinvm-cli/Parser.h>
#include <server/BaseServerFactory.h>
#include <libinvm-cli/CliFrameworkTypes.h>
#include <LogEnterExit.h>
#include <cr_i18n.h>

const std::string cli::nvmcli::SimulatorFeature::Name = "Simulator";
static const std::string UNLOAD_PROPERTY_NAME = "unload";
static const std::string LOAD_PREFIX = "Set default simulator: ";
static const std::string UNLOAD_PREFIX = "Clear default simulator: ";

void cli::nvmcli::SimulatorFeature::getPaths(cli::framework::CommandSpecList &list)
{
 
	cli::framework::CommandSpec addSim(ADDSIMULATOR, TR("Load Simulator"), framework::VERB_LOAD,
			TR("Set or clear the default simulator file to be used when the " NVM_DIMM_NAME " host software loads."));
	addSim.addOption(framework::OPTION_SOURCE)
			.helpText(TR("File path of the simulator to load. Note: Source is not required if the Unload "
				"property is specified."));
	addSim.addTarget(TARGET_SYSTEM_R)
		.helpText(TR("The simulator. No filtering is supported on this target."));
	addSim.addProperty(UNLOAD_PROPERTY_NAME, false, "0|1", true, "Set to 1 to unload and clear the simulator.");

	list.push_back(addSim);
}

cli::nvmcli::SimulatorFeature::SimulatorFeature() :
		framework::FeatureBase()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

}

cli::framework::ResultBase* cli::nvmcli::SimulatorFeature::run(
		const int &commandSpecId,
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	switch (commandSpecId)
	{
		case ADDSIMULATOR:
		{
			wbem::server::BaseServerFactory serverFactory;

			// get the unload property
			bool unloadPropertyExists = false;
			std::string unloadPropertyValue =
				cli::framework::Parser::getPropertyValue(parsedCommand, UNLOAD_PROPERTY_NAME, &unloadPropertyExists);

			// add simulator
			if (!unloadPropertyExists || unloadPropertyValue == "0")
			{
				bool sourceOptionExists = false;
				std::string simulatorPath =
						framework::Parser::getOptionValue(parsedCommand,
						framework::OPTION_SOURCE.name, &sourceOptionExists);
				// missing required source option
				if (!sourceOptionExists)
				{
					pResult = new framework::SyntaxErrorResult(TR("Missing required option '-source'"));
				}
				// missing required source value
				else if (simulatorPath.empty())
				{
					pResult = new framework::SyntaxErrorMissingValueResult(
							framework::TOKENTYPE_OPTION, framework::OPTION_SOURCE.name);
				}
				// syntax is good - send the command
				else
				{
					try
					{
						serverFactory.addDefaultSimulator(simulatorPath);
						pResult = new framework::SimpleResult(LOAD_PREFIX + TRS(cli::framework::SUCCESS_MSG));
					}
					catch (wbem::framework::Exception &e)
					{
						pResult = NvmExceptionToResult(e, LOAD_PREFIX);
					}
				}
			}
			// remove simulator
			else if (unloadPropertyValue == "1")
			{
				// ignore source option if provided
				// send command
				try
				{
					serverFactory.removeDefaultSimulator();
					pResult = new framework::SimpleResult(UNLOAD_PREFIX + TRS(cli::framework::SUCCESS_MSG));
				}
				catch (wbem::framework::Exception &e)
				{
					pResult = NvmExceptionToResult(e, UNLOAD_PREFIX);
				}
			}
			// bad property value
			else
			{
				pResult = new framework::SyntaxErrorBadValueResult(
						framework::TOKENTYPE_PROPERTY, UNLOAD_PROPERTY_NAME, unloadPropertyValue);
			}
			break;
		}
	}
	return pResult;
}


