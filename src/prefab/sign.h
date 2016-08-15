/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : sign.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <math/vector.h>
#include <math/quaternion.h>

class Sign
{
	friend Prefab;
private:
	std::string m_name;
	Float3 m_position;
	Quaternion m_rotation;
	std::string m_model;
	std::string m_part;
};

/* eof */
