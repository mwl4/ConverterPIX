/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : locator.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <math/vector.h>
#include <math/quaternion.h>

class Locator
{
	friend Model;
private:
	String m_name;
	String m_hookup;
	uint32_t m_index;
	Float3 m_position;
	Quaternion m_rotation;
	Float3 m_scale;
};

/* eof */
