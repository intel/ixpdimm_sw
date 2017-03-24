/*
 * Copyright (c) 2017, Intel Corporation
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
#include "fwcmd_args.h"
#include <common/string/s_str.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void add_arg(fwcmd_args *p_args, const char *name, const char *value)
{
	s_strcpy(p_args->arg[p_args->count].name, name,
		sizeof(p_args->arg[p_args->count].name));
	s_strcpy(p_args->arg[p_args->count].value, value,
		sizeof(p_args->arg[p_args->count].value));
	p_args->count++;
}

char *find_arg(fwcmd_args *p_args, const char *name)
{
	for (size_t i = 0; i < p_args->count; i++)
	{
		if (s_strncmp(p_args->arg[i].name, name,
			s_strnlen(p_args->arg[i].name,
				sizeof(p_args->arg[i].name))) == 0)
		{
			return p_args->arg[i].value;
		}
	}
	return NULL;
}

int parse_args(fwcmd_args *p_args, const char *str)
{
	int result = 0;
	const char *eq = strchr(str,'=');
	const char *end = strchr(str, '\0');

	if (eq && end)
	{
		memcpy(p_args->arg[p_args->count].name, str, eq - str);
		memcpy(p_args->arg[p_args->count].value, eq+1, end - eq);
		p_args->count++;
		result = 1;
	}

	return result;
}