/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : trigger_point.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <math/vector.h>

class TriggerPoint
{
	friend Prefab;
private:
	u32 m_id;
	std::string m_action;
	float m_range;
	float m_reset_delay;
	float m_reset_dist;
	u32 m_flags;
	Float3 m_position;
	i32 m_neighbours[2];
};

/* eof */
