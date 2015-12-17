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
 * Result class used when the response is a short text message.
 */

#ifndef _CLI_FRAMEWORK_SIMPLERESULT_H_
#define _CLI_FRAMEWORK_SIMPLERESULT_H_

#include "ResultBase.h"
#include <iostream>

namespace cli
{
namespace framework
{

/*!
 * SimpleResult XML Tag
 */
const std::string simpleResultXmlTag = "Result";

/*!
 * Result class used when the response is a short text message.
 */
class SimpleResult : public ResultBase
{
public:
	/*!
	 * Result class used when the response is a short text message.
	 * @param result
	 * 		The result string that will be returned to the user
	 */
	SimpleResult(const std::string &result);

	/*!
	 * output to the user
	 * @return
	 * 		the result string
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
	 * output to JSON
	 * @return
	 * 	  returns the result as JSON
	 */
	virtual std::string outputJson() const;

protected:
	/*!
	 * result string to print in output
	 */
	std::string m_result;

};

}

}



#endif /* SIMPLERESULT_H_ */
