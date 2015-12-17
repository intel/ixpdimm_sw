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
 * Base class for all features
 */

#ifndef _CLI_FRAMEWORK_FEATUREBASE_H_
#define _CLI_FRAMEWORK_FEATUREBASE_H_

#include <string>
#include "CommandSpec.h"
#include "ResultBase.h"
#include "SyntaxErrorResult.h"

#include "CliFrameworkTypes.h"
#include <vector>
#include <iostream>

namespace cli
{
namespace framework
{
static const std::string YES_OR_NO = TR("(y or [n])"); // yes or no prompt choices:lower case only
static const std::string PROMPT_YES = "y"; // lower case yes answer to y or n prompt

/*!
 * Base class for all features
 * @todo: With the constructor parameters and the "run" parameters there is some redundancy in
 * what a feature is passed. Need to understand better how Features will or will not use these.
 */
class FeatureBase
{

public:
	/*!
	 * Base constructor
	 */
	FeatureBase();
	virtual ~FeatureBase() {}; //!< Deconstructor

	/*!
	 * Abstract function.  After a feature has been found match user input, it's 'run' function
	 * @param commandSpecId
	 * 		Id of the commandSpec matched
	 * @param parsedCommand
	 * 		The user input parsed into a command
	 * @return
	 * 		The result from the feature
	 */
	virtual ResultBase* run(const int &commandSpecId, const ParsedCommand &parsedCommand) = 0;

	/*!
	 * Get the command spec for the feature.  Necessary for registering the feature with the framework.
	 * @param list List of CommandSpecs on return
	 */
	virtual void getPaths(cli::framework::CommandSpecList &list) = 0;

	/*!
	 * Prompt user and give them a choice to answer yes or no to the prompt.
	 * @return
	 * 		true if user answered yes
	 * 		false otherwise
	 */
	bool promptUserYesOrNo(std::string prompt);

	/*
	 * Prompt user for a string response to the prompt.
	 */
	std::string promptUserHiddenString(std::string prompt);

	/*!
	 * Read user input.
	 * @return
	 * 		true if user answered "Y" or "y"
	 * 		false otherwise
	 */
	static bool checkUserResponse();

	/*!
	 * Method that can be overridden for testing purposes.
	 * Defaults to just calling checkUserResponse above.
	 */
	bool (*m_checkUserResponse)();

	static void readUserHiddenString(std::string *p_strResponse);

	/*!
	 * Method that can be overridden for testing purposes.
	 * Defaults to just calling os_readUserHiddenString in osAdapter.
	 */
	void (*m_readUserHiddenString)(std::string *p_strResponse);

};

typedef std::vector<FeatureBase> FeatureList; //!< List of Features
typedef std::vector<FeatureBase *> P_FeatureList; //!< List of Pointers to Features
/*!
 * Function pointer for Feature Factory
 */
typedef std::vector<cli::framework::FeatureBase* (*)(int count, const char **tokens[])> FeatureFactoryList;


} /* namespace framework*/
} /* namespace cli */

#endif
