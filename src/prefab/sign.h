/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : sign.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <math/vector.h>
#include <math/quaternion.h>

class Sign
{
	friend Prefab;
private:
	String m_name;
	Float3 m_position;
	Quaternion m_rotation;
	String m_model;
	String m_part;
};

/* eof */
