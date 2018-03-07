/*
 * Copyright (c) 2016, Intel Corporation
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

#ifndef SHOWCOMMANDPROPERTYUTILITIES_H_
#define SHOWCOMMANDPROPERTYUTILITIES_H_

#include <nvm_types.h>

#include "framework/PropertyDefinitionBase.h"
#include "framework/PropertyDefinitionList.h"
#include "framework/DisplayOptions.h"
#include <libinvm-cli/ResultBase.h>
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include <cli/features/ExportCli.h>

namespace cli
{
namespace nvmcli
{

template<class T>
class ShowCommandPropertyUtilities
{
	public:
		ShowCommandPropertyUtilities() {}
		virtual ~ShowCommandPropertyUtilities() {}

		static bool isPropertyDisplayed(framework::IPropertyDefinition<T> &p,
				framework::DisplayOptions &options);
		static framework::ResultBase *getInvalidDisplayOptionResult(framework::DisplayOptions &options,
				framework::PropertyDefinitionList<T> &props);
};

template<class T>
inline bool ShowCommandPropertyUtilities<T>::isPropertyDisplayed(
		framework::IPropertyDefinition<T>& p, framework::DisplayOptions& options)
{
	return (p.isRequired() ||
		options.isAll() ||
		(p.isDefault() && options.isDefault()) ||
		options.contains(p.getName())) &&
		!p.isIxp();
}

template<class T>
inline framework::ResultBase* cli::nvmcli::ShowCommandPropertyUtilities<T>::getInvalidDisplayOptionResult(
		framework::DisplayOptions& options, framework::PropertyDefinitionList<T>& props)
{
	framework::ResultBase *pResult = NULL;

	std::string invalidDisplay;
	const std::vector<std::string> &display = options.getDisplay();
	for (size_t i = 0; i < display.size() && invalidDisplay.empty(); i++)
	{
		if (!props.contains(display[i]))
		{
			invalidDisplay = display[i];
		}
	}

	if (!invalidDisplay.empty())
	{
		pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_OPTION,
			framework::OPTION_DISPLAY.name, invalidDisplay);
	}

	return pResult;
}

} /* namespace nvmcli */
} /* namespace cli */

#endif /* SHOWCOMMANDPROPERTYUTILITIES_H_ */
