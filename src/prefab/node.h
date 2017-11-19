/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/prefab/node.h
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

class Node
{
private:
	u32 m_terrainPointIdx;
	u32 m_terrainPointCount;
	u32 m_variantIdx;
	u32 m_variantCount;
	Float3 m_position;
	Float3 m_direction;
	i32 m_inputLines[8];
	i32 m_outputLines[8];

	friend Prefab;
};

/* eof */
