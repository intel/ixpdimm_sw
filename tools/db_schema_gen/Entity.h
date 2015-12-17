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
 * This file contains the definition of the Entity class which models
 * a table in the Sqlite DB.
 */

#include <string.h>
#include "Attribute.h"
#include "Relationship.h"

#ifndef ENTITY_CPP_
#define ENTITY_CPP_

/*!
 * @defgroup code_gen Code Generation
 * @details
 * The classes in the Code Generation module are part of a in-house developed tool that's
 * used to generate C code and SQL embedded code for defining an interface between the
 * SQLite DB and the rest of the code.
 */

/*!
 * Represents an entity in the application data model.  Can have multiple Attributes associated.
 * @ingroup code_gen
 * @details
 * The Entity will be used to generate SQL and C code.  A SQLite table and C struct will be generated
 * for each Entity.  Appropriate functions and SQL are added to add and retrieve Entities from the
 * SQLite tables.  The functions receive a db_Entity struct representation of the table to either be
 * filled by the function (if a "get" function) or to be saved in the database (if a "add" function)
 */
class Entity
{
public:
	/*!
	 * Create a new instance of an Entity
	 * @param name Name of the entity.  The generator will create a C struct and a database table with this name.
	 * @param attributes List of the Attributes that define the entities columns/fields
	 */
	Entity(std::string name)
	: m_name(name), m_includesHistory(false)
	{ }

	/*!
	 * getName
	 * @return the Entity's name.
	 */
	std::string getName() { return m_name; }

	/*!
	 * Add a new attribute to the entity
	 * @param name
	 * 		Name of the entity
	 * @return
	 * 		Reference to the attribute added
	 */
	Attribute& addAttribute(const std::string &name)
	{
		m_attributes.push_back(Attribute(name));
		return m_attributes.back();
	}

	void addRelationship(const std::string &name, const std::string &relatedEntity, int count = 1)
	{
		m_relationships.push_back(Relationship(name, relatedEntity, count));
	}

	/*!
	 * getAttributes
	 * @return the Entity's attribute list
	 */
	std::vector<Attribute> getAttributes() { return m_attributes; }

	/*!
	 * getRelationships
	 * @return the Entity's relationship list
	 */
	std::vector<Relationship> getRelationships() { return m_relationships; }

	void includesHistory() { m_includesHistory = true; }
	bool getIncludesHistory() { return m_includesHistory; }

private:
	std::string m_name;
	std::vector<Attribute> m_attributes;
	std::vector<Relationship> m_relationships;
	bool m_includesHistory;
};

#endif /* ENTITY_CPP_ */

