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
 * The parser uses a state machine to verify valid transitions
 * from token to token. Transitions from states are defined in the functions. If there are no valid
 * transitions a ParseErrorResult is returned with information about the token found and the
 * available transitions.
 */

#ifndef _CLI_FRAMEWORK_PARSER_H_
#define _CLI_FRAMEWORK_PARSER_H_

#include "CliFrameworkTypes.h"
#include "CommandSpec.h"
#include "ResultBase.h"
#include "ErrorResult.h"
#include "SyntaxErrorResult.h"
#include "FeatureBase.h"
#include "FeatureRef.h"

#include <map>

namespace cli
{
namespace framework
{

/*!
 * Parser class, responsible for verifying the syntax of a list of tokens
 */
class Parser
{
public:
	/*!
	 * Construct the parser
	 */
	Parser();

	/*!
	 * Parse the tokens
	 * @param tokens
	 * 		A list of tokens
	 * @return
	 * 		If success, parse will return NULL, otherwise it will return a syntax error
	 */
	SyntaxErrorResult *parse(const TokenList &tokens);

	/*!
	 * accessor for the parsed command
	 * @return
	 */
	ParsedCommand getParsedCommand();

	/*!
	 * Retrieve the specified property value from the parsed command
	 * @param parsedCommand
	 * 		The parsed command to search
	 * @param propertyName
	 * 		The property name to look for
	 * @param p_exists
	 * 		If the key exists, this will be set to true
	 * @return
	 * 		the value if found, else an empty string
	 */
	static std::string getPropertyValue(const framework::ParsedCommand& parsedCommand,
			const std::string &propertyName, bool *p_exists = NULL);

	/*!
	 * Retrieve the specified option value from the parsed command
	 * @param parsedCommand
	 * 		The parsed command to search
	 * @param optionName
	 * 		The option name to look for
	 * @param p_exists
	 * 		If the key exists, this will be set to true
	 * @return
	 * 		the value if found, else an empty string
	 */
	static std::string getOptionValue(const framework::ParsedCommand& parsedCommand,
			const std::string &optionName, bool *p_exists = NULL);

	/*!
	 * Retrieve the specified target value from the parsed command
	 * @param parsedCommand
	 * 		The parsed command to search
	 * @param targetName
	 * 		The target name to look for
	 * @param p_exists
	 * 		If the key exists, this will be set to true
	 * @return
	 * 		the value if found, else an empty string
	 */
	static std::string getTargetValue(const framework::ParsedCommand& parsedCommand,
			const std::string &targetName, bool *p_exists = NULL);

	/*!
	 * Retrieve the specified comma-separated list of target values from the parsed command
	 * @param parsedCommand
	 * 		The parsed command to search
	 * @param targetName
	 * 		The target name to look for
	 * @param p_exists
	 * 		If the key exists, this will be set to true
	 * @return
	 * 		the list of target values
	 */
	static std::vector<std::string> getTargetValues(
			const framework::ParsedCommand& parsedCommand,
			const std::string& targetName, bool *p_exists = NULL);

	bool includePotentialCommandsIsRequested();

	bool hasUnknownProperty() { return !m_unknownProperty.lexeme.empty(); }
	UnknownProperty getUnknownProperty() { return m_unknownProperty; }

private:
	// reset globals
	void initialize();

	// States of the state machine
	SyntaxErrorResult *stateVerb(int currentToken, const TokenList &tokens);
	SyntaxErrorResult *stateTarget(int currentToken, const TokenList &tokens);
	SyntaxErrorResult *stateTargetValue(int currentToken, const TokenList &tokens);
	SyntaxErrorResult *stateTargetValueOrProperty(int currentToken, const TokenList &tokens);
	SyntaxErrorResult *stateOption(int currentToken, const TokenList &tokens);
	SyntaxErrorResult *stateOptionValue(int currentToken, const TokenList &tokens);
	SyntaxErrorResult *stateOptionValueOrProperty(int currentToken, TokenList const &tokens);
	SyntaxErrorResult *stateProperty(int currentToken, const TokenList &tokens);
	SyntaxErrorResult *stateEquals(int currentToken, const TokenList &tokens);
	SyntaxErrorResult *stateComma(int currentToken, const TokenList &tokens, TokenType tokenType = TOKENTYPE_OPTION);
	SyntaxErrorResult *statePropertyValue(int currentToken, const TokenList &tokens);
	SyntaxErrorResult *statePropertyOrPropertyValue(int currentToken, const TokenList &tokens);

	ParsedCommand m_parsedCommand;

	// because the property/property value tokens aren't next to each other (there's an '='
	// in between) this member will store the last parsed property
	std::string m_currentProperty;
	std::string m_currentOption; // can be multiple values for a single option. Use this to concat them.
	std::string m_currentTarget; // can be multiple values for a single target value. Use this to concat them.
	bool m_ignorePotentialCommands; // if the framework should display help based on what the parse error might be

	// helper function to get[Target|Property|Option]Value()
	static std::string getValue(const StringMap& map,
			const std::string &keyName, bool *p_keyExists);

	void resolveOutputOption(cli::framework::SyntaxErrorResult *pSyntaxError,
		cli::framework::TokenList const &tokens);

	SyntaxErrorResult *createParseError(const Token &token);
	SyntaxErrorResult *createDuplicateTokenError(const cli::framework::Token &token);

	void setUnknownPropertyTo(const std::string &lexeme, const std::string &token,
			const cli::framework::CommandSpecPartType &type);

	UnknownProperty m_unknownProperty;
};
}
}

#endif /* _CLI_FRAMEWORK_PARSER_H_ */
