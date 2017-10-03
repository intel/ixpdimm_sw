/*
 * Copyright (c) 2017, Intel Corporation
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

#include "SensorCelsius.h"
#include <iomanip>
#include <sstream>

namespace core
{
	namespace device
	{
		namespace sensor
		{
			SensorCelsius::SensorCelsius()
			{
			}

			SensorCelsius::SensorCelsius(const struct sensor sensor) :
				Sensor(sensor)
			{
			}

			SensorCelsius::~SensorCelsius()
			{
			}

			std::string SensorCelsius::GetReading()
			{
				std::stringstream currentValue;
				currentValue << nvm_decode_temperature(m_sensor.reading) << baseUnit().c_str();
				return currentValue.str();
			}

			std::string SensorCelsius::GetName()
			{
				return PROPERTY_SENSOR_TYPE_UNKNOWN;
			}

			std::string SensorCelsius::GetEnabledState()
			{
				return m_sensor.settings.enabled ? SENSORENABLEDSTATESTR_ENABLED : SENSORENABLEDSTATESTR_DISABLED;
			}

			std::string SensorCelsius::GetLowerCriticalThreshold()
			{
				std::stringstream currentValue;
				currentValue << nvm_decode_temperature(m_sensor.settings.lower_critical_threshold) << baseUnit().c_str();
				return currentValue.str();
			}

			std::string SensorCelsius::GetLowerNonCriticalThreshold()
			{
				std::stringstream currentValue;
				currentValue << nvm_decode_temperature(m_sensor.settings.lower_noncritical_threshold) << baseUnit().c_str();
				return currentValue.str();
			}

			std::string SensorCelsius::GetUpperCriticalThreshold()
			{
				std::stringstream currentValue;
				currentValue << nvm_decode_temperature(m_sensor.settings.upper_critical_threshold) << baseUnit().c_str();
				return currentValue.str();
			}

			std::string SensorCelsius::GetUpperNonCriticalThreshold()
			{
				std::stringstream currentValue;
				currentValue << nvm_decode_temperature(m_sensor.settings.upper_noncritical_threshold) << baseUnit().c_str();
				return currentValue.str();
			}

			std::string SensorCelsius::GetUpperFatalThreshold()
			{
				std::stringstream currentValue;
				currentValue << nvm_decode_temperature(m_sensor.settings.upper_fatal_threshold) << baseUnit().c_str();
				return currentValue.str();
			}

			std::string SensorCelsius::baseUnit()
			{
				return " C";
			}
		}
	}
}
