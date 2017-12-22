/*
 * Copyright (c) 2015 2016, Intel Corporation
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
 * Helper class that interprets and wraps CLI inputs for memory provisioning
 */

#ifndef MEMORYPROPERTY_H_
#define MEMORYPROPERTY_H_

#include <string>

#include <common_types.h>
#include <nvm_management.h>
#include <libinvm-cli/Parser.h>
#include <libinvm-cli/SyntaxErrorResult.h>
#include <mem_config/InterleaveSet.h>
#include <cli/features/ExportCli.h>

namespace cli
{
namespace nvmcli
{

/*
 * Helper class to wrap a Memory Mode or App Direct Property
 */
class NVM_CLI_API MemoryProperty
{
public:
	MemoryProperty(const framework::ParsedCommand& parsedCommand,
			const std::string& sizePropertyName,
			const std::string& settingsPropertyName = "");

	/*
	 * Is defined as the "remaining" capacity
	 */
	bool getIsRemaining() const { return framework::stringsIEqual(m_sizeString, wbem::mem_config::SIZE_REMAINING); }

	/*
	 * Is it mirrored
	 */
	bool getIsMirrored() const;

	/*
	 * Is interleave set 1 way
	 */
	bool getIsByOne() const;

	/*
	 * Is the size passed in a valid number
	 */
	bool getIsSizeValidNumber() const;

	/*
	 * Is the settings passed in a valid settings
	 */
	bool getIsSettingsValid() const { return m_settingsValid; }

	/*
 	 * Is the settings passed in a supported settings
 	 */
	bool getIsSettingsSupported(const struct nvm_capabilities &nvmCaps) const;

	/*
	 * Helper function to get an interleave_format with the appropriate sizes set based on user input
	 */
	struct interleave_format getFormatSizes() const { return m_format; }

	/*
	 * Get the size
	 */
	NVM_UINT64 getSizeGiB() const { return m_size; }
	NVM_UINT64 getSizeMiB() const { return GIB_TO_MIB(m_size); }

	/*
	 * Did the user supply a size
	 */
	bool getSizeExists() const { return m_sizePropertyExists && (getIsRemaining() || m_size > 0); }

	/*
	 * Did the user supply a setting
	 */
	bool getSettingsExists() const { return m_settingsPropertyExists; }

	/*
	 * If the MemoryProperty is set, determine if it is valid or not
	 */
	framework::SyntaxErrorResult *validate () const;

private:
	NVM_UINT64 m_size;
	bool m_sizePropertyExists;
	bool m_settingsPropertyExists;
	bool m_settingsValid;
	std::string m_sizePropertyName;
	std::string m_settingPropertyName;
	std::string m_sizeString;
	std::string m_settingsPropertyValue;
	struct interleave_format m_format; //!< contains format sizes

	std::vector<std::string> m_settingsTokens; //!< settings string divided into tokens

	void setParsedCommand(const framework::ParsedCommand& parsedCommand,
			const std::string &sizePropertyName,
			const std::string &settingsPropertyName = "");
	bool getIsFirstSettingPart(const std::string &value) const;
	bool getIsSizePart(const std::string &value) const;

	/*
	 * Breaks settings into tokens on separators.
	 * @return true if the settings property was basically well-formatted
	 */
	bool tokenizeSettings();

	/*
	 * Converts an AppDirect Setting string into an enum value
	 * @return true if the string was converted successfully
	 */
	bool convertSettingStringToInterleaveSize(
			const std::string& setting, enum interleave_size &size);

	/*
	 * Parses the settings string into member variables
	 * @return true if parsing successful
	 */
	bool parseSettings();

	/*
	 * Helper method to see if settings are valid.
	 * @return true if the settings property was valid
	 */
	bool validateSettings();
};

} /* namespace nvmcli */
} /* namespace cli */

#endif /* MEMORYPROPERTY_H_ */
