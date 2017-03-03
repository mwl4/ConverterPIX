/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : map_point.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <math/vector.h>
#include <math/quaternion.h>

class MapPoint
{
	friend Prefab;
private:
	u32 m_mapVisualFlags;
	u32 m_mapNavFlags;
	Float3 m_position;
	i32 m_neighbour[6];
	u32 m_neighbourCount;
};

/* eof */
