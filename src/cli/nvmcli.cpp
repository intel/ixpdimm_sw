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
 * Entry point for the ixpdimm-cli application.
 */

#include <string>
#include <iostream>
#include <ostream>

#include <os/os_adapter.h>
#include <persistence/lib_persistence.h>

#include <libinvm-cli/Framework.h>
#include <lib_interface/NvmContext.h>

#ifdef _INTEL_I18N_
#include <libIntel_i18n.h>
#endif

#ifndef LOCALE_DOMAIN
#define	LOCALE_DOMAIN	"ixpdimm-cli"
#endif

// function pointer to register functions
typedef void (*RegisterLibraryGetFeaturesFunction)();

void *tryLoadLib(std::string name)
{
	char libSuffix[10]; // should be plenty
	name += dlib_suffix(libSuffix, 10);

	return dlib_load(name.c_str());
}

int callDynamicFunc(void *lib_handle, const std::string func)
{
	int rc = -1;
	if (lib_handle)
	{
		RegisterLibraryGetFeaturesFunction regFnc;
		regFnc = (RegisterLibraryGetFeaturesFunction)
				dlib_find_symbol(lib_handle, func.c_str());
		if (regFnc != NULL)
		{
			regFnc();
			rc = 0;
		}
	}
	return rc;
}

void *registerLib()
{
	void *lib_handle = tryLoadLib("libixpdimm-cli");
	if (lib_handle && callDynamicFunc(lib_handle, "registerFeatures"))
	{
		dlib_close(lib_handle);
		lib_handle = NULL;
	}
	return lib_handle;
}

int unregisterLib(void *lib_handle)
{
	int rc = 0;
	if (lib_handle)
	{
		rc = callDynamicFunc(lib_handle, "unRegisterFeatures");
		dlib_close(lib_handle);
	}
	return rc;
}

/*
 * Entry point for ixpdimm-cli application
 */
int main(int argc, char * argv[])
{
	cli::framework::ResultBase *pResult = NULL;
	void *handle = NULL;

	/*
	 * L10n
	 */
	setlocale(LC_MESSAGES, ""); // GNU gettext will use catalomsg to translate marked strings
	// LC_CTYPE is responsible for determining character classes with the isalnum
	// etc functions from ctype. gettext doesn't do anything, but the clib will
	setlocale(LC_CTYPE, "");

	COMMON_PATH locale_dir;
	get_locale_dir(locale_dir);

	bindtextdomain(LOCALE_DOMAIN, locale_dir); // indicate where the domain catelog is
	textdomain(LOCALE_DOMAIN); // tell gettext which domain using

	cli::framework::Framework *pFrameworkInst = cli::framework::Framework::getFramework();

	int rc = 0;

	// use shared libraries to register feature sets
	if (!(handle = registerLib())) // ixpdimm-cli are required .. if they don't load fail
	{
		std::cout << "Couldn't load features" << std::endl;
	}
	// make sure we can get a connection to the db
	else if (!open_default_lib_store())
	{
		std::cout << "Couldn't connect to configuration database" << std::endl;
		unregisterLib(handle);
	}
	else
	{
		// Now that all features have been registered, we can attempt to execute the command
		cli::framework::StringList argList;

		// set executable name, ixpdimm-cli
		pFrameworkInst->executableName = argv[0];

		int i = 1; // skip utility name

	#ifndef RELEASE
		if(argc > 2 && std::string(argv[1]) == "debug")
		{
			cli::framework::ConsoleChannel *pChannel = new cli::framework::ConsoleChannel ();
			cli::framework::logger.setChannel(pChannel);
			cli::framework::logger.setLevel(cli::framework::LogMessage::PRIORITY_INFO);
			i = 2; // skip "debug
		}
	#endif
		// push back command arguments
		for (; i < argc; i++)
		{
			argList.push_back(argv[i]);
		}

		// create a context
		wbem::lib_interface::createNvmContext();

		// execute the CLI command
		pResult = pFrameworkInst->execute(argList);

		// if error, then set the return code
		rc = pResult->getErrorCode();

		std::cout << pResult->output() << std::endl;
		if (NULL != pResult)
		{
			delete pResult;
		}

		// delete the context
		wbem::lib_interface::freeNvmContext();

		// unregister features
		unregisterLib(handle);

		// close the connection to the db for this process
		close_lib_store();
	}
	return rc;
}
