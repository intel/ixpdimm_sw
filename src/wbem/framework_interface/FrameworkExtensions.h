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

#ifndef CR_MGMT_FRAMEWORKEXTENSIONS_H
#define CR_MGMT_FRAMEWORKEXTENSIONS_H

#include <algorithm>

#define ADD_KEY_ATTRIBUTE(keys, key, type, value) \
	(keys)[(key)] = framework::Attribute ((type)(value), true)

#define ADD_ATTRIBUTE(i, a, key, type, value) \
    (i).setAttribute(key, framework::Attribute((type)value, false), a)

#define ADD_DATETIME_ATTRIBUTE(i, a, key, value) \
	instance.setAttribute(key, framework::Attribute ((value), \
	wbem::framework::DATETIME_SUBTYPE_DATETIME, false) , a);

namespace wbem
{

void checkAttributesAreModifiable(framework::Instance *pInstance,
	framework::attributes_t &attributes,
	framework::attribute_names_t modifiableAttributes);

inline bool isNameInAttributeNameList(wbem::framework::attribute_names_t names, std::string toFind)
{
	return (std::find(names.begin(), names.end(), toFind) != names.end());
}
}



#endif //CR_MGMT_FRAMEWORKEXTENSIONS_H
