/*
 * Copyright (c) 2015, Intel Corporation
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
#include <intel_cli_framework/SimpleListResult.h>
#include <intel_cli_framework/FeatureBase.h>
#include <intel_cli_framework/CommandSpec.h>
#include <intel_cli_framework/PropertyListResult.h>
#include <intel_cli_framework/Parser.h>
#include <intel_cli_framework/SyntaxErrorMissingValueResult.h>
#include <intel_cli_framework/SyntaxErrorBadValueResult.h>
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

	list.push_back(injectError);
}


// Constructor, just calls super class 
cli::nvmcli::ValidationFeature::ValidationFeature() : cli::framework::FeatureBase(),
	m_dimmGuid(""), m_temperature(0), m_poison(0), m_errorType(0), m_clearStateExists(false)
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

/*
 * Inject Error
 */
cli::framework::ResultBase* cli::nvmcli::ValidationFeature::injectError(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	// Validate the DIMM list and translate PIDs into GUIDs
	std::vector<std::string> guids;
	pResult = cli::nvmcli::getDimms(parsedCommand, guids);
	// No error found in DIMM list
	if (pResult == NULL)
	{
		pResult = getInjectErrorAttributes(parsedCommand);
		if (!pResult)
		{
			framework::SimpleListResult *pListResult = new framework::SimpleListResult();
			pResult = pListResult;
			for(std::vector<std::string>::iterator iGuid = guids.begin();
						iGuid != guids.end(); iGuid ++)
			{
				std::string prefixMsg;
				try 
				{
					m_dimmGuid = wbem::physical_asset::NVDIMMFactory::guidToDimmIdStr(*iGuid);
					// handle clear error injection 
					if (m_clearStateExists) 
					{
						if (m_errorType == ERROR_TYPE_POISON)
						{
							m_DimmProvider.clearPoisonError(*iGuid, m_poison);
							prefixMsg = framework::ResultBase::stringFromArgList(
								CLEARPOISON_MSG_PREFIX.c_str(), m_poison,
								m_dimmGuid.c_str());
							prefixMsg += ": ";

							pListResult->insert(prefixMsg + cli::framework::SUCCESS_MSG);
						}
					}
					else
					{
						// handle error injection
						if (m_errorType == ERROR_TYPE_POISON)
						{
							prefixMsg = framework::ResultBase::stringFromArgList(
									SETPOISON_MSG_PREFIX.c_str(), m_poison,
									m_dimmGuid.c_str());
							prefixMsg += ": ";
							m_DimmProvider.injectPoisonError(*iGuid, m_poison);
						}
						if (m_errorType == ERROR_TYPE_TEMPERATURE)
						{
							prefixMsg = framework::ResultBase::stringFromArgList(
									SETTEMPERATURE_MSG_PREFIX.c_str(),
									m_dimmGuid.c_str());
							prefixMsg += ": ";
							m_DimmProvider.injectTemperatureError(*iGuid, m_temperature);
						}

						pListResult->insert(prefixMsg + cli::framework::SUCCESS_MSG);
					}
				}
				catch (wbem::exception::NvmExceptionLibError &e)
				{
					cli::framework::ErrorResult *pError = ieNvmExceptionToResult(e);
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
				catch (wbem::framework::Exception &e)
				{
					framework::ErrorResult *pError = NvmExceptionToResult(e);
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

/*
 * try parsing the string str to an int.  If it succeeds return true and put the int value into
 * p_value. Else return false;
 */
bool cli::nvmcli::ValidationFeature::tryParseInt(const std::string& str, NVM_UINT64* p_value) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool result = false;
	std::istringstream stream(str);

	stream >> *p_value; // try to read the number
	stream >> std::ws; // eat any whitespace

	if (!stream.fail() && stream.eof())
	{
        	result = true;
	}
	return result;
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
		m_errorType = ERROR_TYPE_TEMPERATURE;
		if (m_clearStateExists)
		{
			char errbuff[NVM_ERROR_LEN];
			s_snprintf(errbuff, NVM_ERROR_LEN,
					TRS(NOTSUPPORTED_ERROR_STR));
			pResult = new framework::ErrorResult(framework::ResultBase::ERRORCODE_UNKNOWN,
					errbuff);
		}
		else
		{
			NVM_UINT64 temperatureintValue;
			if (!tryParseInt(temperaturePropertyValue, &temperatureintValue)) // it must be an int
			{
				pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
					TEMPERATURE_PROPERTYNAME, temperaturePropertyValue);
			}
			m_temperature = (NVM_UINT16) temperatureintValue;
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
		m_errorType = ERROR_TYPE_POISON;
		NVM_UINT64 dpaIntValue;
		if (!tryParseInt(poisonPropertyValue, &dpaIntValue)) // it must be an int
		{
			pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
					POISON_PROPERTYNAME, poisonPropertyValue);
		}
		m_poison = (NVM_UINT64) dpaIntValue;
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
		if ((!m_temperatureExists && !m_poisonExists) ||
		   (m_temperatureExists && m_poisonExists))
		{
			pResult = new framework::SyntaxErrorResult(TRS(NOMODIFIABLEPROPERTY_ERROR_STR));
		}
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
						m_poison, m_dimmGuid.c_str());
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
