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

#include "PropertyListResult.h"
#include <sstream>


cli::framework::PropertyListResult::PropertyListResult() : m_root("Root")
{
	Trace(__FILE__, __FUNCTION__, __LINE__);
}

/*
 * Output the list of key value pairs
 */
std::string cli::framework::PropertyListResult::outputText() const
{
	Trace(__FILE__, __FUNCTION__, __LINE__);
	std::string result("");

	for (properties_t::const_iterator iter = m_properties.begin();
				iter != m_properties.end(); iter ++)
	{
		result += (*iter).key + ": " + (*iter).value->output() + "\n";
	}

	if (result.empty())
	{
		result = NoResultsStr + "\n";
	}
	return result;
}

std::string cli::framework::PropertyListResult::outputXml() const
{
	Trace(__FILE__, __FUNCTION__, __LINE__);

	std::stringstream result;
	// start tag
	result << "<" << m_root << ">";
	if (m_properties.size() > 0)
	{
		result << "\n";
	}
	// property list
	for (properties_t::const_iterator iter = m_properties.begin();
				iter != m_properties.end(); iter ++)
	{
		// spaces are not valid as tag names so replace them with underscores
		std::string key = (*iter).key;
		std::replace(key.begin(), key.end(), ' ', '_');
		result << "\t" << "<" << key << ">";
		result << (*iter).value->output();
		result << "</" << key << ">\n";
	}
	// end tag
	result << "</" << m_root << ">\n";
	return result.str();
}

std::string cli::framework::PropertyListResult::outputJson() const
{
	Trace(__FILE__, __FUNCTION__, __LINE__);

	std::stringstream result;
	//Open brace
	result << "\t{\n";
	
	// property list
	for (properties_t::const_iterator iter = m_properties.begin();
		iter != m_properties.end(); iter++)
	{
		std::string key = (*iter).key;
		result << "\t\t" << "\"" << key << "\":";
		
		std::string value = (*iter).value->output();

		//if the value is numeric then output the value without quotes.
		if (stringIsNumeric(value))
		{
			result << (*iter).value->output();
		}
		//if the value is "true" then JSON data type is bool; output with no quotes.
		else if (stringsIEqual(value, propertyResultTrueStr))
		{
			result << propertyResultTrueStr;
		}
		//if the value is "false" then JSON data type is bool; output with no quotes.
		else if (stringsIEqual(value, propertyResultFalseStr))
		{
			result << propertyResultFalseStr;
		}
		//all other cases treat value as a string and output with quotes.
		else
		{
			// For JSON output we need to replace double quotes with single quotes
			std::string output = (*iter).value->output();
			std::replace(output.begin(), output.end(), '\"', '\'');
			result << "\"" << output << "\"";
		}

		//Check the next iterator to see if we have reached the end
		properties_t::const_iterator temp_iter = iter + 1;
		if (temp_iter != m_properties.end())
		{
			result << ",";
		}
		result << "\n";
	}
	// end brace
	result << "\t}";
	return result.str();
}

std::string cli::framework::PropertyListResult::outputEsxXml() const
{
	Trace(__FILE__, __FUNCTION__, __LINE__);
	std::stringstream result;
	result << ESX_BEGIN_ROOTTAG;
	result << ESX_BEGIN_STRUCTLIST;
	for (properties_t::const_iterator iter = m_properties.begin();
			iter != m_properties.end(); iter ++)
	{
		result << ESX_BEGIN_KEYVALUESTRUCT;
		result << ESX_BEGIN_KEYFIELD;
		result << ESX_BEGIN_STRING << (*iter).key << ESX_END_STRING;
		result << ESX_END_FIELD;
		result << ESX_BEGIN_VALUEFIELD;
		result << ESX_BEGIN_STRING << (*iter).value->output() << ESX_END_STRING;
		result << ESX_END_FIELD;
		result << ESX_END_STRUCT;
	}

	result << ESX_END_LIST;
	result << ESX_END_ROOTTAG;
	return result.str();
}

std::string cli::framework::PropertyListResult::outputEsxXmlTable() const
{
	Trace(__FILE__, __FUNCTION__, __LINE__);
	std::stringstream result;
	result << ESX_BEGIN_ROOTTAG;
	result << ESX_BEGIN_STRUCTLIST;
	result << stringFromArgList(ESX_BEGIN_STRUCT.c_str(), m_root.c_str());
	for (properties_t::const_iterator iter = m_properties.begin();
			iter != m_properties.end(); iter ++)
	{
		result << stringFromArgList(ESX_BEGIN_FIELD.c_str(), (*iter).key.c_str());
		result << ESX_BEGIN_STRING << (*iter).value->output() << ESX_END_STRING;
		result << ESX_END_FIELD;
	}
	result << ESX_END_STRUCT;
	result << ESX_END_LIST;
	result << ESX_END_ROOTTAG;
	return result.str();
}

/*
 * Add a new key value pair to the internal collection
 */
void cli::framework::PropertyListResult::insert(std::string key, std::string value)
{
	Trace(__FILE__, __FUNCTION__, __LINE__);
	struct PropertyResult item;
	item.key = key;
	item.value = new SimpleResult(value);
	m_properties.push_back(item);
}

/*
 * Add a new key value pair to the internal collection
 */
void cli::framework::PropertyListResult::insert(std::string key, ErrorResult *value)
{
	Trace(__FILE__, __FUNCTION__, __LINE__);
	struct PropertyResult item;
	item.key = key;
	item.value = value;
	m_properties.push_back(item);
}

int cli::framework::PropertyListResult::getCount()
{
	Trace(__FILE__, __FUNCTION__, __LINE__);
	return m_properties.size();
}

void cli::framework::PropertyListResult::setName(std::string root)
{
	Trace(__FILE__, __FUNCTION__, __LINE__);
	m_root = root;
}

std::string cli::framework::PropertyListResult::operator [](const std::string& key)
{
	Trace(__FILE__, __FUNCTION__, __LINE__);
	std::string value = "";
	for(properties_t::const_iterator iter = m_properties.begin();
			iter != m_properties.end(); iter++)
	{
		if (stringsIEqual(iter->key, key))
		{
			value = iter->value->output();
			break;
		}
	}
	return value;
}

bool cli::framework::PropertyListResult::contains(std::string key)
{
	Trace(__FILE__, __FUNCTION__, __LINE__);
	bool found = false;
	for (properties_t::const_iterator iter = m_properties.begin();
			iter != m_properties.end(); iter++)
	{
		if (stringsIEqual(iter->key, key))
		{
			found = true;
			break;
		}
	}
	return found;
}

cli::framework::properties_t::const_iterator cli::framework::PropertyListResult::propertiesBegin() const
{
	Trace(__FILE__, __FUNCTION__, __LINE__);
	return m_properties.begin();
}

cli::framework::properties_t::const_iterator cli::framework::PropertyListResult::propertiesEnd() const
{
	Trace(__FILE__, __FUNCTION__, __LINE__);
	return m_properties.end();
}
