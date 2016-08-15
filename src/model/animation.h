/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : animation.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <math/quaternion.h>

class Animation
{
public:
	class Frame
	{
		friend Animation;
	private:
		Quaternion m_stretch;
		Quaternion m_rotation;
		Float3 m_translation;
		Float3 m_scale;
	};
private:
	std::string m_name;
	float m_totalLength = 0.f;
	std::vector<uint8_t> m_bones;
	std::vector<std::vector<Frame>> m_frames; // @[bone][frame]
	std::vector<float> m_timeframes;
	std::unique_ptr<std::vector<Float3>> m_movement;
	
	std::string m_filePath;
	bool m_loaded = false;
	std::shared_ptr<Model> m_model;
public:
	bool load(std::shared_ptr<Model> model, std::string filePath);
	void saveToPia(std::string exportPath) const;
};

/* eof */
