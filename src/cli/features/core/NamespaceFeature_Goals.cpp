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
 * This file contains the implementation of the NVMCLI commands to create, delete,
 * and show configuration goals.
 */

#include "NamespaceFeature.h"
#include <LogEnterExit.h>
#include <mem_config/MemoryConfigurationFactory.h>
#include <libinvm-cim/ExceptionBadAttribute.h>
#include "CommandParts.h"
#include <libinvm-cli/SimpleListResult.h>

/*
 * Check to see if the user is violating the rule for deleting goal instances.
 * Basically, the rule is that the user may not delete part of a configured socket.
 * If the user asks to delete specific dimms then they must delete the goals of all
 * configured dimms on a socket.
 */
bool cli::nvmcli::NamespaceFeature::validRequest(const wbem::framework::instances_t &allInstances,
						const wbem::framework::instances_t &requestedInstances)
{
	bool result = true;
	wbem::framework::instances_t::const_iterator requestIter = requestedInstances.begin();

	for (; requestIter != requestedInstances.end() && result; requestIter++)
	{
		wbem::framework::Attribute requestIdAttr;
		if (requestIter->getAttribute(wbem::SOCKETID_KEY, requestIdAttr)
					== wbem::framework::SUCCESS)
		{
			int requestedSocket = requestIdAttr.uintValue();
			wbem::framework::instances_t::const_iterator iter = allInstances.begin();
			for(; iter != allInstances.end(); iter++)
			{
				wbem::framework::Attribute idAttr;
				if (iter->getAttribute(wbem::SOCKETID_KEY, idAttr)
						== wbem::framework::SUCCESS)
				{
					int socket = idAttr.uintValue();
					if (socket == requestedSocket)
					{
						wbem::framework::instances_t::const_iterator found;
						found = std::find(requestedInstances.begin(),
									requestedInstances.end(), *iter);
						if (found == requestedInstances.end())
						{
							result = false;
							break;
						}
					}
				}
				else
				{
					throw wbem::framework::ExceptionBadAttribute(
							wbem::SOCKETID_KEY.c_str());
				}
			}
		}
		else
		{
			throw wbem::framework::ExceptionBadAttribute(
							wbem::SOCKETID_KEY.c_str());
		}
	}
	return result;
}

/*
 * Delete a configuration goal from a list of sockets or list of dimms
 */
cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::deleteConfigGoal(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string basePrefix = TRS(DELETECONFIGGOAL_FROMDIMM_MSG);

	wbem::framework::instances_t *pInstances = NULL;
	wbem::framework::instances_t matchedInstances;
	try
	{
		// Get the instances
		wbem::mem_config::MemoryConfigurationFactory provider;
		wbem::framework::attribute_names_t attributes;
		// get applied and unapplied goals
		pInstances = provider.getGoalInstances(attributes, true);
		if (!pInstances)
		{
			throw wbem::framework::Exception("MemoryConfigurationFactory.getInstances returned NULL");
		}

		if (pInstances->size() == 0)
		{
			pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				"The requested " NVM_DIMM_NAME "(s) are not configured.", basePrefix);
		}
		else
		{
			// rename the Parent to UID to work with filterInstances
			RenameAttributeKey(*pInstances, wbem::PARENT_KEY, wbem::DIMMUID_KEY);

			cli::nvmcli::filters_t filters;

			generateDimmFilter(parsedCommand, attributes, filters);
			generateSocketFilter(parsedCommand, attributes, filters);
			filterInstances(*pInstances, wbem::CONFIGGOALTABLENAME, filters, matchedInstances);

			if (!validRequest(*pInstances, matchedInstances))
			{
				pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
					"The configuration goals from all the " NVM_DIMM_NAME "s on the "
					"socket must be deleted together.", basePrefix);
			}
			else if (matchedInstances.size() == 0)
			{
				// verify the user entered correct Dimm IDs
				std::vector<std::string> dimmList;
				pResult = getDimms(parsedCommand, dimmList);
				if (pResult == NULL)
				{
					// verify the user entered correct sockets
					pResult = getDimmsFromSockets(parsedCommand, dimmList);
					// requested dimms/sockets are valid but don't have goals
					if (pResult == NULL)
					{
						pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
							"The requested " NVM_DIMM_NAME "(s) are not configured.", basePrefix);
					}
				}
			}
		}
	}
	catch (wbem::framework::Exception &e)
	{
		if (pResult)
		{
			delete pResult;
			pResult = NULL;
		}
		pResult = NvmExceptionToResult(e);
	}

	if (pResult == NULL)
	{
		framework::SimpleListResult *pListResults = new framework::SimpleListResult();

		wbem::framework::instances_t::iterator iter;
		for (iter = matchedInstances.begin();
				iter != matchedInstances.end(); iter++)
		{
			wbem::framework::Attribute idAttr;
			iter->getAttribute(wbem::INSTANCEID_KEY, idAttr);
			std::string instanceId = idAttr.stringValue();
			std::string dimmId = instanceId.substr(0, instanceId.length() - 1);
			std::string prefix = cli::framework::ResultBase::stringFromArgList(
						(basePrefix + " %s").c_str(),
						wbem::physical_asset::NVDIMMFactory::uidToDimmIdStr(dimmId).c_str()) + ": ";

			wbem::framework::ObjectPath path = (*iter).getObjectPath();

			try
			{
				wbem::mem_config::MemoryConfigurationServiceFactory provider;
				provider.deleteInstance(path);
				pListResults->insert(prefix + TRS(cli::framework::SUCCESS_MSG));
			}
			catch (wbem::framework::Exception &e)
			{
				framework::ErrorResult *pResult = NvmExceptionToResult(e);
				pListResults->insert(prefix + pResult->outputText());
				if (!pListResults->getErrorCode()) // keep existing errors
				{
					pListResults->setErrorCode(pResult->getErrorCode());
				}
				delete pResult;
			}
		}
		pResult = pListResults;
	}

	if (pInstances)
	{
		delete pInstances;
	}
	return pResult;
}

/*
 * Dump the current config into a file.
 */
cli::framework::ResultBase *cli::nvmcli::NamespaceFeature::dumpConfig(
		const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	// Fetch the destination file path
	std::string path = cli::framework::Parser::getOptionValue(parsedCommand, framework::OPTION_DESTINATION_R.name, NULL);

	try
	{
		wbem::mem_config::MemoryConfigurationServiceFactory provider;
		provider.exportSystemConfigToPath(path);
	}
	catch (wbem::framework::Exception &e)
	{
		pResult = dumpConfigNvmExceptionToResult(e);
	}

	if (!pResult) // no errors
	{
		framework::SimpleListResult *pSimpleList = new framework::SimpleListResult();
		pSimpleList->insert(TR("Successfully dumped system configuration to file: ") + path);
		pResult = pSimpleList;
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::dumpConfigNvmExceptionToResult(
		wbem::framework::Exception& e)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string prefix = TRS(DUMPSYSTEMCONFIG_FROMDIMM_MSG);
	wbem::exception::NvmExceptionLibError * pLibError =
			dynamic_cast<wbem::exception::NvmExceptionLibError *>(&e);
	if (pLibError)
	{
		switch (pLibError->getLibError())
		{
			case NVM_ERR_NOTFOUND:
				return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
					"The requested " NVM_DIMM_NAME "(s) are not configured.",
					prefix);
				break;

			default:
				NVM_ERROR_DESCRIPTION errStr;
				nvm_get_error((return_code)pLibError->getLibError(), errStr, NVM_ERROR_LEN);
				return new framework::ErrorResult(framework::ResultBase::ERRORCODE_UNKNOWN, errStr, prefix);
				break;
		}
	}

	return NvmExceptionToResult(e);
}

/*
 * Load the config goal from a file
 */
cli::framework::ResultBase *cli::nvmcli::NamespaceFeature::loadGoal(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string capUnits;
	pResult = GetRequestedCapacityUnits(parsedCommand, capUnits);
	if (!pResult)
	{
		std::string path = framework::Parser::getOptionValue(parsedCommand, framework::OPTION_SOURCE_R.name, NULL);

		// Validate socket and DIMM targets
		bool dimmTargetExists = false;
		bool socketTargetExists = false;
		std::string dimmStr = framework::Parser::getTargetValue(parsedCommand, TARGET_DIMM.name, &dimmTargetExists);
		std::string socketStr = framework::Parser::getTargetValue(parsedCommand, TARGET_SOCKET.name, &socketTargetExists);

		// don't know what to do if -socket and -dimm are both provided
		if (socketTargetExists && dimmTargetExists)
		{
			pResult = new framework::SyntaxErrorResult(framework::ResultBase::stringFromArgList(
					TR("'%s' and '%s' targets cannot be used together."), TARGET_DIMM.name.c_str(),
					TARGET_SOCKET.name.c_str()));
		}
		else
		{
			try
			{
				bool forceOption = parsedCommand.options.find(framework::OPTION_FORCE.name)
							!= parsedCommand.options.end();

				// if user didn't specify the force option, prompt them to continue
				std::string prompt = framework::ResultBase::stringFromArgList(
						LOAD_GOAL_PROMPT.c_str(), path.c_str());
				if (!forceOption && !promptUserYesOrNo(prompt))
				{
					pResult = new framework::SimpleResult(LOAD_CONFIG_GOAL_MSG + cli::framework::UNCHANGED_MSG);
				}
				else
				{
					std::vector<std::string> dimms; // list of DIMM UIDs to be constructed from target list
					if (socketTargetExists) // -socket target provided
					{
						pResult = getDimmsFromSockets(parsedCommand, dimms);
					}
					else // no -socket target, so use -dimm
					{
						pResult = getDimms(parsedCommand, dimms);
					}

					if (!pResult) // no error
					{
						wbem::mem_config::MemoryConfigurationServiceFactory provider;
						provider.importDimmConfigsFromPath(path, dimms);
					}
				}
			}
			catch (wbem::framework::Exception &e)
			{
				if (pResult)
				{
					delete pResult;
				}
				pResult = NvmExceptionToResult(e);
			}
		}

		// Success - show the config goal that was loaded
		if (!pResult)
		{
			pResult = showConfigGoal(parsedCommand);
		}
	}
	return pResult;
}
