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
				result += m_a[i] * m_a[i];

			return sqrtf(result);
		}
		vec_t<T, N> &operator=(const T &value)
		{
			for (size_t i = 0; i < N; ++i) {
				m_a[i] = value;
			}
			return (*this);
		}
		inline size_t count() const
		{
			return N;
		}
	};	ENSURE_SIZE(vec_t<float COMMA 3>, 12);

	typedef vec_t<float, 2> float2; // sizeof(8)
	typedef vec_t<float, 3> float3; // sizeof(12)
	typedef vec_t<float, 4> float4; // sizeof(16)

	typedef vec_t<int, 2> fixed2; // sizeof(8)
	typedef vec_t<int, 3> fixed3; // sizeof(12)
	typedef vec_t<int, 4> fixed4; // sizeof(16)

	static float3 make_float3(float x, float y, float z)
	{
		float3 result;
		result[0] = x;
		result[1] = y;
		result[2] = z;
		return result;
	}

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

typedef prism::float2 Float2;
typedef prism::float3 Float3;
typedef prism::float4 Float4;

#pragma pack(pop)

/* eof */
