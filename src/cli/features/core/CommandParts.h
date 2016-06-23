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
 * Common command syntax parts for the NVMCLI application.
 */

#ifndef _CLI_NVMCLI_COMMANDPARTS_H
#define _CLI_NVMCLI_COMMANDPARTS_H

#include <nvm_types.h>
#include <NvmStrings.h>

namespace cli
{
namespace nvmcli
{

const std::string TARGET_EVENT_STR = "-event";
const std::string EVENTID_STR = "EventID";
const std::string DIMMID_STR = "DimmID";
const std::string DIMMIDS_STR = "DimmIDs";
const std::string NAMESPACEID_STR = "NamespaceID";
const std::string NAMESPACEIDS_STR = "NamespaceIDs";
const std::string SOCKETIDS_STR = "SocketIDs";
const std::string JOBIDS_STR = "JobIDs";
const std::string POOLIDS_STR = "PoolIDs";

/*
 * These are some common targets.  Required and non-required versions are included where it has been
 * found that they are needed within the current command specs
 */
const framework::CommandSpecPart TARGET_SYSTEM = {"-system", false, "", false, N_TR("System target")};
const framework::CommandSpecPart TARGET_SYSTEM_R = {"-system", true, "", false, N_TR("System target is required.")};
const framework::CommandSpecPart TARGET_DIMM = {"-dimm", false, DIMMIDS_STR, false, N_TR("NVDIMM target")};
const framework::CommandSpecPart TARGET_DIMM_R = {"-dimm", true, DIMMIDS_STR, false, N_TR("NVDIMM target is required.")};
const framework::CommandSpecPart TARGET_NAMESPACE = {"-namespace", false, NAMESPACEIDS_STR, false, N_TR("Namespace target.")};
const framework::CommandSpecPart TARGET_NAMESPACE_R = {"-namespace", true, NAMESPACEIDS_STR, false, N_TR("Namespace target is required.")};
const framework::CommandSpecPart TARGET_SENSOR = {"-sensor", false, "", false, N_TR("Sensor target")};
const framework::CommandSpecPart TARGET_SENSOR_R = {"-sensor", true, "", false, N_TR("Sensor target is required.")};
const framework::CommandSpecPart TARGET_SOCKET = {"-socket", false, SOCKETIDS_STR, true, N_TR("Socket target.")};
const framework::CommandSpecPart TARGET_GOAL_R = {"-goal", true, "", false,
		N_TR("The memory allocation goal."), "", false};
const framework::CommandSpecPart TARGET_POOL = {"-pool", false, POOLIDS_STR, false, N_TR("Pool target.")};
const framework::CommandSpecPart TARGET_POOL_R = {"-pool", true, POOLIDS_STR, false, N_TR("Pool target is required.")};
const framework::CommandSpecPart TARGET_SUPPORT = {"-support", false, "", true, N_TR("Support target.")};
const framework::CommandSpecPart TARGET_SUPPORT_R = {"-support", true, "", true, N_TR("Support target is required.")};
const framework::CommandSpecPart TARGET_MEMORYRESOURCES_R = {"-memoryresources", true, "", false, "MemoryResources target is required."};
const framework::CommandSpecPart TARGET_CAPABILITIES_R = {"-capabilities", true, "", false, N_TR("Capabilities target is required.")};
const framework::CommandSpecPart TARGET_CONFIG_R = {"-config", true, "", false, N_TR("Configuration target is required.")};
const framework::CommandSpecPart TARGET_TOPOLOGY_R = {"-topology", true, "", false, N_TR("Show the system " NVM_DIMM_NAME " topology. "
		"No filtering is supported on this target.")};
const framework::CommandSpecPart TARGET_JOB = {"-job", true, "", false, N_TR("Job ID.")};
const framework::CommandSpecPart TARGET_PREFERENCES = {"-preferences", true, "", false, N_TR("The user preferences."), "", true};
const framework::CommandSpecPart TARGET_FIRMWARE_R = {"-firmware", true, "", false, N_TR("The firmware information.")};

const std::string UNKNOWN_ERROR_STR = N_TR("An unknown error occurred.");
const std::string NOTSUPPORTED_ERROR_STR = N_TR("The command is not supported in the current context.");
const std::string NOMEMORY_ERROR_STR = N_TR("There is not enough memory to complete the requested operation.");
const std::string TARGETS_ERROR_STR = N_TR("Invalid target(s): ");
const std::string NOMODIFIABLEPROPERTY_ERROR_STR = N_TR("At least one modifiable property is required.");
const std::string BADTARGETERROR_STR = wbem::EXCEPTION_BADTARGET_MSG;
const std::string BADSECURITY_ERROR_STR = N_TR("The command is not supported by the device in its current security state.");
const std::string NONINTEROPERABLEDIMM_ERROR_STR = N_TR("The command is not supported because some " NVM_DIMM_NAME "s in the "
		"system are not compatible with each other.");
const std::string INVALID_NS_APP_DIRECT_SETTINGS = N_TR("Unable to create a namespace with the specified AppDirectSetting.");
const std::string INVALID_DIMMID_ERROR_STR = N_TR("The device identifier %s is not valid.");
const std::string INVALID_NAMESPACEID_ERROR_STR = N_TR("The namespace identifier %s is not valid.");
const std::string CANT_USE_TOGETHER_ERROR_STR = N_TR("'%s' and '%s' cannot be used together.");
const std::string INVALID_COMMAND_ERROR_STR = N_TR("Invalid command.");

const std::string BAD_REQUEST_STR =
		N_TR("Unable to find a valid mapping for the requested configuration.");
const std::string BAD_REQUEST_MEMORY_SIZE_STR =
		N_TR("The requested memory capacity won't fit on the requested " NVM_DIMM_NAME "(s).");
const std::string BAD_REQUEST_APP_DIRECT_SIZE_STR =
		N_TR("The requested App Direct capacity won't fit on the requested " NVM_DIMM_NAME "(s).");
const std::string BAD_REQUEST_EXCEEDS_SYSTEM_RESOURCES_STR =
		N_TR("The requested configuration exceeds the system resources with too many interleave sets per socket.");
const std::string BAD_REQUEST_GOAL_ALREADY_EXISTS_STR = N_TR("A requested " NVM_DIMM_NAME
		" already has a configuration goal. Delete the existing goal before creating a new one.");
const std::string BAD_REQUEST_GOAL_BREAKS_CONFIG_STR = N_TR("The requested configuration would leave an " NVM_DIMM_NAME
		" unconfigured or break an existing configuration.");
const std::string BAD_REQUEST_MUST_DELETE_NAMESPACES_STR = N_TR("Existing namespaces must be deleted before this method can be executed.");
const std::string BAD_REQUEST_NOT_SUPPORTED_STR = N_TR("The requested configuration is not supported.");
const std::string BAD_REQUEST_APP_DIRECT_SETTINGS_NOT_RECOMMENDED_STR = N_TR("The selected App Direct settings are not recommended by the platform BIOS.");
const std::string BAD_REQUEST_DIMM_SECURITY_STATE = N_TR("This method is not supported by the device in its current security state.");
const std::string BAD_REQUEST_RESERVE_DIMM_STR = N_TR("Cannot create volatile or persistent "
		"regions on an " NVM_DIMM_NAME " and have it reserved as a storage device.");
const std::string BLOCKSIZE_NOT_SUPPORTED_STR = N_TR("The block size '%llu' is not supported.");

}
}

#endif
