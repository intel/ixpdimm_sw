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
 * Logger for the CLI framework. This file contains all classes needed for logging. The ESX Channel
 * will only be included if the ESX build flag is set, but will, in most cases, not be needed.
 */

#ifndef _CLI_FRAMEWORK_LOGGER_H_
#define _CLI_FRAMEWORK_LOGGER_H_

#include <string>
#include <sstream>
#include <iostream>
#ifdef __ESX__
#include <syslog.h>
#endif

namespace cli
{
namespace framework
{

/*!
 * macro helper for logging an error. Currently the only one used, but could expand to include
 * log_warn, log_info, ...
 */
#define	log_error(logger, message) \
	logger << LogMessage::PRIORITY_ERROR << message << std::endl;

/*!
 * Represents a message to be logged
 */
class LogMessage
{
public:
	/*!
	 * enum to represent the priority of a message
	 */
	enum Priority
	{
		PRIORITY_ERROR = 0,      //!< Errors only
		PRIORITY_WARNING = 1,    //!< Warnings + Errors
		PRIORITY_INFO = 2,       //!< Information + Warnings + Errors
		PRIORITY_DEBUG = 3       //!< Debug + Information + Warnings + Errors
	};

	/*!
	 * Constructor
	 * @param priority
	 * 		Priority of the message. See Priority enum.
	 * @param message
	 * 		Text of the message.
	 */
	LogMessage(Priority priority, std::string message);

	/*!
	 * Constructor with default values
	 */
	LogMessage();


	/*!
	 * Getter for the message text
	 * @return
	 * 		returns the message text
	 */
	std::string getMessage() const;

	/*!
	 * Setter for the message text.
	 * @param message
	 * 		The message text
	 */
	void setMessage(const std::string &message);

	/*!
	 * Getter for the priority
	 * @return
	 * 		returns the priority
	 */
	Priority getPriority() const;

	/*!
	 * Setter for the message priority
	 * @param priority
	 * 		the message priority
	 */
	void setPriority(const Priority &priority);

	/*!
	 * Getter for the file name. Some messages track where the message originated.
	 * @return
	 * 		The function name text
	 */
	const std::string& getFileName() const;

	/*!
	 * Setter for the file name.
	 * @param fileName
	 * 		the file name text
	 */
	void setFileName(const std::string& fileName);

	/*!
	 * Getter for the line number where the message originated.
	 * @return
	 * 		The line number
	 */
	int getLineNumber() const;

	/*!
	 * Setter for the line number where the message originated.
	 * @param lineNumber
	 * 		The line number
	 */
	void setLineNumber(const int &lineNumber);

private:
	Priority m_priority;
	std::string m_message;
	std::string m_fileName;
	int m_lineNumber;

};

/*!
 * The base class for Channels.
 */
class LogChannelBase
{
public:
	/*!
	 * virtual destructor
	 */
	virtual ~LogChannelBase() {}

	/*!
	 * Write the log message
	 * @param message
	 * 		The message to write
	 */
	virtual void write(const LogMessage &message) = 0;
};

/*!
 * A Stream channel that takes a stream to write the logs to. The caller provides the stream
 */
class StreamChannel : public LogChannelBase
{
public:
	/*!
	 * Constructor. Caller must provide the stream that will be logged to.
	 * @param pStream
	 * 		The stream that will be logged to.
	 */
	StreamChannel (std::ostream *pStream) : m_pStream(pStream)	{ }

	/*!
	 * 	Write the log message to the stream
	 * @param message
	 * 		Mesasge to be logged
	 */
	void write(const LogMessage &message);

private:
	std::ostream *m_pStream;
};

/*!
 * A logging channel that writes all logs to std::cout
 */
class ConsoleChannel : public StreamChannel
{
public:
	ConsoleChannel() : StreamChannel(&std::cout) { }
};

#ifdef __ESX__
/*!
 * A logging channel used for ESX.
 * ESX has special logging considerations. Desired behavior for logging on ESX is as follows:
 * 		--critical/warning type messages are logged using the syslog API
 * 		--debug/info type messages are logged to stdout or /dev/null as determined by a configuration setting
 */
class EsxLogChannel : public LogChannelBase
{
	/*!
	 * Write the log message to the syslog
	 * @param message
	 * 		The log to write
	 */
	void write(const LogMessage &message);
};
#endif

/*!
 * A Logger class that can have log messages streamed to it. If the channel is set, it will
 * write the log message after a std::endl is sent to it. The priority can also be set by streaming
 * it to the logger. The default priority is PRIORITY_INFO.
 * Example Usage:
 * 		logger << LogMessage::PRIORITY_WARN << "This is a warning" << std::endl;
 * This example writes a "Warning" to whatever channel is set.
 */
class Logger
{
public:
	/*!
	 * Constructor
	 */
	Logger() :
		m_pChannel(NULL),
		m_currentMessagePriority(LogMessage::PRIORITY_INFO),
		m_level(LogMessage::PRIORITY_INFO)
	{
	}

	/*!
	 * A template function that overrides the << operator.
	 * @param x
	 * 		What to add to the logger
	 * @return
	 * 		A const ref to this
	 */
    template<class T>
    Logger &operator << (const T &x)
    {
        _buffer << x;
        return *this;
    }
	/*!
	 * An override of the << operator that takes a LogMessage::Priority. This changes future
	 * messages that will be written.
	 * @param prio
	 * 		Priority.
	 * @return
	 * 		A const ref to this
	 */
    Logger &operator << (const enum LogMessage::Priority &prio)
    {
    	m_currentMessagePriority = prio;
    	return *this;
    }

	/*!
	 * An override of the << operator that takes std::end. This signals the log message to be
	 * written to the channel.
	 * @param endl
	 * 		std::endl
	 * @return
	 * 		A const ref to this
	 */
    Logger &operator<<(std::ostream& (*endl) (std::ostream&))
    {
    	_buffer << endl;
    	flush();
    	return *this;
    }

    /*!
     * Flush the current log message to the channel.
     */
    void flush();

    /*!
     * Setter for the channel to write to.
     * @param pChannel
     * 		A pointer to the channel to use. It is up to the caller to initialize the channel and
     * 		free it when done.
     */
    void setChannel(LogChannelBase *pChannel);

    /*!
     * Getter for the channel currently set
     * @return
     * 		The pointer to the channel being used. If no channel has been set, will return NULL
     */
    LogChannelBase *getChannel();


    /*!
     * Log the message
     * @param message
     * 		The message to log
     */
    void log(const LogMessage &message);

    /*!
     * Log the message
     * @param priority
     * 		Priority of the message
     * @param message
     * 		The log message text
     */
    void log(const LogMessage::Priority &priority, const std::string &message);

    /*!
     * Used for tracing code execution
     * @param message
     * 		The message used for the trace.
     */
    void trace(const std::string &message);

    /*!
     * Used for tracing code execution
     * @param message
     * 		The message used for the trace
     * @param fileName
     * 		File name where the trace originated
     *
     * @param lineNumber
     * 		line number where the trace originated
     */
    void trace(const std::string &message, const std::string &fileName, const int &lineNumber);

    /*!
     * Getter for the level being logged.
     * @return
     * 		Priority set for the current logging
     */
	LogMessage::Priority getLevel() const;

	/*!
	 * Set what should be logged.  Any log message with level or higher priority will be logged.
	 * @param level
	 * 		The level to set.
	 */
	void setLevel(const LogMessage::Priority &level);

private:
    std::stringstream _buffer;
    LogChannelBase *m_pChannel;
    LogMessage::Priority m_currentMessagePriority;
    LogMessage::Priority m_level;
};


} /* framework */
} /* cli */
#endif /* _CLI_FRAMEWORK_LOGGER_H_ */
