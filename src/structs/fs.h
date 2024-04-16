/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/structs/fs.h
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

#include "tobj.h"
#include "../utils/token.h"

#pragma pack(push, 1)

namespace prism
{

enum class fs_compression_t : u8
{
	nocompress =				0,
	zlib =						1,
	zlib_headerless =			2,
	gdeflate =					3,
	zstd =						4,
};

class fs_meta_img_t
{
public:
	static constexpr c_token_t c_name = tn( "img" );

	u32 m_value[ 2 ];

	u32 get_width() const				// [0] 00000000 00000000 XXXXXXXX XXXXXXXX
	{
		return 1 + ( m_value[ 0 ] & 0xFFFF );
	}

	u32 get_height() const				// [0] XXXXXXXX XXXXXXXX 00000000 00000000
	{
		return 1 + ( m_value[ 0 ] >> 16 );
	}

	u32 get_mipmap_count() const		// [1] 00000000 00000000 00000000 0000XXXX
	{
		return ( m_value[ 1 ] & 0b1111 ) + 1;
	}

	format_t get_format() const			// [1] 00000000 00000000 0000XXXX XXXX0000
	{
		return static_cast<format_t>( m_value[ 1 ] >> 4 );
	}

	bool is_cube() const				// [1] 00000000 00000000 00XX0000 00000000
	{
		return ( m_value[ 1 ] >> 12 ) & 0b11;
	}

    u32 get_count() const				// [1] 00000000 0000XXXX XX000000 00000000
    {
        return ( ( m_value[ 1 ] >> 14 ) & 0b111111 ) + 1;
    }

	u32 get_pitch_alignment() const		// [1] 00000000 XXXX0000 00000000 00000000
	{
		return 1 << ( ( m_value[ 1 ] >> 20 ) & 0b1111 );
	}

	u32 get_image_alignment() const		// [1] 0000XXXX 00000000 00000000 00000000
	{
		return 1 << ( ( m_value[ 1 ] >> 24 ) & 0b1111 );
	}
};

inline tobj_addr_t fs_meta_addr_mode_to_tobj( u32 value )
{
	switch( value )
	{
		case 0: return tobj_addr_t::repeat;
		case 1: return tobj_addr_t::mirror;
		case 2: return tobj_addr_t::clamp_to_edge;
		case 3: return tobj_addr_t::clamp_to_border;
	}
	assert( false );
	return tobj_addr_t::repeat;
}

class fs_meta_sample_t
{
public:
	static constexpr c_token_t c_name = tn( "sample" );

	u32 m_value[ 1 ];

	mag_filter_t get_mag_filter() const	// [0] 00000000 00000000 00000000 0000000X
	{
		return static_cast<mag_filter_t>( m_value[ 0 ] & 0b1 );
	}

	min_filter_t get_min_filter() const	// [0] 00000000 00000000 00000000 000000X0
	{
		return static_cast<min_filter_t>( ( m_value[ 0 ] >> 1 ) & 0b1 );
	}

	mip_filter_t get_mip_filter() const	// [0] 00000000 00000000 00000000 0000XX00
	{
		return static_cast<mip_filter_t>( ( m_value[ 0 ] >> 2 ) & 0b11 );
	}

	tobj_addr_t get_addr_u() const		// [0] 00000000 00000000 00000000 0XXX0000
	{
		return fs_meta_addr_mode_to_tobj( ( m_value[ 0 ] >> 4 ) & 0b111 );
	}

	tobj_addr_t get_addr_v() const		// [0] 00000000 00000000 000000XX X0000000
	{
		return fs_meta_addr_mode_to_tobj( ( m_value[ 0 ] >> 7 ) & 0b111 );
	}

	tobj_addr_t get_addr_w() const		// [0] 00000000 00000000 000XXX00 00000000
	{
		return fs_meta_addr_mode_to_tobj( ( m_value[ 0 ] >> 10 ) & 0b111 );	
	}
};

class fs_meta_plain_t
{
public:
	static constexpr c_token_t c_name = tn( "plain" );

	u32 m_value[ 6 ];

	fs_compression_t get_compression() const	// [0] 00000000 00000000 00000000 0000XXXX
	{
		return static_cast<fs_compression_t>( m_value[ 0 ] & 0b1111 );
	}

	u64 get_compressed_size() const				// [1] XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
	{
		return m_value[ 1 ];
	}

	u64 get_size() const						// [3] XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
	{
		return m_value[ 3 ];
	}

	u64 get_offset() const
	{
		return u64( m_value[ 4 ] ) | ( u64( m_value[ 5 ] ) << 32 );
	}
};

} // namespace prism

#pragma pack(pop)

/* eof */
