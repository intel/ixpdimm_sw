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
 * This file contains the NVMCLI sensor related commands.
 */

#include <math.h>
#include <vector>
#include <string>
#include <sstream>
#include <LogEnterExit.h>

#include <server/BaseServerFactory.h>

#include <libinvm-cli/FeatureBase.h>
#include <libinvm-cli/CommandSpec.h>
#include <libinvm-cli/SimpleListResult.h>
#include <libinvm-cli/PropertyListResult.h>
#include <libinvm-cli/ObjectListResult.h>
#include <libinvm-cli/Parser.h>
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include <libinvm-cli/SyntaxErrorMissingValueResult.h>
#include <physical_asset/NVDIMMFactory.h>

#include "CommandParts.h"
#include "WbemToCli_utilities.h"
#include "SensorFeature.h"

static std::vector<std::string> getWbemSensors()
{
	std::vector<std::string> result;
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_MEDIATEMP);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_CONTROLLERTEMP);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_SPARE);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_WEAR);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_POWERCYCLES);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_POWERON);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_UPTIME);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_UNSAFESHUTDOWNS);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_FWERRORLOGCOUNT);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_POWERLIMITED);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_MEDIAERRORS_UNCORRECTABLE);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_MEDIAERRORS_CORRECTED);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_MEDIAERRORS_ERASURECODED);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_WRITECOUNT_MAXIMUM);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_WRITECOUNT_AVERAGE);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_MEDIAERRORS_HOST);
	result.push_back(wbem::support::PROPERTY_SENSOR_TYPE_MEDIAERRORS_NONHOST);
	return result;
}

/*!
 * The sensor type part of the WBEM Element Name Property. These are used as the
 * Type property for the show sensor command
 */
static const std::vector<std::string> WBEM_SENSOR_ELEMENT_NAMES = getWbemSensors();

const std::string cli::nvmcli::SensorFeature::Name = "Sensor";

/*
 * Command Specs the Example Feature supports
 */
void cli::nvmcli::SensorFeature::getPaths(cli::framework::CommandSpecList &list)
{
	cli::framework::CommandSpec showSensor(SHOW_SENSOR, TR("Show Sensor"), framework::VERB_SHOW,
			TR("Show health statistics for one or more " NVM_DIMM_NAME "s."));
	showSensor.addOption(framework::OPTION_DISPLAY);
	showSensor.addOption(framework::OPTION_ALL);
	showSensor.addTarget(TARGET_SENSOR_R)
			.valueText("MediaTemperature|ControllerTemperature|SpareCapacity|WearLevel|UnsafeShutdowns|"
					"PowerOnTime|UpTime|PowerCycles|FWErrorCount|PowerLimited|"
					"MediaErrorsUncorrectable|MediaErrorsCorrected|MediaErrorsErasureCoded|"
					"WriteCountMax|WriteCountAvg|MediaErrorsHost|MediaErrorsNonHost")
			.helpText(TR("Restrict output to a specific sensor type by supplying the name. "
					"The default is to display all sensors."));
	showSensor.addTarget(TARGET_DIMM)
			.isValueRequired(true);

	cli::framework::CommandSpec modifySensor(MODIFY_SENSOR, TR("Change Sensor Settings"), framework::VERB_SET,
			TR("Change the critical threshold or enabled state for one or more " NVM_DIMM_NAME "s sensors. "
				"Use the Show Sensor command to view the current settings."));
	modifySensor.addOption(framework::OPTION_FORCE).helpText(
			TR("Changing the sensor settings is a potentially destructive operation which requires confirmation from "
			"the user for each " NVM_DIMM_NAME ". This option suppresses the confirmation."));
	modifySensor.addTarget(TARGET_SENSOR_R)
			.valueText("MediaTemperature|ControllerTemperature|SpareCapacity")
			.isValueRequired(true)
			.helpText(TR("The sensor type to modify."));
	modifySensor.addTarget(TARGET_DIMM)
			.helpText(TR("Modify the sensor settings for specific " NVM_DIMM_NAME "s by providing one or more "
				"comma-separated " NVM_DIMM_NAME " identifiers. The default is to update all manageable " NVM_DIMM_NAME "s."))
			.isValueRequired(true);
	modifySensor.addProperty(PROPERTY_SENSOR_THRESHOLD, false, "value", true,
			TR("The upper (for temperature) or lower (for spare capacity) critical threshold of the "
				"sensor. If the current value of the sensor is outside of this value, "
				"the sensor will indicate a \"Critical\" state."));
	modifySensor.addProperty(PROPERTY_SENSOR_ENABLED, false, "0|1", true,
			TR("Enable or disable the critical threshold alarm."));

	list.push_back(showSensor);
	list.push_back(modifySensor);
}

// Constructor, just calls super class 
cli::nvmcli::SensorFeature::SensorFeature() : cli::framework::FeatureBase()
{ }

/*
 * Run the appropriate SensorFeature command
 */
cli::framework::ResultBase * cli::nvmcli::SensorFeature::run(
		const int &commandSpecId, const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	switch(commandSpecId)
	{
	case SHOW_SENSOR:
		pResult = showSensor(parsedCommand);
		break;
	case MODIFY_SENSOR:
		pResult = modifySensor(parsedCommand);
		break;
	}

	return pResult;

}



/*
 * Show sensors provided by the NVDIMMSensorFactory::getInstances function.  If 
 * a device UID or a sensor type is provided, then filter the results.
 */
cli::framework::ResultBase* cli::nvmcli::SensorFeature::showSensor(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::vector<std::string> dimms;
	pResult = cli::nvmcli::getDimms(parsedCommand, dimms);

	if (NULL == pResult)
	{
		try
		{
			// define default display attributes
			wbem::framework::attribute_names_t defaultAttributes;
			defaultAttributes.push_back(wbem::DIMMID_KEY);
			defaultAttributes.push_back(wbem::TYPE_KEY);
			defaultAttributes.push_back(wbem::support::CURRENTVALUE_KEY);
			defaultAttributes.push_back(wbem::CURRENTSTATE_KEY);

			// define all display attributes
			wbem::framework::attribute_names_t allAttributes(defaultAttributes);
			allAttributes.push_back(wbem::LOWERTHRESHOLDNONCRITICAL_KEY);
			allAttributes.push_back(wbem::UPPERTHRESHOLDNONCRITICAL_KEY);
			allAttributes.push_back(wbem::LOWERTHRESHOLDCRITICAL_KEY);
			allAttributes.push_back(wbem::UPPERTHRESHOLDCRITICAL_KEY);
			allAttributes.push_back(wbem::UPPERTHRESHOLDFATAL_KEY);
			allAttributes.push_back(wbem::SETTABLETHRESHOLDS_KEY);
			allAttributes.push_back(wbem::SUPPORTEDTHRESHOLDS_KEY);
			allAttributes.push_back(wbem::ENABLEDSTATE_KEY);

			// get user specified attributes
			wbem::framework::attribute_names_t displayAttributes =
					GetAttributeNames(parsedCommand.options, defaultAttributes, allAttributes);

			// make sure we have the dimm id and type in our display
			// this would cover the case the user asks for specific display attributes, but they
			// don't include the dimm ID or type

			if (!wbem::framework_interface::NvmInstanceFactory::
					containsAttribute(wbem::TYPE_KEY, displayAttributes))
			{
				displayAttributes.insert(displayAttributes.begin(), wbem::TYPE_KEY);
			}
			if (!wbem::framework_interface::NvmInstanceFactory::
					containsAttribute(wbem::DIMMID_KEY, displayAttributes))
			{
				displayAttributes.insert(displayAttributes.begin(), wbem::DIMMID_KEY);
			}

			// read the targets
			std::string dimmTarget = cli::framework::Parser::getTargetValue(parsedCommand, TARGET_DIMM.name);
			std::string sensorTargetValue = cli::framework::Parser::getTargetValue(parsedCommand,
					TARGET_SENSOR_R.name);

			if(!sensorTargetValue.empty() && !isValidType(sensorTargetValue))
			{
				pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_TARGET,
									TARGET_SENSOR_R.name, sensorTargetValue);
			}
			else
			{
				// If the user is requesting a specific type of sensor we need to include
				// the type attribute when we get the instances
				wbem::framework::attribute_names_t requestedAttributes = displayAttributes;
				if (!sensorTargetValue.empty())
				{
					requestedAttributes.push_back(wbem::TYPE_KEY);
				}
				if (!dimmTarget.empty())
				{
					requestedAttributes.push_back(wbem::DIMMUID_KEY);
					requestedAttributes.push_back(wbem::DIMMHANDLE_KEY);
				}

				// get all NVDIMMSensor CIM instances
				wbem::support::NVDIMMSensorViewFactory provider;
				wbem::framework::instances_t * pInstances = provider.getInstances(requestedAttributes);

				if (pInstances)
				{
					// create the display filters
					cli::nvmcli::filters_t filters;
					wbem::framework::attribute_names_t empty;
					generateDimmFilter(parsedCommand, empty, filters);
					if (!sensorTargetValue.empty())
					{
						struct instanceFilter sensorFilter;
						sensorFilter.attributeName = wbem::TYPE_KEY;
						sensorFilter.attributeValues.push_back(sensorTargetValue);
						filters.push_back(sensorFilter);
					}
					// format the instances using the attributes list and filter. Using CLIDIMMID key
					// as the unique attribute
					pResult = NvmInstanceToObjectListResult(*pInstances, "Sensor",
							wbem::DIMMID_KEY, displayAttributes, filters);
					delete pInstances;

					// validate if user supplied a dimm target that it is valid
					if (!dimmTarget.empty() && ((framework::ObjectListResult *)pResult)->getCount() == 0)
					{
						if (NULL != pResult)
						{
							delete pResult;
						}
						pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_TARGET,
								TARGET_DIMM.name, dimmTarget);
					}
					// Set layout to table unless the -all or -display option is present
					else if (!framework::parsedCommandContains(parsedCommand, framework::OPTION_DISPLAY) &&
						!framework::parsedCommandContains(parsedCommand, framework::OPTION_ALL))
					{
						pResult->setOutputType(framework::ResultBase::OUTPUT_TEXTTABLE);
					}
				}
				// failures will throw an exception, this would prevent a crash due to an unexpected error
				else
				{
					pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
						TRS(nvmcli::UNKNOWN_ERROR_STR));
				}
			}
		}
		catch (wbem::framework::Exception &e)
		{
			if(pResult)
			{
				delete pResult;
			}
			pResult = NvmExceptionToResult(e);
		}
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::SensorFeature::modifySensor(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string sensorTarget = framework::Parser::getTargetValue(parsedCommand, TARGET_SENSOR.name);

	// DIMM targets
	bool dimmTargetExists = false;
	std::string dimmTarget = framework::Parser::getTargetValue(parsedCommand, TARGET_DIMM.name, &dimmTargetExists);

	// sensor target value must exist
	if (sensorTarget.empty())
	{
		pResult = new framework::SyntaxErrorMissingValueResult(framework::TOKENTYPE_TARGET,
				TARGET_SENSOR.name);
	}
	// only temperature and spare can be changed
	else if (!isSensorTypeModifiable(sensorTarget))
	{
		pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_TARGET,
				TARGET_SENSOR.name, sensorTarget);
	}
	// dimm target requires a value
	else if (dimmTargetExists && dimmTarget.empty())
	{
		pResult = new framework::SyntaxErrorMissingValueResult(framework::TOKENTYPE_TARGET,
						TARGET_DIMM.name);
	}
	// No obvious syntax errors
	else
	{
		// Validate the DIMM list and translate PIDs into UIDs
		std::vector<std::string> uids;
		std::string hostname; // Get the host name as well - we'll need it later
		pResult = cli::nvmcli::getDimms(parsedCommand, uids);
		try
		{
			hostname = wbem::server::getHostName();
		}
		catch (wbem::framework::Exception &e)
		{
			if (NULL != pResult)
			{
				delete pResult;
			}
			pResult = NvmExceptionToResult(e);
		}

		// No error found in DIMM list
		if (pResult == NULL)
		{
			int wbemSensorType;

			if (framework::stringsIEqual(sensorTarget, wbem::support::PROPERTY_SENSOR_TYPE_MEDIATEMP))
			{
				wbemSensorType = wbem::support::NVDIMMSensorFactory::SENSORTYPE_MEDIATEMPERATURE;
			}
			else if (framework::stringsIEqual(sensorTarget, wbem::support::PROPERTY_SENSOR_TYPE_CONTROLLERTEMP))
			{
				wbemSensorType = wbem::support::NVDIMMSensorFactory::SENSORTYPE_CONTROLLER_TEMPERATURE;
			}
			else
			{
				wbemSensorType = wbem::support::NVDIMMSensorFactory::SENSORTYPE_SPARECAPACITY;
			}

			// add the updated attributes from the properties
			wbem::framework::attributes_t attributes;
			pResult = getModifiedSensorAttributes(parsedCommand, wbemSensorType, attributes);
			if (!pResult) // no error
			{
				framework::SimpleListResult *pListResult = new framework::SimpleListResult();
				pResult = pListResult;
				for(std::vector<std::string>::iterator iUid = uids.begin();
						iUid != uids.end(); iUid ++)
				{
					wbem::framework::Instance *pInstance = NULL;
					wbem::framework::attribute_names_t attrNames;

					std::string prefixMsg = framework::ResultBase::stringFromArgList(
							MODIFYSENSOR_MSG_PREFIX.c_str(), sensorTarget.c_str(),
							wbem::physical_asset::NVDIMMFactory::uidToDimmIdStr(*iUid).c_str());
					prefixMsg += ": ";
					try
					{

						bool forceOption = parsedCommand.options.find(framework::OPTION_FORCE.name)
									!= parsedCommand.options.end();

						// if user didn't specify the force option, prompt them to continue
						std::string prompt = framework::ResultBase::stringFromArgList(
								MODIFY_SENSOR_PROMPT.c_str(), sensorTarget.c_str(),
								wbem::physical_asset::NVDIMMFactory::uidToDimmIdStr(*iUid).c_str());
						if (!forceOption && !promptUserYesOrNo(prompt))
						{
							pListResult->insert(prefixMsg + cli::framework::UNCHANGED_MSG);
						}
						else
						{
							// Get the current instance - wbem uses it to see if anything changed
							wbem::framework::ObjectPath path = m_SensorProvider.getSensorPath(
								wbemSensorType, hostname, *iUid);
							pInstance = m_SensorProvider.getInstance(path, attrNames);
							if (pInstance)
							{
								m_SensorProvider.updateSensor(*iUid, wbemSensorType, attributes,
									pInstance);
								pListResult->insert(prefixMsg + cli::framework::SUCCESS_MSG);

								delete pInstance;
							}
						}
					}
					catch (wbem::framework::Exception &e)
					{
						framework::ErrorResult *pError = NvmExceptionToResult(e);
						pListResult->insert(prefixMsg + pError->outputText());
						pListResult->setErrorCode(pError->getErrorCode());
						if (pInstance)
						{
							delete pInstance;
						}
						delete(pError);
						break;
					}
				}
			}
		}
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::SensorFeature::getModifiedSensorAttributes(
		const framework::ParsedCommand& parsedCommand, const int sensorType,
		wbem::framework::attributes_t &attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	// 2 possible properties: CriticalThreshold and EnabledState
	bool thresholdExists = false; // did the user include CriticalThreshold
	std::string thresholdProperty = framework::Parser::getPropertyValue(parsedCommand,
			PROPERTY_SENSOR_THRESHOLD, &thresholdExists);
	bool enabledExists = false; // did the user include EnabledState
	std::string enabledProperty = framework::Parser::getPropertyValue(parsedCommand,
			PROPERTY_SENSOR_ENABLED, &enabledExists);

	if (!thresholdExists && !enabledExists)
	{
		pResult = new framework::SyntaxErrorResult(TRS(NOMODIFIABLEPROPERTY_ERROR_STR));
	}
	else
	{
		pResult = addModifiedSensorThresholdAttribute(sensorType, thresholdProperty, thresholdExists,
				attributes);
		if (!pResult)
		{
			pResult = addModifiedSensorEnabledAttribute(enabledProperty, enabledExists, attributes);
		}
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::SensorFeature::addModifiedSensorThresholdAttribute(
		const int sensorType, const std::string &thresholdProperty, const bool exists,
		wbem::framework::attributes_t &attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	if (exists)
	{
		if (thresholdProperty.empty())
		{
			pResult = new framework::SyntaxErrorMissingValueResult(framework::TOKENTYPE_PROPERTY,
					PROPERTY_SENSOR_ENABLED);
		}
		else
		{
			std::string key;
			wbem::framework::SINT32 thresholdValue = 0;
			if (sensorType == wbem::support::NVDIMMSensorFactory::SENSORTYPE_MEDIATEMPERATURE ||
				sensorType == wbem::support::NVDIMMSensorFactory::SENSORTYPE_CONTROLLER_TEMPERATURE)
			{
				wbem::framework::REAL32 realThresholdValue = 0;
				if (!stringToReal32(thresholdProperty, &realThresholdValue))
				{
					pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
							PROPERTY_SENSOR_THRESHOLD, thresholdProperty);
				}
				else
				{
					roundToNearestSixteenth(realThresholdValue);
					thresholdValue = wbem::support::NVDIMMSensorFactory::realTempToCimTemp(realThresholdValue);
					key = wbem::UPPERTHRESHOLDNONCRITICAL_KEY;
					attributes[key] = wbem::framework::Attribute(thresholdValue, false);
				}
			}
			else // spare capacity
			{
				if (!stringToInt(thresholdProperty, &thresholdValue))
				{
					pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
							PROPERTY_SENSOR_THRESHOLD, thresholdProperty);
				}
				else
				{
					key = wbem::LOWERTHRESHOLDNONCRITICAL_KEY;
					attributes[key] = wbem::framework::Attribute(thresholdValue, false);
				}
			}
		}
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::SensorFeature::addModifiedSensorEnabledAttribute(
		const std::string &enabledProperty, const bool exists, wbem::framework::attributes_t &attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	if (exists)
	{
		if (enabledProperty.empty())
		{
			pResult = new framework::SyntaxErrorMissingValueResult(framework::TOKENTYPE_PROPERTY,
					PROPERTY_SENSOR_ENABLED);
		}
		else
		{
			wbem::framework::SINT32 enabledValue = 0;
			if (!stringToInt(enabledProperty, &enabledValue))
			{
				pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
						PROPERTY_SENSOR_ENABLED, enabledProperty);
			}
			else
			{
				std::string key = wbem::ENABLEDSTATE_KEY;

				// Need to translate to the WBEM values
				NVM_UINT16 enabledWbemValue = 0;
				if (enabledValue != 0 && enabledValue != 1)
				{
					pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
							PROPERTY_SENSOR_ENABLED, enabledProperty);
				}
				else
				{
					enabledWbemValue = (enabledValue == 0) ?
							wbem::support::SENSOR_ENABLEDSTATE_DISABLED : wbem::support::SENSOR_ENABLEDSTATE_ENABLED;
					attributes[key] = wbem::framework::Attribute(enabledWbemValue, false);
				}
			}
		}
	}

	return pResult;
}

/*
 * Is the sensor type the user asked to filter on a valid type
 */
bool cli::nvmcli::SensorFeature::isValidType(const std::string& type) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool isValid = false;
	for(size_t i = 0; i < WBEM_SENSOR_ELEMENT_NAMES.size() && !isValid; i++)
	{
		isValid = framework::stringsIEqual(WBEM_SENSOR_ELEMENT_NAMES[i], type);
	}
	return isValid;
}

bool cli::nvmcli::SensorFeature::isSensorTypeModifiable(
		const std::string& type) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Case-insensitive
	return (framework::stringsIEqual(type, wbem::support::PROPERTY_SENSOR_TYPE_MEDIATEMP)
		|| framework::stringsIEqual(type, wbem::support::PROPERTY_SENSOR_TYPE_CONTROLLERTEMP)
		|| framework::stringsIEqual(type, wbem::support::PROPERTY_SENSOR_TYPE_SPARE));
}

void cli::nvmcli::SensorFeature::roundToNearestSixteenth(NVM_REAL32 &val)
{
#define ROUNDING_FACTOR 16

	val += ((1.0 / float(ROUNDING_FACTOR)) / 2.0);
	int remainder = floor(val * ROUNDING_FACTOR);
	val = (float)remainder / (float)ROUNDING_FACTOR;
}

void cli::nvmcli::SensorFeature::formatThresholdList(
		wbem::framework::Instance& instance, std::string attributeKey)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::framework::Attribute attribute;
	if (instance.getAttribute(attributeKey, attribute) == wbem::framework::SUCCESS)
	{
		wbem::framework::UINT16_LIST list =
				attribute.uint16ListValue();
		std::stringstream ss;
		for (size_t i = 0; i < list.size(); i++)
		{
			if (i > 0)
			{
				ss << ", ";
			}
			ss  << wbem::support::NVDIMMSensorViewFactory::getThresholdTypeStr(list[i]);
		}

		instance.setAttribute(attributeKey, wbem::framework::Attribute(ss.str(), false));
	}
}
