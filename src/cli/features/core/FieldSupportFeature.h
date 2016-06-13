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
 * This file contains the NVMCLI field support commands.
 */
#ifndef _CLI_NVMCLI_FIELDSUPPORTFEATURE_H_
#define _CLI_NVMCLI_FIELDSUPPORTFEATURE_H_

#include <libinvm-cli/FeatureBase.h>
#include <libinvm-cli/SimpleListResult.h>
#include <libinvm-cli/PropertyListResult.h>
#include <libinvm-cli/ObjectListResult.h>
#include <cr_i18n.h>
#include <support/EventLogFilter.h>
#include <libinvm-cim/Instance.h>
#include <nvm_types.h>

namespace cli
{
namespace nvmcli
{

static const std::string UPDATEFIRMWARE_MSG = N_TR("Load FW on " NVM_DIMM_NAME " %s"); //!< update firmware success message
static const std::string UPDATEFIRMWARE_RESET_MSG = N_TR(", a reboot is required to activate the FW.");
static const std::string UPDATEFIRMWARE_EXAMINE_VALID_MSG = N_TR(
		"Valid"); //!< examine FW valid message
static const std::string UPDATEFIRMWARE_EXAMINE_VALID_WITH_FORCE_MSG = N_TR(
		"Valid|Downgrade (with confirmation or the force option)|Invalid"); //!< examine FW valid message
static const std::string UPDATEFIRMWARE_EXAMINE_INVALID_MSG = N_TR(
		"Invalid"); //!< examine FW invalid message
static const std::string UPDATEFIRMWARE_EXAMINE_NO_VERSION_MSG = N_TR(
		"Unable to retrieve version from FW Image."); //!< examine FW retrieved no FW message
static const std::string UPDATEFIRMWARE_ERROR_MSG = N_TR(
		"Load FW on " NVM_DIMM_NAME " (%s): Error - %s"); //!< update firmware error message
static const std::string DOWNGRADE_FW_PROMPT = TR(
		"Downgrade firmware on " NVM_DIMM_NAME " %s?"); //!< prompt for user if not forced
static const std::string CREATESNAPSHOT_MSG = N_TR(
		"Create support snapshot"); //!< create support snapshot success message
static const std::string DUMPSUPPORT_MSG = N_TR(
		"Dump support data"); //!< dump support success message
static const std::string DELETESUPPORT_MSG = N_TR(
		"Delete support data"); //!< delete support success message
static const std::string SETFWRESULT_MSG = N_TR(
		"Set FW log level to %d on " NVM_DIMM_NAME " %s"); //!< FW log level set message
static const std::string RUNDIAGNOSTIC_MSG = N_TR(
		"Run diagnostic"); //!< run diagnostic success message

// count and order should match wbem layer
static const int NUMDIAGTESTTYPES = 5;
// diagnostic test type CLI input strings
static const std::string CLIDIAGNOSTIC_TEST_QUICK = "Quick"; //!< method parameter for quick health check
static const std::string CLIDIAGNOSTIC_TEST_PLATFORM = "Config"; //!< method parameter for platform config check
static const std::string CLIDIAGNOSTIC_TEST_PM = "PM"; //!< method parameter for PM metadata check
static const std::string CLIDIAGNOSTIC_TEST_SECURITY = "Security"; //!< method parameter for security check
static const std::string CLIDIAGNOSTIC_TEST_SETTING = "FW"; //!< method parameter for settings check

static const std::string CLIDIAGNOSTIC_TESTNAME = "TestName"; //!< diagnostic test name key display string
static const std::string CLIDIAGNOSTIC_LOGENTRYTIMESTAMP = "DateTime"; //!< log entry time stamp
static const std::string CLIDIAGNOSTIC_COMPLETIONSTATE = "State"; //!< diagnostic test completion state
static const std::string CLIDIAGNOSTIC_COMPLETIONMESSAGE = "Message"; //!< diagnostic test completion log messages

static const std::string diagTestTypes[NUMDIAGTESTTYPES] = {
				CLIDIAGNOSTIC_TEST_QUICK,
				CLIDIAGNOSTIC_TEST_PLATFORM,
				CLIDIAGNOSTIC_TEST_PM,
				CLIDIAGNOSTIC_TEST_SECURITY,
				CLIDIAGNOSTIC_TEST_SETTING};

const size_t MAX_EVENTS = 50;

static const std::string PREFERENCE_DIMMID_HANDLE = "HANDLE";
static const std::string PREFERENCE_DIMMID_UID = "UID";
static const std::string PREFERENCE_SIZE_AUTO = "AUTO";
static const std::string PREFERENCE_SIZE_B = "B";
static const std::string PREFERENCE_SIZE_MIB = "MiB"; // Mebibytes
static const std::string PREFERENCE_SIZE_GIB = "GiB"; // Gibibytes
static const std::string PREFERENCE_ENABLED = "1";
static const std::string PREFERENCE_DISABLED = "0";

/*!
 * Implements the CR Field Support Commands.
 */
class NVM_API FieldSupportFeature : public cli::framework::FeatureBase
{

public:

	/*!
	 * Constructor
	 */
	FieldSupportFeature();

	/*!
	 *
	 * @param commandSpecId
	 * @param parsedCommand
	 * @return
	 */
	framework::ResultBase * run(const int &commandSpecId,
			const framework::ParsedCommand &parsedCommand);

	// Every feature must have these static members for registration
	static const std::string Name; //!< Required for Feature registration
	void getPaths(cli::framework::CommandSpecList &list); //!< Required for Feature registration

	enum
	{
		LOGGING,
		UPDATE_FIRMWARE,
		SHOW_PERFORMANCE,
		RUN_DIAGNOSTIC,
		CREATE_SUPPORT_SNAPSHOT,
		DUMP_SUPPORT,
		DELETE_SUPPORT,
		SHOW_VERSION,
		ACK_EVENT,
		SHOW_EVENTS,
		SHOW_PREFERENCES,
		CHANGE_PREFERENCES,
		SHOW_DEVICE_FIRMWARE,
	};

	/*!
	 * API for installing from a URI on a single device
	 * @param deviceUid
	 * @param uri
	 * @param force
	 */
	void (*m_InstallFromPath)(const std::string &deviceUid, const std::string &uri, bool force);

	/*!
	 * API for examining a FW image
	 * @param deviceUid
	 * @param path
	 */
	int (*m_ExamineFwImage)(const std::string &deviceUid, const std::string &path,
			std::string &fwVersion);

	/*!
	 * API for getting list of manageable DIMM Uids
	 * @return
	 * 		If there is an error with the parsed command it will return a syntax error
	 */
	framework::ErrorResult *(*m_getDimms)(
			const framework::ParsedCommand &parsedCommand, std::vector<std::string> &dimms);

	/*!
	 * API for getting namespaces
	 * @return
	 * 		If there is an error with the parsed command it will return a syntax error
	 */
	framework::ErrorResult *(*m_getNamespaces)(
			const framework::ParsedCommand &parsedCommand, std::vector<std::string> &dimms);

	/*!
	 * API for dumping support to a file
	 */
	void (*m_DumpSupport)(const std::string &path);

	/*!
	 * API for deleting support data
	 */
	void (*m_ClearSupport)();

	/*!
	 * API for the WBEM get events function
	 */
	wbem::framework::instances_t *(*m_getEvents)(wbem::support::EventLogFilter &);

	/*!
	 * API for converting a Dimm UID to a DIMM ID string
	 */
	std::string (*m_uidToDimmIdStr)(const std::string &dimmUid)
			throw (wbem::framework::Exception);

	/*
	 * Retrieve a list of supported preferences
	 */
	static std::vector<std::string> getSupportedPreferences();

protected:

	cli::framework::ResultBase *getDeviceFirmwareInstances(
			const framework::ParsedCommand& parsedCommand,
			wbem::framework::instances_t &Instances);

private:

	/*!
	 * A utility function presented to the user, enabling the modification of the log level
	 * of the CR Management software stack.
	 * @param parsedCommand
	 * 		A reference to a structure representing the parsed CLI command
	 * @return
	 * 		A pointer to a ResultBase object, which is the base-class for derived-classes
	 * 		representing success or failure.
	 */
	framework::ResultBase *changeMgmtLogLevel(const framework::ParsedCommand &parsedCommand);
	framework::ResultBase *createSnapshot(const framework::ParsedCommand &parsedCommand);
	framework::ResultBase *updateFirmware(const framework::ParsedCommand &parsedCommand);
	framework::ResultBase *dumpSupport(const framework::ParsedCommand &parsedCommand);
	framework::ResultBase *deleteSupport(const framework::ParsedCommand &parsedCommand);
	framework::ResultBase *showPerformance(const framework::ParsedCommand &parsedCommand);
	framework::ResultBase *showEvents(const framework::ParsedCommand &parsedCommand);

	/*!
	 * Acknowledge an event
	 */
	framework::ResultBase *ackEvent(const framework::ParsedCommand &parsedCommand);

	/*!
	 * Show the Intel DIMM Gen 1 software inventory
	 */
	framework::ResultBase *showVersion(const framework::ParsedCommand &parsedCommand);

	/*!
	 * Run a diagnostic test on NVM-DIMM(s)
	 */
	framework::ResultBase *runDiagnostic(const framework::ParsedCommand &parsedCommand);

	/*!
	 * Wrapper around wbem installFromUri
	 * @param deviceUid
	 * @param uri
	 * @param force
	 */
	static void wbemInstallFromPath(const std::string &deviceUid, const std::string &uri, bool force);

	/*!
	 * Wrapper around wbem examinFwImage
	 * @param deviceUid
	 * @param path
	 * @return
	 */
	static int wbemExamineFwImage(const std::string &deviceUid, const std::string &path,
			std::string &fwVersion);

	/*!
	 * Wrapper around wbem
	 * @param path
	 */
	static void wbemDumpSupport(const std::string &path);

	/*!
	 * Wrapper around wbem
	 */
	static void wbemClearSupport();

	/*!
	 * Method that can be overridden for testing purposes.
	 * Defaults to just calling checkUserResponse above.
	 */
	static bool localpromptUser(std::string promptString);

	/*
	 * Helper functions for the show -events command
	 */
	cli::framework::SyntaxErrorResult *showEvents_getTimeFromOption(
			std::string optionName, cli::framework::ParsedCommand const &parsedCommand, time_t *pTime,
			bool *pOptionExists);
	cli::framework::ErrorResult *showEvents_inputToFilter(
			cli::framework::ParsedCommand const &parsedCommand,
			wbem::support::EventLogFilter &filter);
	cli::framework::ResultBase *showEvents_logEntriesToObjectList(
			wbem::framework::instances_t *pInstances);
	static wbem::framework::instances_t *wbemGetEvents(wbem::support::EventLogFilter &filter);
	bool parseCliDateTime(std::string timeString, tm *pTm);
	bool isInRange(int val, int min, int max);

	/*!
	 * Show the user preferences
	 */
	framework::ResultBase *showPreferences(const framework::ParsedCommand &parsedCommand);

	/*!
	 * Change user preferences
	 */
	framework::ResultBase *changePreferences(const framework::ParsedCommand &parsedCommand);

	bool stringIsNumberInRange(const std::string &str, const int min, const int max);

	static cli::framework::ErrorResult *wbemToCliGetNamespaces(
			const framework::ParsedCommand &parsedCommand, std::vector<std::string> &namespaces);

	/*
	 * Return true if the value is valid for the key
	 */
	bool valueIsValidNumberForKey(const std::string key, const std::string value);

	/*!
	 * Show Device Firmware
	 */
	cli::framework::ResultBase *showDeviceFirmware(const framework::ParsedCommand &parsedCommand);
};

};
}
#endif // _CLI_NVMCLI_FIELDSUPPORTFEATURE_H_
