/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : part.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <structs/pmg.h>
#include <math/vector.h>

class Part
{
public:
	std::string m_name;
	uint32_t m_locatorCount;
	uint32_t m_locatorId; // start index
	uint32_t m_pieceCount;
	uint32_t m_pieceId; // start index
};

/* eof */
