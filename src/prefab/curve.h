/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : curve.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <math/vector.h>
#include <math/quaternion.h>

class Curve
{
	friend Prefab;
private:
	std::string m_name;
	u32 m_flags;
	u32 m_leadsToNodes;

	Float3 m_startPosition;
	Quaternion m_startRotation;

	Float3 m_endPosition;
	Quaternion m_endRotation;

	float m_length;

	i32 m_nextLines[4];
	i32 m_prevLines[4];
	u32 m_nextLinesCount;
	u32 m_prevLinesCount;

	i32 m_semaphoreId;
	std::string m_trafficRule;
};

/* eof */
