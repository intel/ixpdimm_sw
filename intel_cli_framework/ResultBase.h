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
 * Base class for command results
 */

#ifndef _CLI_FRAMEWORK_RESULTBASE_H_
#define _CLI_FRAMEWORK_RESULTBASE_H_

#include "CliFrameworkTypes.h"
#include "cr_i18n.h"

namespace cli
{
namespace framework
{

const std::string NoResultsStr = TR("No results"); //!< No results string

const std::string ESX_BEGIN_ROOTTAG = "<?xml version=\"1.0\"?>" //!< ESX root tag
		"<output xmlns=\"http://www.vmware.com/Products/ESX/5.0/esxcli/\">";
const std::string ESX_END_ROOTTAG = "</output>"; //!< ESX root tag
const std::string ESX_BEGIN_STRINGLIST = "<list type=\"string\">"; //!< ESX string list tag
const std::string ESX_BEGIN_STRUCTLIST = "<list type=\"structure\">"; //!< ESX structure list tag
const std::string ESX_END_LIST = "</list>"; //!< ESX list tag

const std::string ESX_BEGIN_STRING = "<string>"; //!< ESX string tag
const std::string ESX_END_STRING = "</string>"; //!< ESX string tag

const std::string ESX_BEGIN_KEYVALUESTRUCT = "<structure typeName=\"KeyValue\">"; //!< ESX Common KeyValue structure tag
const std::string ESX_BEGIN_STRUCT = "<structure typeName=\"%s\">"; //!< ESX generic structure tag
const std::string ESX_END_STRUCT = "</structure>"; //!< ESX end structure tag
const std::string ESX_BEGIN_KEYFIELD = "<field name=\"Attribute Name\">"; //!< ESX common field tag
const std::string ESX_BEGIN_VALUEFIELD = "<field name=\"Value\">"; //!< ESX common field tag
const std::string ESX_BEGIN_FIELD = "<field name=\"%s\">"; //!< ESX generic field tag
const std::string ESX_END_FIELD = "</field>"; //!< ESX end field tag

/*!
 * Base class for command results
 */
class ResultBase
{
public:

	/*!
	 * List of error types
	 */
	enum ErrorCode
	{
		ERRORCODE_SUCCESS = 0,   //!< ERRORCODE_SUCCESS
		ERRORCODE_UNKNOWN = 1,   //!< ERRORCODE_UNKNOWN
		ERRORCODE_OUTOFMEMORY = 2,//!< ERRORCODE_OUTOFMEMORY
		ERRORCODE_NOTSUPPORTED = 3, //!< ERRORCODE_NOTSUPPORTED
		ERRORCODE_SYNTAX = 201    //!< ERRORCODE_SYNTAX
	};

	/*!
	 * Types of output formats
	 */
	enum outputTypes
	{
		OUTPUT_TEXT = 0,//!< OUTPUT_TEXT
		OUTPUT_XML = 1,  //!< OUTPUT_XML
		OUTPUT_ESXXML = 2, //!< OUTPUT_ESXXML
		OUTPUT_TEXTTABLE = 3,//!< OUTPUT_TEXTTABLE
		OUTPUT_JSON = 4,//!< OUTPUT_JSON
		OUTPUT_ESXXMLTABLE = 5, //!< OUTPU_ESXXMLTABLE
	};
	ResultBase();

	virtual ~ResultBase() {};

	/*!
	 * output is used by the client to display the results to the user.
	 * @return
	 * 		What to display to the user
	 */
	std::string output() const;

	/*!
	 * outputs the result as text
	 * @return
	 */
	virtual std::string outputText() const = 0;
	/*!
	 * outputs the results as XML
	 * @return
	 */
	virtual std::string outputXml() const = 0;

	/*!
	 * outputs the results such that esx can understand them
	 * @return
	 */
	virtual std::string outputEsxXml() const = 0;

	/*!
	 * outputs the results such that esx can understand them
	 * @return
	 */
	virtual std::string outputEsxXmlTable() const;

	/*!
	 * outputs the result as text table
	 * @return
	 */
	virtual std::string outputTextTable() const;

	/*!
	 * outputs the result as JSON
	 * @return
	 */
	virtual std::string outputJson() const = 0;

	/*!
	 * Setter for outputType
	 * @param type
	 * 		value for the setter
	 */
	void setOutputType(enum outputTypes type);

	/*!
	 * getter for the outputType
	 * @return
	 * 		outputType
	 */
	enum outputTypes getOutputType();

	/*!
	 * set the output option from the list of options parsed
	 * @param options
	 * 		List of options parsed
	 * @return
	 * 		if there is an issue with the -output option then will return false, otherwise, true
	 */
	bool setOutputOption(StringMap options);

	/*!
	 * Convert a variable argument list into a std::string
	 * @param format - the format string
	 * @return
	 * 	  returns the string
	 */
	static std::string stringFromArgList(const char *format, ...);

	/*!
	 * getter for m_errorCode
	 * @return
	 * 	The error code
	 */
	int getErrorCode() const;

	/*!
	 * setter for m_errorCode
	 * @param errorCode
	 * 	The error code
	 */
	void setErrorCode(int errorCode);

private:
	int m_errorCode;
	enum outputTypes m_outputType;

};

}
}


#endif /* RESULTBASE_H_ */
