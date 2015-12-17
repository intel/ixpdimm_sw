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
 * Used when the command results are a list of key/value pairs.
 */

#ifndef _CLI_FRAMEWORK_PROPERTYLISTRESULT_H_
#define _CLI_FRAMEWORK_PROPERTYLISTRESULT_H_

#include "ResultBase.h"
#include "ErrorResult.h"
#include "SimpleResult.h"

#include <map>

namespace cli {
namespace framework {

const std::string propertyResultTrueStr = N_TR("true"); //!< true string
const std::string propertyResultFalseStr = N_TR("false"); //!< true string
const std::string propertyObjectNameStr = "Object"; //!< Object name string.

struct PropertyResult
{
	std::string key;
	SimpleResult *value;
};

typedef std::vector<PropertyResult> properties_t;

/*!
 * Used when the command results are a list of key/value pairs.
 */
class PropertyListResult : public ResultBase
{
public:
	PropertyListResult();

	/*!
	 * output
	 * @return
	 * 		Looks something like:
	 * 		key = value
	 * 		...
	 */
	virtual std::string outputText() const;

	/*!
	 * output to XML
	 * @return
	 * 	  returns the result as XML
	 */
	virtual std::string outputXml() const;

	/*!
	 * output to ESX XML
	 * @return
	 * 	  returns the result such that esx can understand it
	 */
	virtual std::string outputEsxXml() const;

	/*!
	 * output to ESX XML
	 * @return
	 * 	  returns the result such that esx can understand it in table format
	 */
	virtual std::string outputEsxXmlTable() const;

	/*!
	 * output to JSON
	 * @return
	 * 	  returns the result as JSON
	 */
	virtual std::string outputJson() const;

	/*!
	 * Add a new key value pair
	 * @param key
	 * 		The key
	 * @param value
	 * 		The value
	 */
	void insert(std::string key, std::string value);

	/*!
	 * Add a new key value pair where the value is an ErrorResult
	 * @param key
	 * @param value
	 */
	void insert(std::string key, ErrorResult *value);

	/*!
	 * Set the root for the property list.  Mostly used for generating the XML.
	 * @param root
	 */
	void setName(std::string root);


	/*!
	 * Get the number of Properties in the list
	 * @return
	 */
	int getCount();

	/*!
	 * return if the key is in the property list
	 * @param key
	 * @return
	 */
	bool contains(std::string key);

	/*!
	 * return the value for the key
	 * @param key
	 * @return
	 */
	std::string operator[] (const std::string &key);

	/*!
	 * Return the beginning iterator
	 * @return
	 */
	properties_t::const_iterator propertiesBegin() const;

	/*!
	 * Return the end iterator
	 * @return
	 */
	properties_t::const_iterator propertiesEnd() const;

private:
	properties_t m_properties;
	std::string m_root;
};

} /* namespace framework */
} /* namespace cli */
#endif /* _CLI_FRAMEWORK_PROPERTYLISTRESULT_H_ */
