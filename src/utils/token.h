/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : token.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once
#pragma pack(push, 1)

namespace prism
{
	namespace token_data
	{
		constexpr u32 g_num_letters = 38;
		constexpr char g_letters[g_num_letters] = {
			'\0', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',
			 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
			 'w', 'x', 'y', 'z', '_'
		};

		constexpr u64 g_ids[256] = {
			/*       00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F */
			/* 00 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			/* 10 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			/* 20 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			/* 30 */  1,  2,  3,  4,  5,  6,  7,  8,  9, 10,  0,  0,  0,  0,  0,  0,
			/* 40 */  0, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
			/* 50 */ 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,  0,  0,  0,  0, 37,
			/* 60 */  0, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
			/* 70 */ 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,  0,  0,  0,  0,  0,
			/* 80 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			/* 90 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			/* A0 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			/* B0 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			/* C0 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			/* D0 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			/* E0 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			/* F0 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };
	}

	token_t string_to_token(const String &s);
	String token_to_string(const prism::token_t &token);

	using c_token_t = u64;

	template < size_t N >
	constexpr c_token_t c_string_to_token(const char(&str)[N], size_t index = 0, u64 multiplier = 1)
	{
		return (str[index] ? (multiplier * token_data::g_ids[str[index]]) + c_string_to_token(str, index + 1, multiplier * 38) : 0);
	}

	template < c_token_t v > struct cexpr_token_t { static constexpr c_token_t value = v; };

#	define tn(s) prism::cexpr_token_t< prism::c_string_to_token(s) >::value

	class token_t
	{
	public:
		c_token_t m_value;
	public:
		token_t()
		{
			set(0ull);
		}
		token_t(c_token_t value)
		{
			set(value);
		}
		token_t(int value)
		{
			set(value);
		}
		token_t(const String &s)
		{
			from_string(s);
		}
		token_t(const token_t &rhs)
		{
			set(rhs);
		}
		inline void set(const token_t &rhs)
		{
			m_value = rhs.m_value;
		}
		inline void set(c_token_t value)
		{
			m_value = value;
		}
		inline c_token_t get() const
		{
			return m_value;
		}
		inline void from_string(const String &s)
		{
			set(string_to_token(s));
		}
		inline String to_string() const
		{
			return token_to_string(*this);
		}
		inline operator String() const
		{
			return to_string();
		}
		inline token_t &operator=(const token_t& rhs)
		{
			set(rhs.m_value);
			return (*this);
		}
		inline token_t &operator=(const String &s)
		{
			set(s);
			return (*this);
		}
		inline token_t &operator=(u64 rhs)
		{
			set(rhs);
			return (*this);
		}
	};	ENSURE_SIZE(token_t, 8);
} // namespace prism

#pragma pack(pop)

/* eof */
