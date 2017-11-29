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

#include "Sensor.h"
#include <iomanip>
#include <sstream>
#include <limits>

namespace core
{
	namespace device
	{
		namespace sensor
		{
			Sensor::Sensor()
			{
			}

			Sensor::Sensor(const struct sensor sensor) :
				m_sensor(sensor)
			{
				initialize();
			}

			Sensor::~Sensor()
			{
			}

			std::string Sensor::GetName()
			{
				return PROPERTY_SENSOR_TYPE_UNKNOWN;
			}

			std::string Sensor::GetReading()
			{
				std::stringstream currentValue;
				NVM_INT32 scaled = 0;
				NVM_INT32 scaler = 0;
				scaleNumberBaseTen(m_sensor.reading, &scaled, &scaler);
				currentValue << scaled << baseUnit();
				if (scaler > 0)
				{
					currentValue << "* 10 ^" << scaler;
				}
				return currentValue.str();
			}

			std::string Sensor::GetState()
			{
				std::string result;
				switch (m_sensor.current_state)
				{
				case SENSOR_NORMAL:
					result = N_TR("Normal");
					break;
				case SENSOR_CRITICAL:
					result = N_TR("Critical");
					break;
				case SENSOR_FATAL:
					result = N_TR("Fatal");
					break;
				case SENSOR_NONCRITICAL:
					result = N_TR("NonCritical");
					break;
				case SENSOR_UNKNOWN:
				default:
					result = N_TR("Unknown");
					break;
				}
				return result;
			}

			std::string Sensor::GetEnabledState()
			{
				return SENSORENABLEDSTATESTR_NA;
			}

			std::vector<std::string> Sensor::GetSupportedThresholdTypes()
			{
				std::vector<std::string> supportedThresholds;

				if (m_sensor.lower_noncritical_support)
				{
					supportedThresholds.push_back(SENSORTHRESHOLDTYPE_LOWERNONCRITICAL);
				}
				if (m_sensor.upper_noncritical_support)
				{
					supportedThresholds.push_back(SENSORTHRESHOLDTYPE_UPPERNONCRITICAL);
				}
				if (m_sensor.lower_critical_support)
				{
					supportedThresholds.push_back(SENSORTHRESHOLDTYPE_LOWERCRITICAL);
				}
				if (m_sensor.upper_critical_support)
				{
					supportedThresholds.push_back(SENSORTHRESHOLDTYPE_UPPERCRITICAL);
				}
				if (m_sensor.upper_fatal_support)
				{
					supportedThresholds.push_back(SENSORTHRESHOLDTYPE_UPPERFATAL);
				}
				return supportedThresholds;
			}

			bool Sensor::IsLowerCriticalThresholdSupported()
			{
				return !m_sensor.lower_critical_support ? false : true;
			}

			bool Sensor::IsLowerNonCriticalThresholdSupported()
			{
				return !m_sensor.lower_noncritical_support ? false : true;
			}

			bool Sensor::IsUpperCriticalThresholdSupported()
			{
				return !m_sensor.upper_critical_support ? false : true;
			}

			bool Sensor::IsUpperNonCriticalThresholdSupported()
			{
				return !m_sensor.upper_noncritical_support ? false : true;
			}

			bool Sensor::IsUpperFatalThresholdSupported()
			{
				return !m_sensor.upper_fatal_support ? false : true;
			}

			std::vector<std::string> Sensor::GetSettableThresholdTypes()
			{
				std::vector<std::string> settableThresholds;
				if (m_sensor.lower_noncritical_settable)
				{
					settableThresholds.push_back(SENSORTHRESHOLDTYPE_LOWERNONCRITICAL);
				}
				if (m_sensor.upper_noncritical_settable)
				{
					settableThresholds.push_back(SENSORTHRESHOLDTYPE_UPPERNONCRITICAL);
				}
				return settableThresholds;
			}

			std::string Sensor::GetLowerCriticalThreshold()
			{
				return SSTR(m_sensor.settings.lower_critical_threshold);
			}

			std::string Sensor::GetLowerNonCriticalThreshold()
			{
				return SSTR(m_sensor.settings.lower_noncritical_threshold);
			}

			std::string Sensor::GetUpperCriticalThreshold()
			{
				return SSTR(m_sensor.settings.upper_critical_threshold);
			}

			std::string Sensor::GetUpperNonCriticalThreshold()
			{
				return SSTR(m_sensor.settings.upper_noncritical_threshold);
			}

			std::string Sensor::GetUpperFatalThreshold()
			{
				return SSTR(m_sensor.settings.upper_fatal_threshold);
			}

			std::string Sensor::baseUnit()
			{
				return "";
			}

			void Sensor::initialize()
			{
				m_reading = m_sensor.reading;
			}

			/*
			* Helper function to scale a 64 bit number to "fit" into two 32 bit return
			* values such that:
			* num = pscaled * (10 ^ pscaler)
			*/
			void Sensor::scaleNumberBaseTen(NVM_UINT64 num, NVM_INT32 *pscaled,
				NVM_INT32 *pscaler)
			{
				*pscaler = 0;
				while (num > ((NVM_UINT64)std::numeric_limits<NVM_INT32>::max()))
				{
					num /= 10;
					(*pscaler)++;
				}
				*pscaled = (NVM_INT32)num;
			}
		}
	}
}
