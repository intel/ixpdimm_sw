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
 * Represents the syntax spec for a command including what verb, targets, and properties are supported
 * and which are required.
 */

#ifndef _CLI_FRAMEWORK_COMMANDSPEC_H_
#define _CLI_FRAMEWORK_COMMANDSPEC_H_

#include <string>
#include <vector>
#include "CliFrameworkTypes.h"

namespace cli
{
namespace framework
{

/*!
 * Represents the syntax spec for a command including what verb, targets, and properties are supported
 * and which are required.
 */
class CommandSpec
{
public:
	/*!
	 * Default Constructor
	 */
	CommandSpec();

	/*!
	 * Initialize a new Command Spec
	 * @param id
	 * 		id is used to easily identify the command spec.  Needs to be unique within a feature.
	 * @param name
	 * 		String name of the command
	 * @param verb
	 * 		All commands must have exactly one verb
	 * @param help
	 * 		A helpful comment about the command
	 */
	CommandSpec(int id, std::string name, std::string verb, std::string help = "");

	int id; //!< unique id for the command spec within the feature
	std::string name; //!< name of the command
	std::string verb; //!< verb storage
	CommandSpecPartList options; //!< options storage
	CommandSpecPartList targets; //!< targets storage
	CommandSpecPartList properties; //!< properties storage
	std::string help; //!< help information for the command

	/*!
	 * Return a string representing the command spec
	 * @return
	 * 		string
	 */
	std::string asStr() const;

	/*!
	 * Add an option
	 * @param part
	 */
	CommandSpecPartDecorator addOption(const CommandSpecPart &part);

	/*!
	 * Add an option
	 * @param name
	 * @param isRequired
	 * @param valueText
	 * @param isValueRequired
	 * @param helpText
	 */
	CommandSpecPartDecorator addOption(const std::string &name, const bool isRequired = false,
			const std::string &valueText = "", const bool isValueRequired = false,
			const std::string &helpText = "", const std::string &abr = "");
	/*!
	 * Add a target
	 * @param part
	 */
	CommandSpecPartDecorator addTarget(const CommandSpecPart &part);
	/*!
	 * Add a target
	 * @param name
	 * @param isRequired
	 * @param valueText
	 * @param isValueRequired
	 * @param helpText
	 */
	CommandSpecPartDecorator  addTarget(const std::string &name, const bool isRequired = false,
			const std::string &valueText = "", const bool isValueRequired = false,
			const std::string &helpText = "");
	/*!
	 * Add a property
	 * @param part
	 */
	CommandSpecPartDecorator  addProperty(const CommandSpecPart &part);
	/*!
	 * Add a property
	 * @param name
	 * @param isRequired
	 * @param valueText
	 * @param isValueRequired
	 * @param helpText
	 */
	CommandSpecPartDecorator  addProperty(const std::string &name, const bool isRequired = false,
			const std::string &valueText = "", const bool isValueRequired = false,
			const std::string &helpText = "");

	CommandSpecPartList &getCommandSpecPartList(TokenType type);

	// getter/setter for the Feature this commandspec belongs to. Set by the Framework as it's
	// putting the complete list of commands together. The reason why these are void * and not
	// FeatureBase * is to avoid circular dependencies. Need navigation both ways, but C++ makes
	// that difficult
	void *getFeature() const;
	void setFeature(void *);

private:
	static CommandSpecPartDecorator addPart(CommandSpecPartList &partList, const CommandSpecPart &part);
	std::string listToString(const CommandSpecPartList &list, bool isProperty = false) const;
	void *m_pFeature;
};

typedef std::vector<CommandSpec> CommandSpecList; //!< List of CommandSpecs
}
}

#endif
