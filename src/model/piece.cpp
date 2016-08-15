/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : piece.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include "piece.h"

void AnimBind::setBoneCount(size_t bones)
{
	m_binds.resize(bones);
}

int8_t &AnimBind::bone(size_t idx) 
{
	assert(idx >= 0 && idx < m_binds.size());
	return m_binds[idx].m_bone; 
}

uint8_t &AnimBind::weight(size_t idx) 
{ 
	assert(idx >= 0 && idx < m_binds.size());
	return m_binds[idx].m_weight; 
}

const int8_t &AnimBind::bone(size_t idx) const
{
	assert(idx >= 0 && idx < m_binds.size());
	return m_binds[idx].m_bone;
}

const uint8_t &AnimBind::weight(size_t idx) const
{
	assert(idx >= 0 && idx < m_binds.size());
	return m_binds[idx].m_weight;
}

std::vector<uint32_t> Piece::texCoords(uint32_t uvChannel) const
{
	std::vector<uint32_t> result;
	for (uint32_t i = 0; i < 8; ++i)
	{
		if (((m_maskUV >> (i * 4)) & 0xF) == uvChannel)
		{
			result.push_back(i);
		}
	}
	return result;
}

/* eof */
