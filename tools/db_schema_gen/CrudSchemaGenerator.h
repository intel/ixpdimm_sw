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
 * This file contains the CrudSchemaGenerator class.
 * The CrudSchemaGenerator uses google ctemplate library as a code generation engine using
 * a predefined template and ctemplate::TemplateDictionary's to expand the templates into a
 * C header file (schema.h) and a C source file (schema.h) that contains appropriate embedded
 * SQL code and C code to generate a SQLite database with a defined model using the Entity/Attribute
 * API.
 */

#ifndef CRUDSCHEMAGENERATOR_H_
#define CRUDSCHEMAGENERATOR_H_

#include <stdio.h>

#include <cstdlib>
#include <iostream>
#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <vector>

#include "TemplateModifiers.h"
#include "Attribute.h"
#include "Entity.h"

/*!
 *
 * Static class that builds the TemplateDictionaries and expands the templates to the header and source files
 * @ingroup code_gen
 */
class CrudSchemaGenerator
{
private:
//	/*!
//	 * private GetTableName returns the parent table name from an attribute that's a foreign key
//	 * @private
//	 * @param foreignKey foreign key into parent table.
//	 * @pre foreignKey should be in format [parent table]_[attribute name]
//	 * @return returns the parent table name
//	 */
//	static std::string GetTableName(std::string foreignKey);

	/*!
	 * private Writes text to a file path
	 * @private
	 * @param text The text to write
	 * @param path Where to write the text
	 */
	static void WriteTextToFile(std::string text, std::string path);

	/*!
	 * Returns whether a char is the Carriage Return (\r) character or not.
	 * @details
	 * Used for a hack to fix the output generated on Windows. For some reason the Windows template
	 * expander adds an extra carriage return on all lines which breaks and multiline macro
	 * @param value character to evaluate
	 * @return true if character is '\r'. false if character is not '\r'
	 */
	static bool IsCr(char value);

	/*!
	 * Fill the TemplateDictionary with the attribute
	 * @param pDictionary
	 * 		Dictionary to fill
	 * @param pAttribute
	 * 		Entity Attribute that will be used to fill the dictionary
	 * @param attributeIndex
	 * 		Some of the dictionary values require the index for the current attribute
	 */
	static void fillAttributeDictionary(ctemplate::TemplateDictionary *pDictionary,
			Attribute *pAttribute, int attributeIndex);


public:
	/*!
	 * Static function to generate the schema code.
	 * @param schema The input schema model.  This is a list of Entities with defined attributes
	 * and relationships.
	 * @param templatePath The path to where the template files are that the generator will used.
	 * @param outputPath Where to generate the output files.
	 * @details
	 * If the generator cannot find the template files, then it prints a warning but will exit cleanly.
	 *
	 */
	static void Generate(std::vector<Entity> schema,
			std::string templatePath,
			std::string outputPath);
};



#endif /* CRUDSCHEMAGENERATOR_H_ */
