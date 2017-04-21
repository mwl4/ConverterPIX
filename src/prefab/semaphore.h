/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : semaphore.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <math/vector.h>
#include <math/quaternion.h>

class Semaphore
{
	friend Prefab;
private:
	Float3 m_position;
	Quaternion m_rotation;
	u32 m_type;
	u32 m_semaphoreId;
	Float4 m_intervals;
	float m_cycle;
	String m_profile;
};

/* eof */
