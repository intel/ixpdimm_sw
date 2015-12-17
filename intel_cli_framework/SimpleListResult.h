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

/*!
 * Used when the command results in multiple SimpleResults
 */

#ifndef _CLI_FRAMEWORK_SIMPLELISTRESULT_H_
#define _CLI_FRAMEWORK_SIMPLELISTRESULT_H_

#include "ResultBase.h"

#include "SimpleResult.h"

namespace cli
{
namespace framework
{

/*!
 * Defines a map of PropertyListResults
 */
typedef std::vector<cli::framework::SimpleResult*> simpleResults_t;

/*!
 * SimpleListResult XML Tag
 */
const std::string simpleListResultXmlTag = "Results";

/*!
 * Used when the command results in multiple SimpleResults
 */
class SimpleListResult : public ResultBase
{
	public:
		/*!
		 * Constructor
		 */
		SimpleListResult();

		/*!
		 * Destructor
		 */
		virtual ~SimpleListResult();

		/*!
		 * Retrieve output in text format
		 * @return
		 * 		Displays a collection of SimpleResults
		 */
		std::string outputText() const;

		/*!
		 * Retrieve output in XML format
		 * @return
		 * 		Displays a collection of SimpleResults
		 */
		std::string outputXml() const;

		/*!
		 * Retrieve output in the ESX XML format
		 * @return
		 * 		Returns a string in the ESX XML format.
		 */
		std::string outputEsxXml() const;

		/*!
		 * Retrieve output in JSON format
		 * @return
		 * 		Displays a collection of SimpleResults in JSON format.
		 */
		std::string outputJson() const;

		/*!
		 * Insert a result
		 * @param pResult
		 * 		The simple result to insert
		 */
		void insert(cli::framework::SimpleResult *pResult);

		/*!
		 * * Insert a simple string result
		 * @param stringResult
		 * 		A string that will get added as a SimpleResult
		 */
		void insert(const std::string &stringResult);

		/*!
		 * Retrieve the number of results
		 */
		int getCount();

		/*!
		 * Retrieve the beginning of the property objects list.
		 * @return
		 * 		An iterator pointing to the beginning of the property objects list.
		 */
		simpleResults_t::iterator resultsBegin();

		/*!
		 * Retrieve the end of the property objects list.
		 * @return
		 * 		An iterator pointing to the end of the property objects list.
		 */
		simpleResults_t::iterator resultsEnd();

	private:
		simpleResults_t m_results;

};

} /* namespace framework */
} /* namespace cli */
#endif /* _CLI_FRAMEWORK_SIMPLELISTRESULT_H_ */
