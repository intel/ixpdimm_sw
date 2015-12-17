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
 * Declaration of the framework's main entry point
 */

#ifndef _CLI_FRAMEWORK_FRAMEWORK_H_
#define _CLI_FRAMEWORK_FRAMEWORK_H_

#include "CliFrameworkTypes.h"
#include "CommandSpec.h"
#include "ResultBase.h"
#include "ErrorResult.h"
#include "SyntaxErrorResult.h"
#include "FeatureBase.h"
#include "Parser.h"

namespace cli
{
namespace framework
{

typedef std::map<std::string, cli::framework::FeatureBase*> featureList; //!< List of Features

/*!
 * Static class that contains the main entry point to the framework
 */
class Framework
{
public:
	/*!
	 * Main entry point into the framework. Parses the user's input and tries to match it against
	 * registered features.  Upon successful match it will run that feature.
	 * @param tokenCount
	 * 		Number of strings in the user's input
	 * @param tokens
	 * 		The user's input as a list of strings
	 * @return
	 * 		The result from the feature
	 */
	ResultBase* execute(int tokenCount, const char *tokens[]);

	/*!
	 * Main entry point into the framework. Parses the user's input and tries to match it against
	 * registered features.  Upon successful match it will run that feature.
	 * @param tokens
	 * 		List of strings as a vector
	 * @return
	 * 		The result from the feature
	 */
	ResultBase* execute(const StringList &tokens);

	/*!
	 * Name of the executable that is running the CLI framework.  It is expected that it is set
	 * before the execute method is run.  It's mostly used within showing help to the user.
	 */
	std::string executableName;

	/*!
	 * From the registered features, build a list of CommandSpecs that are mapped back to the
	 * feature they belong to.
	 * @return
	 * 		The list of Commands
	 */
	CommandSpecList getRegisteredCommands();

	/*!
	 * From the registered features, build a list of CommandSpecs that are mapped back to the feature they
	 * belong to.
	 * @return
	 * 		The list of CommandRefs
	 */
	void registerFeature(std::string featurename, cli::framework::FeatureBase *featureInst);

	/*!
	 * Get the single instance of Framework
	 */
	static Framework* getFramework();

	/*!
	 * Remove a particular feature from the feature list & free it's resources
	 */
	void removeFeature(std::string featureName);

	/*!
	 * Clear the feature registry list & free resources
	 */
	void clearFeatureList();

	~Framework()
	{
		instanceFlag = false;
		clearFeatureList();
	}

private:
	/*!
	 * static single instance of Framework class
	 */
	static Framework *pFrameworkInst;

	/*!
	 * Flag to know if we have our instance yet
	 */
	static bool instanceFlag;

	/*!
	 * Parse a list of strings into identified tokens
	 * @param tokenCount
	 * 		Number of input tokens
	 * @param tokens
	 * 		input tokens
	 * @param allCommandSpecs
	 * 		A list of all CommandSpecs utility knows about. This is used to get all valid token types
	 * @return
	 * 		The list of resulting tokens
	 */
	TokenList tokenize(const int tokenCount, const char *input[], CommandSpecList allCommandSpecs);

	featureList m_featureList;

	/*!
	 * Internal helper function for displaying a list of tokens as a sentence
	 * @param tokenCount
	 * @param tokens
	 * @return
	 */
	std::string tokenArrayToString(TokenList tokens);

	/*!
	 * Internal helper function for displaying a list of tokens as a sentence
	 * @param tokenCount
	 * @param tokens
	 * @return
	 */
	std::string tokenArrayToString(const int tokenCount, const char *tokens[]);

	FeatureBase *getCommandFeature(const CommandSpec &command);

	void addDefaultOptions(cli::framework::CommandSpec &command);
};

}
}

#endif /* FRAMEWORK_H_ */
