/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/utils/string_tokenizer.h
 *
 *		  _____                          _            _____ _______   __
 *		 / ____|                        | |          |  __ \_   _\ \ / /
 *		| |     ___  _ ____   _____ _ __| |_ ___ _ __| |__) || |  \ V /
 *		| |    / _ \| '_ \ \ / / _ \ '__| __/ _ \ '__|  ___/ | |   > <
 *		| |___| (_) | | | \ V /  __/ |  | ||  __/ |  | |    _| |_ / . \
 *		 \_____\___/|_| |_|\_/ \___|_|   \__\___|_|  |_|   |_____/_/ \_\
 *
 *
 *  Copyright (C) 2017 Michal Wojtowicz.
 *  All rights reserved.
 *
 *   This software is ditributed WITHOUT ANY WARRANTY; without even
 *   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE. See the copyright file for more information.
 *
 *****************************************************************************/

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
