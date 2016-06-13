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
#ifndef CR_MGMT_PROPERTYDEFINITIONLIST_H
#define CR_MGMT_PROPERTYDEFINITIONLIST_H

#include <vector>
#include <nvm_types.h>
#include <libinvm-cli/CliFrameworkTypes.h>
#include "PropertyDefinitionBase.h"

namespace cli
{
namespace framework
{

/*
 * PropertyDefinitionList is helpful for defining results for a CLI command. Using templates, a
 * PropertyDefinitionList will "wrap" a core business logic class and its methods. It can then
 * convert an instance/object of the core class to a PropertyListResult object for the CLI. Each
 * "PropertyDefinitionList::add" method takes the "key" for the property and the method of the
 * class being "wrapped" that will return the value for the property.
 *
 * In order to convert the core business object to a PropertyListResult all values must be converted
 * to strings. Each type has a default conversion. This conversion can be overwritten by using a 3rd
 * parameter to many of the "PropertyDefinitionList::add" methods. This third parameter is a pointer
 * to a function that will do the appropriate conversion. The signature of this function must be
 * std::string func([type needing to convert].
 */
template<class T>
class PropertyDefinitionList
{
public:
	~PropertyDefinitionList()
	{
		for (size_t i = 0; i < m_props.size(); i++)
		{
			delete m_props[i];
		}
	}

	IPropertyDefinition<T> &addCustom(std::string key, std::string (*function)(T &))
	{
		ObjectPropertyDefinition<T> *v = new ObjectPropertyDefinition<T>(key, function);
		m_props.push_back(v);
		return *v;
	}

	IPropertyDefinition<T> &addStr(std::string key, std::string (T::*function)())
	{
		StringPropertyDefinition<T> *v = new StringPropertyDefinition<T>(key, function);
		m_props.push_back(v);
		return *v;
	}
	IPropertyDefinition<T> &addUint64(std::string key, NVM_UINT64 (T::*function)(),
		std::string (*converter)(NVM_UINT64) = NULL)
	{
		Uint64PropertyDefinition<T> *v = new Uint64PropertyDefinition<T>(key, function, converter);
		m_props.push_back(v);
		return *v;
	}

	IPropertyDefinition<T> &addUint32(std::string key, NVM_UINT32 (T::*function)(),
		std::string (*converter)(NVM_UINT32) = NULL)
	{
		Uint32PropertyDefinition<T> *v = new Uint32PropertyDefinition<T>(key, function, converter);
		m_props.push_back(v);
		return *v;
	}

	IPropertyDefinition<T> &addUint16(std::string key, NVM_UINT16 (T::*function)(),
		std::string (*converter)(NVM_UINT16) = NULL)
	{
		Uint16PropertyDefinition<T> *v = new Uint16PropertyDefinition<T>(key, function, converter);
		m_props.push_back(v);
		return *v;
	}

	IPropertyDefinition<T> &addUint8(std::string key, NVM_UINT8 (T::*function)(),
		std::string (*converter)(NVM_UINT8) = NULL)
	{
		Uint8PropertyDefinition<T> *v = new Uint8PropertyDefinition<T>(key, function, converter);
		m_props.push_back(v);
		return *v;
	}

	IPropertyDefinition<T> &addBool(std::string key, bool (T::*function)())
	{
		BoolPropertyDefinition<T> *v = new BoolPropertyDefinition<T>(key, function);
		m_props.push_back(v);
		return *v;
	}

	template<class R>
	IPropertyDefinition<T> &addList(std::string key, std::vector<R> (T::*function)(),
		std::string (*converter)(R) = NULL)
	{
		ListPropertyDefinition<T, R> *var = new ListPropertyDefinition<T, R>(key, function,
			converter);
		m_props.push_back(var);
		return *var;
	}

	template<class R>
	IPropertyDefinition<T> &addOther(std::string key, R (T::*function)(),
			std::string (*converter)(R) = NULL)
	{
		PropertyDefinitionBase<T, R> *v =
			new PropertyDefinitionBase<T, R>(key, function, converter);
		m_props.push_back(v);
		return *v;
	}

	IPropertyDefinition<T> &operator[](const int i) const
	{
		return *m_props[i];
	}

	size_t size() const
	{
		return m_props.size();
	}

	bool contains(const std::string &key) const
	{
		bool result = false;
		for (size_t i = 0; i < m_props.size() && !result; i++)
		{
			if (stringsIEqual(m_props[i]->getName(), key))
			{
				result = true;
			}
		}
		return result;
	}
private:
	std::vector<IPropertyDefinition<T> *> m_props;
};
}
}

#endif //CR_MGMT_PROPERTYDEFINITIONLIST_H
