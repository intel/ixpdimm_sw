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
 * This file defines a class to log trace entry and exit for CPP code
 */

#ifndef _LOG_ENTER_EXIT_H_
#define	_LOG_ENTER_EXIT_H_

#include <persistence/logging.h>

/*
 * Log the entrance and exiting of a method/function automatically.
 */
class LogEnterExit
{
	public:
		/*
		 *
		 * @param funcName
		 * 		Function name from which log is being written
		 * @param srcFile
		 * 		File name from which log is being written
		 * @param lineNum
		 * 		Line number within file from which log is being written
		 * @details
		 *  WARNING: Since no copying of data occurs, and the pointer to that data is maintained
		 * 	for the life of this object, thus the constant string must exist for the
		 *	life of this obj or risk seg faulting. An alternative solution, but slower,
		 *	is to copy the strings internal to this obj during construction.
		 *
		 */
		LogEnterExit(const char *funcName, const char *srcFile, const int lineNum) :
			m_FuncName(funcName), m_SrcFile(srcFile), m_LineNum(lineNum)
		{
			log_trace_f(LOGGING_LEVEL_INFO, m_SrcFile, m_LineNum, "Entering: %s", m_FuncName);
		}

		virtual ~LogEnterExit()
		{
			log_trace_f(LOGGING_LEVEL_INFO, m_SrcFile, m_LineNum, "Exiting: %s", m_FuncName);
		}

	private:
		const char *m_FuncName;
		const char *m_SrcFile;
		const int m_LineNum;
};


#endif /* _LOG_ENTER_EXIT_H_ */
