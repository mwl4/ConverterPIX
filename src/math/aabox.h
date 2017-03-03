/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : aabox.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <math/vector.h>

#pragma pack(push, 1)

namespace prism
{
	struct aabox_t
	{
		float3 m_start;
		float3 m_end;
	};
} // namespace prism

#pragma pack(pop)

/* eof */
