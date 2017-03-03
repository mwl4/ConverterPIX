/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : intersection.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>

class Intersection
{
	friend Prefab;
private:
	u32 m_curveId;
	float m_position;
	float m_radius;
	u32 m_flags;
};

/* eof */
