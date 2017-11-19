/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/prefab/curve.h
 *
 *		  _____                          _            _____ _______   __
 *		 / ____|                        | |          |  __ \_   _\ \ / /
 *		| |     ___  _ ____   _____ _ __| |_ ___ _ __| |__) || |  \ V /
 *		| |    / _ \| '_ \ \ / / _ \ '__| __/ _ \ '__|  ___/ | |   > <
 *		| |___| (_) | | | \ V /  __/ |  | ||  __/ |  | |    _| |_ / . \
 *		 \_____\___/|_| |_|\_/ \___|_|   \__\___|_|  |_|   |_____/_/ \_\
 *
 *
 *  Copyright (C) 2017 Michal Wojtowicz.
 *  All rights reserved.
 *
 *   This software is ditributed WITHOUT ANY WARRANTY; without even
 *   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE. See the copyright file for more information.
 *
 *****************************************************************************/

#pragma once

#include <math/vector.h>
#include <math/quaternion.h>

class Curve
{
private:
	String m_name;
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
	String m_trafficRule;

	friend Prefab;
};

/* eof */
