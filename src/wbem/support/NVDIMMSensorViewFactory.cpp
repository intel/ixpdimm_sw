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
 * This file contains the provider for the NVDIMMSensorView instances
 * which is an internal only sensor view used by the CLI show -sensor command.
 */

#include <LogEnterExit.h>
#include "NVDIMMSensorViewFactory.h"
#include "NVDIMMSensorFactory.h"
#include <intel_cim_framework/Attribute.h>
#include <guid/guid.h>
#include <intel_cim_framework/ObjectPath.h>
#include <intel_cim_framework/ExceptionBadParameter.h>
#include <physical_asset/NVDIMMViewFactory.h>
#include <sstream>
#include <iomanip>
#include <exception/NvmExceptionLibError.h>

wbem::support::NVDIMMSensorViewFactory::NVDIMMSensorViewFactory()
throw (wbem::framework::Exception)
{ }

wbem::support::NVDIMMSensorViewFactory::~NVDIMMSensorViewFactory()
{ }

void wbem::support::NVDIMMSensorViewFactory::populateAttributeList(framework::attribute_names_t &attributes)
		throw (wbem::framework::Exception)
{
	// add key attribute
	attributes.push_back(DEVICEID_KEY);

	// Add the View attributes needed by the CLI
	attributes.push_back(TYPE_KEY);
	attributes.push_back(DIMMID_KEY); // Id is what gets displayed in CLI
	attributes.push_back(DIMMGUID_KEY); // GUID is for filtering
	attributes.push_back(DIMMHANDLE_KEY); // Handle is for filtering
	attributes.push_back(CURRENTVALUE_KEY);
	attributes.push_back(CURRENTSTATE_KEY);
	attributes.push_back(LOWERTHRESHOLDCRITICAL_KEY);
	attributes.push_back(UPPERTHRESHOLDCRITICAL_KEY);
	attributes.push_back(SETTABLETHRESHOLDS_KEY);
	attributes.push_back(SUPPORTEDTHRESHOLDS_KEY);
	attributes.push_back(ENABLEDSTATE_KEY);
}

std::string wbem::support::NVDIMMSensorViewFactory::getThresholdTypeStr(int threshold)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string result;
	if (threshold == SENSOR_LOWER_CRITICAL_THRESHOLD)
	{
		result = SENSORTHRESHOLDTYPE_LOWERCRITICAL;
	}
	else if (threshold == SENSOR_UPPER_CRITICAL_THRESHOLD)
	{
		result = SENSORTHRESHOLDTYPE_UPPERCRITICAL;
	}
	else
	{
		result = SENSORTHRESHOLDTYPE_UNKNOWN;
	}
	return result;
}

std::string wbem::support::NVDIMMSensorViewFactory::getEnabledStateStr(int state)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string result;
	if (state == SENSOR_ENABLEDSTATE_ENABLED)
	{
		result = SENSORENABLEDSTATESTR_ENABLED;
	}
	else if (state == SENSOR_ENABLEDSTATE_DISABLED)
	{
		result = SENSORENABLEDSTATESTR_DISABLED;
	}
	else if (state == SENSOR_ENABLEDSTATE_NA)
	{
		result = wbem::NA;
	}
	else
	{
		result = SENSORENABLEDSTATESTR_UNKNOWN;
	}
	return result;
}
/*
 * Based on the WBEM sensor type get the CLI appropriate name.
 */
std::string wbem::support::NVDIMMSensorViewFactory::getSensorNameStr(int type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string result;
	// can't use switch/case here because SENSORTYPE_... aren't constant expressions
	if (type ==  wbem::support::NVDIMMSensorFactory::SENSORTYPE_MEDIATEMPERATURE)
	{
	   result = PROPERTY_SENSOR_TYPE_MEDIATEMP;
	}
	else if (type ==  wbem::support::NVDIMMSensorFactory::SENSORTYPE_SPARECAPACITY)
	{
	   result = PROPERTY_SENSOR_TYPE_SPARE;
	}
	else if (type ==  wbem::support::NVDIMMSensorFactory::SENSORTYPE_WEARLEVEL)
	{
	   result = PROPERTY_SENSOR_TYPE_WEAR;
	}
	else if (type ==  wbem::support::NVDIMMSensorFactory::SENSORTYPE_POWERCYCLES)
	{
	   result = PROPERTY_SENSOR_TYPE_POWERCYCLES;
	}
	else if (type ==  wbem::support::NVDIMMSensorFactory::SENSORTYPE_POWERONTIME)
	{
	   result = PROPERTY_SENSOR_TYPE_POWERON;
	}
	else if (type ==  wbem::support::NVDIMMSensorFactory::SENSORTYPE_UNSAFESHUTDOWNS)
	{
	   result = PROPERTY_SENSOR_TYPE_UNSAFESHUTDOWNS;
	}
	else if (type == wbem::support::NVDIMMSensorFactory::SENSORTYPE_FWERRORLOGCOUNT)
	{
		result = PROPERTY_SENSOR_TYPE_FWERRORLOGCOUNT;
	}
	else if (type ==  wbem::support::NVDIMMSensorFactory::SENSORTYPE_UPTIME)
	{
	   result = PROPERTY_SENSOR_TYPE_UPTIME;
	}
	else if (type == wbem::support::NVDIMMSensorFactory::SENSORTYPE_POWERLIMITED)
	{
		result = PROPERTY_SENSOR_TYPE_POWERLIMITED;
	}
	else if (type == wbem::support::NVDIMMSensorFactory::SENSORTYPE_MEDIAERRORS_UNCORRECTABLE)
	{
	   result = PROPERTY_SENSOR_TYPE_MEDIAERRORS_UNCORRECTABLE;
	}
	else if (type == wbem::support::NVDIMMSensorFactory::SENSORTYPE_MEDIAERRORS_CORRECTED)
	{
	   result = PROPERTY_SENSOR_TYPE_MEDIAERRORS_CORRECTED;
	}
	else if (type == wbem::support::NVDIMMSensorFactory::SENSORTYPE_MEDIAERRORS_ERASURECODED)
	{
	   result = PROPERTY_SENSOR_TYPE_MEDIAERRORS_ERASURECODED;
	}
	else if (type == wbem::support::NVDIMMSensorFactory::SENSORTYPE_WRITECOUNT_MAXIMUM)
	{
	   result = PROPERTY_SENSOR_TYPE_WRITECOUNT_MAXIMUM;
	}
	else if (type == wbem::support::NVDIMMSensorFactory::SENSORTYPE_WRITECOUNT_AVERAGE)
	{
	   result = PROPERTY_SENSOR_TYPE_WRITECOUNT_AVERAGE;
	}
	else if (type == wbem::support::NVDIMMSensorFactory::SENSORTYPE_MEDIAERRORS_HOST)
	{
	   result = PROPERTY_SENSOR_TYPE_MEDIAERRORS_HOST;
	}
	else if (type == wbem::support::NVDIMMSensorFactory::SENSORTYPE_MEDIAERRORS_NONHOST)
	{
	   result = PROPERTY_SENSOR_TYPE_MEDIAERRORS_NONHOST;
	}
	else if (type ==  wbem::support::NVDIMMSensorFactory::SENSORTYPE_CONTROLLER_TEMPERATURE)
	{
	   result = PROPERTY_SENSOR_TYPE_CONTROLLERTEMP;
	}
	else
	{
		result = PROPERTY_SENSOR_TYPE_UNKNOWN;
	}
	return result;
}

std::string wbem::support::NVDIMMSensorViewFactory::baseUnitToString(int baseUnit)
{
	std::string value;
	switch(baseUnit)
	{
		case UNIT_CELSIUS:
			value = " C";
			break;
		case UNIT_SECONDS:
			value = " Sec";
			break;
		case UNIT_MINUTES:
			value = " Min";
			break;
		case UNIT_HOURS:
			value = " Hrs";
			break;
		case UNIT_PERCENT:
			value = "%";
			break;
		default:
			value = "";
			break;
	}
	return value;
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::support::NVDIMMSensorViewFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		// Get the sensorViewAttributes
		checkAttributes(attributes);

		framework::Attribute attribute = path.getKeyValue(DEVICEID_KEY);

		std::string guidStr;
		enum sensor_type type;
		if(!NVDIMMSensorFactory::splitDeviceIdAttribute(attribute, guidStr, (int &)type))
		{
			throw framework::ExceptionBadParameter(DEVICEID_KEY.c_str());
		}

		NVM_GUID guid;
		str_to_guid(guidStr.c_str(), guid);

		struct sensor sensor;
		int rc = NVM_SUCCESS;
		if ((rc = nvm_get_sensor(guid, type, &sensor)) != NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(rc);
		}

		// DimmID = handle or guid depending on user selection
		if (containsAttribute(DIMMID_KEY, attributes))
		{
				framework::Attribute attrDimmId = physical_asset::NVDIMMViewFactory::guidToDimmIdAttribute(guidStr);
				pInstance->setAttribute(DIMMID_KEY, attrDimmId, attributes);
		}
		// DimmGUID
		if (containsAttribute(DIMMGUID_KEY, attributes))
		{
			framework::Attribute attrDimmHandle(guidStr, false);
			pInstance->setAttribute(DIMMGUID_KEY, attrDimmHandle, attributes);
		}
		// DimmHandle = NFIT Handle
		if (containsAttribute(DIMMHANDLE_KEY, attributes))
		{
			NVM_UINT32 handle;
			physical_asset::NVDIMMViewFactory::guidToHandle(guidStr, handle);
			framework::Attribute attrDimmHandle(handle, false);
			pInstance->setAttribute(DIMMHANDLE_KEY, attrDimmHandle, attributes);
		}
		if (containsAttribute(TYPE_KEY, attributes))
		{
			framework::Attribute a(getSensorNameStr(type), false);
			pInstance->setAttribute(TYPE_KEY, a, attributes);
		}
		if (containsAttribute(CURRENTVALUE_KEY, attributes))
		{
			std::stringstream currentValue;
			if (sensor.units == UNIT_SECONDS)
			{
				// convert to HH:MM:SS, note HH can grow beyond 2 digits which is fine.
				NVM_UINT64 hours, minutes, seconds, remainder = 0;
				// 60 seconds in a minute, 60 minutes in an hour
				hours = sensor.reading / (60*60);
				remainder = sensor.reading % (60*60);
				minutes = remainder / 60;
				seconds = remainder % 60;
				currentValue << std::setfill('0') << std::setw(2) << hours << ":";
				currentValue << std::setfill('0') << std::setw(2) << minutes << ":";
				currentValue << std::setfill('0') << std::setw(2) << seconds;
			}
			else if (sensor.type == SENSOR_MEDIA_TEMPERATURE || sensor.type == SENSOR_CONTROLLER_TEMPERATURE)
			{
				float celsius = nvm_decode_temperature(sensor.reading);
				currentValue << celsius << baseUnitToString(sensor.units);
			}
			else
			{
				NVM_INT32 scaled = 0;
				NVM_INT32 scaler = 0;
				NVDIMMSensorFactory::scaleNumberBaseTen(sensor.reading, &scaled, &scaler);
				currentValue << scaled << baseUnitToString(sensor.units);
				if (scaler > 0)
				{
					currentValue << "* 10 ^" << scaler;
				}
			}

			framework::Attribute a(currentValue.str(), false);
			pInstance->setAttribute(CURRENTVALUE_KEY, a, attributes);
		}
		if (containsAttribute(ENABLEDSTATE_KEY, attributes))
		{
			std::string enabledState;
			if ((sensor.type == SENSOR_MEDIA_TEMPERATURE) || (sensor.type == SENSOR_SPARECAPACITY)
				|| (sensor.type == SENSOR_CONTROLLER_TEMPERATURE))
			{
				enabledState = getEnabledStateStr(
						sensor.settings.enabled ? SENSOR_ENABLEDSTATE_ENABLED : SENSOR_ENABLEDSTATE_DISABLED);
			}
			else
			{
				enabledState = getEnabledStateStr(SENSOR_ENABLEDSTATE_NA);
			}
			framework::Attribute a(enabledState, false);
			pInstance->setAttribute(ENABLEDSTATE_KEY, a, attributes);
		}
		if (containsAttribute(LOWERTHRESHOLDCRITICAL_KEY, attributes))
		{
			framework::Attribute a(sensor.settings.lower_critical_threshold, false);
			pInstance->setAttribute(LOWERTHRESHOLDCRITICAL_KEY, a, attributes);
		}
		if (containsAttribute(UPPERTHRESHOLDCRITICAL_KEY, attributes))
		{
			if (sensor.type == SENSOR_MEDIA_TEMPERATURE || sensor.type == SENSOR_CONTROLLER_TEMPERATURE)
			{
				float celsius = nvm_decode_temperature(sensor.settings.upper_critical_threshold);
				pInstance->setAttribute(UPPERTHRESHOLDCRITICAL_KEY,
						framework::Attribute (celsius, false),
						attributes);
			}
			else
			{
				pInstance->setAttribute(UPPERTHRESHOLDCRITICAL_KEY,
						framework::Attribute (sensor.settings.upper_critical_threshold, false),
						attributes);
			}

		}
		if (containsAttribute(CURRENTSTATE_KEY, attributes))
		{
			framework::Attribute a(NVDIMMSensorFactory::getSensorStateStr(sensor.current_state), false);
			pInstance->setAttribute(CURRENTSTATE_KEY, a, attributes);
		}
		if (containsAttribute(SUPPORTEDTHRESHOLDS_KEY, attributes))
		{
			framework::STR_LIST supportedThresholds;
			if (sensor.lower_critical_support)
			{
				supportedThresholds.push_back(getThresholdTypeStr(SENSOR_LOWER_CRITICAL_THRESHOLD));
			}
			if (sensor.upper_critical_support)
			{
				supportedThresholds.push_back(getThresholdTypeStr(SENSOR_UPPER_CRITICAL_THRESHOLD));
			}
			framework::Attribute a(supportedThresholds, false);
			pInstance->setAttribute(SUPPORTEDTHRESHOLDS_KEY, a, attributes);
		}
		if (containsAttribute(SETTABLETHRESHOLDS_KEY, attributes))
		{
			framework::STR_LIST settableThresholds;
			if (sensor.lower_critical_settable)
			{
				settableThresholds.push_back(getThresholdTypeStr(SENSOR_LOWER_CRITICAL_THRESHOLD));
			}
			if (sensor.upper_critical_settable)
			{
				settableThresholds.push_back(getThresholdTypeStr(SENSOR_UPPER_CRITICAL_THRESHOLD));
			}
			framework::Attribute a(settableThresholds, false);
			pInstance->setAttribute(SETTABLETHRESHOLDS_KEY, a, attributes);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		if (pInstance != NULL)
		{
			delete pInstance;
		}
		throw;
	}

	return pInstance;
}

wbem::framework::instance_names_t* wbem::support::NVDIMMSensorViewFactory::getInstanceNames()
		throw (framework::Exception)
{
	return NVDIMMSensorFactory::getNames();
}
