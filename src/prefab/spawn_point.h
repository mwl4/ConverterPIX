/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : spawn_point.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <math/vector.h>
#include <math/quaternion.h>

class SpawnPoint
{
	friend Prefab;
private:
	Float3 m_position;
	Quaternion m_rotation;
	u32 m_type;
};

/* eof */
