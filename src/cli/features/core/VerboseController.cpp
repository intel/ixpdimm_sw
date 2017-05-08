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

/*
 * This file contains the functionality to turn on verbose.
 */

#include "VerboseController.h"
#include <LogEnterExit.h>
#include <persistence/logging.h>

cli::nvmcli::VerboseController::VerboseController() : m_verbose(false)
{
	m_printMask = get_print_mask();
}

cli::nvmcli::VerboseController::~VerboseController()
{}

void cli::nvmcli::VerboseController::setVerbose(bool value)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	m_verbose = value;
}

bool cli::nvmcli::VerboseController::getVerbose()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_verbose;
}

bool cli::nvmcli::VerboseController::set_print_mask(int mask)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	COMMON_BOOL isSet = set_current_print_mask(mask);
	return isSet;
}

int cli::nvmcli::VerboseController::get_print_mask()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return get_current_print_mask();
}

void cli::nvmcli::VerboseController::enableVerbose()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int debug_trace = FLAG_PRINT_DEBUG;

	m_printMask = get_print_mask();

	if (!getVerbose())
	{
		if (set_print_mask(debug_trace | m_printMask))
		{
			setVerbose(true);
		}
	}
}

void cli::nvmcli::VerboseController::disableVerbose()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (getVerbose())
	{
		if (set_print_mask(m_printMask))
		{
			setVerbose(false);
		}
	}
}
