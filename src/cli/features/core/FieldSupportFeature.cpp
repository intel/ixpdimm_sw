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

#include <vector>
#include <string>

#include <string/s_str.h>

#include <LogEnterExit.h>

#include <server/BaseServerFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include <libinvm-cim/ExceptionBadParameter.h>

#include <libinvm-cli/FeatureBase.h>
#include <libinvm-cli/SimpleResult.h>
#include <libinvm-cli/CommandSpec.h>
#include <libinvm-cli/PropertyListResult.h>
#include <libinvm-cli/ObjectListResult.h>
#include <libinvm-cli/SyntaxErrorResult.h>
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include <libinvm-cli/SyntaxErrorMissingValueResult.h>
#include <libinvm-cli/Parser.h>
#include <uid/uid.h>

#include <support/NVDIMMDiagnosticFactory.h>
#include <support/SupportDataServiceFactory.h>
#include <performance/NVDIMMPerformanceViewFactory.h>
#include <software/NVDIMMSoftwareInstallationServiceFactory.h>
#include <software/ManagementSoftwareIdentityFactory.h>
#include <software/NVDIMMDriverIdentityFactory.h>
#include <software/NVDIMMFWVersionFactory.h>
#include <support/DiagnosticCompletionRecordFactory.h>
#include <support/NVDIMMLogEntryFactory.h>
#include <string.h>
#include <support/NVDIMMEventLogFactory.h>
#include "CommandParts.h"
#include "WbemToCli.h"
#include "WbemToCli_utilities.h"
#include "FieldSupportFeature.h"
#include "SystemFeature.h"
#include <persistence/config_settings.h>
#include <persistence/lib_persistence.h>
#include <exception/NvmExceptionLibError.h>
#include <core/device/DeviceFirmwareService.h>
#include <framework_interface/FrameworkExtensions.h>
#include <libinvm-cim/ExceptionNoMemory.h>

#include "ShowVersionCommand.h"

const std::string cli::nvmcli::FieldSupportFeature::Name = "Field Support";
const std::string LOG_PROPERTY_NAME = "LogLevel";

static const std::string NAME_PROPERTY_NAME = "Name";
static const std::string PERFORMANCE_TARGET = "-performance";
static const std::string FWLOGLEVEL_PROPERTY = "FwLogLevel";
static const std::string DIAGNOSTIC_TARGET = "-diagnostic";

const std::string ACTIONREQUIRED_PROPERTY_NAME = "ActionRequired";

static const std::string EVENT_PROPERTY_SEVERITY = "Severity";
static const std::string EVENT_PROPERTY_CATEGORY = "Category";
static const std::string EVENT_OPTION_STARTTIME = "-starttime";
static const std::string EVENT_OPTION_ENDTIME = "-endtime";

/*
 * Command Specs the Example Feature supports
 */
void cli::nvmcli::FieldSupportFeature::getPaths(cli::framework::CommandSpecList &list)
{
	cli::framework::CommandSpec showDeviceFirmware(SHOW_DEVICE_FIRMWARE, TR("Show Device Firmware"),
			framework::VERB_SHOW,
			TR("Show detailed information about the firmware on one or more " NVM_DIMM_NAME "s."));
	showDeviceFirmware.addOption(framework::OPTION_ALL);
	showDeviceFirmware.addOption(framework::OPTION_DISPLAY)
			.helpText(TR("Filter the returned attributes by explicitly specifying a comma separated "
			"list of attributes."));
	showDeviceFirmware.addTarget(TARGET_FIRMWARE_R).isValueAccepted(false);
	showDeviceFirmware.addTarget(TARGET_DIMM.name, true, DIMMIDS_STR, false,
			TR("Restrict output to the firmware information for specific " NVM_DIMM_NAME "s by "
			"supplying one or more comma-separated " NVM_DIMM_NAME " identifiers. The default "
			"is to display the firmware information for all manageable " NVM_DIMM_NAME "s."));

	cli::framework::CommandSpec updateFirmware(UPDATE_FIRMWARE, TR("Update Firmware"), framework::VERB_LOAD,
			TR("Update the firmware on one or more " NVM_DIMM_NAME "s. On the next reboot the firmware will become active."));
	updateFirmware.addOption(framework::OPTION_SOURCE_R)
			.helpText(TR("File path of the firmware image. A firmware image refers to a local file containing "
					"the entire firmware in one package."));
	updateFirmware.addOption(framework::OPTION_FORCE)
			.helpText(TR("Downgrading the firmware to an older version is a potentially destructive "
					"operation which requires confirmation from the user for each " NVM_DIMM_NAME ". This "
					"option supresses the confirmation when attempting to downgrade."));
	updateFirmware.addOption(framework::OPTION_EXAMINE)
			.helpText(TR("Verify the firmware image specified in the source option and return its version "
					"without actually downloading the image to the specified " NVM_DIMM_NAME "(s)."));
	updateFirmware.addTarget(TARGET_DIMM_R)
			.valueText(DIMMIDS_STR)
			.helpText(TR("Update the firmware on specific " NVM_DIMM_NAME "s by supplying one or more comma-separated "
					"" NVM_DIMM_NAME " identifiers. However, this is not recommended as it may put the system in an "
					"undesirable state. The default is to update all manageable " NVM_DIMM_NAME "s."));

	cli::framework::CommandSpec showPerformance(SHOW_PERFORMANCE, TR("Show Device Performance"), framework::VERB_SHOW,
			TR("Show performance metrics for one or more " NVM_DIMM_NAME "s."));
	showPerformance.addTarget(TARGET_DIMM.name, true, DIMMIDS_STR, false,
			TR("Restrict output to the performance metrics for specific " NVM_DIMM_NAME "s by "
					"supplying one or more comma-separated " NVM_DIMM_NAME " identifiers. The default is to "
					"display performance metrics for all manageable " NVM_DIMM_NAME "s."));
	showPerformance.addTarget(PERFORMANCE_TARGET, true,
		"BytesRead|BytesWritten|HostReads|HostWrites|BlockWrites|BlockReads", false,
			TR("Restrict output to a specific performance metric by supplying the metric name. "
					"The default is to display all performance metrics."));

	cli::framework::CommandSpec runDiag(RUN_DIAGNOSTIC, TR("Run Diagnostic"), framework::VERB_START,
			TR("Run a diagnostic test on one or more " NVM_DIMM_NAME "s."));
	runDiag.addTarget(DIAGNOSTIC_TARGET, true, "Quick|Config|PM|Security|FW", false,
			TR("Run a specific test by supplying its name. By default "
					"all tests are run. "));
	runDiag.addTarget(TARGET_DIMM.name, false, DIMMIDS_STR, true,
			TR("Run a diagnostic on specific " NVM_DIMM_NAME "s by supplying one or more comma-separated "
					"" NVM_DIMM_NAME " identifiers. The default is to run the specified tests on all manageable "
					"" NVM_DIMM_NAME "s."));

	cli::framework::CommandSpec createSupport(CREATE_SUPPORT_SNAPSHOT, TR("Create Support Snapshot"), framework::VERB_CREATE,
			TR("Capture a snapshot of the system state for support purposes. The snapshot includes "
					"relevant host configuration data as well as details about each " NVM_DIMM_NAME " and its "
					"configuration. Snapshots are stored in an internal configuration file for inclusion in the "
					"support package generated by the Dump Support Data command. The maximum "
					"number of snapshots that are kept by the software is configurable with the Change "
					"Preferences command. Stored snapshots may be removed with the Delete Support "
					"Data command."));
	createSupport.addTarget(TARGET_SUPPORT.name, true, "", false,
			TR("A support snapshot. No filtering is supported on this target."));
	createSupport.addProperty(NAME_PROPERTY_NAME, false, STRING_PARAM, true,
			TR("Optional user defined name to help identify the snapshot."));

	cli::framework::CommandSpec dumpSupport(DUMP_SUPPORT, TR("Dump Support Data"), framework::VERB_DUMP,
			TR("Dump support data to a file for offline analysis by support personnel. Support "
					"data is encrypted and compressed and includes system log(s), error log(s), trace log(s), captured "
					"support snapshots, sensor information, and diagnostic results."));
	dumpSupport.addOption(framework::OPTION_DESTINATION.name, true, "path", true,
			TR("The file path in which to store the support data. The file is encrypted and compressed."))
			.isValueRequired(true);
	dumpSupport.addTarget(TARGET_SUPPORT.name, true, "", false,
			TR("The support data. No filtering is supported on this command."))
			.isValueAccepted(false);


	cli::framework::CommandSpec deleteSupport(DELETE_SUPPORT, TR("Delete Support Data"), framework::VERB_DELETE,
			TR("Clear support data previously captured with the Create Support Snapshot command."));
	deleteSupport.addTarget(TARGET_SUPPORT.name, true, "", false,
			TR("The collected support data. No filtering is supported on this command."))
			.isValueAccepted(false);

	cli::framework::CommandSpec logging(LOGGING, TR("Toggle Software Logging"), framework::VERB_SET,
			TR("Enable or disable logging in the " NVM_DIMM_NAME " host software. By default, logging is off and "
					"only error messages are captured. Turning on logging may impact the performance of the " NVM_DIMM_NAME " "
					"software so it's recommended only to turn it on to reproduce an issue and then turn it back off."));
	logging.addTarget(TARGET_SYSTEM_R).
			helpText(TR("The " NVM_DIMM_NAME " software logging level. No filtering is supported on this target."));
	logging.addProperty(LOG_PROPERTY_NAME, true, "0|1", true,
			TR("Whether logging is enabled in the " NVM_DIMM_NAME " host software."));

	cli::framework::CommandSpec showVersion(SHOW_VERSION, TR("Version"), framework::VERB_VERSION,
			TR("Show the " NVM_DIMM_NAME " host software inventory."));

	cli::framework::CommandSpec acknowledgeEvent(ACK_EVENT, TR("Acknowledge event"), framework::VERB_SET,
				TR("Turns off the flag that signals a corrective action on the event."));
	acknowledgeEvent.addTarget(TARGET_EVENT_STR, true, EVENTID_STR, true,
				TR("The identifier of the event to be acknowledged."));
	acknowledgeEvent.addProperty(ACTIONREQUIRED_PROPERTY_NAME, true, "false|0", true,
			TR("A flag indicating whether the event needs a corrective action or acknowledgment."));

	cli::framework::CommandSpec showEvents(SHOW_EVENTS, TR("Show Events"), framework::VERB_SHOW,
			TR("Show " NVM_DIMM_NAME " related events. The options, targets, and properties can be used to "
					"filter the events. If no filters are provided, the default is to display up to 50 events."));
	showEvents.addTarget(TARGET_EVENT_STR).isRequired(true)
			.isValueRequired(false)
			.valueText(EVENTID_STR)
			.helpText(TR("Filter output to a single event by supplying the event identifier."));
	showEvents.addTarget(TARGET_DIMM.name).isRequired(false)
			.isValueRequired(true)
			.valueText(DIMMID_STR)
			.helpText(TR("Filter output to events on a specific " NVM_DIMM_NAME " by supplying the dimm target "
					"and one NVMDIMM identifier."));
	showEvents.addTarget(TARGET_NAMESPACE.name).isRequired(false)
			.isValueRequired(true)
			.valueText(NAMESPACEID_STR)
			.helpText(TR("Filter output to events on a specific namespace by supplying the namespace "
					"target and one namespace identifiers."));
	showEvents.addOption(EVENT_OPTION_STARTTIME)
			.isRequired(false)
			.isValueRequired(true)
			.valueText("MM:dd:yyyy:hh:mm:ss")
			.helpText(TR("A vendor specific command option in format: MM:dd:yyyy:hh:mm:ss. Applies a filter to "
					"only include events at or after the specified start time."));
	showEvents.addOption(EVENT_OPTION_ENDTIME)
			.isRequired(false)
			.isValueRequired(true)
			.valueText("MM:dd:yyyy:hh:mm:ss")
			.helpText(TR("A vendor specific command option in format: MM:dd:yyyy:hh:mm:ss. Applies a filter to "
					"only include events at or before the specified end time."));
	showEvents.addProperty(EVENT_PROPERTY_SEVERITY)
			.isRequired(false)
			.isValueRequired(true)
			.valueText("Info|Warn|Critical|Fatal")
			.helpText(TR("Filter output of events based on the severity of the event."));
	showEvents.addProperty(EVENT_PROPERTY_CATEGORY)
			.isRequired(false)
			.isValueRequired(true)
			.valueText("Diag|FW|PlatformConfig|PM|Quick|Security|Health|Mgmt")
			.helpText(TR("Filter output to events of a specific category."));
	showEvents.addProperty(ACTIONREQUIRED_PROPERTY_NAME)
			.isRequired(false)
			.isValueRequired(true)
			.valueText("1|0")
			.helpText(TR("Filter output to events that require corrective action or acknowledgment."));

	cli::framework::CommandSpec showPreferences(SHOW_PREFERENCES, TR("Show Preferences"), framework::VERB_SHOW,
			TR("Display a list of the " NVM_DIMM_NAME " management software user preferences and their current values."));
	showPreferences.addTarget(TARGET_PREFERENCES);

	cli::framework::CommandSpec changePreferences(CHANGE_PREFERENCES, TR("Change Preferences"), framework::VERB_SET,
			TR("Modify one or more user preferences in the " NVM_DIMM_NAME " management software."));
	changePreferences.addTarget(TARGET_PREFERENCES);
	changePreferences.addProperty(SQL_KEY_CLI_DIMM_ID, false, "HANDLE|UID",
			true, "The default display of " NVM_DIMM_NAME " identifiers.");
	changePreferences.addProperty(SQL_KEY_CLI_SIZE, false, "Auto|B|MiB|GiB",
			true, "The default display of capacities in the NVMCLI.");
	changePreferences.addProperty(SQL_KEY_PERFORMANCE_MONITOR_ENABLED, false, "0|1",
			true, "Whether or not the monitor is periodically storing performance metrics "
					"for the " NVM_DIMM_NAME "s in the host server.");
	changePreferences.addProperty(SQL_KEY_PERFORMANCE_MONITOR_INTERVAL_MINUTES, false, "minutes",
			true, "The interval in minutes that the monitor is "
					"storing performance metrics (if enabled). "
					"The default value is 180 minutes and must be > 0.");
	changePreferences.addProperty(SQL_KEY_EVENT_MONITOR_ENABLED, false, "0|1",
			true, "Whether or not the monitor is periodically checking for " NVM_DIMM_NAME " events.");
	changePreferences.addProperty(SQL_KEY_EVENT_MONITOR_INTERVAL_MINUTES, false, "minutes",
			true, "The interval in minutes that the monitor is checking for "
					"" NVM_DIMM_NAME " events (if enabled). "
					"The default value is 1 minute and must be > 0.");
	changePreferences.addProperty(SQL_KEY_EVENT_LOG_MAX, false, "count",
			true, "The maximum number of events to keep in the management software. "
					"The default value is 10000. The valid range is 0-100000.");
	changePreferences.addProperty(SQL_KEY_LOG_MAX, false, "count",
			true, "The maximum number of debug log entries to keep in the management software. "
					"The default value is 10000. The valid range is 0-100000.");
	changePreferences.addProperty(SQL_KEY_SUPPORT_SNAPSHOT_MAX, false, "count",
			true, "The maximum number of support snapshots to keep in the management software. "
					"The default value is 100. The valid range is 0-100.");

	list.push_back(showDeviceFirmware);
	list.push_back(updateFirmware);
	list.push_back(showPerformance);
	list.push_back(runDiag);
	list.push_back(createSupport);
	list.push_back(dumpSupport);
	list.push_back(deleteSupport);
	list.push_back(logging);
	list.push_back(showVersion);
	list.push_back(acknowledgeEvent);
	list.push_back(showEvents);
	list.push_back(showPreferences);
	list.push_back(changePreferences);
}

// Constructor, just calls super class and initializes member variables
cli::nvmcli::FieldSupportFeature::FieldSupportFeature() :
		cli::framework::FeatureBase(),
		m_InstallFromPath(wbemInstallFromPath),
		m_ExamineFwImage(wbemExamineFwImage),
		m_getDimms(getDimms),
		m_getNamespaces(wbemToCliGetNamespaces),
		m_DumpSupport(wbemDumpSupport),
		m_ClearSupport(wbemClearSupport),
		m_getEvents(wbemGetEvents),
		m_uidToDimmIdStr(wbem::physical_asset::NVDIMMFactory::uidToDimmIdStr)
{
}

/*
 * Get all the BaseServer Instances from the wbem base server factory.
 */
cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::run(
		const int &commandSpecId, const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	switch (commandSpecId)
	{
		case LOGGING:
			pResult = changeMgmtLogLevel(parsedCommand);
			break;
		case CREATE_SUPPORT_SNAPSHOT:
			pResult = createSnapshot(parsedCommand);
			break;
		case UPDATE_FIRMWARE:
			pResult = updateFirmware(parsedCommand);
			break;
		case DUMP_SUPPORT:
			pResult = dumpSupport(parsedCommand);
			break;
		case DELETE_SUPPORT:
			pResult = deleteSupport(parsedCommand);
			break;
		case SHOW_PERFORMANCE:
			pResult = showPerformance(parsedCommand);
			break;
		case SHOW_VERSION:
			pResult = showVersion(parsedCommand);
			break;
		case ACK_EVENT:
			pResult = ackEvent(parsedCommand);
			break;
		case RUN_DIAGNOSTIC:
			pResult = runDiagnostic(parsedCommand);
			break;
		case SHOW_EVENTS:
			pResult = showEvents(parsedCommand);
			break;
		case SHOW_PREFERENCES:
			pResult = showPreferences(parsedCommand);
			break;
		case CHANGE_PREFERENCES:
			pResult = changePreferences(parsedCommand);
			break;
		case SHOW_DEVICE_FIRMWARE:
			pResult = showDeviceFirmware(parsedCommand);
			break;
	}
	return pResult;
}

/*
 * A utility function presented to the user, enabling the modification of the log level
 * of the CR Management software stack.
 */
cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::changeMgmtLogLevel(
		const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;
	try
	{
		// since the target "-system" and property "LogLevel" are defined as required for this
		// CommandSpec, there is no need to check them.  However, we must check that the
		// "LogLevel" property has an associated value
		// get the unload property
		bool logPropertyExists = false;
		std::string logPropertyValue =
				cli::framework::Parser::getPropertyValue(parsedCommand, LOG_PROPERTY_NAME, &logPropertyExists);
		wbem::server::BaseServerFactory serverFactory;

		// add simulator
		if (!logPropertyExists || logPropertyValue.empty())
		{
			pResult = new framework::SyntaxErrorMissingValueResult(
					framework::TOKENTYPE_PROPERTY, LOG_PROPERTY_NAME);
		}
		else if (logPropertyValue == "1")
		{
			serverFactory.setDebugLogging(1);
			std::string resultMsg = TR("Enable " NVM_DIMM_NAME " host software debug logging: ");
			resultMsg += TRS(cli::framework::SUCCESS_MSG);
			framework::SimpleListResult *pSimpleList = new framework::SimpleListResult();
			pSimpleList->insert(resultMsg);
			pResult = pSimpleList;
		}
		else if (logPropertyValue == "0")
		{
			serverFactory.setDebugLogging(0);
			std::string resultMsg = TR("Disable " NVM_DIMM_NAME " host software debug logging: ");
			resultMsg += TRS(cli::framework::SUCCESS_MSG);
			framework::SimpleListResult *pSimpleList = new framework::SimpleListResult();
			pSimpleList->insert(resultMsg);
			pResult = pSimpleList;
		}
		else
		{
			pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, LOG_PROPERTY_NAME, logPropertyValue);
		}
	}
	catch (wbem::exception::NvmExceptionLibError &e)
	{
		pResult = NvmExceptionToResult(e);
	}
	return pResult;
}

/*
 * Provide the ability to create a snapshot of the state of the system.
 */
cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::createSnapshot(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;
	wbem::support::SupportDataServiceFactory provider;
	wbem::framework::ObjectPath outOpaqueManagementData;

	std::string support = framework::Parser::getTargetValue(parsedCommand, TARGET_SUPPORT_R.name);
	if (!support.empty())
	{
		pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_TARGET,
				TARGET_SUPPORT_R.name, support);
	}
	else
	{
		bool propertyPresent = false;
		std::string name = cli::framework::Parser::getPropertyValue(parsedCommand, NAME_PROPERTY_NAME, &propertyPresent);
		if (name.empty() && propertyPresent)
		{
			pResult = new framework::SyntaxErrorMissingValueResult(framework::TOKENTYPE_PROPERTY, NAME_PROPERTY_NAME);
		}
		else
		{
			std::string prefix = TRS(CREATESNAPSHOT_MSG);
			try
			{
				provider.create(name, outOpaqueManagementData);
				if (pResult == NULL)
				{
					framework::SimpleListResult *pSimpleList = new framework::SimpleListResult();
					pSimpleList->insert(std::string(prefix + ": " + TRS(cli::framework::SUCCESS_MSG)));
					pResult = pSimpleList;
				}
			}
			catch (wbem::framework::Exception &e)
			{
				if (pResult)
				{
					delete pResult;
				}
				pResult = NvmExceptionToResult(e, prefix);
			}
		}
	}
	return pResult;
}

cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::updateFirmware(
		const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::ResultBase *pResult = NULL;

	framework::StringMap::const_iterator source = parsedCommand.options.find(framework::OPTION_SOURCE.name);
	if (source != parsedCommand.options.end() && !source->second.empty())
	{
		// get path
		std::string path = source->second;

		// get options
		bool forceOption = parsedCommand.options.find(framework::OPTION_FORCE.name)
				!= parsedCommand.options.end();
		bool examineOption = parsedCommand.options.find(framework::OPTION_EXAMINE.name)
				!= parsedCommand.options.end();

		std::vector<std::string> uids;
		pResult = m_getDimms(parsedCommand, uids);

		if (pResult == NULL) // no syntax error from getDimm
		{
			if (examineOption) // examine only
			{
				// used to store results
				framework::SimpleListResult *pSimpleList = new framework::SimpleListResult();

				for (size_t i = 0; i < uids.size(); i++)
				{
					std::string prefix = framework::ResultBase::stringFromArgList(
							TRS(UPDATEFIRMWARE_MSG),
							m_uidToDimmIdStr(uids[i]).c_str());
					prefix += ": ";
					try
					{
						std::string fwVersion;
						int rc = m_ExamineFwImage(uids[i], path, fwVersion);
						if (fwVersion.empty())
						{
							fwVersion = TRS(UPDATEFIRMWARE_EXAMINE_NO_VERSION_MSG);
						}

						// fwVersion will be same every time so only need to do this once
						if (i == 0)
						{
							pSimpleList->insert(path + ": " + fwVersion);
						}

						std::string result;
						switch (rc)
						{
							case wbem::framework::SUCCESS:
								result = UPDATEFIRMWARE_EXAMINE_VALID_MSG;
								break;
							case wbem::framework::REQUIRES_FORCE:
								result = UPDATEFIRMWARE_EXAMINE_VALID_WITH_FORCE_MSG;
								break;
							case wbem::framework::FAIL:
								result = UPDATEFIRMWARE_EXAMINE_INVALID_MSG;
								break;
						}
						pSimpleList->insert(prefix + result);
					}
					catch (wbem::framework::Exception &e)
					{
						framework::ErrorResult *pError = NvmExceptionToResult(e);
						pSimpleList->insert(m_uidToDimmIdStr(uids[i]).c_str() +
								pError->outputText());
						if (!pSimpleList->getErrorCode())  // keep existing errors
						{
							pSimpleList->setErrorCode(pError->getErrorCode());
						}
						delete(pError);
					}
				}
				pResult = pSimpleList;
			}
			else // do FW update
			{
				framework::SimpleListResult *pSimpleList = new framework::SimpleListResult();
				for (size_t i = 0; i < uids.size(); i++)
				{
					std::string prefix = framework::ResultBase::stringFromArgList(
							TRS(UPDATEFIRMWARE_MSG),
							m_uidToDimmIdStr(uids[i]).c_str());
					prefix += ": ";
					try
					{
						std::string fwVersion;
						// if user didn't specify the force option, and it's required, prompt them to continue
						std::string prompt = framework::ResultBase::stringFromArgList(
								DOWNGRADE_FW_PROMPT.c_str(), m_uidToDimmIdStr(uids[i]).c_str());
						if (!forceOption && wbem::framework::REQUIRES_FORCE == m_ExamineFwImage(uids[i], path, fwVersion) && !promptUserYesOrNo(prompt))
						{
							pSimpleList->insert(prefix + TRS(cli::framework::UNCHANGED_MSG));
						}
						else
						{
							m_InstallFromPath(uids[i], path, true);
							pSimpleList->insert(prefix +
									std::string(TRS(cli::framework::SUCCESS_MSG)) + TRS(UPDATEFIRMWARE_RESET_MSG));
						}
					}
					catch (wbem::framework::Exception &e)
					{
						cli::framework::ErrorResult *eResult = NvmExceptionToResult(e);
						pSimpleList->insert(prefix + eResult->outputText());
						pSimpleList->setErrorCode(eResult->getErrorCode());
						delete(eResult);
						break; // don't continue on failure
					}
				}
				pResult = pSimpleList;
			}
		}
	}
	else
	{
		pResult = new framework::SyntaxErrorMissingValueResult(framework::TOKENTYPE_OPTION, "source");
	}

	return pResult;
}

/*
 * Simple wrapper around WBEM
 */
void cli::nvmcli::FieldSupportFeature::wbemInstallFromPath(const std::string &deviceUid,
		const std::string &uri, bool force)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::software::NVDIMMSoftwareInstallationServiceFactory provider;
	provider.installFromPath(deviceUid, uri, false, force);
}

int cli::nvmcli::FieldSupportFeature::wbemExamineFwImage(const std::string &deviceUid,
		const std::string &path, std::string &fwVersion)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::software::NVDIMMSoftwareInstallationServiceFactory provider;

	return provider.examineFwImage(deviceUid, path, fwVersion);
}

/*
 * cli command to dump the support data to a file
 */
cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::dumpSupport(
		const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::ResultBase *pResult = NULL;
	// get command parameters
	std::string destination =
			framework::Parser::getOptionValue(parsedCommand, framework::OPTION_DESTINATION.name);
	std::string support =
			framework::Parser::getTargetValue(parsedCommand, TARGET_SUPPORT_R.name);

	std::string prefix = TRS(DUMPSUPPORT_MSG);
	try
	{
		m_DumpSupport(destination);
		framework::SimpleListResult *pSimpleList = new framework::SimpleListResult();
		pSimpleList->insert(std::string(prefix +  ": " + TRS(cli::framework::SUCCESS_MSG)));
		pResult = pSimpleList;
	}
	catch (wbem::framework::Exception &e)
	{
		if (NULL != pResult)
		{
			delete pResult;
		}
		pResult = NvmExceptionToResult(e);
	}

	return pResult;
}

/*
 * Simple wrapper around WBEM
 */
void cli::nvmcli::FieldSupportFeature::wbemDumpSupport(const std::string &path)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::support::SupportDataServiceFactory provider;
	int rc = provider.m_GatherSupportProvider(path.c_str(), path.length());
	if (rc != COMMON_SUCCESS)
	{
		throw wbem::exception::NvmExceptionLibError(rc);
	}
}

/*
 * delete the support data
 */
cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::deleteSupport(
		const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;
	std::string support = framework::Parser::getTargetValue(parsedCommand, TARGET_SUPPORT_R.name);
	if (!support.empty())
	{
		pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_TARGET,
				TARGET_SUPPORT_R.name, support);
	}
	else
	{
		std::string prefix = TRS(DELETESUPPORT_MSG);
		try
		{
			m_ClearSupport();
			framework::SimpleListResult *pSimpleList = new framework::SimpleListResult();
			pSimpleList->insert(std::string(prefix + ": " + TRS(cli::framework::SUCCESS_MSG)));
			pResult = pSimpleList;
		}
		catch (wbem::framework::Exception &e)
		{
			if (NULL != pResult)
			{
				delete pResult;
			}
			pResult = NvmExceptionToResult(e, prefix);
		}
	}
	return pResult;
}

/*
 * show performance metric data
 */
cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::showPerformance(
		const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;
	// CLI named attributes (these differ slightly from the CIM Schema spec)

	std::vector<std::string> dimms;
	pResult = cli::nvmcli::getDimms(parsedCommand, dimms);

	if (NULL == pResult)
	{
		try
		{

			// read the targets
			std::string dimmTarget = cli::framework::Parser::getTargetValue(parsedCommand, TARGET_DIMM.name);
			std::string performanceTargetValue = cli::framework::Parser::getTargetValue(parsedCommand,
					PERFORMANCE_TARGET);

			wbem::framework::attribute_names_t displayAttributes;
			displayAttributes.push_back(wbem::DIMMID_KEY);
			displayAttributes.push_back(wbem::BYTESREAD_KEY);
			displayAttributes.push_back(wbem::BYTESWRITTEN_KEY);
			displayAttributes.push_back(wbem::HOSTREADREQUESTS_KEY);
			displayAttributes.push_back(wbem::HOSTWRITECOMMANDS_KEY);
			displayAttributes.push_back(wbem::BLOCKREADREQUESTS_KEY);
			displayAttributes.push_back(wbem::BLOCKWRITECOMMANDS_KEY);
			FilterAttributeNames(displayAttributes, performanceTargetValue);

			// Change CLI properties to WBEM attribute names

			if (displayAttributes.empty()) // invalid filter
			{
				pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_TARGET,
						PERFORMANCE_TARGET, performanceTargetValue);
			}
			else
			{
				// make sure we have the id in our display
				// this would cover the case the user asks for specific display attributes, but they
				// don't include the handle
				if (!wbem::framework_interface::NvmInstanceFactory::containsAttribute(wbem::DIMMID_KEY, displayAttributes))
				{
					displayAttributes.insert(displayAttributes.begin(), wbem::DIMMID_KEY);
				}

				// make sure we have the appropriate dimm ids for filtering
				wbem::framework::attribute_names_t requestedAttributes(displayAttributes);
				if (!dimmTarget.empty())
				{
					requestedAttributes.push_back(wbem::DIMMHANDLE_KEY);
				}

				filters_t filters;
				generateDimmFilter(parsedCommand, requestedAttributes, filters, wbem::INSTANCEID_KEY);

				wbem::performance::NVDIMMPerformanceViewFactory perfProvider;
				wbem::framework::instances_t *pInstances = perfProvider.getInstances(requestedAttributes);

				framework::ObjectListResult *pTable =
						NvmInstanceToObjectListResult(*pInstances, "DimmPerformance",
						wbem::INSTANCEID_KEY, displayAttributes, filters);
				if (!dimmTarget.empty() && pTable->getCount() == 0)
				{
					delete pTable;
					pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_TARGET,
							TARGET_DIMM.name, dimmTarget);
				}
				else
				{
					pTable->setOutputType(framework::ResultBase::OUTPUT_TEXTTABLE);
					pResult = pTable;
				}

				delete pInstances;
			}
		}
		catch (wbem::framework::Exception &e)
		{
			if (pResult)
			{
				delete pResult;
			}
			pResult = NvmExceptionToResult(e);
		}
	}
	return pResult;
}

void cli::nvmcli::FieldSupportFeature::wbemClearSupport()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::support::SupportDataServiceFactory provider;
	try
	{
		provider.clear();
	}
	catch (wbem::framework::Exception &)
	{
		throw;
	}
}

/*
 * Display the Intel DIMM Gen 1 host software inventory
 */
cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::showVersion(
		const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	ShowVersionCommand command;
	return command.execute(parsedCommand);
}

/*
 * Acknowledge an event
 */
cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::ackEvent(
		const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;
	try
	{
		std::string requestedEvent =
				cli::framework::Parser::getTargetValue(parsedCommand, TARGET_EVENT_STR);

		// build an object path for the instance we are trying to get
		wbem::framework::Attribute key(requestedEvent, true);
		wbem::framework::attributes_t keys;
		keys.insert(std::pair<std::string, wbem::framework::Attribute>(wbem::INSTANCEID_KEY, key));
		wbem::framework::ObjectPath path("dontcare", "dontcare", "dontcare", keys);

		bool arPropertyExists = false;
		std::string arPropertyValue =
				cli::framework::Parser::getPropertyValue(parsedCommand, ACTIONREQUIRED_PROPERTY_NAME, &arPropertyExists);
		if (!arPropertyExists || arPropertyValue.empty())
		{
			pResult = new framework::SyntaxErrorMissingValueResult(
					framework::TOKENTYPE_PROPERTY, ACTIONREQUIRED_PROPERTY_NAME);
		}
		if ((cli::framework::stringsIEqual(arPropertyValue, "false")) ||
				(cli::framework::stringsIEqual(arPropertyValue, "0")))
		{
			// call modifyInstance
			wbem::support::NVDIMMLogEntryFactory provider;
			wbem::framework::attributes_t attributes;
			wbem::framework::Attribute attr(0, false);
			attributes[wbem::ACTIONREQUIRED_KEY] = attr;
			provider.modifyInstance(path, attributes);
			std::string resultMsg = TR("Acknowledge event: ");
			resultMsg += TRS(cli::framework::SUCCESS_MSG);
			if (pResult)
			{
				delete pResult;
			}
			framework::SimpleListResult *pSimpleList = new framework::SimpleListResult();
			pSimpleList->insert(resultMsg);
			pResult = pSimpleList;
		}
		else
		{
			if (pResult)
			{
				delete pResult;
			}
			pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, ACTIONREQUIRED_PROPERTY_NAME, arPropertyValue);
		}
	}
	catch (wbem::framework::ExceptionBadParameter &)
	{
		std::string requestedEvent =
			cli::framework::Parser::getTargetValue(parsedCommand, TARGET_EVENT_STR);
		char resultMsg[256];
		s_snprintf(resultMsg, 256, "Error: The event identifier '%s' is not valid.", requestedEvent.c_str());
		if (pResult)
		{
			delete pResult;
		}
		framework::SimpleListResult *pSimpleList = new framework::SimpleListResult();
		pSimpleList->insert(resultMsg);
		pResult = pSimpleList;
	}
	catch (wbem::framework::Exception &e)
	{
		if (pResult)
		{
			delete pResult;
		}
		pResult = NvmExceptionToResult(e);
	}
	return pResult;
}

/*
 * Run a diagnostic test
 */
cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::runDiagnostic(
		const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;
	wbem::support::NVDIMMDiagnosticFactory provider;

	// set up as multiple property lists (key/value pairs), one per test attempt


	std::vector<std::string> dimmTargets;

	// set up list of tests to run - all false for now
	bool runTest[NUMDIAGTESTTYPES];
	for (int i = 0; i < NUMDIAGTESTTYPES; i++)
	{
		runTest[i] = false;
	}

	std::string requestedTest =
			cli::framework::Parser::getTargetValue(parsedCommand, DIAGNOSTIC_TARGET);

	try
	{
		// pull dimms from command, if no dimms specified, get all manageable dimms
		pResult = cli::nvmcli::getDimms(parsedCommand, dimmTargets);

		// if no error from getting Dimms, keep going
		if (NULL == pResult)
		{
			try
			{
				// if no dimms found after getDimm call...
				if (0 == dimmTargets.size())
				{
					std::string requestedTarget =
							cli::framework::Parser::getTargetValue(parsedCommand, TARGET_DIMM.name);

					if (requestedTarget.length() > 0)
					{
						// if the user specified a dimm, it was bad
						pResult = new framework::SyntaxErrorBadValueResult(
								framework::TOKENTYPE_TARGET, TARGET_DIMM.name, requestedTarget);
					}
					else
					{
						// if the user didn't specify a dimm, there are just no manageable dimms to use
						pResult = new framework::ErrorResult(NVM_ERR_UNKNOWN,
								TR("No manageable " NVM_DIMM_NAME "s found."));
					}
				}
				bool foundTest = false;
				// if test input is blank, run all tests
				if (0 == requestedTest.length())
				{
					foundTest = true;
					for (int i = 0; i < wbem::support::NVDIMMDIAGNOSTIC_NUMTESTTYPES; i++)
					{
						runTest[i] = true;
					}
				}
				else
				{
					// validate test type
					for (int i = 0; i < NUMDIAGTESTTYPES; i++)
					{
						if (framework::stringsIEqual(requestedTest, diagTestTypes[i]))
						{
							runTest[i] = true;
							foundTest = true;
						}
					}

					if (!foundTest)
					{
						if (pResult)
						{
							delete pResult;
						}
						// invalid test type, set error, nothing runs in this case, just set error as result
						pResult = new framework::SyntaxErrorBadValueResult(
								framework::TOKENTYPE_TARGET, DIAGNOSTIC_TARGET, requestedTest);
					}
				}
				if (pResult == NULL)
				{
					framework::ObjectListResult *pObjListResult = new framework::ObjectListResult();
					pObjListResult->setRoot("Diagnostic");
					pResult = pObjListResult;
					// for each requested test
					for (int i = 0; i < NUMDIAGTESTTYPES; i++)
					{
						if (runTest[i])
						{
							// keep running the test on the next dimm
							bool perDimm = true;

							// for each dimm in the list
							for (std::vector<std::string>::const_iterator dimmTargetIter = dimmTargets.begin();
								 (dimmTargetIter != dimmTargets.end()) && perDimm;
								 dimmTargetIter++)
							{
								// these tests run on the system, not per dimm, so only run them once
								if (CLIDIAGNOSTIC_TEST_QUICK != diagTestTypes[i])
								{
									perDimm = false;
								}

								framework::PropertyListResult propertyList;
								propertyList.setName("Diagnostic");
								propertyList.insert(CLIDIAGNOSTIC_TESTNAME, diagTestTypes[i]);

								// convert uid string to uid
								COMMON_UID uid;
								uid_copy((*dimmTargetIter).c_str(), uid);
								wbem::framework::UINT16_LIST ignoreResults;

								try
								{
									// run diagnostic test
									provider.RunDiagnosticService(uid, ignoreResults, wbem::support::validTestTypes[i]);
									// if we got this far, pull the results out of the event table for this test
									// there should only be one event table entry per test,
									// and per uid if it's uid specific

									// get results
									wbem::framework::attribute_names_t attributes;
									wbem::support::DiagnosticCompletionRecordFactory diagProvider;
									wbem::framework::instances_t *pInstances = diagProvider.getInstances(attributes);

									if (pInstances->size() == 0)
									{
										// something went wrong, no test info found
										propertyList.insert(TR("Error"), TR("No log information found for test ") + diagTestTypes[i]);
									}
									// for each diagnostic completion record of info...
									for (wbem::framework::instances_t::const_iterator instanceIter = pInstances->begin();
										 instanceIter != pInstances->end();
										 instanceIter++)
									{
										wbem::framework::Attribute nameAttr;
										(*instanceIter).getAttribute(wbem::SERVICENAME_KEY, nameAttr);

										// compare to wbem test names since that's what's in the db
										if (nameAttr.stringValue().compare(wbem::support::validTestTypes[i]) == 0)
										{
											// get dimmID for comparison
											std::string dimmStr = "";
											size_t prefixStrLen = 0;
											size_t dimmStrLen = 0;
											wbem::framework::Attribute dimmAttr;
											if (perDimm && (*instanceIter).getAttribute(
													wbem::MANAGEDELEMENTNAME_KEY,
													dimmAttr) == wbem::framework::SUCCESS)
											{
												size_t dimmStrLen = dimmAttr.stringValue().length();
												prefixStrLen = wbem::physical_asset::NVDIMM_ELEMENTNAME_PREFIX.length();
												dimmStr = dimmAttr.stringValue().substr(prefixStrLen, dimmStrLen - prefixStrLen);
											}

											if (!perDimm || (*dimmTargetIter).compare(dimmStr) == 0)
											{
												// found a match, collect the data & add results to output
												if (perDimm)
												{
													// convert dimm UID to dimm ID
													std::string dimmIdStr = m_uidToDimmIdStr(
															dimmAttr.stringValue().substr(prefixStrLen, dimmStrLen - prefixStrLen));
													propertyList.insert(wbem::DIMMID_KEY, dimmIdStr);
												}

												// get timestamp
												wbem::framework::Attribute timetAttr;
												(*instanceIter).getAttribute(wbem::CREATIONTIMESTAMP_KEY, timetAttr);
												time_t time = (time_t) (timetAttr.uint64Value());
												std::string timeStr = ctime(&time);
												std::size_t nullpos = timeStr.find("\n");
												if (nullpos != std::string::npos)
												{
													timeStr.erase(nullpos, 1);
												}
												propertyList.insert(CLIDIAGNOSTIC_LOGENTRYTIMESTAMP,
														timeStr);

												// get CompletionState
												wbem::framework::Attribute StatAttr;
												(*instanceIter).getAttribute(wbem::COMPLETIONSTATE_KEY, StatAttr);
												propertyList.insert(CLIDIAGNOSTIC_COMPLETIONSTATE, StatAttr.stringValue());

												// get errorCodes
												wbem::framework::Attribute ErrAttr;
												(*instanceIter).getAttribute(wbem::ERRORCODE_KEY, ErrAttr);
												wbem::framework::STR_LIST ErrList = ErrAttr.strListValue();


												for (wbem::framework::STR_LIST::const_iterator ei = ErrList.begin();
													 ei != ErrList.end();
													 ei++)
												{
													propertyList.insert(CLIDIAGNOSTIC_COMPLETIONMESSAGE, *ei);
												}
											}
										}
									}
								}
								catch (wbem::exception::NvmExceptionLibError &e)
								{
									propertyList.insert(CLIDIAGNOSTIC_COMPLETIONSTATE, "Aborted");
									propertyList.insert(CLIDIAGNOSTIC_COMPLETIONMESSAGE, e.what());
								}

								// add test worth of info to the overal object result list
								pObjListResult->insert((*dimmTargetIter) + " " + wbem::support::validTestTypes[i], propertyList);
							}// for each dimm
						} // if runTest
					} // for each test
				}
			}
			catch (wbem::framework::Exception &e)
			{
				if (NULL != pResult)
				{
					delete pResult;
				}
				pResult = NvmExceptionToResult(e);
				// errors are already added to the results, shouldn't need to do anything else here
			}
		}
	}
	catch (wbem::exception::NvmExceptionLibError &e)
	{
		if (NULL != pResult)
		{
			delete pResult;
		}
		pResult = NvmExceptionToResult(e);
	}

	return pResult;
}

cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::showEvents(
		cli::framework::ParsedCommand const &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;
	if (m_getEvents == NULL)
	{
		COMMON_LOG_ERROR("getEvents provider is NULL");
		pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(nvmcli::UNKNOWN_ERROR_STR));
	}
	else
	{
		wbem::support::EventLogFilter filter;

		if ((pResult = showEvents_inputToFilter(parsedCommand, filter)) == NULL)
		{
			wbem::framework::instances_t *pInstances = NULL;
			try
			{
				pInstances = m_getEvents(filter);
				if (pInstances == NULL)
				{
					COMMON_LOG_ERROR("getEvents provider returned NULL");
					if (pResult)
					{
						delete pResult;
					}
					pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
							TRS(nvmcli::UNKNOWN_ERROR_STR));
				}
				else
				{
					if (pResult)
					{
						delete pResult;
					}
					pResult = showEvents_logEntriesToObjectList(pInstances);
				}
			}
			catch (wbem::framework::Exception &e)
			{
				if (pResult)
				{
					delete pResult;
				}
				pResult = NvmExceptionToResult(e);

			}
			if (pInstances)
			{
				delete pInstances;
			}
		}
	}

	return pResult;
}

/*
 * Convert NVDIMMLogEntry instances to and object list result formatting the properties
 * appropriately
 */
cli::framework::ResultBase *
cli::nvmcli::FieldSupportFeature::showEvents_logEntriesToObjectList(
		wbem::framework::instances_t *pInstances)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	cli::framework::ResultBase *pResult;
	cli::framework::ObjectListResult *pListResult = new cli::framework::ObjectListResult();
	pListResult->setRoot("Event");
	pResult = pListResult;

	size_t events_to_show = pInstances->size() < MAX_EVENTS ? pInstances->size() : MAX_EVENTS;
	for (size_t i = 0; i < events_to_show; i++)
	{
		// Get the attributes from the CIM Instance
		wbem::framework::Instance &instance = (*pInstances)[i];
		cli::framework::PropertyListResult propertyList;
		propertyList.setName("Event");

		wbem::framework::Attribute instanceIdAttribute;
		wbem::framework::Attribute creationTimeAttribute;
		wbem::framework::Attribute severityAttribute;
		wbem::framework::Attribute messageAttribute;
		wbem::framework::Attribute messageArgsAttribute;
		wbem::framework::Attribute actionRequiredAttribute;

		if (instance.getAttribute(wbem::INSTANCEID_KEY, instanceIdAttribute)
					!= wbem::framework::SUCCESS ||
			instance.getAttribute(wbem::CREATIONTIMESTAMP_KEY, creationTimeAttribute)
					!= wbem::framework::SUCCESS ||
			instance.getAttribute(wbem::PERCEIVEDSEVERITY_KEY, severityAttribute)
					!= wbem::framework::SUCCESS ||
			instance.getAttribute(wbem::MESSAGE_KEY, messageAttribute)
					!= wbem::framework::SUCCESS ||
			instance.getAttribute(wbem::MESSAGEARGS_KEY, messageArgsAttribute)
					!= wbem::framework::SUCCESS ||
			instance.getAttribute(wbem::ACTIONREQUIRED_KEY, actionRequiredAttribute)
					!= wbem::framework::SUCCESS)
		{
			COMMON_LOG_ERROR("Issue getting expected attributes from NVDIMMLogEntry.");
			delete pListResult;
			pListResult = NULL;
			pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
					TRS(nvmcli::UNKNOWN_ERROR_STR));
			break;
		}
		else
		{
			// Convert the attributes to appropriate output for the user

			/*
			 * date/time
			 */
			char datetime[255];
			time_t t = (time_t) creationTimeAttribute.uint64Value();
			struct tm *p_localTime = localtime(&t);
			if (!p_localTime)
			{
				// this should never happen
				COMMON_LOG_ERROR("Unable to get local time for log entry.");
				s_strcpy(datetime, "00:00:0000:00:00:00", 255);
			}
			else
			{
				strftime(datetime, sizeof(datetime), "%m:%d:%Y:%H:%M:%S", p_localTime);
			}
			propertyList.insert("Time", datetime);

			/*
			 * Event ID
			 */
			propertyList.insert("EventID", instanceIdAttribute.asStr());

			/*
			 * Severity
			 */
			propertyList.insert("Severity", severityAttribute.asStr());

			/*
			 * ActionRequired
			 */
			if (actionRequiredAttribute.boolValue())
			{
				propertyList.insert("ActionRequired", "1");
			}
			else
			{
				propertyList.insert("ActionRequired", "0");
			}

			/*
			 * Message
			 */
			char msg[NVM_EVENT_MSG_LEN + (3 * NVM_EVENT_ARG_LEN)];
			s_snprintf(msg, (NVM_EVENT_MSG_LEN + (3 * NVM_EVENT_ARG_LEN)),
					messageAttribute.asStr().c_str(),
					messageArgsAttribute.strListValue()[0].c_str(),
					messageArgsAttribute.strListValue()[1].c_str(),
					messageArgsAttribute.strListValue()[2].c_str());
			std::string message(msg);
			propertyList.insert("Message", message);

			pListResult->insert(instanceIdAttribute.asStr(), propertyList);
		}
		pListResult->setOutputType(cli::framework::ResultBase::OUTPUT_TEXTTABLE);
	}

	return pResult;
}

/*
 * Convert the user input (options/targets/properties) into a filter that can be passed
 * to WBEM to filter the results of events. Returns a Syntax error if there is an error
 * or NULL on success
 */
cli::framework::ErrorResult *cli::nvmcli::FieldSupportFeature::showEvents_inputToFilter(
		cli::framework::ParsedCommand const &parsedCommand,
		wbem::support::EventLogFilter &filter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	cli::framework::ErrorResult *pResult = NULL;
	/*
	 * Setup maps for valid property values and associated WBEM value
	 */
	std::map<std::string, COMMON_UINT32> severityMap;
	severityMap["info"] = wbem::support::EVENT_SEVERITY_INFO;
	severityMap["warn"] = wbem::support::EVENT_SEVERITY_WARN;
	severityMap["critical"] = wbem::support::EVENT_SEVERITY_CRITICAL;
	severityMap["fatal"] = wbem::support::EVENT_SEVERITY_FATAL;

	std::map<std::string, COMMON_UINT32> categoryMap;
	categoryMap["diag"] = wbem::support::EVENT_TYPE_DIAG;
	categoryMap["fw"] = wbem::support::EVENT_TYPE_DIAG_FW_CONSISTENCY;
	categoryMap["platformconfig"] = wbem::support::EVENT_TYPE_DIAG_PLATFORM_CONFIG;
	categoryMap["pm"] = wbem::support::EVENT_TYPE_DIAG_PM_META;
	categoryMap["quick"] = wbem::support::EVENT_TYPE_DIAG_QUICK;
	categoryMap["security"] = wbem::support::EVENT_TYPE_DIAG_SECURITY;
	categoryMap["health"] = wbem::support::EVENT_TYPE_HEALTH;
	categoryMap["mgmt"] = wbem::support::EVENT_TYPE_MGMT;
	categoryMap["config"] = wbem::support::EVENT_TYPE_CONFIG;

	/*
	 * -----------------------------------------------------------------------------------------
	 * Get and validate user input
	 * -----------------------------------------------------------------------------------------
	 */
	/*
	 * Start Time
	 */
	{
		bool startTimeExists;
		time_t startTime;
		pResult = cli::nvmcli::FieldSupportFeature::showEvents_getTimeFromOption(EVENT_OPTION_STARTTIME, parsedCommand, &startTime, &startTimeExists);

		if (pResult == NULL && startTimeExists)
		{
			filter.setAfterTimestamp(startTime - 1);
		}
	}

	/*
	 * End Time
	 */
	if (pResult == NULL)
	{
		bool endTimeExists;
		time_t endTime;
		pResult = cli::nvmcli::FieldSupportFeature::showEvents_getTimeFromOption(EVENT_OPTION_ENDTIME, parsedCommand, &endTime, &endTimeExists);
		if (pResult == NULL && endTimeExists)
		{
			filter.setBeforeTimestamp(endTime + 1);
		}
	}

	/*
	 * Severity
	 */
	if (pResult == NULL)
	{
		bool severityExists;

		std::string severityString = cli::framework::Parser::getPropertyValue(
				parsedCommand, EVENT_PROPERTY_SEVERITY, &severityExists);
		if (severityExists)
		{
			std::string lower = cli::framework::toLower(severityString);
			if (severityMap.find(lower) == severityMap.end())
			{
				pResult = new cli::framework::SyntaxErrorBadValueResult(
						cli::framework::TOKENTYPE_PROPERTY, EVENT_PROPERTY_SEVERITY, severityString);
			}
			else
			{
				filter.setSeverity(severityMap[lower]);
			}
		}
	}

	/*
	 * Category/Type
	 */
	if (pResult == NULL)
	{
		bool categoryExists;
		std::string categoryString = cli::framework::Parser::getPropertyValue(
				parsedCommand, EVENT_PROPERTY_CATEGORY, &categoryExists);
		if (categoryExists)
		{
			std::string lower = cli::framework::toLower(categoryString);
			if (categoryMap.find(lower) == categoryMap.end())
			{
				pResult = new cli::framework::SyntaxErrorBadValueResult(
						cli::framework::TOKENTYPE_PROPERTY, EVENT_PROPERTY_CATEGORY, categoryString);
			}
			else
			{
				filter.setType(categoryMap[lower]);
			}
		}
	}

	/*
	 * Dimm ID - specifying only 1 DIMM is supported, but want to take advantage of the
	 * getDimms function to convert handles to UIDs and DIMM validation. So checking the dimm target manually to see if
	 * user supplied a DIMM UID, then will call getDimms to convert to UID (if needed) and check
	 * DIMM is valid (exists and is manageable). As long as only one UID was found all is good.
	 */
	if (pResult == NULL)
	{
		bool dimmExists;
		std::string dimmString =
				cli::framework::Parser::getTargetValue(parsedCommand, cli::nvmcli::TARGET_DIMM.name, &dimmExists);
		if (dimmExists)
		{
			std::vector<std::string> dimmsFound;
			pResult = m_getDimms(parsedCommand, dimmsFound);
			if (pResult == NULL)
			{
				if (dimmsFound.size() != 1u)
				{
					pResult = new framework::SyntaxErrorBadValueResult(
							framework::TOKENTYPE_TARGET, TARGET_DIMM.name, dimmString);
				}
				else
				{
					filter.setUid(dimmsFound[0]);
				}
			}
		}
	}

	/*
	 * Namespace ID - specifying only 1 namespace ID is supported, but take advantage getNamespaces if
	 * one is provided.
	 */
	if (pResult == NULL)
	{
		bool nsExists;
		std::string nsString =
				cli::framework::Parser::getTargetValue(parsedCommand,
						cli::nvmcli::TARGET_NAMESPACE.name, &nsExists);
		if (nsExists)
		{
			std::vector<std::string> nsList;
			pResult = m_getNamespaces(parsedCommand, nsList);
			if (pResult == NULL)
			{
				if (nsList.size() != 1u)
				{
					pResult = new framework::SyntaxErrorBadValueResult(
						framework::TOKENTYPE_TARGET, TARGET_NAMESPACE.name, nsString);
				}
				else
				{
					filter.setUid(nsList[0]);
				}
			}
		}

	}

	/*
	 * Event ID
	 */
	if (pResult == NULL)
	{
		std::string eventString =
				cli::framework::Parser::getTargetValue(parsedCommand, cli::nvmcli::TARGET_EVENT_STR);
		bool validEventId = cli::nvmcli::isStringValidNumber(eventString);
		if (!eventString.empty())
		{
			filter.setEventId((COMMON_UINT32) cli::nvmcli::stringToUInt64(eventString));
		}

		if (!validEventId)
		{
			pResult = new cli::framework::SyntaxErrorBadValueResult(cli::framework::TOKENTYPE_TARGET,
					cli::nvmcli::TARGET_EVENT_STR, eventString);
		}
	}

	/*
	 * Action Required 1/0
	 */
	if (pResult == NULL)
	{
		bool actionRequiredExists;
		std::string actionRequiredString =
				cli::framework::Parser::getPropertyValue(parsedCommand, ACTIONREQUIRED_PROPERTY_NAME, &actionRequiredExists);
		if (actionRequiredExists)
		{
			if (framework::stringsIEqual(actionRequiredString, "0"))
			{
				filter.setActionRequired(false);
			}
			else if (framework::stringsIEqual(actionRequiredString, "1"))
			{
				filter.setActionRequired(true);
			}
			else
			{
				pResult = new cli::framework::SyntaxErrorBadValueResult(cli::framework::TOKENTYPE_PROPERTY,
						cli::nvmcli::TARGET_EVENT_STR, actionRequiredString);
			}
		}
	}

	return pResult;
}

cli::framework::SyntaxErrorResult *cli::nvmcli::FieldSupportFeature::showEvents_getTimeFromOption(
		std::string optionName, cli::framework::ParsedCommand const &parsedCommand, time_t *pTime,
		bool *pOptionExists)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::SyntaxErrorResult *pResult = NULL;

	*pOptionExists = false;
	std::string timeString =
			cli::framework::Parser::getOptionValue(parsedCommand, optionName, pOptionExists);

	if (*pOptionExists)
	{
		struct tm t;
		memset(&t, 0, sizeof(struct tm));

		if (!parseCliDateTime(timeString, &t))
		{
			pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_OPTION, optionName, timeString);
		}
		else
		{
			*pTime = mktime(&t);
		}
	}
	return pResult;
}

wbem::framework::instances_t *cli::nvmcli::FieldSupportFeature::wbemGetEvents(wbem::support::EventLogFilter &filter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::support::NVDIMMEventLogFactory log;
	return log.getFilteredEvents(&filter);

}

bool cli::nvmcli::FieldSupportFeature::parseCliDateTime(std::string timeString, tm *pTm)
{
	memset(pTm, 0, sizeof(struct tm));
	std::stringstream stream(timeString);
	int month, day, year, hour24, min, sec;
	const int NUM_COL = 5;
	char c[NUM_COL];

	stream >> month >> c[0] >>
			day >> c[1] >>
			year >> c[2] >>
			hour24 >> c[3] >>
			min >> c[4] >>
			sec;

	bool allColons = true;
	for(int i = 0; i < NUM_COL; i++)
	{
		allColons &= c[i] == ':';
	}

	bool isValid = allColons && !stream.fail() && stream.eof()
			&& isInRange(month, 1, 31)
			&& isInRange(day, 1, 31)
			&& isInRange(hour24, 0, 23)
			&& isInRange(min, 0, 59)
			&& isInRange(sec, 0, 59);

	if (isValid)
	{
		pTm->tm_sec = sec; /* Seconds.	[0-60] (1 leap second) */
		pTm->tm_min = min; /* Minutes.	[0-59] */
		pTm->tm_hour = hour24; /* Hours.	[0-23] */
		pTm->tm_mday = day; /* Day.		[1-31] */
		pTm->tm_mon = month - 1; /* Month.	[0-11] */
		pTm->tm_year = year - 1900; /* Year	- 1900.  */
	}
	return isValid;
}

bool cli::nvmcli::FieldSupportFeature::isInRange(int val, int min, int max)
{
	return min <= val && val <= max;
}

/*!
 * Show the user preferences
 */
cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::showPreferences(
		const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::PropertyListResult *pResult = new framework::PropertyListResult();
	pResult->setName("Preferences");

	std::vector<std::string> preferences = getSupportedPreferences();
	for (std::vector<std::string>::const_iterator prefIter = preferences.begin();
					prefIter != preferences.end(); prefIter++)
	{
		char currentSetting[CONFIG_VALUE_LEN];
		if ((*prefIter == SQL_KEY_CLI_DIMM_ID) ||
			(*prefIter == SQL_KEY_CLI_SIZE) ||
			(*prefIter == SQL_KEY_PERFORMANCE_MONITOR_ENABLED) ||
			(*prefIter == SQL_KEY_EVENT_MONITOR_ENABLED))
		{
			memset(currentSetting, 0, sizeof (currentSetting));
			get_config_value((*prefIter).c_str(), currentSetting);
			pResult->insert((*prefIter).c_str(), currentSetting);
		}
		else
		{
			memset(currentSetting, 0, sizeof (currentSetting));
			get_bounded_config_value((*prefIter).c_str(), currentSetting);
			pResult->insert((*prefIter).c_str(), currentSetting);
		}

	}

	return pResult;
}

/*
 * Retrieve a list of supported preferences
 */
std::vector<std::string> cli::nvmcli::FieldSupportFeature::getSupportedPreferences()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::vector<std::string> preferences;

	preferences.push_back(SQL_KEY_CLI_DIMM_ID);
	preferences.push_back(SQL_KEY_CLI_SIZE);
	preferences.push_back(SQL_KEY_PERFORMANCE_MONITOR_ENABLED);
	preferences.push_back(SQL_KEY_PERFORMANCE_MONITOR_INTERVAL_MINUTES);
	preferences.push_back(SQL_KEY_EVENT_MONITOR_ENABLED);
	preferences.push_back(SQL_KEY_EVENT_MONITOR_INTERVAL_MINUTES);
	preferences.push_back(SQL_KEY_EVENT_LOG_MAX);
	preferences.push_back(SQL_KEY_LOG_MAX);
	preferences.push_back(SQL_KEY_SUPPORT_SNAPSHOT_MAX);

	return preferences;
}

/*!
 * Modify user preferences
 */
cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::changePreferences(
		const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	cli::framework::ResultBase *pResult = NULL;

	// at least one preference specified
	if (parsedCommand.properties.empty())
	{
		pResult = new framework::SyntaxErrorResult(TRS(NOMODIFIABLEPROPERTY_ERROR_STR));
	}
	else
	{
		// check for valid property valud
		for (cli::framework::StringMap::const_iterator propIter = parsedCommand.properties.begin();
				propIter != parsedCommand.properties.end(); propIter++)
		{
			bool validValue = true;
			if (framework::stringsIEqual(propIter->first, SQL_KEY_CLI_DIMM_ID))
			{
				if (!framework::stringsIEqual(propIter->second, PREFERENCE_DIMMID_HANDLE) &&
					!framework::stringsIEqual(propIter->second, PREFERENCE_DIMMID_UID))
				{
					validValue = false;
				}
			}
			else if (framework::stringsIEqual(propIter->first, SQL_KEY_CLI_SIZE))
			{
				// Auto|B|MB|GB
				if (!framework::stringsIEqual(propIter->second, PREFERENCE_SIZE_AUTO) &&
					!framework::stringsIEqual(propIter->second, PREFERENCE_SIZE_B) &&
					!framework::stringsIEqual(propIter->second, PREFERENCE_SIZE_MIB) &&
					!framework::stringsIEqual(propIter->second, PREFERENCE_SIZE_GIB))
				{
					validValue = false;
				}
			}
			else if (framework::stringsIEqual(propIter->first, SQL_KEY_PERFORMANCE_MONITOR_ENABLED) ||
					framework::stringsIEqual(propIter->first, SQL_KEY_EVENT_MONITOR_ENABLED))
			{
				// 0|1 only
				if (!framework::stringsIEqual(propIter->second, PREFERENCE_ENABLED) &&
					!framework::stringsIEqual(propIter->second, PREFERENCE_DISABLED))
				{
					validValue = false;
				}
			}
			else if (framework::stringsIEqual(propIter->first, SQL_KEY_PERFORMANCE_MONITOR_INTERVAL_MINUTES) ||
					 framework::stringsIEqual(propIter->first, SQL_KEY_EVENT_MONITOR_INTERVAL_MINUTES) ||
					 framework::stringsIEqual(propIter->first, SQL_KEY_EVENT_LOG_MAX) ||
					 framework::stringsIEqual(propIter->first, SQL_KEY_LOG_MAX) ||
					 framework::stringsIEqual(propIter->first, SQL_KEY_SUPPORT_SNAPSHOT_MAX))
			{
				if (!valueIsValidNumberForKey(propIter->first, propIter->second))
				{
					validValue = false;
				}
			}
			if (!validValue)
			{
				pResult = new framework::SyntaxErrorBadValueResult(
						framework::TOKENTYPE_PROPERTY, propIter->first, propIter->second);
				COMMON_LOG_ERROR_F("%s", pResult->outputText().c_str());
				break;
			}
		}

		// input checks out, proceed with changing preferences
		if (!pResult)
		{
			framework::SimpleListResult *pListResult = new framework::SimpleListResult();

			// make the changes
			for (cli::framework::StringMap::const_iterator propIter = parsedCommand.properties.begin();
					propIter != parsedCommand.properties.end(); propIter++)
			{
				std::stringstream setPreferenceOutput;
				setPreferenceOutput << "Set " << propIter->first << "=" << propIter->second;
				int tmpRc = add_config_value(propIter->first.c_str(), propIter->second.c_str());
				if (tmpRc != COMMON_SUCCESS)
				{
					COMMON_LOG_ERROR_F("Failed to change preference '%s' to '%s', error: %d",
							propIter->first.c_str(), propIter->second.c_str(), tmpRc);
					wbem::exception::NvmExceptionLibError err(tmpRc);
					framework::ErrorResult *pError = NvmExceptionToResult(err, setPreferenceOutput.str());
					if (!pListResult->getErrorCode())  // keep existing errors
					{
						pListResult->setErrorCode(pError->getErrorCode());
					}
					pListResult->insert(pError);
				}
				else
				{
					setPreferenceOutput << " : " << TRS(cli::framework::SUCCESS_MSG);
					pListResult->insert(setPreferenceOutput.str());
				}
			}

			pResult = pListResult;
		}
	}

	return pResult;
}

bool cli::nvmcli::FieldSupportFeature::stringIsNumberInRange(const std::string &str, const int min, const int max)
{
	bool result = false;
	int value = strtol(str.c_str(), NULL, 0);
	if (isStringValidNumber(str) &&
	    value >= min && value <= max)
	{
		result = true;
	}

	return result;
}

bool cli::nvmcli::FieldSupportFeature::valueIsValidNumberForKey(const std::string key, const std::string value)
{
	bool isValid = false;
	if (isStringValidNumber(value))
	{
		int intValue = strtol(value.c_str(), NULL, 0);
		if (is_valid_value(key.c_str(), intValue))
		{
			isValid = true;
		}
	}
	return isValid;
}

cli::framework::ErrorResult *cli::nvmcli::FieldSupportFeature::wbemToCliGetNamespaces(
		const framework::ParsedCommand &parsedCommand, std::vector<std::string> &namespaces)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	cli::nvmcli::WbemToCli wbemToCliProvider;
	return wbemToCliProvider.getNamespaces(parsedCommand, namespaces);
}

cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::getDeviceFirmwareInstances(
		const framework::ParsedCommand& parsedCommand,
		wbem::framework::instances_t &instances)
{
	cli::framework::ResultBase *pResult = NULL;

	std::vector<std::string> uids;

	std::map<device_fw_type, std::string> fwTypeMap;
	fwTypeMap[DEVICE_FW_TYPE_UNKNOWN] = TR("Unknown");
	fwTypeMap[DEVICE_FW_TYPE_PRODUCTION] = TR("Production");
	fwTypeMap[DEVICE_FW_TYPE_DFX] = TR("DFx");
	fwTypeMap[DEVICE_FW_TYPE_DEBUG] = TR("Debug");

	wbem::framework::Instance instance;

	pResult = cli::nvmcli::getDimms(parsedCommand, uids);
	if (pResult == NULL)
	{
		for (size_t i = 0; i < uids.size(); i++)
		{
			core::device::DeviceFirmwareService &service = core::device::DeviceFirmwareService::getService();

			core::Result<core::device::DeviceFirmwareInfo> fwInfoResult = service.getFirmwareInfo(uids[i]);

			wbem::framework::Attribute dimmId = wbem::physical_asset::NVDIMMFactory::uidToDimmIdAttribute(uids[i]);
			instance.setAttribute(wbem::DIMMID_KEY, dimmId);

			wbem::framework::Attribute fwRevAttr(fwInfoResult.getValue().getActiveRevision(), false);
			instance.setAttribute(wbem::ACTIVEFWVERSION_KEY, fwRevAttr);

			instance.setAttribute(wbem::ACTIVEFWTYPE_KEY,
					wbem::framework::Attribute(fwTypeMap[fwInfoResult.getValue().getActiveType()], false));

			// commit ID is displayed as N/A if it's not available
			std::string commitIdStr = fwInfoResult.getValue().getActiveCommitId();
			if (commitIdStr.empty())
			{
				commitIdStr = wbem::NA;
			}
			instance.setAttribute(wbem::ACTIVEFWCOMMITID_KEY, wbem::framework::Attribute (commitIdStr, false));

			std::string buildConfiguration = fwInfoResult.getValue().getActiveBuildConfiguration();
			if (buildConfiguration.empty())
			{
				buildConfiguration = wbem::NA;
			}
			instance.setAttribute(wbem::ACTIVEBUILDCONFIGURATION_KEY, wbem::framework::Attribute (buildConfiguration, false));

			std::string stagedFwTypeStr = wbem::NA;
			std::string stagedFwRevStr = wbem::NA;
			if (fwInfoResult.getValue().isStagedPending())
			{
				stagedFwTypeStr =  fwTypeMap[fwInfoResult.getValue().getStagedType()];
				stagedFwRevStr = fwInfoResult.getValue().getStagedRevision();
			}

			instance.setAttribute(wbem::STAGEDFWVERSION_KEY, wbem::framework::Attribute (stagedFwRevStr, false));
			instance.setAttribute(wbem::STAGEDFWTYPE_KEY, wbem::framework::Attribute (stagedFwTypeStr, false));

			instances.push_back(instance);
		}
	}

	return pResult;
}

/*!
 * Show Device Firmware
 */
cli::framework::ResultBase *cli::nvmcli::FieldSupportFeature::showDeviceFirmware(
		const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	cli::framework::ResultBase *pResult = NULL;
	wbem::framework::instances_t instances;
	try
	{
		pResult = getDeviceFirmwareInstances(parsedCommand, instances);
		if (pResult == NULL)
		{
			wbem::framework::attribute_names_t defaultAttributes;
			defaultAttributes.push_back(wbem::DIMMID_KEY);
			defaultAttributes.push_back(wbem::ACTIVEFWVERSION_KEY);
			defaultAttributes.push_back(wbem::STAGEDFWVERSION_KEY);

			wbem::framework::attribute_names_t allAttributes = defaultAttributes;
			allAttributes.push_back(wbem::ACTIVEFWTYPE_KEY);
			allAttributes.push_back(wbem::ACTIVEFWCOMMITID_KEY);
			allAttributes.push_back(wbem::ACTIVEBUILDCONFIGURATION_KEY);
			allAttributes.push_back(wbem::STAGEDFWTYPE_KEY);

			wbem::framework::attribute_names_t displayAttributes =
					GetAttributeNames(parsedCommand.options, defaultAttributes, allAttributes);

			// make sure we have the DIMMID in the display when user asks for specific display attributes
			if (!wbem::framework_interface::NvmInstanceFactory::containsAttribute(wbem::DIMMID_KEY,
					displayAttributes))
			{
				displayAttributes.insert(displayAttributes.begin(), wbem::DIMMID_KEY);
			}

			// format the return data
			cli::nvmcli::filters_t filters;
			pResult = NvmInstanceToObjectListResult(instances, "DimmFirmware",
					wbem::DIMMID_KEY, displayAttributes, filters);

			// Set layout to table unless the -all or -display option is present
			if (!framework::parsedCommandContains(parsedCommand, framework::OPTION_DISPLAY) &&
					!framework::parsedCommandContains(parsedCommand, framework::OPTION_ALL))
			{
				pResult->setOutputType(framework::ResultBase::OUTPUT_TEXTTABLE);
			}
		}
	}
	catch (wbem::framework::Exception &e)
	{
		if (pResult)
		{
			delete pResult;
		}
		pResult = NvmExceptionToResult(e);
	}
	return pResult;
}
