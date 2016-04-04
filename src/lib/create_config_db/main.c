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
 * This file simply calls the create_PersistentStore and closes it to generate the
 * config database at build time.
 */

#include <stdio.h>
#include <common_types.h>
#include <persistence/lib_persistence.h>

/*
 * Entry point for a simple tool to create the config database at build time.
 * @param arg_count
 * 		Implicitly defined
 * @param args
 * 		It is expected that the user's first argument will be an absolute path
 * 		for the directory where the configuration db will be output.
 * @return 0
 */
int main(int arg_count, char **args)
{
	int rc = 1;
	char path[COMMON_PATH_LEN];

	// if the caller passed in a path, prepend it to the CONFIG_FILE
	if (arg_count >= 2)
	{
		snprintf(path, COMMON_PATH_LEN, "%s/%s", args[1], CONFIG_FILE);
	}
	else
	{
		snprintf(path, COMMON_PATH_LEN, "%s", CONFIG_FILE);
	}

	if (create_default_config(path) == 0)
	{
		// output that the the DB file was created successfully
		printf("Created %s\n", path);
		rc = 0;
	}
	else
	{
		// output that something failed
		printf("There was an issue creating %s\n", path);
	}

	return rc;
}
