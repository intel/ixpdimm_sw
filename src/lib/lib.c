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
 * This file contains the implementation of basic native API functions to get
 * information about the NVM library and translate it into a human-readable format.
 */

#include "nvm_management.h"
#include <persistence/logging.h>
#include <string/s_str.h>
#include <cr_i18n.h>

/*
 * Get the major version number of the library.
 */
int nvm_get_major_version()
{
	return NVM_VERSION_MAJOR;
}

/*
 * Get the minor version number of the library.
 */
int nvm_get_minor_version()
{
	return NVM_VERSION_MINOR;
}

/*
 * Get the hotfix version number of the library
 */
int nvm_get_hotfix_number()
{
	return NVM_VERSION_HOTFIX;
}

/*
 * Get the build number
 */
int nvm_get_build_number()
{
	return NVM_VERSION_BUILDNUM;
}

/*
 * Get the library version as a string
 */
int nvm_get_version(NVM_VERSION version_str, const NVM_SIZE str_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	if (version_str == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, version buffer in NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (str_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, version buffer length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		s_strcpy(version_str, VERSION_STR, str_len);
		COMMON_LOG_DEBUG_F("version is %s", version_str);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Given an numeric return_code, returns a textual description of the code in English
 */
int nvm_get_error(const enum return_code code, NVM_ERROR_DESCRIPTION description,
		const NVM_SIZE description_len)
{
	COMMON_LOG_ENTRY_PARAMS("return_code: %d", code);
	int rc = NVM_SUCCESS;

	if (description == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, description buffer in NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (description_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, description buffer length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		switch (code)
		{
			case NVM_SUCCESS:
				s_strcpy(description, TR("The method succeeded."), description_len);
				break;
			case NVM_ERR_UNKNOWN:
				s_strcpy(description, TR("An unknown error occurred."), description_len);
				break;
			case NVM_ERR_NOMEMORY:
				s_strcpy(description,
						TR("There was not enough memory to complete the requested operation."),
						description_len);
				break;
			case NVM_ERR_NOTSUPPORTED:
				s_strcpy(description,
						TR("This method is not supported in the current context."),
						description_len);
				break;
			case NVM_ERR_INVALIDPARAMETER:
				s_strcpy(description,
						TR("One or more of the input parameters are out of range "
						"or otherwise invalid."),
						description_len);
				break;
			case NVM_ERR_NOTMANAGEABLE:
				s_strcpy(description,
						TR("The device is not manageable by the management software."),
						description_len);
				break;
			case NVM_ERR_INVALIDPERMISSIONS:
				s_strcpy(description,
						TR("The caller does not have appropriate privileges "
						"to complete the operation."),
						description_len);
				break;
			case NVM_ERR_BADERRORCODE:
				s_strcpy(description, TR("The return code is not valid."),
						description_len);
				break;
			case NVM_ERR_DATATRANSFERERROR:
				s_strcpy(description,
						TR("The device is unable to complete the command "
						"because of a data transfer error."),
						description_len);
				break;
			case NVM_ERR_DEVICEERROR:
				s_strcpy(description,
						TR("The device is unable to complete the command "
						"because of an internal error."),
						description_len);
				break;
			case NVM_ERR_DEVICEBUSY:
				s_strcpy(description,
						TR("The device is unable to complete the command because it is busy."),
						description_len);
				break;
			case NVM_ERR_BADPASSPHRASE:
				s_strcpy(description, TR("The passphrase is not valid."), description_len);
				break;
			case NVM_ERR_INVALIDPASSPHRASE:
				s_strcpy(description,
						TR("The new passphrase does not meet the minimum requirements."),
						description_len);
				break;
			case NVM_ERR_SECURITYFROZEN:
				s_strcpy(description,
						TR("No changes can be made to the security state of the device."),
						description_len);
				break;
			case NVM_ERR_LIMITPASSPHRASE:
				s_strcpy(description,
						TR("The maximum passphrase submission limit has been reached. "
								"A power cycle is required to change the security state."),
						description_len);
				break;
			case NVM_ERR_SECURITYDISABLED:
				s_strcpy(description, TR("Data at rest security is not enabled."),
						description_len);
				break;
			case NVM_ERR_BADDEVICE:
				s_strcpy(description, TR("The device identifier is not valid."),
						description_len);
				break;
			case NVM_ERR_ARRAYTOOSMALL:
				s_strcpy(description, TR("The array is not big enough."), description_len);
				break;
			case NVM_ERR_BADCALLBACK:
				s_strcpy(description, TR("The callback identifier is not valid."),
						description_len);
				break;
			case NVM_ERR_BADFILE:
				s_strcpy(description, TR("The file is not valid."), description_len);
				break;
			case NVM_ERR_NOSIMULATOR:
				s_strcpy(description, TR("No simulator is loaded."), description_len);
				break;
			case NVM_ERR_BADPOOL:
				s_strcpy(description, TR("The pool identifier is not valid."),
						description_len);
				break;
			case NVM_ERR_BADNAMESPACE:
				s_strcpy(description, TR("The namespace identifier is not valid."),
						description_len);
				break;
			case NVM_ERR_BADBLOCKSIZE:
				s_strcpy(description, TR("The block size is not valid."), description_len);
				break;
			case NVM_ERR_BADSIZE:
				s_strcpy(description, TR("The size is not valid."), description_len);
				break;
			case NVM_ERR_BADFIRMWARE:
				s_strcpy(description,
						TR("The firmware image is not valid for the device."),
						description_len);
				break;
			case NVM_ERR_DRIVERFAILED:
				s_strcpy(description,
						TR("The communications with the device driver failed."),
						description_len);
				break;
			case NVM_ERR_BADSOCKET:
				s_strcpy(description,
						TR("The processor socket identifier is not valid."), description_len);
				break;
			case NVM_ERR_BADSECURITYSTATE:
				s_strcpy(description,
					TR("This method is not supported by the device in its current security state."),
					description_len);
				break;
			case NVM_ERR_REQUIRESFORCE:
				s_strcpy(description,
					TR("This method requires the force flag to proceed."),
					description_len);
				break;
			case NVM_ERR_NAMESPACESEXIST:
				s_strcpy(description,
					TR("Existing namespaces must be deleted before this method can be executed."),
					description_len);
				break;
			case NVM_ERR_NOTFOUND:
				s_strcpy(description,
					TR("The requested item was not found."),
					description_len);
				break;
			case NVM_ERR_BADDEVICECONFIG:
				s_strcpy(description,
					TR("The configuration data is invalid or unrecognized."),
					description_len);
				break;
			case NVM_ERR_DRIVERNOTALLOWED:
				s_strcpy(description,
				TR("The device driver is not allowing this command because "
					"it might destabilize the system."),
					description_len);
				break;
			case NVM_ERR_BADALIGNMENT:
				s_strcpy(description,
				TR("The specified size does not have the required alignment."),
					description_len);
				break;
			case NVM_ERR_EXCEEDSMAXSUBSCRIBERS:
				s_strcpy(description,
				TR("Exceeded maximum number of notify subscribers."),
					description_len);
				break;
			case NVM_ERR_BADTHRESHOLD:
				s_strcpy(description,
				TR("The specified sensor threshold is invalid or out of range."),
					description_len);
				break;
			case NVM_ERR_BADNAMESPACETYPE:
				s_strcpy(description,
						TR("The specified namespace type is invalid."),
						description_len);
				break;
			case NVM_ERR_BADNAMESPACEENABLESTATE:
				s_strcpy(description,
						TR("The specified namespace enable state is invalid."),
						description_len);
				break;
			case NVM_ERR_BADNAMESPACESETTINGS:
				s_strcpy(description,
						TR("Unable to create a namespace with the specified namespace settings."),
						description_len);
				break;
			case NVM_ERR_BADPCAT:
					s_strcpy(description,
						TR("The platform capabilities data is invalid or unrecognized."),
						description_len);
					break;
			case NVM_ERR_TOOMANYNAMESPACES:
				s_strcpy(description,
						TR("The maximum number of namespaces is already present in the system."),
						description_len);
				break;
			case NVM_ERR_CONFIGNOTSUPPORTED:
				s_strcpy(description,
						TR("The requested configuration is not supported."),
						description_len);
				break;
			case NVM_ERR_SKUVIOLATION:
				s_strcpy(description,
						TR("The method is not supported because one or more " NVM_DIMM_NAME "s "
							"are configured in violation of the license."),
						description_len);
				break;
			case NVM_ERR_BADDRIVER:
				s_strcpy(description,
						TR("The underlying software is missing or incompatible with this "
								"version of the management software."),
						description_len);
				break;
			case NVM_ERR_ARSINPROGRESS:
				s_strcpy(description,
						TR("Namespaces are unavailable because an address range scrub operation "
								"is currently in progress."),
						description_len);
				break;
			case NVM_ERR_BADSECURITYGOAL:
				s_strcpy(description,
						TR("Unable to create a namespace with the specified security goal."),
						description_len);
				break;
			case NVM_ERR_INVALIDPASSPHRASEFILE:
				s_strcpy(description,
						TR("The passphrase file is invalid."),
						description_len);
				break;
			default:
				s_strcpy(description, TR("The return code is not valid."),
						description_len);
				rc = NVM_ERR_BADERRORCODE;
				break;
		}
		COMMON_LOG_DEBUG_F(TR("return code description is %s"), description);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
