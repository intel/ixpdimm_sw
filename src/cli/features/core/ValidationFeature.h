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

#ifndef _CLI_NVMCLI_VALIDATIONFEATURE_H_
#define _CLI_NVMCLI_VALIDATIONFEATURE_H_

#include <libinvm-cli/FeatureBase.h>
#include <nvm_types.h>
#include <physical_asset/NVDIMMFactory.h>
#include <libinvm-cli/SimpleListResult.h>

namespace cli
{
namespace nvmcli
{

static const std::string SETPOISON_INVALIDPARAMETER_MSG = N_TR("The specified address is not in the persistent memory region.");

static const std::string SETTEMPERATURE_MSG_PREFIX = N_TR("Set temperature on " NVM_DIMM_NAME " %s");
static const std::string SETPOISON_MSG_PREFIX = N_TR("Poison address %llu on " NVM_DIMM_NAME " %s");
static const std::string SETDIESPARING_MSG_PREFIX = N_TR("Trigger die sparing on " NVM_DIMM_NAME " %s");
static const std::string SETSPARECAPACITYALARM_MSG_PREFIX = N_TR("Trigger a spare capacity on " NVM_DIMM_NAME " %s");
static const std::string SETFATALERROR_MSG_PREFIX = N_TR("Create a media fatal error on " NVM_DIMM_NAME " %s");

static const std::string CLEARPOISON_MSG_PREFIX = N_TR("Clear poison of address %llu on " NVM_DIMM_NAME " %s");
static const std::string CLEARTEMPERATURE_MSG_PREFIX = N_TR("Clear injected temperature on " NVM_DIMM_NAME " %s");
static const std::string CLEARDIESPARING_MSG_PREFIX = N_TR("Clear injected die sparing on " NVM_DIMM_NAME " %s");
static const std::string CLEARSPARECAPACITYALARM_MSG_PREFIX = N_TR("Clear injected spare capacity alarm on " NVM_DIMM_NAME " %s");
static const std::string CLEARFATALERROR_MSG_PREFIX = N_TR("Clear injected media fatal error on " NVM_DIMM_NAME " %s");

static std::string CLEAR_PROPERTYNAME = "Clear";
static std::string TEMPERATURE_PROPERTYNAME = "Temperature";
static std::string POISON_PROPERTYNAME = "Poison";
static std::string DIE_SPARING_PROPERTYNAME = "DieSparing";
static std::string SPARE_ALARM_PROPERTYNAME = "SpareAlarm";
static std::string FATAL_MEDIA_ERROR_PROPERTYNAME = "FatalMediaError";

/*!
 * Implements for validation related commands
 */
class NVM_API ValidationFeature : public cli::framework::FeatureBase
{
public:

	/*!
	 * Constructor
	 */
	ValidationFeature();

	/*!
	 *
	 * @param commandSpecId
	 * @param parsedCommand
	 * @return
	 */
	framework::ResultBase * run(const int &commandSpecId,
			const framework::ParsedCommand &parsedCommand);

	/*
	 * Every feature must have this static members for registration
	 */
	void getPaths(cli::framework::CommandSpecList &list); //!< Required for Feature registration
	static const std::string Name; //!< Required for Feature registration

	enum
	{
		INJECT_ERROR,
	};
private:
	wbem::physical_asset::NVDIMMFactory m_DimmProvider;
	framework::ResultBase *injectError(const framework::ParsedCommand &parsedCommand);
	
	/*
	 * member variables for storing parsed information
	 */
	std::string m_dimmUid;
	NVM_UINT16 m_temperature;
	NVM_UINT64 m_poison;
	bool m_clearStateExists;
	bool m_temperatureExists;
	bool m_poisonExists;
	bool m_dieSparingExists;
	bool m_spareAlarmExists;
	bool m_fatalMediaErrorExists;

	/*
	 * Helper for inject error.
	 * Fetches the properties from the parsed command, performs validation, and converts to WBEM
	 * input attributes.
	 * @param parsedCommand
	 * @return NULL if success, SyntaxErrorResult otherwise
	 */
	cli::framework::ResultBase* getInjectErrorAttributes(
		const framework::ParsedCommand& parsedCommand);

	/*
	 * Helper functions to fetch properties from the parsed command
	 * @param parsedCommand
	 * @return NULL if success, SyntaxErrorResult otherwise
	 */
	cli::framework::ResultBase* parseClearProperty(
		const framework::ParsedCommand& parsedCommand);

	cli::framework::ResultBase* parsePoisonProperty(
		const framework::ParsedCommand& parsedCommand);

	cli::framework::ResultBase* parseTemperatureProperty(
		const framework::ParsedCommand& parsedCommand);

	cli::framework::ResultBase* parseDieSparingProperty(
		const framework::ParsedCommand& parsedCommand);

	cli::framework::ResultBase* parseSpareAlarmProperty(
		const framework::ParsedCommand& parsedCommand);

	cli::framework::ResultBase* parseFatalMediaErrorProperty(
		const framework::ParsedCommand& parsedCommand);

	cli::framework::ResultBase* verifyPropertyCount(
		const framework::ParsedCommand& parsedCommand);

	cli::framework::ResultBase* errorIfMoreThanOnePropertyIsModified();

	cli::framework::ResultBase* checkClearState();

	cli::framework::ResultBase* verifySWTriggerPropertyValue(
		const std::string& propValue, std::string &propertyName);

	/*
	 * Error injection specific exception handling
	 */
	framework::ErrorResult *ieNvmExceptionToResult(wbem::framework::Exception &e,
				std::string prefix = "");

	bool tryParseInt (const std::string& str, NVM_UINT64 *p_value) const;
	void inject_error(std::string &prefixMsg,
			std::vector<std::string>::iterator iUid,
			framework::SimpleListResult &listResult);
	void clear_injected_error(std::string &prefixMsg,
			std::vector<std::string>::iterator iUid,
			framework::SimpleListResult &listResult);
};

}
}
#endif // _CLI_NVMCLI_VALIDATIONFEATURE_H_
