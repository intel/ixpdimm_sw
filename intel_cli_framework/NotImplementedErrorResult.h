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
 * An error result meaning that the CLI command isn't implemented
 */

#ifndef _CLI_FRAMEWORK_NOTIMPLEMENTEDERRORRESULT_H
#define _CLI_FRAMEWORK_NOTIMPLEMENTEDERRORRESULT_H

#include "ErrorResult.h"
#include "FeatureBase.h"

namespace cli {
namespace framework {
/*!
 * TODO: class description
 */
class NotImplementedErrorResult : public ErrorResult
{
public:
	/*!
	 * Constructor for the result
	 * @param commandSpecId
	 * 		The CommandSpec Id that isn't implemented yet
	 * @param featureName
	 * 		The name of the feature that the the CommandSpec belongs to
	 */
	NotImplementedErrorResult(const int commandSpecId, const std::string &featureName);

	/*!
	 * Getter for m_commandSpecId
	 * @return
	 */
	int getCommandSpecId();

	/*!
	 * getter for the m_feature
	 * @return
	 */
	std::string getFeatureName();

private:
	int m_commandSpecId;
	std::string m_featureName;
};
}
}

#endif
