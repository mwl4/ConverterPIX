/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : animation.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <math/quaternion.h>
#include <math/vector.h>

class Animation
{
public:
	class Frame
	{
		friend Animation;
	private:
		Quaternion m_scaleOrientation;
		Quaternion m_rotation;
		Float3 m_translation;
		Float3 m_scale;
	};
private:
	String m_name;
	float m_totalLength = 0.f;
	Array<uint8_t> m_bones;
	Array<Array<Frame>> m_frames; // @[bone][frame]
	Array<float> m_timeframes;
	UniquePtr<Array<Float3>> m_movement;
	
	String m_filePath;
	bool m_loaded = false;
	SharedPtr<Model> m_model;
public:
	bool load(SharedPtr<Model> model, String filePath);
	void saveToPia(String exportPath) const;
};

/* eof */
