/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : node.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <math/vector.h>

class Node
{
	friend Prefab;
private:
	u32 m_terrainPointIdx;
	u32 m_terrainPointCount;
	u32 m_variantIdx;
	u32 m_variantCount;
	Float3 m_position;
	Float3 m_direction;
	i32 m_inputLines[8];
	i32 m_outputLines[8];
};

/* eof */
