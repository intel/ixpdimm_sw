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
 * Some common/utility free functions used within the CLI framework.
 * For example enum to string functions and definition of the internal logging.
 */

#ifndef _CLI_FRAMEWORK_CLIFRAMEWORKTYPES_H_
#define _CLI_FRAMEWORK_CLIFRAMEWORKTYPES_H_

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <ostream>
#include <algorithm>
#include "Logger.h"

#include "Trace.h"
#include "cr_i18n.h"

namespace cli
{
namespace framework
{
static const std::string SUCCESS_MSG = N_TR("Success"); //!< Command success string
static const std::string UNCHANGED_MSG = N_TR("Unchanged"); //!< no modification done

/*!
 * Returns the lower case version of the string passed
 * @param value
 * 		The string to change
 * @return
 * 		lower case version of value passed
 */
std::string toLower(const std::string &value);


/*!
 * Returns a copy of value with all leading and trailing whitespace removed.
 * @param value
 * 		string to trim
 * @return
 * 		trimmed version of the value passed
 */
std::string trim(const std::string &value);

/*!
 * Utility method to tokenize a string
 */
std::vector<std::string> tokenizeString(const char *str, char delimeter);

/*!
 * Utility method to compare two strings ignoring case
 */
bool stringsIEqual(const std::string &str1, const std::string &str2);

/*!
 * Utility method to check if a string is numeric. Will ignore '.' decimal point and leading '-' (negative numbers).
 */
bool stringIsNumeric(const std::string &str);

/*!
 * logger for the framework.  Different Channels can be set to change where the logs go.
 * By default doesn't output anything
 */
extern cli::framework::Logger logger;

/*!
 * The different types of a token
 */
enum TokenType
{
	TOKENTYPE_UNKNOWN = 0,
	TOKENTYPE_VALUE = 1,
	TOKENTYPE_VERB = 2,
	TOKENTYPE_OPTION = 3,
	TOKENTYPE_TARGET = 4,
	TOKENTYPE_PROPERTY = 5,
	TOKENTYPE_EQUAL = 6,
	TOKENTYPE_COMMA = 7
};

enum CommandSpecPartType
{
	PARTTYPE_OPTION = 0,
	PARTTYPE_TARGET = 1,
	PARTTYPE_PROPERTY = 2
};

/*!
 * Simple enum to string function
 * @param type
 * 		token type
 * @return
 * 		string representation of the token type
 */
std::string tokenTypeToString(TokenType type);

/*!
 * Defines a token within a CommandSpec
 */
typedef struct
{
	std::string name; //!< name of the token
	bool required; //!< is it required to be a successful match
	std::string value; //!< string indicating the value for the part. Used for help.
	bool valueRequired; //!< whether the option/target/property requires a value or not
	std::string help; //!< a helpful comment
	std::string abr; //!< abbreviated form of the name - typically only applies to options
	bool noValueAccepted; //!< whether the option/target/property accepts a value or not
} CommandSpecPart;

/*!
 * Wraps a CommandSpecPart to make it easier and more readable to update specific fields. Used
 * by a CommandSpec when adding command parts.
 */
class CommandSpecPartDecorator
{
public:
	CommandSpecPartDecorator(CommandSpecPart &part): m_Part(part){}
	inline CommandSpecPartDecorator &isRequired(bool value = true) { m_Part.required = value; return *this; }
	inline CommandSpecPartDecorator &isValueRequired(bool value = true) {m_Part.valueRequired = value; return *this;}
	inline CommandSpecPartDecorator &isValueAccepted(bool value = true) {m_Part.noValueAccepted = !value; return *this;}
	inline CommandSpecPartDecorator &helpText(const std::string &help) {m_Part.help = help; return *this;}
	inline CommandSpecPartDecorator &valueText(const std::string &value) {m_Part.value = value; return *this;}
	inline CommandSpecPartDecorator &abbreviation(const std::string &value) {m_Part.abr = value; return *this;}


private:
	CommandSpecPart &m_Part;
};

/*!
 * Known token with type
 */
typedef struct
{
	std::string lexeme; //!< the lexeme of the token
	TokenType tokenType; //!< type of the token
} Token;

typedef struct
{
	std::string lexeme; // string value that could be a property or a token value
	std::string tokenKey; // name of the token that would take the lexeme as a value
	CommandSpecPartType type; // type of the tokenKey
} UnknownProperty;

/*!
 * List of CommandSpec Tokens
 */
typedef std::vector<CommandSpecPart> CommandSpecPartList;

typedef std::vector<Token> TokenList; //!< List of tokens
typedef std::vector<TokenType> TokenTypeList; //!< List of token types
typedef std::vector<std::string> StringList; //!< List of strings

typedef std::map<std::string, std::string> StringMap; //!< Map with key/value pairs of strings

/*!
 * Return the index of a lexeme within a CommandSpecPartList
 */
int getTokenIndex(const std::string &lexeme, const cli::framework::CommandSpecPartList &tokenList);

/*!
 * Defines the completely parsed command inputed by the user
 */
typedef struct
{
	std::string verb; //!< the parsed verb
	StringMap options; //!< the parsed options and values
	StringMap targets; //!< the parsed targets and values
	StringMap properties; //!< the parsed properties and values
} ParsedCommand;

/*!
 * Does the parsed command contain the command part?
 * @param parsedCommand parsed command to search
 * @param cmdPart command part caller is seeking
 */
bool parsedCommandContains(const ParsedCommand& parsedCommand, const CommandSpecPart& cmdPart);

/*!
 * Common Command parts
 */
// supported verbs
const std::string VERB_SHOW = "show"; //!< show
const std::string VERB_START = "start"; //!< start
const std::string VERB_CREATE = "create"; //!< create
const std::string VERB_DUMP = "dump"; //!< dump
const std::string VERB_SET = "set"; //!< set
const std::string VERB_LOAD = "load"; //!< load
const std::string VERB_DELETE = "delete"; //!< delete
const std::string VERB_HELP = "help"; //!< help
const std::string VERB_VERSION = "version"; //!< version
const std::string VERB_RESET = "reset"; //!< reset

const std::string OPTION_SOURCE_NAME = "-source";

const std::string OUTPUT_OPTION = "-output";
const std::string OPTION_OUTPUT_TEXT = "text"; //!< output type text
const std::string OPTION_OUTPUT_XML = "nvmxml"; //!< output type nvmxml
const std::string OPTION_OUTPUT_ESX = "esx"; //!< output type esx
const std::string OPTION_OUTPUT_JSON = "json"; //!< output type json
const std::string OPTION_OUTPUT_ESXTABLE = "esxtable"; //!< output type esx

// List of valid output types
const std::string OUTPUT_TYPES = OPTION_OUTPUT_TEXT
		+ "|" + OPTION_OUTPUT_XML
#ifdef CLI_OUTPUT_JSON
		+ "|" + OPTION_OUTPUT_JSON
#endif
#ifdef CLI_OUTPUT_ESX
		+ "|" + OPTION_OUTPUT_ESX
		+ "|" + OPTION_OUTPUT_ESXTABLE
#endif
		;

// options
const CommandSpecPart OPTION_ALL = {"-all", false, "", false, N_TR("Show all attributes."), "-a", true}; //!< OPTION_ALL
const CommandSpecPart OPTION_DISPLAY = {"-display", false, "Attributes", true,
		N_TR("Filter the returned attributes by explicitly specifying a comma separated list of attributes."), "-d"}; //!< OPTION_DISPLAY
const CommandSpecPart OPTION_WAIT = {"-wait", false, "", false, N_TR("Wait for the command to finish before returning."), "-w", true}; //!< OPTION_WAIT
const CommandSpecPart OPTION_FORCE = {"-force", false, "", false, N_TR("Force the operation"), "-f", true}; //!< OPTION_FORCE
const CommandSpecPart OPTION_EXAMINE = {"-examine", false, "", false, N_TR("Examine the source file"), "-x", true}; //!< OPTION_EXAMINE
const CommandSpecPart OPTION_SOURCE = {OPTION_SOURCE_NAME, false, "path", true, N_TR("Path to the source file.")}; //!< OPTION_SOURCE
const CommandSpecPart OPTION_SOURCE_R = {OPTION_SOURCE_NAME, true, "path", true, N_TR("Path to the source file.")}; //!< OPTION_SOURCE
const CommandSpecPart OPTION_DESTINATION = {"-destination", false, "path", true, N_TR("Path to the destination file.")}; //!< OPTION_DESTINATION
const CommandSpecPart OPTION_DESTINATION_R = {"-destination", true, "path", true, N_TR("Path to the destination file.")}; //!< required OPTION_DESTINATION
const CommandSpecPart OPTION_OUTPUT = {OUTPUT_OPTION, false, OUTPUT_TYPES, true, N_TR("Change the output format."), "-o"}; //!< OPTION_OUTPUT
const CommandSpecPart OPTION_HELP = {"-help", false, "", false, N_TR("Display help for the command."), "-h"}; //!< OPTION_HELP
const CommandSpecPart OPTION_UNITS = {"-units", false, "GB|GiB", false, N_TR("Change the units the capacities are displayed in.")}; //!< OPTION_UNITS

// properties used within framework features
const CommandSpecPart PROPERTY_VERB = {"verb", false, "verb", true, N_TR("Filter help to a specific verb.")}; //!< PROPERTY_VERB
const CommandSpecPart PROPERTY_COMMANDNAME = {"Name", false, "command", true, N_TR("Filter help to a specific command by name.")}; //!< PROPERTY_COMMANDNAME

}
}
#endif /* CLIFRAMEWORKTYPES_H_ */
