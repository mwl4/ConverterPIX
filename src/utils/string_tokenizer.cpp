/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : string_tokenizer.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "string_tokenizer.h"

StringTokenizer::StringTokenizer(const String &text, const String &separator)
	: m_text(text)
	, m_separator(separator)
	, m_offset(0)
{
}

bool StringTokenizer::getNext(String *out)
{
	size_t current = m_offset;
	if (current != String::npos)
	{
		m_offset = m_text.find(m_separator, current);
		*out = m_text.substr(current, m_offset - current);
		if (m_offset != String::npos)
		{
			m_offset += m_separator.size();
		}
		return true;
	}
	return false;
}

/* eof */
