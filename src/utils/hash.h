/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : hash.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

class Hash
{
private:
	uint32_t m_value;
public:
	Hash(const char *const data, uint32_t seed = 0)
	{
		m_value = seed;
		for (const char *s = data; *s; m_value = m_value * 101 + *s++);
	}
	Hash(const String &s, uint32_t seed = 0)
		: Hash(s.c_str(), seed)
	{
	}
};

/* eof */
