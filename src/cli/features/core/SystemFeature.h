/*
 * Copyright (c) 2016, Intel Corporation
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
 * This file contains the NVMCLI system related commands.
 */

#ifndef _CLI_NVMCLI_SYSTEMFEATURE_H_
#define _CLI_NVMCLI_SYSTEMFEATURE_H_

#include <nvm_management.h>
#include <cr_i18n.h>

#include <libinvm-cli/FeatureBase.h>
#include <libinvm-cli/ObjectListResult.h>
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include <libinvm-cim/Instance.h>
#include <physical_asset/MemoryTopologyViewFactory.h>
#include "WbemToCli_utilities.h"

#include "MemoryProperty.h"
#include <nvm_types.h>



namespace cli
{
namespace nvmcli
{

static const std::string UNLOCKED_PROPERTYVALUE = "Unlocked"; //!< Unlock Property value
static const std::string DISABLED_PROPERTYVALUE = "Disabled"; //!< Disabled Property value
static const std::string ZERO_PROPERTYVALUE = "0"; //!< Disable property value
static const std::string ONE_PROPERTYVALUE = "1"; //!< Enable property value
static const std::string ERROR_PROPERTYVALUE = "Error"; // !< Error property value
static const std::string WARNING_PROPERTYVALUE = "Warning"; // !< Warning property value
static const std::string INFO_PROPERTYVALUE = "Info"; // !< Info property value
static const std::string DEBUG_PROPERTYVALUE = "Debug"; // !< Debug property value
static const std::string LOCKSTATE_PROPERTYNAME = "LockState"; //!< Lockstate Property name
static const std::string PASSPHRASE_PROPERTYNAME = "Passphrase"; //!< Passphrase Property
static const std::string FIRSTFASTREFRESH_PROPERTYNAME = "FirstFastRefresh"; //!< FirstFastRefresh Property
static const std::string FWLOGLEVEL_PROPERTYNAME = "FwLogLevel"; //!< FwLogLevel Property
static const std::string VIRALPOLICY_PROPERTYNAME = "ViralPolicy"; //!< ViralPolicy Property
static const std::string NEWPASSPHRASE_PROPERTYNAME = "NewPassphrase"; //!< New Passphrase Property
static const std::string CONFIRMPASSPHRASE_PROPERTYNAME = "ConfirmPassphrase"; //!< Confirm Passphrase Property
static const std::string ERASETYPE_PROPERTYNAME = "EraseType"; //!< which type of erase
static const std::string ERASETYPE_PROPERTY_CRYPTO = "crypto"; //!< which type of erase

static const std::string MODIFYDEVICE_MSG = N_TR("Modify " NVM_DIMM_NAME ""); //!< Modify NVM-DIMM message
static const std::string SETFWLOGGING_MSG = N_TR("Set firmware log level on " NVM_DIMM_NAME); //!< Set fw logging message
static const std::string UNLOCK_MSG = N_TR("Unlock " NVM_DIMM_NAME); //!< Unlock passphrase success message
static const std::string CHANGEPASSPHRASE_MSG = "Change passphrase on " NVM_DIMM_NAME; //!< Change passphrase message
static const std::string SETPASSPHRASE_MSG = "Set passphrase on " NVM_DIMM_NAME; //!< Set passphrase message
static const std::string ERRORMSG_SECURITY_PASSPHRASEMISSMATCH = N_TR("'NewPassphrase' and 'ConfirmPassphrase' must match."); //!< Confirm mismatch error message
static const std::string REMOVEPASSPHRASE_MSG = N_TR("Remove passphrase from " NVM_DIMM_NAME); //!< Remove passphrase message
static const std::string REMOVEPASSPHRASE_ALREADYDISABLED_MSG = N_TR("Security is already disabled."); //!< Security already disabled message
static const std::string UNLOCK_ALREADYDISABLED_MSG = N_TR("Security is disabled."); //!< Security already disabled message
static const std::string ERASEDEVICEDATA_MSG = N_TR("Erase " NVM_DIMM_NAME); //!< Unlock passphrase success message
static const std::string ERASE_DEV_PROMPT = N_TR(
		"Erase all data on " NVM_DIMM_NAME " %s?"); //!< prompt for user if not forced
static const std::string MODIFY_DEV_PROMPT = N_TR(
		"Change settings for " NVM_DIMM_NAME " %s?"); //!< prompt for user if not forced
static const std::string PASSPHRASE_FILE_AND_COMMAND_LINE_PARAMS_MSG =
		N_TR("An empty string is expected for the passphrase properties when using the source option.");

static const std::string STRING_PARAM = "string";

static const std::string PASSPHRASE_PROMPT = N_TR("Current passphrase: ");
static const std::string NEW_PASSPHRASE_PROMPT = N_TR("New passphrase: ");
static const std::string CONFIRM_NEW_PASSPHRASE_PROMPT = N_TR("Confirm new passphrase: ");

static const int ERRORCODE_SECURITY_PASSPHRASEMISSMATCH = -1000;   //!< New and Confirm passphrase do not match

/*!
 * Implements the CR show -host command to display host server information.
 */
class NVM_API SystemFeature : public cli::framework::FeatureBase
{
	public:

		/*!
		 * Constructor
		 */
		SystemFeature();

		virtual ~SystemFeature();

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
			SHOW_SYSTEM,
			SHOW_DEVICES,
			MODIFY_DEVICE,
			SET_FW_LOGGING,
			CHANGE_DEVICE_SECURITY,
			CHANGE_DEVICE_PASSPHRASE,
			ENABLE_DEVICE_SECURITY,
			ERASE_DEVICE_DATA,
			SHOW_MEMORYRESOURCES,
			SHOW_SYSTEM_CAPABILITIES,
			SHOW_TOPOLOGY
		};


		/*
		 * Update the WBEM providers.
		 * The parameter must be a dynamically-allocated instance.
		 * The Feature class takes over responsibility for cleaning up this memory.
		 */

		void setTopologyProvider(wbem::physical_asset::MemoryTopologyViewFactory *m_pTopologyProvider);

private:
		framework::ResultBase *showSystem(const framework::ParsedCommand &parsedCommand);
		framework::ResultBase *showDimms(const framework::ParsedCommand &parsedCommand);
		framework::ResultBase *modifyDevice(const framework::ParsedCommand &parsedCommand);
		framework::ResultBase *setFwLogging(const framework::ParsedCommand &parsedCommand);
		framework::ResultBase *setSecurity(const framework::ParsedCommand &parsedCommand);
		framework::ResultBase *changeDevicePassphrase(const framework::ParsedCommand &parsedCommand);
		framework::ResultBase *changeDeviceSecurity(const framework::ParsedCommand &parsedCommand);
		framework::ResultBase *enableDeviceSecurity(const framework::ParsedCommand &parsedCommand);
		framework::ResultBase *eraseDeviceData(const framework::ParsedCommand &parsedCommand);
		framework::ResultBase *showMemoryResources(const framework::ParsedCommand &parsedCommand);
		framework::ResultBase *showSystemCapabilities(const framework::ParsedCommand &parsedCommand);
		framework::ResultBase *showTopology(const framework::ParsedCommand &parsedCommand);
		void convertCapabilitiesInstance(const wbem::framework::Instance &wbemInstance,
					wbem::framework::Instance &displayInstance,
					const wbem::framework::attribute_names_t &displayAttributes);
		void generateJobFilter(const cli::framework::ParsedCommand &parsedCommand,
				wbem::framework::attribute_names_t &attributes, cli::nvmcli::filters_t &filters);
		void updateLastShutDownStatus(wbem::framework::Instance &instance);
		void convertSecurityCapabilities(wbem::framework::Instance &wbemInstance);
		void updateLastShutDownTime(wbem::framework::Instance &instance);
		void convertSystemOpStatusToSku(wbem::framework::Instance& instance);
		void displayUnknownIfDriverReportsNoBlockSizes(wbem::framework::Instance &wbemInstance);

		/*
		 * Helper routine to convert a logLevel string to fw_log_level enum
		 */
		enum fw_log_level logLevelStringToEnum(std::string logLevel);

		/*
		 * helper method to read passphrases from passphrase file
		 */
		enum return_code readPassphrases(std::string passphraseFile,
				std::string *pPassphrase, std::string *pNewPassphrase);

		wbem::physical_asset::MemoryTopologyViewFactory *m_pTopologyProvider;

		enum return_code setFirstPassphrase(std::string *pPassphrase,
				std::string newValue);

		enum return_code getPassphrasesFromString(std::string lineStr,
				std::string *pPassphrase, std::string *pNewPassphrase);

		/*
		 * pPassphrase is optional - expect NULL if not used
		 */
		cli::framework::ResultBase *getPassphraseProperties(
				const framework::ParsedCommand &parsedCommand,
				const std::string &basePrefix, const std::vector<std::string> &dimms,
				std::string *pPassphrase, std::string &newPassphrase, std::string &confirmPassphrase);

		cli::framework::ResultBase *validateCommandLinePropertiesEmptyWhenUsingPassphraseFile(
				const framework::ParsedCommand &parsedCommand);

		std::string getPassphrasePropertyValueFromCommandLine(
				const std::string &propertyName,
				const framework::ParsedCommand &parsedCommand,
				const std::string &prompt);

		cli::framework::ResultBase *generateErrorResult(
				enum return_code rc, std::string basePrefix, std::vector<std::string> dimms);
		cli::framework::ResultBase *generateErrorResultFromString(
				std::string errorMsg, std::string basePrefix, std::vector<std::string> dimms);
};

}
}
#endif
