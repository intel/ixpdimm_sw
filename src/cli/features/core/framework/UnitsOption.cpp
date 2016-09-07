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

#include "UnitsOption.h"

std::string cli::framework::UnitsOption::getCapacityUnits() const
{
	std::string units = "";
	if (m_options.find("-units") != m_options.end())
	{
		units = m_options.at("-units");
	}

	return units;
}

cli::framework::UnitsOption &cli::framework::UnitsOption::operator=(const cli::framework::UnitsOption &other)
{
	if (this == &other)
		return *this;

	m_options = other.m_options;

	return *this;
}

bool cli::framework::UnitsOption::isValid(std::string units) const
{
	bool validType = false;

	if (units.empty())
	{
		validType = true;
	}
	else if (m_options.find(OPTION_UNITS.name) != m_options.end())
	{
		std::vector<std::string> validUnits;
		validUnits.push_back("B");
		validUnits.push_back("MB");
		validUnits.push_back("MiB");
		validUnits.push_back("GB");
		validUnits.push_back("GiB");
		validUnits.push_back("TB");
		validUnits.push_back("TiB");

		for (std::vector<std::string>::const_iterator iter = validUnits.begin();
				iter != validUnits.end(); iter++)
		{
			if (stringsIEqual(units, *iter))
			{
				validType = true;
				break;
			}
		}
	}

	return validType;
}

bool cli::framework::UnitsOption::isEmpty(std::string units) const
{
	bool emptyUnits = false;

	if (m_options.find("-units") != m_options.end())
	{
		units = m_options.at("-units");
		if (units.empty())
		{
			emptyUnits = true;
		}
	}

	return emptyUnits;
}
