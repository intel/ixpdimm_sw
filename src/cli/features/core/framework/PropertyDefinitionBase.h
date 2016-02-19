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
#ifndef CR_MGMT_PROPERTYDEFINITIONBASE_H
#define CR_MGMT_PROPERTYDEFINITIONBASE_H

#include <vector>
#include <string>
#include <sstream>

namespace cli
{
namespace framework
{

template<class T>
class IPropertyDefinition
{
public:
	IPropertyDefinition(const std::string &name) :
		m_name(name),
		m_isRequired(false),
		m_isDefault(false) { }
	virtual ~IPropertyDefinition() { }

	virtual std::string getValue(T &obj) = 0;

	std::string getName() { return m_name; }

	void setIsRequired() { m_isRequired = true; }
	bool isRequired() { return m_isRequired; }

	void setIsDefault() { m_isDefault = true; }
	bool isDefault() { return m_isDefault; }

protected:
	std::string m_name;

	bool m_isRequired;
	bool m_isDefault;
};

template<class T>
class ObjectPropertyDefinition : public IPropertyDefinition<T>
{
public:
	ObjectPropertyDefinition(const std::string &name, std::string (*func)(T &))
		: IPropertyDefinition<T>(name), m_func(func) { }

	virtual std::string getValue(T &obj)
	{
		return m_func(obj);
	}
private:
	std::string (*m_func)(T &);

};

template<class T, class R>
class PropertyDefinitionBase : public IPropertyDefinition<T>
{
public:
	virtual ~PropertyDefinitionBase() { }
	PropertyDefinitionBase(std::string name,
		R (T::*pFunction)(),
		std::string (*conversionFunction)(R) = NULL)
		: IPropertyDefinition<T>(name),
		m_pFunction(pFunction), m_pConversionFunction(conversionFunction)
	{

	}

	virtual std::string getValue(T &obj)
	{
		std::string result;
		R value = (obj.*m_pFunction)();
		if (m_pConversionFunction)
		{
			result = m_pConversionFunction(value);
		}
		else
		{
			std::stringstream resultStream;
			resultStream << value;
			result = resultStream.str();
		}

		return result;
	}

protected:
	R (T::*m_pFunction)();
	std::string (*m_pConversionFunction)(R);
};

template<class T>
class StringPropertyDefinition : public PropertyDefinitionBase<T, std::string>
{
public:
	StringPropertyDefinition(const std::string &name, std::string (T::*stringFunction)()) :
		PropertyDefinitionBase<T, std::string>(name, stringFunction)
	{

	}

	virtual std::string getValue(T &obj)
	{
		return (obj.*this->m_pFunction)();
	}
};

template<class T>
class Uint64PropertyDefinition : public PropertyDefinitionBase<T, unsigned long long int>
{
public:
	Uint64PropertyDefinition(std::string name, unsigned long long int (T::*pFunction)())
		: PropertyDefinitionBase<T, unsigned long long int>(name, pFunction)
	{
	}
	Uint64PropertyDefinition(std::string name, unsigned long long int (T::*pFunction)(),
		std::string (*conversionFunction)(unsigned long long))
		: PropertyDefinitionBase<T, unsigned long long int>(name, pFunction, conversionFunction)
	{
	}
};


template<class T>
class Uint32PropertyDefinition : public PropertyDefinitionBase<T, unsigned int>
{
public:
	Uint32PropertyDefinition(std::string name, unsigned int (T::*pFunction)())
		: PropertyDefinitionBase<T, unsigned int>(name, pFunction)
	{
	}
	Uint32PropertyDefinition(std::string name, unsigned int (T::*pFunction)(),
		std::string(*converter)(unsigned int))
		: PropertyDefinitionBase<T, unsigned int>(name, pFunction, converter)
	{
	}
};


template<class T>
class Uint16PropertyDefinition : public PropertyDefinitionBase<T, unsigned short>
{
public:
	Uint16PropertyDefinition(std::string name, unsigned short (T::*pFunction)())
		: PropertyDefinitionBase<T, unsigned short>(name, pFunction)
	{
	}
	Uint16PropertyDefinition(std::string name, unsigned short (T::*pFunction)(),
		std::string(*converter)(unsigned short))
		: PropertyDefinitionBase<T, unsigned short>(name, pFunction, converter)
	{
	}
};

template<class T>
class Uint8PropertyDefinition : public PropertyDefinitionBase<T, unsigned char>
{
public:
	Uint8PropertyDefinition(std::string name, unsigned char (T::*pFunction)())
		: PropertyDefinitionBase<T, unsigned char>(name, pFunction)
	{
	}
	Uint8PropertyDefinition(std::string name, unsigned char (T::*pFunction)(),
		std::string(*converter)(unsigned char))
		: PropertyDefinitionBase<T, unsigned char>(name, pFunction, converter)
	{
	}


	virtual std::string getValue(T &obj)
	{
		std::string result;
		unsigned char value = (obj.*this->m_pFunction)();
		if (this->m_pConversionFunction)
		{
			result = this->m_pConversionFunction(value);
		}
		else
		{
			// Have to cast as unsigned short otherwise is outputted as hex value
			std::stringstream resultStream;
			resultStream << (unsigned short) value;
			result = resultStream.str();
		}

		return result;
	}
};

template<class T>
class BoolPropertyDefinition : public PropertyDefinitionBase<T, bool>
{

public:
	BoolPropertyDefinition(const std::string &name, bool (T::*function)()) :
		PropertyDefinitionBase<T, bool>(name, function) { }

	virtual std::string getValue(T &obj)
	{
		return (obj.*this->m_pFunction)() ? "1" : "0";
	}
};

template<class T, class R>
class ListPropertyDefinition : public IPropertyDefinition<T>
{

public:
	ListPropertyDefinition(const std::string &name,
		std::vector<R> (T::*pFunction)(),
		std::string (*conversionFunction)(R) = NULL)
		: IPropertyDefinition<T>(name),
		m_pFunction(pFunction),
		m_pConversionFunction(conversionFunction) { }

	virtual std::string getValue(T &obj)
	{
		std::vector<R> value = (obj.*m_pFunction)();
		std::string result = "";

		if (value.size() > 0)
		{
			for (size_t i = 0; i < value.size(); i++)
			{
				if (i > 0)
				{
					result += ", ";
				}
				R item = value[i];
				if (m_pConversionFunction)
				{
					result += m_pConversionFunction(item);
				}
				else
				{
					std::stringstream stream;
					stream << item;
					result += stream.str();
				}
			}
		}
		else
		{
			result = "N/A";
		}
		return result;
	}

protected:
	std::vector<R> (T::*m_pFunction)();
	std::string (*m_pConversionFunction)(R);
};

}
}


#endif //CR_MGMT_PROPERTYDEFINITIONBASE_H
