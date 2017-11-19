/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/model/bone.h
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
#include <math/matrix.h>
#include <math/quaternion.h>

class Bone
{
private:
	int32_t m_index = 0;
	String m_name;
	Float4x4 m_transformation;
	Float4x4 m_transReversed;
	Quaternion m_stretch;
	Quaternion m_rotation;
	Float3 m_translation;
	Float3 m_scale;
	float m_signOfDeterminantOfMatrix = 1.f;
	int32_t m_parent = -1;

	friend Model;
	friend Animation;
};

/* eof */
