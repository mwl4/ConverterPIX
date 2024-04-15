/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/utils/compression.cpp
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

#include "prerequisites.h"

#include "utils/compression.h"

bool unCompress(void *output, uint64_t outputCapacity, const void *input, uint64_t inputSize)
{
    z_stream stream = {};
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;
    if (inflateInit(&stream) != Z_OK)
    {
        return false;
    }

    stream.avail_in = static_cast<decltype(stream.avail_in)>(inputSize);
    stream.next_in = static_cast<decltype(stream.next_in)>(const_cast<void *>(input));

    stream.avail_out = static_cast<decltype(stream.avail_out)>(outputCapacity);
    stream.next_out = static_cast<decltype(stream.next_out)>(output);

    int ret = inflate(&stream, Z_FINISH);
    assert(ret != Z_STREAM_ERROR);

    inflateEnd(&stream);

    return ret == Z_OK || ret == Z_STREAM_END;
}

/* eof */
