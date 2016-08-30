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

#include <iostream>
#include <libinvm-cli/ResultBase.h>
#include <LogEnterExit.h>
#include "YesNoPrompt.h"

cli::framework::YesNoPrompt::YesNoPrompt(const ConsoleAdapter &consoleAdapter) :
	m_consoleAdapter(consoleAdapter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

bool cli::framework::YesNoPrompt::prompt(const std::string &message) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string question = buildQuestion(message);
	askQuestion(question);
	std::string answer = getAnswer();
	return isAnswerCorrect(answer);
}

std::string cli::framework::YesNoPrompt::buildQuestion(const std::string &message) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	const std::string yesOrNo = TR("(y or [n])");
	return framework::ResultBase::stringFromArgList(
			(message + " " + yesOrNo + " ").c_str());
}

void cli::framework::YesNoPrompt::askQuestion(const std::string &question) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	m_consoleAdapter.write(question);
}

std::string cli::framework::YesNoPrompt::getAnswer() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_consoleAdapter.getLine();
}

bool cli::framework::YesNoPrompt::isAnswerCorrect(const std::string &answer) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	const std::string correctAnswer = "y";

	bool isCorrectAnswer = false;
	std::string answerCopy = answer;
	if (answerCopy.length() > 0)
	{
		answerCopy[0] = (char)tolower(answerCopy[0]);
	}
	if ((answerCopy.length() == correctAnswer.length()) &&
		(0 == answerCopy.compare(correctAnswer)))
	{
		isCorrectAnswer = true;
	}
	return isCorrectAnswer;
}
