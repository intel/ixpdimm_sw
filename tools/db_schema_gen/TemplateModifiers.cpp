/*
 * Copyright (c) 2015, Intel Corporation
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
 * This file contains the implementation of a variety of TemplateModifier classes.
 * These classes are registered in CrudSchemaGenerator and used within the schema.c.template
 * file to slightly modify template markers
 */

#include "TemplateModifiers.h"
#include <iostream>
#include <algorithm>

/*
 * Remove the '*' from a string
 */
void ReplaceModifier::Modify(const char* in, size_t inlen,
		const ctemplate::PerExpandData* per_expand_data,
		ctemplate::ExpandEmitter* outbuf, const std::string& compare) const
{

	std::string input(in, inlen);
	char star = '*';
	size_t i = input.find(star, 0);
	if (i != std::string::npos)
	{
		input = input.replace(i, 1, 1, ' ');
	}

	outbuf->Emit(input);
}

void SingleModifier::Modify(const char* in, size_t inlen,
		const ctemplate::PerExpandData* per_expand_data,
		ctemplate::ExpandEmitter* outbuf, const std::string& compare) const
{

	std::string input(in, inlen);
	if (input[inlen - 1] == 's')
	{
		input = input.substr(0, inlen - 1);
	}
	outbuf->Emit(input);

}

void CapsModifier::Modify(const char* in, size_t inlen,
		const ctemplate::PerExpandData* per_expand_data,
		ctemplate::ExpandEmitter* outbuf, const std::string& arg) const
{
	std::string input(in, inlen);
	std::transform(input.begin(), input.end(), input.begin(), ::toupper);
	outbuf->Emit(input);
}
