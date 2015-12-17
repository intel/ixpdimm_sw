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
 * This file contains the definition for the Attribute class.
 * The Attribute class represents the different types of Attributes that an @ref Entity can have.
 * The current Attribute types are TEXT, INTEGER32, and INTEGER64.  There are various fluent API
 * functions that can further define an attribute is a Primary Key, Foreign Key, or array
 */

#include <string>

#ifndef ATTRIBUTE_H_
#define ATTRIBUTE_H_

/*!
 * @class Attribute
 * @ingroup code_gen
 * @brief Attributes define @ref Entity attributes.
 */
class Attribute
{
public:

	/*!
	 * Create a new Attribute
	 * @param name
	 * 		string name of the Attribute
	 */
	Attribute(std::string name)
	: m_name(name), m_type(UNKOWN), m_arrayLen(0),  m_strLen(0),
	  m_isPk(false), m_isFk(false), m_isClearable(false),
	  m_isUnsigned(false), m_isIndexPk(false), m_orderByDesc(false), m_orderBy(false),  m_isAutoIncrement(false)
	{}

	/*!
	 * Sets the Attribute as a primary key.
	 * @param
	 * 		autoincrement - should the PK autoincrement or be user defined
	 * @return this
	 * @details A fluent api function (named parameter idiom) which returns the object it
	 * belongs to, allowing for chaining the functions together and defining several properties of the attribute
	 */
	inline Attribute& isPk(bool autoincrement = false) { m_isPk = true; m_isAutoIncrement = autoincrement; return *this; }

	/*!
	 * The pk is an integer index which means there are a
	 * few helper functions added to get the max and roll.
	 * @return this
	 * @details
	 */
	inline Attribute& isIndexPk() { m_isIndexPk = true; m_isPk = true;  m_type = INTEGER32; return *this; }

	/*!
	 * When selecting, this attribute will be used to order the results. Cannot be both orderBy and
	 * orderByDesc
	 */
	inline Attribute& orderByDesc() { m_orderByDesc = true; m_orderBy = false; return *this;}
	inline Attribute& orderBy() { m_orderBy = true; m_orderByDesc = false; return *this;}

	/*!
	 * Sets the Attribute as a foreign key.  This foreign key name must match exactly the PK name of the parent table
	 * @return this
	 * @details
	 * @copydetails isPk
	 */
	inline Attribute& isFk(std::string fkTarget, std::string fkTargetAttribute)
	{
		m_fkTarget = fkTarget;
		m_fkTargetAttribute = fkTargetAttribute;
		m_isFk = true;
		return *this;
	}

	/*!
	 * Sets the Attribute as being clearable.
	 * @details
	 * @copydetails isPk
	 *
	 * This will create a function that will delete the data in all rows for this attribute
	 */
	inline Attribute& isClearable() { m_isClearable = true; return *this; }

	/*!
	 * Sets the Attribute as being a TEXT or string attribute.
	 * @param length
	 * 		Maximum characters this string can be
	 * @details
	 * @copydetails isPk
	 */
	inline Attribute& isText(int length) { m_type = TEXT; m_strLen = length; return *this; }

	/*!
	 * Sets the Attribute as being an INTEGER attribute.
	 * @details
	 * @copydetails isPk
	 */
	inline Attribute& isInt32() { m_type = INTEGER32; return *this; }

	/*!
	 * Sets the Attribute as being an array of whatever type it is
	 * @param length
	 * 		Max length for the array
	 * @details
	 * @copydetails isPk
	 * An Attribute that is an array cannot also be a primary key or foreign key.  The Attribute
	 * API will allow that configuration, however, if an Attribute is an array it will ignore PK
	 * or FK configurations.
	 */
	inline Attribute& isArray(int length) { m_arrayLen = length; return *this; }

	/*!
	 * Sets the Attribute as being an INTEGER attribute.
	 * @details
	 * @copydetails isPk
	 */
	inline Attribute& isInt64() { m_type = INTEGER64; return *this; }

	/*!
	 * Set that the C Type is unsigned
	 * @return this
	 * @details
	 * @copydetails isPk
	 */
	inline Attribute& isUnsigned() { m_isUnsigned = true; return *this; }

	/*!
	 * Is this a TEXT Attribute?
	 * @return true or false
	 */
	bool getIsText() { return m_type == TEXT; }
	/*!
	 * Is this a Primary Key Attribute?
	 * @return true or false
	 */
	bool getIsPk() { return m_isPk; }
	/*!
	 * Is this a Foreign Key Attribute?
	 * @return true or false
	 */
	bool getIsFk() { return m_isFk; }
	/*!
	 * Is this Attribute clearable.
	 * @return true or false
	 */
	bool getIsClearable() { return m_isClearable; }

	/*!
	 * Is this Attribute an integer index primary key
	 * @return true or false
	 */
	bool getIsIndexPk() { return m_isIndexPk; }

	/*!
	 * Should this Attribute be used to order the results of a select
	 * @ return true or false
	 */
	bool getIsOrderByDesc() { return m_orderByDesc; }
	bool getIsOrderBy() { return m_orderBy; }

	/*!
	 *  Returns if multiple instances of this attribute are needed.
	 * @return bool
	 * @details
	 * This does not include char arrays
	 */
	bool getIsArray() { return m_arrayLen > 1; }

	/*!
	 * Return the name of the Attribute
	 * @return name
	 */
	std::string getName() { return m_name; }

	/*!
	 * Return the length of the array for the Attribute.
	 * @return int array length
	 */
	int getArrayLen() {return m_arrayLen;}

	/*!
	 * Return the max length of the string (char array) for the Attribute.
	 * @return int array length
	 */
	int getStringLen() { return m_strLen; }

	/*!
	 * Gets the SQL type used in creating SQL statements
	 * @return string representation of the @ref AttributeTypes "Attribute type"
	 */
	std::string getSqlType();

	/*!
	 * Get the string that represents a C type of this attribute used for creating C code
	 * @return "int", "char *", or related struct
	 */
	std::string getCType();

	/*!
	 * Get the string to use in a printf statement for the attribute type
	 */
	std::string getCFormatType();

	/*!
	 * Gets the target Entity for the foreign key
	 * @return Entity name
	 */
	std::string getFkTarget() { return m_fkTarget; }

	/*!
	 * Gets the target Entity.Attribute for the foreign key. This should be the Target Entity's Primary Key
	 * @return Entity name
	 */
	std::string getFkTargetAttribute() { return m_fkTargetAttribute; }

	/*!
	 * Gets if the PK is auto incrementing. This would mean that on inserts, the column
	 * should not be provided
	 */
	bool getIsAutoIncrementPk() { return m_isAutoIncrement;}

private:
	/*!
	 * Type of the attribute. Used internally
	 * @private
	 * @details
	 * INTEGER and TEXT should be self explanatory.  RELATIONSHIP indicates that the
	 * attribute is a foreign key into a parent table.
	 */
	enum AttributeTypes
	{
		UNKOWN = 0,
		INTEGER32 = 1,
		INTEGER64 = 2,
		TEXT = 3
	};

	std::string m_name; //!< private storage for the name of the attribute
	enum AttributeTypes m_type; //!< private storage for the type of this Attribute
	int m_arrayLen; //!< private storage for the length of an array if appropriate
	int m_strLen; //!< private storage for the max length of a string if appropriate
	bool m_isPk; //!< private storage determining if this attribute is a primary key
	bool m_isFk; //!< private storage determining if this attribute is a foreign key
	bool m_isClearable; //!< private storage determining if this attribute is clear-able
	std::string m_fkTarget; //!< private storage for the name of the related parent tables
	std::string m_fkTargetAttribute; //!< private storage for foreign key targets
	bool m_isUnsigned; //!< private storage for whether the type is also unsigned. All types can be unsigned
	bool m_isIndexPk; //!< private storage determining if this attribute is an index primary key
	bool m_orderByDesc; //!< private storage determining if this attribute should be ordered upon when selecting
	bool m_orderBy; //!< private storage determining if this attribute should be ordered upon when selecting
	bool m_isAutoIncrement; //!< private storage to determine if is an auto incrementing PK

};
#endif /* ATTRIBUTE_H_ */
