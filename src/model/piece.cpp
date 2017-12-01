/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/model/piece.cpp
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

#include <prerequisites.h>

#include "piece.h"

Array<uint32_t> Piece::texCoords(uint32_t uvChannel) const
{
	Array<uint32_t> result;
	for (uint32_t i = 0; i < 8; ++i)
	{
		if (((m_texcoordMask >> (i * 4)) & 0xF) == uvChannel)
		{
			result.push_back(i);
		}
	}
	return result;
}

/* eof */
