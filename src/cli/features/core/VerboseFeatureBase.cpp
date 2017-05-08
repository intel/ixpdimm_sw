/*
 * Copyright (c) 2017, Intel Corporation
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

#include "VerboseFeatureBase.h"
#include <LogEnterExit.h>

cli::nvmcli::VerboseFeatureBase::VerboseFeatureBase() :
		cli::framework::FeatureBase(),
		m_pVerboseController(new cli::nvmcli::VerboseController())
{}

cli::nvmcli::VerboseFeatureBase::~VerboseFeatureBase()
{
	delete m_pVerboseController;
}

void cli::nvmcli::VerboseFeatureBase::enableVerbose(const framework::ParsedCommand &cmd)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::OutputOptions options(cmd);
	bool verbose = options.getVerbose();

	if (verbose)
	{
		m_pVerboseController->enableVerbose();
	}
}

void cli::nvmcli::VerboseFeatureBase::disableVerbose(const framework::ParsedCommand &cmd)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::OutputOptions options(cmd);
	bool verbose = options.getVerbose();

	if (verbose)
	{
		m_pVerboseController->disableVerbose();
	}
}

void cli::nvmcli::VerboseFeatureBase::setVerboseController(
		cli::nvmcli::VerboseController *pVerboseController)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	delete m_pVerboseController;
	m_pVerboseController = pVerboseController;
}
