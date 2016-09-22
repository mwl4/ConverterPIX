/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : token.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include "token.h"

namespace prism
{
	struct _ulldiv_t
	{
		u64 quot;
		u64 rem;
		_ulldiv_t() : quot(0ull), rem(0ull) { }
	};

	inline _ulldiv_t _div(u64 num, u64 divider)
	{
		_ulldiv_t result;
		result.rem = num % divider;
		result.quot = num / divider;
		return result;
	}

	std::string token_to_string(const prism::token_t &token)
	{
		std::string resultStr(16, '\0');
		_ulldiv_t result;
		prism::token_t tokenCopy = token;
		for (size_t i = 0; tokenCopy.get() != 0; ++i)
		{
			result = _div(tokenCopy.get(), token_data::g_num_letters);
			tokenCopy = result.quot;
			resultStr[i] = token_data::g_letters[result.rem];
		}
		return resultStr.c_str();
	}

	u64 _string_to_token(const char *str, size_t index = 0, u64 multiplier = 1)
	{
		return (str[index] ? (multiplier * token_data::g_ids[str[index]]) + _string_to_token(str, index + 1, multiplier * 38) : 0);
	}

	token_t string_to_token(const std::string &s)
	{
		return _string_to_token(s.c_str());
	}
} // namespace prism

/* eof */
