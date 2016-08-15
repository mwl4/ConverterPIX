/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : bone.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <math/vector.h>
#include <math/matrix.h>
#include <math/quaternion.h>

class Bone
{
	friend Model;
	friend Animation;
private:
	int32_t m_index;
	std::string m_name;
	Float4x4 m_transformation;
	Float4x4 m_transReversed;
	Quaternion m_stretch;
	Quaternion m_rotation;
	Float3 m_translation;
	Float3 m_scale;
	float m_signOfDeterminantOfMatrix;
	int32_t m_parent;
};

/* eof */
