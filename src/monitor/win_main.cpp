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
 * This file contains the entry point for the NvmMonitor service on Windows.
 */


#include <cr_i18n.h>
#include "win_service.h"

#define SERVICE_NAME "ixpdimm-monitor"
#define SERVICE_DISPLAY_NAME "Intel ixpdimm-monitor"
#define SUCCESS TR("Success")
#define FAIL TR("Fail")

void usage(int argc, char **argv);

int main(int argc, char **argv)
{
	bool success = true;
	if (argc == 2)
	{
		std::string arg = argv[1];

		if (arg == "install")
		{
			success = serviceInstall(SERVICE_NAME, SERVICE_DISPLAY_NAME);
			printf(TR("Service Install: %s"), (success ? SUCCESS : FAIL));
		}
		else if (arg == "uninstall")
		{
			success = serviceUninstall(SERVICE_NAME) == COMMON_SUCCESS;
			printf(TR("Service Uninstall: %s"), (success ? SUCCESS : FAIL));
		}
		else
		{
			usage(argc, argv);
		}
	}
	else if (argc == 1)
	{
		// Being called by SCM so do service work
		success = serviceInit(SERVICE_NAME);
		usage(argc, argv); // print usage in case attempting from command line
	}
	else
	{
		usage(argc, argv);
	}

	return success ? 0 : -1;
}

void usage(int argc, char **argv)
{
	printf(TR("Usage: %s {install|uninstall}\n"), argv[0]);
}
