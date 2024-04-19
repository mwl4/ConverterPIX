/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/utils/format_utils.h
 *
 *		  _____                          _            _____ _______   __
 *		 / ____|                        | |          |  __ \_   _\ \ / /
 *		| |     ___  _ ____   _____ _ __| |_ ___ _ __| |__) || |  \ V /
 *		| |    / _ \| '_ \ \ / / _ \ '__| __/ _ \ '__|  ___/ | |   > <
 *		| |___| (_) | | | \ V /  __/ |  | ||  __/ |  | |    _| |_ / . \
 *		 \_____\___/|_| |_|\_/ \___|_|   \__\___|_|  |_|   |_____/_/ \_\
 *
 *
 *  Copyright (C) 2024 Michal Wojtowicz.
 *  All rights reserved.
 *
 *   This software is ditributed WITHOUT ANY WARRANTY; without even
 *   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE. See the copyright file for more information.
 *
 *****************************************************************************/

#pragma once

namespace prism { enum class format_t : u8; }

u32 bitsPerPixel( prism::format_t fmt );

void getSurfaceInfo( size_t width, size_t height, prism::format_t fmt, size_t *outNumBytes, size_t *outRowBytes, size_t *outNumRows );

class SubresourceData
{
public:
    const void *m_data;
    uint32_t    m_rowPitch;
    uint32_t    m_slicePitch;
};

bool fillInitData( size_t width,
                   size_t height,
                   size_t depth,
                   size_t mipCount,
                   size_t arraySize,
                   prism::format_t format,
                   size_t maxsize,
                   size_t bitSize,
                   const uint8_t *bitData,
                   size_t &twidth,
                   size_t &theight,
                   size_t &tdepth,
                   size_t &skipMip,
                   /* [mipCount * arraySize] */ SubresourceData *initData );

/* eof */
