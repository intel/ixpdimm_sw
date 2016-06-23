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
 * This file contains the NVMCLI debug and validation commands.
 */

#include <vector>
#include <string>

#include <LogEnterExit.h>
#include <string/s_str.h>
#include <libinvm-cli/SimpleListResult.h>
#include <libinvm-cli/FeatureBase.h>
#include <libinvm-cli/CommandSpec.h>
#include <libinvm-cli/PropertyListResult.h>
#include <libinvm-cli/Parser.h>
#include <libinvm-cli/SyntaxErrorMissingValueResult.h>
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include "CommandParts.h"
#include "WbemToCli_utilities.h"
#include "ValidationFeature.h"

#include <cr_i18n.h>
#include <exception/NvmExceptionLibError.h>

const std::string cli::nvmcli::ValidationFeature::Name = "Validation";

/*
 * Command Specs the Validation Feature supports
 */
void cli::nvmcli::ValidationFeature::getPaths(cli::framework::CommandSpecList &list)
{
	cli::framework::CommandSpec injectError(INJECT_ERROR, TR("Inject Error"), framework::VERB_SET,
			TR("Inject an error or clear a previously injected error on one or more " NVM_DIMM_NAME "s for "
			"testing purposes."));
	injectError.addTarget(TARGET_DIMM_R)
			.helpText(TR("Inject or clean an error on specific " NVM_DIMM_NAME "s by supplying one or more comma separated "
			"" NVM_DIMM_NAME " identifiers. The default is to inject the error on all manageable " NVM_DIMM_NAME "s. "));
	injectError.addProperty(CLEAR_PROPERTYNAME, false, "1", true,
			TR("Clear a previously injected poison error. If this property is provided, it will "
			"clear the poison error at the specified address. There is no need to clear an "
			"injected temperature. "));
	injectError.addProperty(TEMPERATURE_PROPERTYNAME, false, "degrees", true,
			TR("Inject a particular artificial temperature in degrees Celsius into the "
			"" NVM_DIMM_NAME ".  The firmware that is monitoring the temperature of the " NVM_DIMM_NAME " will "
			"then be alerted and take necessary precautions to preserve the " NVM_DIMM_NAME ". Once the "
			"firmware performs it's next temperature diagnostic, the temperature will be "
			"overridden to the correct temperature of the part. Therefore there is no need to "
			"clear an injected temperature."));
	injectError.addProperty(POISON_PROPERTYNAME, false, "address", true,
			TR(" The physical address to poison. Poison is not possible for any address in "
			"the persistent memory region if it is locked. Injected poison errors are only "
			"triggered on a subsequent read of the poisoned address. The caller is responsible "
			"for keeping a list of injected poison errors in order to properly clear them after "
			"testing is complete."));
	injectError.addProperty(DIE_SPARING_PROPERTYNAME, false, "1", true,
			TR("Trigger an artificial die sparing. If die sparing is enabled and " NVM_DIMM_NAME " still "
			"has spares remaining, this will cause the firmware to report that there are no spares "
			"remaining."));
	injectError.addProperty(SPARE_ALARM_PROPERTYNAME, false, "1", true,
			TR("Trigger a spare capacity threshold alarm which will cause the firmware to generate "
			"an error log and an alert."));
	injectError.addProperty(FATAL_MEDIA_ERROR_PROPERTYNAME, false, "1", true,
			TR("Inject a fake media fatal error which will cause the firmware to generate an error log "
			"and an alert."));

	list.push_back(injectError);
}

// Constructor, just calls super class 
cli::nvmcli::ValidationFeature::ValidationFeature() : cli::framework::FeatureBase(),
	m_dimmUid(""), m_temperature(0), m_poison(0), m_clearStateExists(false),
	m_temperatureExists(false), m_poisonExists(false), m_dieSparingExists(false),
	m_spareAlarmExists(false), m_fatalMediaErrorExists(false)
{ }

/*
 * Get all the BaseServer Instances from the wbem base server factory.
 */
cli::framework::ResultBase * cli::nvmcli::ValidationFeature::run(
		const int &commandSpecId, const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	switch(commandSpecId)
	{
		case INJECT_ERROR:
			pResult = injectError(parsedCommand);
			break;
	}

	return pResult;
}

void cli::nvmcli::ValidationFeature::inject_error(std::string &prefixMsg,
		std::vector<std::string>::iterator iUid, framework::SimpleListResult &listResult)
{
	if (m_poisonExists)
	{
		prefixMsg = framework::ResultBase::stringFromArgList(
				SETPOISON_MSG_PREFIX.c_str(), m_poison, m_dimmUid.c_str());
		prefixMsg += ": ";
		m_DimmProvider.injectPoisonError(*iUid, m_poison);
		listResult.insert(prefixMsg + cli::framework::SUCCESS_MSG);
	}
	else if (m_temperatureExists)
	{
		prefixMsg = framework::ResultBase::stringFromArgList(
				SETTEMPERATURE_MSG_PREFIX.c_str(), m_dimmUid.c_str());
		prefixMsg += ": ";
		m_DimmProvider.injectTemperatureError(*iUid, m_temperature);
		listResult.insert(prefixMsg + cli::framework::SUCCESS_MSG);
	}
	else if (m_dieSparingExists)
	{
		prefixMsg = framework::ResultBase::stringFromArgList(
				SETDIESPARING_MSG_PREFIX.c_str(), m_dimmUid.c_str());
		prefixMsg += ": ";
		m_DimmProvider.injectSoftwareTrigger(*iUid, ERROR_TYPE_DIE_SPARING);
		listResult.insert(prefixMsg + cli::framework::SUCCESS_MSG);
	}
	else if (m_spareAlarmExists)
	{
		prefixMsg = framework::ResultBase::stringFromArgList(
				SETSPARECAPACITYALARM_MSG_PREFIX.c_str(), m_dimmUid.c_str());
		prefixMsg += ": ";
		m_DimmProvider.injectSoftwareTrigger(*iUid, ERROR_TYPE_SPARE_ALARM);
		listResult.insert(prefixMsg + cli::framework::SUCCESS_MSG);
	}
	else if (m_fatalMediaErrorExists)
	{
		prefixMsg = framework::ResultBase::stringFromArgList(
				SETFATALERROR_MSG_PREFIX.c_str(), m_dimmUid.c_str());
		prefixMsg += ": ";
		m_DimmProvider.injectSoftwareTrigger(*iUid, ERROR_TYPE_MEDIA_FATAL_ERROR);
		listResult.insert(prefixMsg + cli::framework::SUCCESS_MSG);
	}
}

void cli::nvmcli::ValidationFeature::clear_injected_error(std::string &prefixMsg,
		std::vector<std::string>::iterator iUid, framework::SimpleListResult &listResult)
{
	if (m_poisonExists)
	{
		prefixMsg = framework::ResultBase::stringFromArgList(
				CLEARPOISON_MSG_PREFIX.c_str(), m_poison, m_dimmUid.c_str());
		prefixMsg += ": ";
		m_DimmProvider.clearPoisonError(*iUid, m_poison);
		listResult.insert(prefixMsg + cli::framework::SUCCESS_MSG);
	}
	else if (m_temperatureExists)
	{
		prefixMsg = framework::ResultBase::stringFromArgList(
				CLEARTEMPERATURE_MSG_PREFIX.c_str(), m_dimmUid.c_str());
		prefixMsg += ": ";
		m_DimmProvider.clearTemperatureError(*iUid);
		listResult.insert(prefixMsg + cli::framework::SUCCESS_MSG);
	}
	else if (m_dieSparingExists)
	{
		prefixMsg = framework::ResultBase::stringFromArgList(
				CLEARDIESPARING_MSG_PREFIX.c_str(), m_dimmUid.c_str());
		prefixMsg += ": ";
		m_DimmProvider.clearSoftwareTrigger(*iUid, ERROR_TYPE_DIE_SPARING);
		listResult.insert(prefixMsg + cli::framework::SUCCESS_MSG);
	}
}

/*
 * Inject Error
 */
cli::framework::ResultBase* cli::nvmcli::ValidationFeature::injectError(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	// Validate the DIMM list and translate PIDs into UIDs
	std::vector<std::string> uids;
	pResult = cli::nvmcli::getDimms(parsedCommand, uids);

	if (!pResult)
	{
		pResult = getInjectErrorAttributes(parsedCommand);
		if (!pResult)
		{
			framework::SimpleListResult *pListResult = new framework::SimpleListResult();
			pResult = pListResult;
			for(std::vector<std::string>::iterator iUid = uids.begin();
						iUid != uids.end(); iUid ++)
			{
				std::string prefixMsg;
				try 
				{
					m_dimmUid = wbem::physical_asset::NVDIMMFactory::uidToDimmIdStr(*iUid);

					if (m_clearStateExists) 
					{
						clear_injected_error(prefixMsg, iUid, *pListResult);
					}
					else
					{
						inject_error(prefixMsg, iUid, *pListResult);
					}
				}
				catch (wbem::framework::Exception &e)
				{
					framework::ErrorResult *pError = ieNvmExceptionToResult(e);
					if (pError)
					{
						pListResult->insert(prefixMsg + pError->outputText());
						if (!pListResult->getErrorCode()) // keep any existing errors
						{
							pListResult->setErrorCode(pError->getErrorCode());
						}
						delete pError;
					}
				}
			} // end of for loop
		} // getInjectErrorAttributes
	} // getDimms

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::ValidationFeature::parseClearProperty(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string clearPropertyValue = framework::Parser::getPropertyValue(parsedCommand,
			CLEAR_PROPERTYNAME, &m_clearStateExists);

	if (m_clearStateExists)
	{
		if (clearPropertyValue.empty())
		{
			pResult = new framework::SyntaxErrorMissingValueResult(
					framework::TOKENTYPE_PROPERTY, CLEAR_PROPERTYNAME);
		}
		else if (!cli::framework::stringsIEqual(clearPropertyValue, "1"))
		{
			pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY,
					CLEAR_PROPERTYNAME.c_str(),
					clearPropertyValue);
		}
		else
		{
			m_clearStateExists = true;
		}
	}
	else
	{
		m_clearStateExists = false;
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::ValidationFeature::parseTemperatureProperty(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string temperaturePropertyValue = framework::Parser::getPropertyValue(parsedCommand,
		TEMPERATURE_PROPERTYNAME, &m_temperatureExists);

	if (m_temperatureExists)
	{
		if (!m_clearStateExists)
		{
			if(!isStringValidNumber(temperaturePropertyValue))
			{
				pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
					TEMPERATURE_PROPERTYNAME, temperaturePropertyValue);
			}
			else
			{
				m_temperature = stringToUInt64(temperaturePropertyValue);
			}
		}
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::ValidationFeature::parsePoisonProperty(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string poisonPropertyValue = framework::Parser::getPropertyValue(parsedCommand,
		POISON_PROPERTYNAME, &m_poisonExists);

	if (m_poisonExists)
	{
		if(!isStringHex(poisonPropertyValue))
		{
			pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
					POISON_PROPERTYNAME, poisonPropertyValue);
		}
		else
		{
			pResult = errorIfMoreThanOnePropertyIsModified();
			if (!pResult)
			{
				m_poison = stringToUInt64(poisonPropertyValue);
			}
		}
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::ValidationFeature::parseDieSparingProperty(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string propValue =
		framework::Parser::getPropertyValue(parsedCommand, DIE_SPARING_PROPERTYNAME, &m_dieSparingExists);
	if (m_dieSparingExists)
	{
		pResult = verifySWTriggerPropertyValue(propValue, DIE_SPARING_PROPERTYNAME);
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::ValidationFeature::parseSpareAlarmProperty(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string propValue =
			framework::Parser::getPropertyValue(parsedCommand, SPARE_ALARM_PROPERTYNAME, &m_spareAlarmExists);
	if (m_spareAlarmExists)
	{
		pResult = checkClearState();
		if (!pResult)
		{
			pResult = verifySWTriggerPropertyValue(propValue, SPARE_ALARM_PROPERTYNAME);
		}
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::ValidationFeature::parseFatalMediaErrorProperty(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string propValue =
			framework::Parser::getPropertyValue(parsedCommand, FATAL_MEDIA_ERROR_PROPERTYNAME, &m_fatalMediaErrorExists);
	if (m_fatalMediaErrorExists)
	{
		pResult = checkClearState();
		if (!pResult)
		{
			pResult = verifySWTriggerPropertyValue(propValue, FATAL_MEDIA_ERROR_PROPERTYNAME);
		}
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::ValidationFeature::getInjectErrorAttributes(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	pResult = parseClearProperty(parsedCommand);
	if (!pResult)
	{
		pResult = parseTemperatureProperty(parsedCommand);
	}
	if (!pResult)
	{
		pResult = parsePoisonProperty(parsedCommand);
	}
	if (!pResult)
	{
		pResult = parseDieSparingProperty(parsedCommand);
	}
	if (!pResult)
	{
		pResult = parseSpareAlarmProperty(parsedCommand);
	}
	if (!pResult)
	{
		pResult = parseFatalMediaErrorProperty(parsedCommand);
	}

	if (!pResult)
	{
		pResult = verifyPropertyCount(parsedCommand);
	}

	return pResult;
}

cli::framework::ErrorResult* cli::nvmcli::ValidationFeature::ieNvmExceptionToResult(
		wbem::framework::Exception& e, std::string prefix)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::exception::NvmExceptionLibError * pLibError =
			dynamic_cast<wbem::exception::NvmExceptionLibError *>(&e);
	framework::ErrorResult *pResult = NULL;

	if (pLibError)
	{
		switch (pLibError->getLibError())
		{
			case NVM_ERR_INVALIDPARAMETER:
			{
				char errbuff[NVM_ERROR_LEN];
				s_snprintf(errbuff, NVM_ERROR_LEN,
						TRS(SETPOISON_INVALIDPARAMETER_MSG), 
						m_poison, m_dimmUid.c_str());
				pResult = new framework::ErrorResult(framework::ResultBase::ERRORCODE_UNKNOWN,
						errbuff);
				break;
			}
			default: 
			{
				pResult = NvmExceptionToResult(e, prefix);
				break;
			}		
		} // end switch
	}
	
	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::ValidationFeature::errorIfMoreThanOnePropertyIsModified()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::vector<std::string> list;
	if (m_temperatureExists)
	{
		list.push_back(TEMPERATURE_PROPERTYNAME);
	}
	if (m_poisonExists)
	{
		list.push_back(POISON_PROPERTYNAME);
	}
	if (m_dieSparingExists)
	{
		list.push_back(DIE_SPARING_PROPERTYNAME);
	}
	if (m_spareAlarmExists)
	{
		list.push_back(SPARE_ALARM_PROPERTYNAME);
	}
	if (m_fatalMediaErrorExists)
	{
		list.push_back(FATAL_MEDIA_ERROR_PROPERTYNAME);
	}

	if (list.size() > 1)
	{
		std::string errorString = framework::ResultBase::stringFromArgList(
				TR(CANT_USE_TOGETHER_ERROR_STR.c_str()),
				list[0].c_str(), list[1].c_str());
		pResult = new framework::SyntaxErrorResult(errorString);
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::ValidationFeature::verifyPropertyCount(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	if ((m_clearStateExists && parsedCommand.properties.size() > 2) ||
			(m_clearStateExists && parsedCommand.properties.size() == 1) ||
			(!m_clearStateExists && parsedCommand.properties.size() > 1) ||
			parsedCommand.properties.empty())
	{
		pResult = new framework::SyntaxErrorResult(TRS(INVALID_COMMAND_ERROR_STR));
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::ValidationFeature::checkClearState()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	if (m_clearStateExists)
	{
		char errbuff[NVM_ERROR_LEN];
		s_snprintf(errbuff, NVM_ERROR_LEN,
				TRS(NOTSUPPORTED_ERROR_STR));
		pResult = new framework::ErrorResult(framework::ResultBase::ERRORCODE_UNKNOWN,
				errbuff);
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::ValidationFeature::verifySWTriggerPropertyValue(
		const std::string& propValue, std::string &propertyName)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	if (!framework::stringsIEqual(propValue, "1"))
	{
		pResult = new framework::SyntaxErrorBadValueResult(
				framework::TOKENTYPE_PROPERTY, propertyName.c_str(),
				propValue);
	}
	else
	{
		pResult = errorIfMoreThanOnePropertyIsModified();
	}

	return pResult;
}
