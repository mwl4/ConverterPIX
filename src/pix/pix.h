/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : pix.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

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
			Int, Int2, Int3, Int4,
			UInt, UInt2, UInt3, UInt4,
			Real, Real2, Real3, Real4, Real4x4,
			Boolean,
			String,
			Object
		};
	//private:
		struct ValueHolder
		{
			union
			{
				LargestInt	m_int;
				LargestInt	m_int2[2];
				LargestInt	m_int3[3];
				LargestInt	m_int4[4];

				LargestUInt	m_uint;
				LargestUInt	m_uint2[2];
				LargestUInt	m_uint3[3];
				LargestUInt	m_uint4[4];

				float		m_real;
				float		m_real2[2];
				float		m_real3[3];
				float		m_real4[4];
				float		m_real4x4[4][4];

				bool		m_boolean;
				bool		m_boolean[2];
				bool		m_boolean[3];
				bool		m_boolean[4];
			};
			String			m_string;
		};

		struct : ValueHolder
		{
			Array<ValueHolder> m_array;

			struct Object
			{
				String m_name;
				UniquePtr<Value> m_value;
			};
			Array<Object> m_object;
		} m_value;

		bool m_isArray = false;
		bool m_isIndexedArray = false;
		Type m_type;
		
	public:
		Value(Type type = Type::Null);

		template < typename T >
		Value(T value, typename EnableIfArithmetic<T>::type = 0)
		{
			if (IsFloatingPoint<T>::value)
			{
				m_type = Type::Real;
				m_value.m_real = static_cast<decltype(m_value.m_real)>(value);
			}
			else if (std::numeric_limits<T>::is_signed)
			{
				m_type = Type::Int;
				m_value.m_int = static_cast<LargestInt>(value);
			}
			else
			{
				m_type = Type::UInt;
				m_value.m_uint = static_cast<LargestUInt>(value);
			}
		}

		template < typename T, size_t N >
		Value(const prism::vec_t<T, N> &value, typename EnableIfArithmetic<T>::type = 0)
		{
			if (IsFloatingPoint<T>::value)
			{
				switch (N)
				{
					case 4: m_value.m_real4[3] = value[3];
					case 3: m_value.m_real3[2] = value[2];
					case 2: m_value.m_real2[1] = value[1];
					case 1: m_value.m_real = value[0];
				}
				switch (N)
				{
					case 4: m_type = Type::Real4; break;
					case 3: m_type = Type::Real3; break;
					case 2: m_type = Type::Real2; break;
					case 1: m_type = Type::Real; break;
				}
			}
			else if (std::numeric_limits<T>::is_signed)
			{
				switch (N)
				{
					case 4: m_value.m_int4[3] = value[3];
					case 3: m_value.m_int3[2] = value[2];
					case 2: m_value.m_int2[1] = value[1];
					case 1: m_value.m_int = value[0];
				}
				switch (N)
				{
					case 4: m_type = Type::Int4; break;
					case 3: m_type = Type::Int3; break;
					case 2: m_type = Type::Int2; break;
					case 1: m_type = Type::Int; break;
				}
			}
			else
			{
				switch (N)
				{
					case 4: m_value.m_uint4[3] = value[3];
					case 3: m_value.m_uint3[2] = value[2];
					case 2: m_value.m_uint2[1] = value[1];
					case 1: m_value.m_uint = value[0];
				}
				switch (N)
				{
					case 4: m_type = Type::UInt4; break;
					case 3: m_type = Type::UInt3; break;
					case 2: m_type = Type::UInt2; break;
					case 1: m_type = Type::UInt; break;
				}
			}
		}

		template < size_t N >
		Value(const prism::vec_t<bool, N> &value)
		{

		}

		Value(const String &value);
		Value(bool value);
		Value(const Value &rhs);

		Value &operator=(const Value &rhs);

		Type type() const { return m_type; }
		bool isArray() const { return m_isArray; }

		LargestInt asInt() const;
		LargestUInt asUInt() const;

		float asFloat() const;
		double asDouble() const;
		bool asBool() const;




	};
}

/* eof */
