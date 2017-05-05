/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : pix.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "pix.h"

String toString(const float value[], const size_t count);
String toString(const double value[], const size_t count);
String toString(const Pix::Value::LargestInt value[], const size_t count);

using namespace Pix;

Value::Value(Type type/*= Type::Null*/)
{
	m_type = type;
}

Value::Value(const char *const value)
	: Value(String(value))
{
}

Value::Value(const String &value)
{
	m_type = Type::String;
	ValueHolder valueHolder;
	valueHolder.m_valueCount = 1;
	valueHolder.m_string = value;
	m_values.push_back(valueHolder);
}

Value::Value(const Enumeration &value)
{
	m_type = Type::Enum;
	ValueHolder valueHolder;
	valueHolder.m_valueCount = 1;
	valueHolder.m_string = value.m_name;
	m_values.push_back(valueHolder);
}

Value::Value(bool value)
{
	m_type = Type::Boolean;
	ValueHolder valueHolder;
	valueHolder.m_valueCount = 1;
	valueHolder.m_boolean[0] = value;
	m_values.push_back(valueHolder);
}

Value::Value(const Array<String> &value)
{
	m_type = Type::String;
	for (const String &s : value)
	{
		ValueHolder valueHolder;
		valueHolder.m_valueCount = 1;
		valueHolder.m_string = s;
		m_values.push_back(valueHolder);
	}
}

Value::Value(Quaternion quat)
{
	m_type = Type::Float;
	ValueHolder valueHolder;
	valueHolder.m_valueCount = 4;
	valueHolder.m_float[0] = quat.m_w;
	valueHolder.m_float[1] = quat.m_x;
	valueHolder.m_float[2] = quat.m_y;
	valueHolder.m_float[3] = quat.m_z;
	valueHolder.m_parentheses = true;
	m_values.push_back(valueHolder);
}

Value::Value(const Value &rhs)
{
	m_type = rhs.m_type;
	switch (m_type)
	{
		case Type::Null:
		case Type::Int:
		case Type::UInt:
		case Type::Double:
		case Type::Float:
		case Type::FloatMatrix:
		case Type::Boolean:
			m_values = rhs.m_values;
			break;
		case Type::String:
		case Type::Enum:
			for (const auto &value : rhs.m_values)
			{
				ValueHolder valueHolder;
				valueHolder.m_valueCount = 1;
				valueHolder.m_string = value.m_string;
				m_values.push_back(valueHolder);
			}
			break;
		case Type::Object:
			m_objects = rhs.m_objects;
		break;
		default:
			assert(false); // Unreachable assert
	}
}

Value::~Value()
{
}

Value &Value::operator=(const Value &rhs)
{
	m_type = rhs.m_type;
	switch (m_type)
	{
		case Type::Null:
		case Type::Int:
		case Type::UInt:
		case Type::Double:
		case Type::Float:
		case Type::FloatMatrix:
		case Type::Boolean:
			m_values = rhs.m_values;
			break;
		case Type::String:
		case Type::Enum:
			for (const auto &value : rhs.m_values)
			{
				ValueHolder valueHolder;
				valueHolder.m_valueCount = 1;
				valueHolder.m_string = value.m_string;
				m_values.push_back(valueHolder);
			}
			break;
		case Type::Object:
			m_objects = rhs.m_objects;
		break;
		default:
			assert(false); // Unreachable assert
	}
	return (*this);
}

Value &Value::addObject(const String &name)
{
	m_type = Type::Object;
	m_objects.m_named.push_back(ObjectsHolder::NamedObject(name));
	return m_objects.m_named.back().m_value;
}

Value &Value::operator[](const String &name)
{
	return addObject(name);
}

Value &Value::operator[](const size_t index)
{
	return m_objects.m_indexed.data()[index];
}

void Value::allocateNamedObjects(const size_t size)
{
	m_type = Type::Object;
	m_objects.m_named.reserve(size);
}

void Value::allocateIndexedObjects(const size_t size)
{
	m_type = Type::Object;
	m_objects.m_indexed.resize(size);
}

Writer::Writer()
{
}

Writer::~Writer()
{
}

StyledWriter::StyledWriter(String indentation/* = String(4, ' ')*/, String newLine/* = "\n"*/)
{
	m_defaultIndentation = indentation;
	m_defaultNewLine = newLine;
}

StyledWriter::~StyledWriter()
{
}

void StyledWriter::writeValue(const Value &value)
{
	switch (value.type())
	{
		case Value::Type::Null:
			push("null");
			break;
		case Value::Type::Int:
			for (const auto &v : value.m_values)
			{
				push(toString(v.m_int, v.m_valueCount));
				if (&v != &value.m_values.back())
				{
					push(" ");
				}
			}
			break;
		case Value::Type::UInt:
			for (const auto &v : value.m_values)
			{
				push(std::to_string(v.m_uint[0]));
				if (&v != &value.m_values.back())
				{
					push(" ");
				}
			}
			break;
		case Value::Type::Float:
			for (const auto &v : value.m_values)
			{
				push(toString(v.m_float, v.m_valueCount));
				if (&v != &value.m_values.back())
				{
					push(" ");
				}
			}
			break;
		case Value::Type::FloatMatrix:
			for (size_t i = 0; i < value.m_values[0].m_valueCount; ++i)
			{
				for (size_t j = 0; j < value.m_values[0].m_valueCount; ++j)
				{
					push(((j != 0 ? "  " : "") + fmt::sprintf(FLT_FT, flh(value.m_values[0].m_float4x4[i][j]))));
				}
				if (i != (value.m_values[0].m_valueCount - 1))
				{
					push(m_defaultNewLine + m_indent + String(7, ' '));
				}
			}
			break;
		case Value::Type::Double:
			for (const auto &v : value.m_values)
			{
				push(toString(v.m_double, v.m_valueCount));
				if (&v != &value.m_values.back())
				{
					push(" ");
				}
			}
			break;
		case Value::Type::String:
			for (const auto &v : value.m_values)
			{
				push(valueToQuotedString(v.m_string));
				if (&v != &value.m_values.back())
				{
					push(" ");
				}
			}
			break;
		case Value::Type::Enum:
			push(value.m_values[0].m_string);
			break;
		case Value::Type::Object:
			indent();
			for (auto it = value.m_objects.m_named.cbegin(); it != value.m_objects.m_named.cend(); ++it)
			{
				const auto &object = *it;
				const bool header = object.m_value.m_type == Value::Type::Object;
				const bool parentheses = object.m_value.m_values.size() != 0 ? object.m_value.m_values[0].m_parentheses : false;
				writeWithIndent(object.m_name);
				push(header ? (" {" + m_defaultNewLine) : ": ");
				push(parentheses ? "( " : "");
				writeValue(object.m_value);
				push(parentheses ? " )" : "");
				if(header)
				{
					writeWithIndent("}");
				}
				push(m_defaultNewLine);
			}
			for (size_t i = 0; i < value.m_objects.m_indexed.size(); ++i)
			{
				const auto &object = value.m_objects.m_indexed[i];
				writeWithIndent(fmt::sprintf("%-5i", i));
				push("( ");
				writeValue(object);
				push(" )" + m_defaultNewLine);
			}
			unindent();
			break;
		default:
			; // Unreachable
	}
}

void StyledWriter::indent()
{
	++m_indentSize;
	if (m_indentSize > 1)
	{
		m_indent += m_defaultIndentation;
	}
}

void StyledWriter::unindent()
{
	if (m_indentSize > 0)
	{
		--m_indentSize;
		if (m_indentSize > 0)
		{
			m_indent.resize(m_indent.length() - m_defaultIndentation.length());
		}
	}
}

void StyledWriter::writeWithIndent(const String &value)
{
	push(m_indent + value);
}

String StyledStringWriter::write(const Value &value)
{
	m_data = "";
	writeValue(value);
	push(m_defaultNewLine);
	return m_data;
}

void StyledStringWriter::push(const String &value)
{
	m_data += value;
}

StyledFileWriter::StyledFileWriter()
{
}

StyledFileWriter::~StyledFileWriter()
{
}

void StyledFileWriter::write(File *const file, const Value &value)
{
	m_file = file;
	writeValue(value);
	push(m_defaultNewLine);
}

void StyledFileWriter::push(const String &value)
{
	(*m_file) << value;
}

inline void floatToBuffer(char *const buffer, uint32_t fl)
{
#define TO_HEX(i) (i <= 9 ? '0' + i : 'A' - 10 + i)
	buffer[0] = '&';
	buffer[1] = TO_HEX(((fl & 0xF0000000) >> 28));
	buffer[2] = TO_HEX(((fl & 0x0F000000) >> 24));
	buffer[3] = TO_HEX(((fl & 0x00F00000) >> 20));
	buffer[4] = TO_HEX(((fl & 0x000F0000) >> 16));
	buffer[5] = TO_HEX(((fl & 0x0000F000) >> 12));
	buffer[6] = TO_HEX(((fl & 0x00000F00) >> 8));
	buffer[7] = TO_HEX(((fl & 0x000000F0) >> 4));
	buffer[8] = TO_HEX(((fl & 0x0000000F)));
	buffer[9] = '\0';
#undef TO_HEX
}

inline String toString(const float value[], const size_t count)
{
	String result;
	result.reserve(64);

	for (size_t i = 0; i < count; ++i)
	{
		if (i != 0)
		{
			result.append("  ");
		}

		char buffer[10];
		floatToBuffer(buffer, flh(value[i]));
		result.append(buffer);
	}
	return result;
}

inline String toString(const double value[], const size_t count)
{
	String result;
	for (size_t i = 0; i < count; ++i)
	{
		result += ((i != 0 ? "  " : "") + fmt::sprintf("%f", value[i]));
	}
	return result;
}

inline String toString(const Pix::Value::LargestInt value[], const size_t count)
{
	const String format = count > 1 ? "%-5lli" : "%lli";
	String result;
	for (size_t i = 0; i < count; ++i)
	{
		result += ((i != 0 ? "  " : "") + fmt::sprintf(format, value[i]));
	}
	return result;
}

void pix_test()
{
	/*Pix::Value root;

	Pix::Value &header = root["Header"];
	header["FormatVersion"] = 5;
	header["Source"] = String("Blender 2.76.0 (hash: 48f7dd6), SCS Blender Tools: 0.6.e9daf33");
	header["Type"] = "Model";
	header["Name"] = "Name";

	Pix::Value &global = root["Global"];
	global["VertexCount"] = 3815;
	global["TriangleCount"] = 4672;
	global["MaterialCount"] = 9;
	global["TotalTime"] = 0.10000000149011612;

	for (int i = 0; i < 5; ++i)
	{
	Pix::Value &material = root["Material"];
	material["aa"] = 1;
	}

	for (int i = 0; i < 5; ++i)
	{
	Pix::Value &piece = root["Piece"];
	piece["Index"] = i;

	for (int j = 0; j < 10; ++j)
	{
	Pix::Value &stream = piece["Stream"];
	stream["Format"] = Pix::Value::Enumeration("FLOAT3");
	stream["Tag"] = "_POSITION";
	stream.allocateIndexedObjects(10);
	for (auto &i : stream.m_objects.m_indexed)
	{
	i = Float3(0.2f, 0.3f, 0.4f);
	}
	}
	}

	Pix::Value &skin = root["Skin"];
	skin["StreamCount"] = 1;
	skin["Test"] = Array<int>{ 1, 2, 3, 4, 5, 6, 7, 8 };

	Pix::Value &bone = root["BoneChannel"];
	bone["Name"] = "joint4";
	bone["StreamCount"] = 2;
	bone["KeyframeCount"] = 2;
	{
	Pix::Value &stream = root["Stream"];
	stream["Format"] = Pix::Value::Enumeration("FLOAT");
	stream["Tag"] = "_TIME";
	stream.allocateIndexedObjects(2);
	stream.m_objects.m_indexed[0] = 0.1f;
	stream.m_objects.m_indexed[1] = 0.2f;
	}

	Pix::StyledWriter writer;
	String result = writer.write(root);
	printf("result = \n%s\n", result.c_str());*/

	const int a = sizeof(Pix::Value);
}

/* eof */
