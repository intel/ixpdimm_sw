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
 * This file contains the CrudSchemaGenerator class.
 * The CrudSchemaGenerator uses google ctemplate library as a code generation engine using
 * a predefined template and ctemplate::TemplateDictionary's to expand the templates into a
 * C header file (schema.h) and a C source file (schema.h) that contains appropriate embedded
 * SQL code and C code to generate a SQLite database with a defined model using the Entity/Attribute
 * API.
 */


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctemplate/template.h>

#include "CrudSchemaGenerator.h"

void CrudSchemaGenerator::WriteTextToFile(std::string text, std::string path)
{
#ifdef __WINDOWS__
	// HACK - ctemplate adds extra new carriage return in Windows
	text.erase(std::remove_if(text.begin(), text.end(), &CrudSchemaGenerator::IsCr), text.end());
#endif
	std::ofstream cfile;
	cfile.open (path.c_str());
	cfile << text;
	cfile.close();
}

bool CrudSchemaGenerator::IsCr(char value)
{
	return value == '\r';
}

void CrudSchemaGenerator::fillAttributeDictionary(ctemplate::TemplateDictionary* pDictionary,
		Attribute* pAttribute, int attributeIndex)
{
	(*pDictionary)["ATTRIBUTE_NAME"] = pAttribute->getName();
	(*pDictionary)["COLUMN_NAME"] = pAttribute->getName();
	(*pDictionary)["FIELD_INDEX_NAME"] = pAttribute->getName();

	(*pDictionary)["ATTRIBUTE_TYPE"] = pAttribute->getSqlType();
	(*pDictionary)["ATTRIBUTE_C_TYPE"] = pAttribute->getCType();
	(*pDictionary)["ATTRIBUTE_C_FORMAT_TYPE"] = pAttribute->getCFormatType();
	(*pDictionary)["ATTRIBUTE_INDEX"] = attributeIndex;
	(*pDictionary)["ATTRIBUTE_INDEX_PLUS_ONE"] = attributeIndex + 1;

	if (pAttribute->getIsText())
	{
		(*pDictionary).ShowSection("TXT_LEN");
		(*pDictionary)["ATTRIBUTE_STR_LEN"] = pAttribute->getStringLen();
	}

	if (pAttribute->getIsArray())
	{
		(*pDictionary).ShowSection("ARY_LEN");
		(*pDictionary)["ATTRIBUTE_ARRAY_LEN"] = pAttribute->getArrayLen();
	}
	if (pAttribute->getIsClearable())
	{
		(*pDictionary).ShowSection("CLEAR_ATTRIBUTE");
	}
	if (pAttribute->getIsIndexPk())
	{
		(*pDictionary).ShowSection("INDEXPK_ATTRIBUTE");
	}
	if (!pAttribute->getIsAutoIncrementPk())
	{
		// this section is used for when inserting into a table. Auto incremented
		// columns should not be included in the SQL INSERT
		(*pDictionary).ShowSection("NOTAUTOPK_ATTRIBUTE");
		// seperators are still used even if the NOTAUTOPK section is hidden, so need to dynamically
		// set/hide the ','
		(*pDictionary)["ATTRIBUTE_SEPERATOR"] = ", ";
	}
	else
	{
		(*pDictionary)["ATTRIBUTE_SEPERATOR"] = "";
		(*pDictionary).ShowSection("AUTOINC_ATTRIBUTE");
	}

	if (pAttribute->getIsOrderBy())
	{
		(*pDictionary).ShowSection("ORDERBY_ATTRIBUTE");
	}
	if (pAttribute->getIsOrderByDesc())
	{
		(*pDictionary).ShowSection("ORDERBYDESC_ATTRIBUTE");
	}
}

/*!
 * @copybrief Generate
 * @details
 * Using the schema passed in:
 * 	- Generate ctemplate dictionaries for each entity
 * 	- For each table dictionary create a dictionary for each attribute
 * Create the schema.h/c files by expanding the template files with the generated dictionaries
 *
 */
void CrudSchemaGenerator::Generate(std::vector<Entity> schema,
		std::string templatePath, std::string outputPath)
{
	CapsModifier *capsModifier = new CapsModifier();
	ReplaceModifier *replaceModifier = new ReplaceModifier();
	SingleModifier *singleModifier = new SingleModifier();

	ctemplate::AddModifier("x-caps", capsModifier);
	ctemplate::AddModifier("x-replace", replaceModifier);
	ctemplate::AddModifier("x-single", singleModifier);


	ctemplate::TemplateDictionary dict("main");
	dict["NON_TEMPLATED"] = templatePath + "/schema_non_template.c";
	ctemplate::TemplateDictionary* non_templated = dict.AddIncludeDictionary("NON_TEMPLATED");
	non_templated->SetFilename(templatePath + "/schema_non_template.c");

	int tableCount = 0; // number of tables creating
	std::vector<Entity>::iterator iter;
	for (iter = schema.begin(); iter != schema.end(); iter++)
	{
		tableCount++;
		std::string pk;

		Entity entity = *iter;
		ctemplate::TemplateDictionary *tableDict = dict.AddSectionDictionary("TABLE");
		if (entity.getIncludesHistory())
		{
			tableCount++; // add another table for the history table
		}
		else
		{
			/*
			 * HACK Alert
			 * Would prefer to use sections to hide non-history tables, however
			 * a bug in ctemplate causes it to abort on the VMware workstation
			 * build. Might be a 32bit vs 64bit issue?? The workaround is to
			 * generate the history table code for all entities, but then
			 * use tags to comment out the code for non-history tables.
			 *
			 * 	// (*tableDict).ShowSection("HISTORY"); // doesn't work :(
			 */

			(*tableDict)["HISTORY_START"] = "\n#if 0\n"
					"//NON-HISTORY TABLE\n";
			(*tableDict)["HISTORY_END"] = "\n#endif\n";

		}
		(*tableDict)["TABLE_NAME"] = entity.getName(); // tables[t].name;

		(*tableDict)["F_ADD"] = "db_add_" + entity.getName();
		(*tableDict)["F_UPDATE"] = "db_update_" + entity.getName();
		(*tableDict)["F_DELETE_TABLE"] = "db_delete_all_" + entity.getName() + "s";

		(*tableDict)["F_SAVE_HISTORY"] = "db_save_" + entity.getName() + "_history";
		(*tableDict)["F_SAVE_STATE"] = "db_save_" + entity.getName() + "_state";
		(*tableDict)["F_GET_ALL"] = "db_get_" + entity.getName() + "s";
		(*tableDict)["F_GET_ALL_BY_HISTORY_ID"] = "db_get_" + entity.getName() + "_history_by_history_id";
		(*tableDict)["F_DELETE_HISTORY"] = "db_delete_" + entity.getName() + "_history";

		(*tableDict)["F_GET_COUNT"] = "db_get_" + entity.getName() + "_count";
		(*tableDict)["F_GET_HISTORY_BY_HISTORY_ID_COUNT"] = "db_get_" + entity.getName() + "_history_by_history_id_count";
		(*tableDict)["F_GET_HISTORY_COUNT"] = "db_get_" + entity.getName() + "_history_count";
		(*tableDict)["F_ROW_TO_ENTITY"] = "local_row_to_" + entity.getName();
		(*tableDict)["F_GET_ENTITY_RELATIONSHIPS"] = "local_get_" + entity.getName() + "_relationships";
		(*tableDict)["F_GET_ENTITY_RELATIONSHIPS_HISTORY"] = "local_get_" + entity.getName()
				+ "_relationships_history";
		(*tableDict)["F_BIND_ENTITY_TO_STMT"] = "local_bind_" + entity.getName();

		(*tableDict)["STRUCT_NAME"] = "struct db_" + entity.getName();
		(*tableDict)["STRUCT_POINTER"] = "p_" + entity.getName();

		std::vector<Attribute> attributes = entity.getAttributes();
		std::vector<Attribute>::iterator att;
		int attribute_index = 0;
		for (att = attributes.begin(); att != attributes.end(); att++)
		{
			Attribute *attribute = &(*att);

			ctemplate::TemplateDictionary *collapsedAttDict =
					(*tableDict).AddSectionDictionary("COLLAPSED_ATTRIBUTE");

			fillAttributeDictionary(collapsedAttDict, attribute, attribute_index);

			if (attribute->getIsArray())
			{

				for (int i = 0; i < attribute->getArrayLen(); i++)
				{
					ctemplate::TemplateDictionary *attDict = (*tableDict).AddSectionDictionary("ATTRIBUTE");
					fillAttributeDictionary(attDict, attribute, attribute_index);
					// for some reason have to cast i to long long to avoid ambiguous call??

					std::stringstream columnName;
					columnName << attribute->getName() << "_" << i;

					(*attDict)["COLUMN_NAME"] = columnName.str();

					std::stringstream fieldIndex;
					fieldIndex << attribute->getName() << "[" << i << "]";
					(*attDict)["FIELD_INDEX_NAME"] = fieldIndex.str();

					attribute_index++; // because this is creating an additional attribute
				}
			}
			else
			{
				ctemplate::TemplateDictionary *attDict = (*tableDict).AddSectionDictionary("ATTRIBUTE");
				fillAttributeDictionary(attDict, attribute, attribute_index);
				if (attribute->getIsPk()) // assuming only 1 PK
				{
					(*attDict).ShowSection("PK");
					(*tableDict).ShowSection("TABLE_PK");

					(*tableDict)["PK_ATTRIBUTE_NAME"] = attribute->getName();
					(*tableDict)["PK_ATTRIBUTE_TYPE"] = attribute->getSqlType();
					(*tableDict)["PK_ATTRIBUTE_C_TYPE"] = attribute->getCType();

					(*tableDict)["F_UPDATE_BY_PK"] = "db_update_" + entity.getName() + "_by_" + attribute->getName();
					(*tableDict)["F_DELETE_BY_PK"] = "db_delete_" + entity.getName() + "_by_" + attribute->getName();
					(*tableDict)["F_GET_BY_PK"] = "db_get_" + entity.getName() + "_by_" + attribute->getName();
					pk = attribute->getName();

					// only primary keys can be indexed
					if (attribute->getIsIndexPk())
					{
						(*tableDict)["F_GET_MAX_ATTRIBUTE"] = "db_get_next_" + entity.getName() + "_" + attribute->getName();
						(*tableDict)["F_ROLL_BY_ATTRIBUTE"] = "db_roll_" + entity.getName() + "s_by_" + attribute->getName();
					}
				}
				if (attribute->getIsFk())
				{
					(*attDict).ShowSection("FK");
					// requires FK fields' name to be "[tablename]_fieldname"
					(*attDict)["ATTRIBUTE_FK_TARGET"] = attribute->getFkTarget();
					(*attDict)["ATTRIBUTE_FK_TARGET_ATTRIBUTE"] = attribute->getFkTargetAttribute();

					// these functions must stay inline with the related relationship functions
					(*attDict)["F_GET_BY_FK"] =
							"db_get_" + entity.getName() + "s_by_"
							+ attribute->getFkTarget() + "_" + attribute->getFkTargetAttribute();
					(*attDict)["F_GET_BY_FK_HISTORY"] =
							"db_get_" + entity.getName() + "s_by_"
							+ attribute->getFkTarget() + "_" + attribute->getFkTargetAttribute()
							+ "_history";
					(*attDict)["F_GET_COUNT_BY_FK"] =
							"db_get_" + entity.getName() + "_count_by_"
							+ attribute->getFkTarget() + "_" + attribute->getFkTargetAttribute();
					(*attDict)["F_GET_COUNT_BY_FK_HISTORY"] =
							"db_get_" + entity.getName() + "_count_by_"
							+ attribute->getFkTarget() + "_" + attribute->getFkTargetAttribute()
							+ "_history";
					(*attDict)["F_DELETE_BY_FK"] =
							"db_delete_" + entity.getName() + "_by_"
							+ attribute->getFkTarget() + "_" + attribute->getFkTargetAttribute();
				}

				attribute_index++;
			}

			if (attribute->getIsClearable())
			{
				(*tableDict)["F_CLEAR_ATTRIBUTE"] = "db_clear_" + entity.getName() + "_" + attribute->getName();
			}
		}
	}

	// +1 more for the history table (an automatic table, not included in the schema map
	dict["TABLE_COUNT"] = tableCount + 1;

	std::string c_output;
	ctemplate::ExpandTemplate(templatePath + "/schema.c.template", ctemplate::STRIP_BLANK_LINES, &dict, &c_output);
	WriteTextToFile(c_output, outputPath + "/schema.c");

	std::string h_output;
	ctemplate::ExpandTemplate(templatePath + "/schema.h.template", ctemplate::STRIP_BLANK_LINES, &dict, &h_output);
	WriteTextToFile(h_output, outputPath + "/schema.h");

	delete capsModifier;
	delete replaceModifier;
	delete singleModifier;
}

/*!
 * @copybrief CreateDoc
 * @details
 * 	- Creates database schema document
 *
 */
void CrudSchemaGenerator::CreateDoc(std::vector<Entity> schema,
		std::string outputPath)
{
	std::stringstream schemaDoc;

	std::vector<Entity>::iterator iter;
	for (iter = schema.begin(); iter != schema.end(); iter++)
	{
		Entity entity = *iter;

		schemaDoc << "Table(s): " << "db_" << entity.getName() << " \n";
		if (entity.getIncludesHistory())
		{
			schemaDoc << " \t  " << "db_" << entity.getName() << + "_history" << " \n";
		}
		schemaDoc << "Description: " << entity.getDescription() << " \n";

		std::vector<Attribute> attributes = entity.getAttributes();
		std::vector<Attribute>::iterator att;
		int attribute_index = 0;
		schemaDoc << "Attributes: \n";
		for (att = attributes.begin(); att != attributes.end(); att++)
		{
			Attribute *attribute = &(*att);
			if (attribute->getIsArray())
			{
				schemaDoc << "\t" << attribute->getCType() << " " << attribute->getName();
				for (int i = 0; i < attribute->getArrayLen(); i++)
				{
					attribute_index++;
				}
				schemaDoc << "\n";
			}
			else
			{
				if (attribute->getIsPk())
				{
					schemaDoc << "\t" << attribute->getCType() << " " << attribute->getName() << "(PK)";
					if (attribute->getIsIndexPk())
					{
						schemaDoc << "(Indexed)";
					}
				}
				else if (attribute->getIsFk())
				{
					schemaDoc << "\t" << attribute->getCType() << " " << attribute->getName() << "(FK)";
				}
				else
				{
					schemaDoc << "\t" << attribute->getCType() << " " << attribute->getName();
				}
				schemaDoc << "\n";
				attribute_index++;
			}
		}

		schemaDoc << "\n";
	}

	WriteTextToFile(schemaDoc.str(), outputPath + "/schema.txt");
}
