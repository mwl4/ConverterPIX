/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : locator.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <math/vector.h>

class Locator
{
	friend Model;
private:
	std::string m_name;
	std::string m_hookup;
	uint32_t m_index;
	Float3 m_position;
	Quaternion m_rotation;
	Float3 m_scale;
};

/* eof */
