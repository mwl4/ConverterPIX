/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : string_tokenizer.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

class StringTokenizer
{
public:
	StringTokenizer(const String &text, const String &separator);

	bool getNext(String *out);

private:
	String m_text;
	String m_separator;
	size_t m_offset;
};

/* eof */
