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

#include "Sensors.h"
#include <iomanip>
#include <sstream>
#include <limits>

namespace core
{
	namespace device
	{
		namespace sensor
		{

			Sensor* SensorFactory::CreateSensor(const struct sensor sensor)
			{
				switch (sensor.type)
				{
				case SENSOR_MEDIA_TEMPERATURE:
					return new SensorMediaTemp(sensor);
				case SENSOR_SPARECAPACITY:
					return new SensorSpareCapacity(sensor);
				case SENSOR_WEARLEVEL:
					return new SensorWearLevel(sensor);
				case SENSOR_POWERCYCLES:
					return new SensorPwrCycles(sensor);
				case SENSOR_POWERONTIME:
					return new SensorPwrOnTime(sensor);
				case SENSOR_UPTIME:
					return new SensorUpTime(sensor);
				case SENSOR_UNSAFESHUTDOWNS:
					return new SensorUnsafeShutdowns(sensor);
				case SENSOR_FWERRORLOGCOUNT:
					return new SensorFwErrorLogCnt(sensor);
				case SENSOR_POWERLIMITED:
					return new SensorPwrLimited(sensor);
				case SENSOR_CONTROLLER_TEMPERATURE:
					return new SensorControllerTemp(sensor);
				case SENSOR_HEALTH:
					return new SensorHealth(sensor);
				default:
					return new Sensor(sensor);
				}
			}

			SensorMediaTemp::SensorMediaTemp()
			{
			}

			SensorMediaTemp::SensorMediaTemp(const struct sensor sensor) :
				SensorCelsius(sensor)
			{
			}

			SensorMediaTemp::~SensorMediaTemp()
			{
			}

			std::string SensorMediaTemp::GetName()
			{
				return PROPERTY_SENSOR_TYPE_MEDIATEMP;
			}

			/*
			*SensorSpareCapacity
			*/
			SensorSpareCapacity::SensorSpareCapacity()
			{
			}

			SensorSpareCapacity::SensorSpareCapacity(const struct sensor sensor) :
				Sensor(sensor)
			{
			}

			SensorSpareCapacity::~SensorSpareCapacity()
			{
			}

			std::string SensorSpareCapacity::GetName()
			{
				return PROPERTY_SENSOR_TYPE_SPARE;
			}

			std::string SensorSpareCapacity::GetEnabledState()
			{
				return m_sensor.settings.enabled ? SENSORENABLEDSTATESTR_ENABLED : SENSORENABLEDSTATESTR_DISABLED;
			}

			std::string SensorSpareCapacity::baseUnit()
			{
				return std::string("%");
			}

			/*
			*SensorWearLevel
			*/
			SensorWearLevel::SensorWearLevel()
			{
			}

			SensorWearLevel::SensorWearLevel(const struct sensor sensor) :
				Sensor(sensor)
			{
			}

			SensorWearLevel::~SensorWearLevel()
			{
			}

			std::string SensorWearLevel::GetName()
			{
				return PROPERTY_SENSOR_TYPE_WEAR;
			}

			std::string SensorWearLevel::baseUnit()
			{
				return std::string("%");
			}

			/*
			*SensorPwrCycles
			*/
			SensorPwrCycles::SensorPwrCycles()
			{
			}

			SensorPwrCycles::SensorPwrCycles(const struct sensor sensor) :
				Sensor(sensor)
			{
			}

			SensorPwrCycles::~SensorPwrCycles()
			{
			}

			std::string SensorPwrCycles::GetName()
			{
				return PROPERTY_SENSOR_TYPE_POWERCYCLES;
			}

			/*
			* SensorPwrOnTime.
			*/
			SensorPwrOnTime::SensorPwrOnTime()
			{
			}

			SensorPwrOnTime::SensorPwrOnTime(const struct sensor sensor) :
				SensorTime(sensor)
			{
			}

			SensorPwrOnTime::~SensorPwrOnTime()
			{
			}

			std::string SensorPwrOnTime::GetName()
			{
				return PROPERTY_SENSOR_TYPE_POWERON;
			}

			/*
			* SensorUpTime.
			*/
			SensorUpTime::SensorUpTime()
			{
			}

			SensorUpTime::SensorUpTime(const struct sensor sensor) :
				SensorTime(sensor)
			{
			}

			SensorUpTime::~SensorUpTime()
			{
			}

			std::string SensorUpTime::GetName()
			{
				return PROPERTY_SENSOR_TYPE_UPTIME;
			}

			/*
			* SensorUnsafeShutdowns.
			*/
			SensorUnsafeShutdowns::SensorUnsafeShutdowns()
			{
			}

			SensorUnsafeShutdowns::SensorUnsafeShutdowns(const struct sensor sensor) :
				Sensor(sensor)
			{
			}

			SensorUnsafeShutdowns::~SensorUnsafeShutdowns()
			{
			}

			std::string SensorUnsafeShutdowns::GetName()
			{
				return PROPERTY_SENSOR_TYPE_UNSAFESHUTDOWNS;
			}

			/*
			* SensorFwErrorLogCnt.
			*/
			SensorFwErrorLogCnt::SensorFwErrorLogCnt()
			{
			}

			SensorFwErrorLogCnt::SensorFwErrorLogCnt(const struct sensor sensor) :
				Sensor(sensor)
			{
			}

			SensorFwErrorLogCnt::~SensorFwErrorLogCnt()
			{
			}

			std::string SensorFwErrorLogCnt::GetName()
			{
				return PROPERTY_SENSOR_TYPE_FWERRORLOGCOUNT;
			}

			/*
			* SensorPwrLimited.
			*/
			SensorPwrLimited::SensorPwrLimited()
			{
			}

			SensorPwrLimited::SensorPwrLimited(const struct sensor sensor) :
				Sensor(sensor)
			{
			}

			SensorPwrLimited::~SensorPwrLimited()
			{
			}

			std::string SensorPwrLimited::GetName()
			{
				return PROPERTY_SENSOR_TYPE_POWERLIMITED;
			}

			/*
			* SensorControllerTemp.
			*/
			SensorControllerTemp::SensorControllerTemp()
			{
			}

			SensorControllerTemp::SensorControllerTemp(const struct sensor sensor) :
				SensorCelsius(sensor)
			{
			}

			SensorControllerTemp::~SensorControllerTemp()
			{
			}

			std::string SensorControllerTemp::GetName()
			{
				return PROPERTY_SENSOR_TYPE_CONTROLLERTEMP;
			}

			/*
			* SensorHealth.
			*/
			SensorHealth::SensorHealth()
			{
			}

			SensorHealth::SensorHealth(const struct sensor sensor) :
				Sensor(sensor)
			{
			}

			SensorHealth::~SensorHealth()
			{
			}

			std::string SensorHealth::GetName()
			{
				return PROPERTY_SENSOR_TYPE_HEALTH;
			}
		}
	}
}