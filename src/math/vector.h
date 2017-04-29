/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : vector.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once
#pragma pack(push, 1)

namespace prism
{
	template < typename T, size_t N >
	class vec_t // sizeof(sizeof(T) * N)
	{
	public:
		T m_a[N];
	public:
		vec_t()
		{
			for (size_t i = 0; i < N; ++i)
			{
				m_a[i] = T();
			}
		}

		template < size_t COUNT = N >
		vec_t(T x, typename EnableIf<COUNT == 1, int>::type = 0)
		{
			m_a[0] = x;
		}

		template < size_t COUNT = N >
		vec_t(T x, T y, typename EnableIf<COUNT == 2, int>::type = 0)
		{
			m_a[0] = x;
			m_a[1] = y;
		}

		template < size_t COUNT = N >
		vec_t(T x, T y, T z, typename EnableIf<COUNT == 3, int>::type = 0)
		{
			m_a[0] = x;
			m_a[1] = y;
			m_a[2] = z;
		}

		template < size_t COUNT = N >
		vec_t(T w, T x, T y, T z, typename EnableIf<COUNT == 4, int>::type = 0)
		{
			m_a[0] = w;
			m_a[1] = x;
			m_a[2] = y;
			m_a[3] = z;
		}

		T &get_at(size_t idx)
		{
			return m_a[idx];
		}

		T &operator[](size_t idx)
		{
			return get_at(idx);
		}

		const T &get_at(size_t idx) const
		{
			return m_a[idx];
		}

		const T &operator[](size_t idx) const
		{
			return get_at(idx);
		}

		float length() const
		{
			float result = 0.f;
			for (size_t i = 0; i < N; ++i)
			{
				result += m_a[i] * m_a[i];
			}
			return sqrtf(result);
		}

		vec_t<T, N> &operator=(const T &value)
		{
			for (size_t i = 0; i < N; ++i)
			{
				m_a[i] = value;
			}
			return (*this);
		}

		inline size_t count() const
		{
			return N;
		}
	};

	typedef vec_t<float, 1> float1; // sizeof(4)
	typedef vec_t<float, 2> float2; // sizeof(8)
	typedef vec_t<float, 3> float3; // sizeof(12)
	typedef vec_t<float, 4> float4; // sizeof(16)

	ENSURE_SIZE(float1, 4);
	ENSURE_SIZE(float2, 8);
	ENSURE_SIZE(float3, 12);
	ENSURE_SIZE(float4, 16);

	typedef vec_t<int, 1> int1; // sizeof(4)
	typedef vec_t<int, 2> int2; // sizeof(8)
	typedef vec_t<int, 3> int3; // sizeof(12)
	typedef vec_t<int, 4> int4; // sizeof(16)

	ENSURE_SIZE(int1, 4);
	ENSURE_SIZE(int2, 8);
	ENSURE_SIZE(int3, 12);
	ENSURE_SIZE(int4, 16);

	typedef vec_t<int, 1> fixed1; // sizeof(8)
	typedef vec_t<int, 2> fixed2; // sizeof(8)
	typedef vec_t<int, 3> fixed3; // sizeof(12)
	typedef vec_t<int, 4> fixed4; // sizeof(16)

	ENSURE_SIZE(fixed1, 4);
	ENSURE_SIZE(fixed2, 8);
	ENSURE_SIZE(fixed3, 12);
	ENSURE_SIZE(fixed4, 16);

	typedef vec_t<double, 1> double1; // sizeof(4)
	typedef vec_t<double, 2> double2; // sizeof(8)
	typedef vec_t<double, 3> double3; // sizeof(12)
	typedef vec_t<double, 4> double4; // sizeof(16)

	ENSURE_SIZE(double1, 8);
	ENSURE_SIZE(double2, 16);
	ENSURE_SIZE(double3, 24);
	ENSURE_SIZE(double4, 32);

	static glm::vec3 glm_cast(const float3 &xyz)
	{
		return glm::vec3(xyz[0], xyz[1], xyz[2]);
	}

	template < size_t N >
	static String to_string(const prism::vec_t<float, N> &vec)
	{
		String result;
		for (int i = 0; i < N; ++i)
		{
			result += String(i == 0 ? "" : "  ") + fmt::sprintf(FLT_FT, flh(vec[i]));
		}
		return result;
	}

	template < size_t N >
	static String to_string(const int (&vec)[N])
	{
		String result;
		for (int i = 0; i < N; ++i)
		{
			result += String(i == 0 ? "" : " ") + fmt::sprintf("%i", vec[i]).c_str();
		}
		return result;
	}

	template < size_t N >
	vec_t<float, N> vec_s2lin(const vec_t<float, N> &vec)
	{
		vec_t<float, N> result;
		for (size_t i = 0; i < N; ++i)
		{
			result[i] = s2lin(vec[i]);
		}
		return result;
	}

	template < size_t N >
	vec_t<float, N> vec_lin2s(const vec_t<float, N> &vec)
	{
		vec_t<float, N> result;
		for (size_t i = 0; i < N; ++i)
		{
			result[i] = lin2s(vec[i]);
		}
		return result;
	}
} // namespace prism

typedef prism::float1 Float1;
typedef prism::float2 Float2;
typedef prism::float3 Float3;
typedef prism::float4 Float4;

typedef prism::int1 Int1;
typedef prism::int2 Int2;
typedef prism::int3 Int3;
typedef prism::int4 Int4;

typedef prism::double1 Double1;
typedef prism::double2 Double2;
typedef prism::double3 Double3;
typedef prism::double4 Double4;

#pragma pack(pop)

/* eof */
