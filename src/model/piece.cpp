/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : piece.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include "piece.h"

std::vector<uint32_t> Piece::texCoords(uint32_t uvChannel) const
{
	std::vector<uint32_t> result;
	for (uint32_t i = 0; i < 8; ++i)
	{
		if (((m_texcoordMask >> (i * 4)) & 0xF) == uvChannel)
		{
			result.push_back(i);
		}
	}
	return result;
}

/* eof */
