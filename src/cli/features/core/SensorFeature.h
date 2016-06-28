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

#ifndef _CLI_NVMCLI_SENSORFEATURE_H_
#define _CLI_NVMCLI_SENSORFEATURE_H_

#include <cr_i18n.h>
#include <libinvm-cli/FeatureBase.h>
#include <support/NVDIMMSensorViewFactory.h>
#include <nvm_types.h>

namespace cli
{
namespace nvmcli
{

static const std::string MODIFYSENSOR_MSG_PREFIX = N_TR("Modify %s settings on " NVM_DIMM_NAME " %s");
static std::string PROPERTY_SENSOR_THRESHOLD = "NonCriticalThreshold"; //!< Property threshold
static std::string PROPERTY_SENSOR_ENABLED = "EnabledState"; //!< Property enabled
static const std::string MODIFY_SENSOR_PROMPT = N_TR(
		"Modify %s settings on " NVM_DIMM_NAME " %s?"); //!< prompt for user if not forced

/*!
 * Implements the CR sensor commands
 */
class NVM_API SensorFeature : public cli::framework::FeatureBase
{
public:

	/*!
	 * Constructor
	 */
	SensorFeature();
	/*!
	 *
	 * @param commandSpecId
	 * @param parsedCommand
	 * @return
	 */
	framework::ResultBase * run(const int &commandSpecId,
			const framework::ParsedCommand &parsedCommand);

	// Every feature must have this static members for registration
	void getPaths(cli::framework::CommandSpecList &list); //!< Required for Feature registration

	static const std::string Name; //!< Required for Feature registration

	enum
	{
		SHOW_SENSOR,
		MODIFY_SENSOR
	};

private:

	wbem::support::NVDIMMSensorFactory m_SensorProvider;
	wbem::support::NVDIMMSensorViewFactory m_SensorViewProvider;
	framework::ResultBase *showSensor(const framework::ParsedCommand &parsedCommand);
	framework::ResultBase *modifySensor(const framework::ParsedCommand &parsedCommand);
	std::string getSensorName(int type);

	bool isValidType(const std::string &type) const;
	bool isSensorTypeModifiable(const std::string &type) const;
	bool tryParseVal (const std::string& str, int *p_value) const;
	bool tryParseVal (const std::string& str, NVM_REAL32 *p_value) const;
	void roundToNearestSixteenth(NVM_REAL32 &val);
	/*
	 * Helper for modify sensor.
	 * Fetches the properties from the parsed command, performs validation, and converts to WBEM
	 * input attributes.
	 * @param parsedCommand
	 * @param sensorType
	 * @param attributes - if successful, returns the WBEM attribute list with new values
	 * @return NULL if success, SyntaxErrorResult otherwise
	 */
	cli::framework::ResultBase* getModifiedSensorAttributes(const framework::ParsedCommand& parsedCommand,
			const int sensorType,
			wbem::framework::attributes_t &attributes);

	/*
	 * Helper for modify sensor.
	 * Convert the CriticalThreshold property value to the correct WBEM attribute and add to attributes.
	 * @param sensorType
	 * @param thresholdProperty - property value passed in via CLI
	 * @param exists - did the user actually pass in this property?
	 * @param attributes - if successful, the WBEM attribute is added
	 * @return NULL if success, SyntaxErrorResult otherwise
	 */
	cli::framework::ResultBase* addModifiedSensorThresholdAttribute(const int sensorType,
			const std::string &thresholdProperty, const bool exists, wbem::framework::attributes_t &attributes);

	/*
	 * Helper for modify sensor.
	 * Convert the EnabledState property value to the correct WBEM attribute and add to attributes.
	 * @param enabledProperty - property value passed into CLI
	 * @param exists - did the user actually pass in this property?
	 * @param attributes - if successful, the WBEM attribute is added
	 * @return NULL if success, SyntaxErrorResult otherwise
	 */
	cli::framework::ResultBase* addModifiedSensorEnabledAttribute(const std::string &enabledProperty,
			const bool exists, wbem::framework::attributes_t &attributes);

	/*
	 * Helper function for formatting uint16 list
	 */
	void formatThresholdList(wbem::framework::Instance &instance, std::string attributeKey);
};

}
}
#endif // _CLI_NVMCLI_SENSORFEATURE_H_
