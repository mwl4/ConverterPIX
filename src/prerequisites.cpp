/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : prerequisites.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include <cityhash/city.h>

float s2lin(float x)
{
	const float a = 0.055f;
	return ((x <= 0.04045f) ? (x * (1.f / 12.92f)) : (pow((x + a) * (1.f / (1.f + a)), 2.4f)));
}

float lin2s(float x)
{
	const float a = 0.055f;
	return ((x <= 0.0031308f) ? (x * 12.92f) : ((1.f + a) * pow(x, 1.f / 2.4f) - a));
}

String removeSpaces(String str)
{
	str.erase(std::remove_if(str.begin(), str.end(), [](char c)->bool { return !!isspace(c); }), str.end());
	return str;
}

String betweenQuotes(String str)
{
	size_t left = str.find('\"');
	if (left != String::npos)
	{
		size_t right = str.find('\"', left + 1);
		if (right != String::npos)
		{
			return str.substr(left + 1, right - left - 1);
		}
	}
	return "ERROR";
}

void remove(String &str, const String &substr)
{
	for (size_t pos = String::npos; (pos = str.find(substr)) != String::npos;)
	{
		str.erase(pos, substr.length());
	}
}

String removeSlashAtEnd(const String &s)
{
	const char lastch = s[s.length() - 1];
	return s.substr(0, s.length() - ((lastch == '\\' || lastch == '/') ? 1 : 0));
}

String removeSlashAtBegin(const String &s)
{
	const char firstch = s[0];
	return s.substr(firstch == '\\' || firstch == '/' ? 1 : 0);
}

String makeSlashAtEnd(const String &s)
{
	const bool noneedslash = (s[s.length() - 1] == '/') || (s[s.length() - 1] == '\\');
	return noneedslash ? s : s + '/';
}

String trimSlashesAtBegin(const String &s)
{
	size_t count = 0;
	for (const char ch : s)
	{
		if (ch == '\\' || ch == '/')
		{
			count++;
		}
		else
		{
			break;
		}
	}
	return s.substr(count);
}

String trimSlashesAtEnd(const String &s)
{
	size_t count = 0;
	for (auto it = s.rbegin(); it != s.rend(); ++it)
	{
		const char ch = (*it);
		if (ch == '\\' || ch == '/')
		{
			count++;
		}
		else
		{
			break;
		}
	}
	return s.substr(0, s.length() - count);
}

void backslashesToSlashes(String &str)
{
	std::replace(str.begin(), str.end(), '\\', '/');
}

bool fl_eq(float a, float b)
{
	return fabs(a - b) < FLT_EPSILON;
}

String valueToQuotedString(const String &value)
{
	return "\"" + value + "\"";
}

String valueToQuotedString(const char *const value)
{
	return valueToQuotedString(String(value));
}

namespace prism
{
	u64 city_hash_64(const char *const data, size_t size)
	{
		return CityHash64(data, size);
	}
} // namespace prism

/* eof */
