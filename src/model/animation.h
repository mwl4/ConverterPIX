/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/model/animation.h
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

#include <math/quaternion.h>
#include <math/vector.h>

class Animation
{
public:
	class Frame
	{
	private:
		Quaternion m_scaleOrientation;
		Quaternion m_rotation;
		Float3 m_translation;
		Float3 m_scale;

		friend Animation;
	};

public:
	bool load(SharedPtr<Model> model, String filePath);
	bool loadAnim0x03(const uint8_t *const buffer, const size_t size);
	bool loadAnim0x04(const uint8_t *const buffer, const size_t size);
	void saveToPia(String exportPath) const;

private:
	float m_totalLength = 0.f;
	Array<uint8_t> m_bones;
	Array<Array<Frame>> m_frames; // @[bone][frame]
	Array<float> m_timeframes;
	UniquePtr<Array<Float3>> m_movement;
	UniquePtr<Array<Quaternion>> m_rotation;

	String m_filePath;
	bool m_loaded = false;
	SharedPtr<Model> m_model;
};

/* eof */
