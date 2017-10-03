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
#ifndef CR_MGMT_SENSOR_H
#define CR_MGMT_SENSOR_H


#include <nvm_management.h>
#include <core/Helper.h>
#include <common_types.h>
#include <system/jedec_manufacturer.h>
#include <utility.h>
#include <nvm_types.h>
#include <string>
#include <vector>
#include <string/s_str.h>
#include <sstream>
#include <core/exceptions/LibraryException.h>
#include <core/Collection.h>
#include <core/NvmLibrary.h>

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

namespace core
{
	namespace device
	{
		namespace sensor
		{
			// Sensor properties unique to thi CLI command
			static std::string CURRENTVALUE_KEY = "CurrentValue"; //!< current value key

			// Sensor strings
			static std::string SENSORENABLEDSTATESTR_ENABLED = N_TR("Enabled");
			static std::string SENSORENABLEDSTATESTR_DISABLED = N_TR("Disabled");
			static std::string SENSORENABLEDSTATESTR_NA = N_TR("N/A");
			static std::string SENSORENABLEDSTATESTR_UNKNOWN = N_TR("Unknown");
			static std::string SENSORTHRESHOLDTYPE_LOWERNONCRITICAL = N_TR("LowerThresholdNonCritical");
			static std::string SENSORTHRESHOLDTYPE_UPPERNONCRITICAL = N_TR("UpperThresholdNonCritical");
			static std::string SENSORTHRESHOLDTYPE_LOWERCRITICAL = N_TR("LowerThresholdCritical");
			static std::string SENSORTHRESHOLDTYPE_UPPERCRITICAL = N_TR("UpperThresholdCritical");
			static std::string SENSORTHRESHOLDTYPE_UPPERFATAL = N_TR("UpperThresholdFatal");
			static std::string SENSORTHRESHOLDTYPE_UNKNOWN = N_TR("Unknown");

			// Sensor properties used by the CLI
			static std::string PROPERTY_SENSOR_TYPE_MEDIATEMP = N_TR("MediaTemperature"); //!< type value
			static std::string PROPERTY_SENSOR_TYPE_SPARE = N_TR("SpareCapacity"); //!< type value
			static std::string PROPERTY_SENSOR_TYPE_WEAR = N_TR("WearLevel"); //!< type value
			static std::string PROPERTY_SENSOR_TYPE_POWERCYCLES = N_TR("PowerCycles"); //!< type value
			static std::string PROPERTY_SENSOR_TYPE_POWERON = N_TR("PowerOnTime"); //!< type value
			static std::string PROPERTY_SENSOR_TYPE_UPTIME = N_TR("UpTime"); //!< type value
			static std::string PROPERTY_SENSOR_TYPE_UNSAFESHUTDOWNS = N_TR("UnsafeShutDowns"); //!< type value
			static std::string PROPERTY_SENSOR_TYPE_FWERRORLOGCOUNT = N_TR("FWErrorCount"); //!< type value
			static std::string PROPERTY_SENSOR_TYPE_POWERLIMITED = N_TR("PowerLimited"); //!< type value
			static std::string PROPERTY_SENSOR_TYPE_CONTROLLERTEMP = N_TR("ControllerTemperature"); //!< type value
			static std::string PROPERTY_SENSOR_TYPE_HEALTH = N_TR("Health"); //!< type value
			static std::string PROPERTY_SENSOR_TYPE_UNKNOWN = N_TR("Unknown"); //!< unknown type

			class NVM_API Sensor
			{
			public:
				Sensor();
				Sensor(const struct sensor sensor);
				virtual ~Sensor();
				virtual std::string GetDeviceUid();
				virtual std::string GetReading();
				virtual std::string GetName();
				virtual std::string GetState();
				virtual std::string GetEnabledState();
				virtual std::vector<std::string> GetSupportedThresholdTypes();
				virtual std::vector<std::string> GetSettableThresholdTypes();
				virtual bool IsLowerCriticalThresholdSupported();
				virtual std::string GetLowerCriticalThreshold();
				virtual bool IsLowerNonCriticalThresholdSupported();
				virtual std::string GetLowerNonCriticalThreshold();
				virtual bool IsUpperCriticalThresholdSupported();
				virtual std::string GetUpperCriticalThreshold();
				virtual bool IsUpperNonCriticalThresholdSupported();
				virtual std::string GetUpperNonCriticalThreshold();
				virtual bool IsUpperFatalThresholdSupported();
				virtual std::string GetUpperFatalThreshold();
			protected:
				struct sensor m_sensor;
				virtual std::string baseUnit();
				void scaleNumberBaseTen(NVM_UINT64 num, NVM_INT32 *pscaled,
					NVM_INT32 *pscaler);
			private:
				void initialize();
				NVM_UINT64 m_reading;
			};

		}
	}
}
#endif //CR_MGMT_SENSOR_H
