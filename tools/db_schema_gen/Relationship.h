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
 * This file contains the definition and implementation for the Relationship class
 * which defines relationships between @ref Entity Entities.
 */

#ifndef RELATIONSHIP_H_
#define RELATIONSHIP_H_

/*!
 * Represents a 1 to many relationship between two entities.  The relationship is defined on the "Parent" Entity
 * while the child Entity has a Foreign Key pointing back to the parent.
 * @ingroup code_gen
 */
class Relationship
{
public:
	/*!
	 * Constructor
	 * @param name
	 * 		Name of the Relationship.  This will be the field name used when building the C struct.
	 * @param relatedEntity
	 * 		Name of the child Entity
	 * @param count
	 * 		It's a 1 to many relationship, however, C structs must have a predefined max value for arrays.
	 */
	Relationship(std::string name, std::string relatedEntity, int count = 1)
	: _name(name), _relatedEntity(relatedEntity), _count(count)
	{}

	/*!
	 * Accessor for name
	 * @return name of the Relationship
	 */
	std::string getName() { return _name; }

	/*!
	 * Accessor for the Related Entity
	 * @return name of the child Entity
	 */
	std::string getRelatedEntity() { return _relatedEntity; }

	/*!
	 * Accessor for count
	 * @return size of the array
	 */
	int getCount() { return _count; }

private:
	std::string _name;
	std::string _relatedEntity;
	int _count;

};

#endif /* RELATIONSHIP_H_ */
