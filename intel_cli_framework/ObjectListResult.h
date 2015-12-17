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
 * Used when the command results in multiple PropertyListResults.
 */

#ifndef _CLI_FRAMEWORK_OBJECTLISTRESULT_H_
#define _CLI_FRAMEWORK_OBJECTLISTRESULT_H_

#include "ResultBase.h"

#include "PropertyListResult.h"
#include <map>

namespace cli
{
namespace framework
{

const std::string ObjectListResultListXmlTag = "List"; //!< List Tag

/*!
 * Defines a map of PropertyListResults
 */
typedef std::vector<std::pair<std::string, PropertyListResult> > propertyObjects_t;


/*!
 * Used when the command results in multiple PropertyListResults.
 */
class ObjectListResult : public ResultBase
{
	public:
		ObjectListResult();
		/*!
		 * output
		 * @return
		 * 		Displays a collection of PropertyListResults
		 */
		std::string outputText() const;

		/*!
		 * output to XML
		 * @return
		 * 		returns the result as XML
		 */
		std::string outputXml() const;

		/*!
		 * output to ESX XML
		 * @return
		 * 		returns the result such that esx can understand it
		 */
		std::string outputEsxXml() const;

		/*!
		 * output to ESX XML Table
		 * @return
		 * 		returns the result such that esx can understand it as a table
		 */
		std::string outputEsxXmlTable() const;

		/*!
		 * output a text table
		 * @return
		 * 		returns the result as a table
		 */
		std::string outputTextTable() const;

		/*!
		 * output to JSON
		 * @return
		 * 		returns the result as JSON
		 */
		std::string outputJson() const;

		/*!
		 * Insert a Key/Value pair where the value is a PropertyListResult
		 * @param key
		 * 		The key to insert
		 * @param value
		 * 		The value to insert
		 */
		void insert(const std::string &key, PropertyListResult &value);

		/*!
		 * set the root of the Object List.  Mostly for XML output.
		 * @param root
		 */
		void setRoot(const std::string &root);

		/*!
		 * Retrieve the number of objects
		 */
		int getCount() const;

		/*!
		 * Retrieve the beginning of the property objects list.
		 * @return
		 * 		An iterator pointing to the beginning of the property objects list.
		 */
		propertyObjects_t::iterator objectsBegin();

		/*!
		 * Retrieve the end of the property objects list.
		 * @return
		 * 		An iterator pointing to the end of the property objects list.
		 */
		propertyObjects_t::iterator objectsEnd();

	private:
		unsigned int getColumnWidth(const std::string key) const; // helper fn for table output
		propertyObjects_t m_objects;
		std::string m_root;

};

} /* namespace framework */
} /* namespace cli */
#endif /* _CLI_FRAMEWORK_OBJECTLISTRESULT_H_ */
