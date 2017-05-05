/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : pix.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <fs/file.h>

#include <math/vector.h>
#include <math/quaternion.h>
#include <math/matrix.h>

namespace Pix
{
	class Value
	{
	public:
		using LargestInt	= int64_t;
		using LargestUInt	= uint64_t;

		enum class Type
		{
			Null = 0,
			Int,
			UInt,
			Float,
			FloatMatrix,
			Double,
			Boolean,
			String,
			Enum,
			Object
		};

		Type m_type;

		class Enumeration
		{
		public:
			explicit Enumeration(const String &s)
				: m_name(s)
			{
			}

		public:
			String m_name;
		};

		class Custom
		{
		public:
			virtual String Get() const = 0;
		};

	private:
		struct ValueHolder
		{
			size_t	m_valueCount = 0;
			bool	m_parentheses = false;
			union
			{
				LargestInt	m_int[4];
				LargestUInt	m_uint[4];
				double		m_double[4];
				float		m_float[4];
				float		m_float4x4[4][4];
				bool		m_boolean[4];
			};
			String	m_string;
		};
		Array<ValueHolder> m_values;

		struct ObjectsHolder
		{
			struct NamedObject;

			Array<NamedObject>	m_named;
			Array<Value>		m_indexed;
		} m_objects;

	public:
		Value(Type type = Type::Null);

		template < typename T >
		Value(T value, typename EnableIfArithmetic<T>::type = 0)
		{
			ValueHolder valueHolder;
			valueHolder.m_valueCount = 1;
			if (IsFloatingPoint<T>::value)
			{
				if (std::is_same<T, float>::value)
				{
					m_type = Type::Float;
					valueHolder.m_float[0] = static_cast<float>(value);
				}
				else
				{
					m_type = Type::Double;
					valueHolder.m_double[0] = static_cast<double>(value);
				}
			}
			else if (std::numeric_limits<T>::is_signed)
			{
				m_type = Type::Int;
				valueHolder.m_int[0] = static_cast<LargestInt>(value);
			}
			else
			{
				m_type = Type::UInt;
				valueHolder.m_uint[0] = static_cast<LargestUInt>(value);
			}
			m_values.push_back(valueHolder);
		}

		template < typename T, size_t N >
		Value(const prism::vec_t<T, N> &value, size_t valueCount = N, typename EnableIfArithmetic<T>::type = 0)
		{
			ValueHolder valueHolder;
			valueHolder.m_valueCount = valueCount;
			valueHolder.m_parentheses = true;
			if (IsFloatingPoint<T>::value)
			{
				if (std::is_same<T, float>::value)
				{
					m_type = Type::Float;
					for (size_t i = 0; i < valueCount; ++i)
					{
						valueHolder.m_float[i] = static_cast<float>(value.m_a[i]);
					}
				}
				else
				{
					m_type = Type::Double;
					for (size_t i = 0; i < valueCount; ++i)
					{
						valueHolder.m_double[i] = static_cast<double>(value.m_a[i]);
					}
				}
			}
			else if (std::numeric_limits<T>::is_signed)
			{
				for (size_t i = 0; i < valueCount; ++i)
				{
					valueHolder.m_int[i] = static_cast<LargestInt>(value.m_a[i]);
				}
				m_type = Type::Int;
			}
			else
			{
				for (size_t i = 0; i < valueCount; ++i)
				{
					valueHolder.m_uint[i] = static_cast<LargestUInt>(value.m_a[i]);
				}
				m_type = Type::UInt;
			}
			m_values.push_back(valueHolder);
		}

		template < size_t N >
		Value(const prism::mat_sq_t<float, N> &value)
		{
			ValueHolder valueHolder;
			valueHolder.m_valueCount = N;
			valueHolder.m_parentheses = true;
			for (size_t i = 0; i < N; ++i)
			{
				for (size_t j = 0; j < N; ++j)
				{
					valueHolder.m_float4x4[i][j] = value.m[j][i];
				}
			}
			m_type = Type::FloatMatrix;
			m_values.push_back(valueHolder);
		}

		template < typename T >
		Value(const Array<T> &values, typename EnableIfArithmetic<T>::type = 0)
		{
			for (const auto &value : values)
			{
				ValueHolder valueHolder;
				valueHolder.m_valueCount = 1;
				if (IsFloatingPoint<T>::value)
				{
					valueHolder.m_float[0] = static_cast<float>(value);
					m_type = Type::Float;
				}
				else if (std::numeric_limits<T>::is_signed)
				{
					valueHolder.m_int[0] = static_cast<LargestInt>(value);
					m_type = Type::Int;
				}
				else
				{
					valueHolder.m_uint[0] = static_cast<LargestUInt>(value);
					m_type = Type::UInt;
				}
				m_values.push_back(valueHolder);
			}
		}

		template < size_t N >
		Value(const prism::vec_t<bool, N> &value)
		{
			ValueHolder valueHolder;
			valueHolder.m_valueCount = N;
			valueHolder.m_parentheses = true;
			for (size_t i = 0; i < N; ++i)
			{
				valueHolder.m_boolean[i] = value.m_a[i];
			}
		}

		Value(const char *const value);
		Value(const String &value);
		Value(const Enumeration &value);
		Value(bool value);
		Value(const Array<String> &value);
		Value(Quaternion quat);
		Value(const Value &rhs);
		~Value();

		Value &operator=(const Value &rhs);

		Type type() const { return m_type; }

		Value &addObject(const String &name);

		Value &operator[](const String &name);
		Value &operator[](const size_t index);

		void allocateNamedObjects(const size_t size);
		void allocateIndexedObjects(const size_t size);
		inline Array<Value> &getIndexedObjects() { return m_objects.m_indexed; }

		friend class Writer;
		friend class StyledWriter;
		friend class FileWriter;
		friend class StyledFileWriter;
	};

	struct Value::ObjectsHolder::NamedObject
	{
		String m_name;
		Value m_value;

		NamedObject(const String &name)
			: m_name(name)
		{
		}
	};

	class Writer
	{
	public:
		Writer();
		virtual ~Writer();

	protected:
		virtual void push(const String &value) = 0;
	};

	class StyledWriter : public Writer
	{
	public:
		StyledWriter(String indentation = String(4, ' '), String newLine = "\n");
		virtual ~StyledWriter();

	protected:
		void writeValue(const Value &value);

		void indent();
		void unindent();
		void writeWithIndent(const String &value);

	protected:
		String m_indent;
		size_t m_indentSize = 0;
		String m_defaultIndentation;
		String m_defaultNewLine;
	};

	class StyledStringWriter : public StyledWriter
	{
	public:
		String write(const Value &value);

	protected:
		virtual void push(const String &value) override;

	private:
		String m_data;
	};

	class StyledFileWriter : public StyledWriter
	{
	public:
		StyledFileWriter();
		virtual ~StyledFileWriter();

	public:
		void write(File *const file, const Value &value);

	protected:
		virtual void push(const String &value) override;

	private:
		File *m_file = nullptr;
	};

} // namespace Pix

/* eof */
