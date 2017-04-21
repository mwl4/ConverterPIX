/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : part.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

class Part
{
public:
	String m_name;
	uint32_t m_locatorCount = 0;
	uint32_t m_locatorId = 0; // start index
	uint32_t m_pieceCount = 0;
	uint32_t m_pieceId = 0; // start index
};

/* eof */
