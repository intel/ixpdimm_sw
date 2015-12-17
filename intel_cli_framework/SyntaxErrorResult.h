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
 * Sub class of the ErrorResult.  Used when there is a syntax error.
 * Adds syntax part to indicate which part of the command syntax is potentially wrong.
 */

#ifndef _CLI_FRAMEWORK_SYNTAXERRORRESULT_H_
#define _CLI_FRAMEWORK_SYNTAXERRORRESULT_H_

#include "ErrorResult.h"
#include "CommandSpec.h"

namespace cli
{
namespace framework
{
/*!
 * Sub class of the ErrorResult.  Used when there is a syntax error.
 * Adds syntax part to indicate which part of the command syntax is potentially wrong.
 */
class SyntaxErrorResult : public ErrorResult
{
public:
	/*!
	 * Constructor
	 */
	SyntaxErrorResult();

	/*!
	 * Constructor
	 */
	SyntaxErrorResult(std::string errorMessage);

	/*!
	 * accessor for potential commands of this error
	 * @return
	 */
	CommandSpecList getPotentialCommands();

	/*!
	 * setter for potential commands of this error
	 * @param commands
	 * 		The commands that the user may have intended
	 */
	void setPotentialCommands(CommandSpecList commands);

	
	/*!
	 * output the Syntax error
	 * @return
	 * 		returns the result message
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
	 * List of commands that the user might have intended before the syntax error was encountered.
	 */
	CommandSpecList m_potentialCommands;
};

}
}



#endif /* _CLI_FRAMEWORK_SYNTAXERRORRESULT_H_ */
