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
 * This file contains the implementation of the Attribute class.
 * The Attribute class represents the different types of Attributes that an @ref Entity can have.
 * The current Attribute types are TEXT, INTEGER32, and INTEGER64.  There are various fluent API
 * functions that can further define an attribute is a Primary Key, Foreign Key, or array
 */

#include "Attribute.h"

std::string Attribute::getSqlType()
{
	std::string result = "";
	switch(m_type)
	{
	case INTEGER32:
	case INTEGER64:
		result = "INTEGER";
		break;
	case TEXT:
		result ="TEXT";
		break;
	case UNKOWN:
		result = "Attribute Type not set";
		break;
	}
	return result;
}

std::string Attribute::getCType()
{
	std::string result = "";
	switch(m_type)
	{
	case INTEGER32:
		result =  "int";
		break;
	case INTEGER64:
		result =  "long long";
		break;
	case TEXT:
		result =  "char *";
		break;
	case UNKOWN:
		result =  "Attribute Type not set";
		break;
	}

	if (m_isUnsigned)
	{
		result = "unsigned " + result;
	}

	return result;
}
std::string Attribute::getCFormatType()
{
	std::string result = "";
	switch(m_type)
	{
	case INTEGER32:
		result =  "%d";
		break;
	case INTEGER64:
		result =  "%lld";
		break;
	case TEXT:
		result =  "%s";
		break;
	case UNKOWN:
		result =  "Attribute Type not set";
		break;
	}

	if (m_isUnsigned)
	{
		result = "unsigned " + result;
	}

	return result;
}
