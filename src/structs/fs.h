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

enum class format_t : u8 // surface format
{
	format_unknown =						0,
	format_r32g32b32a32_typeless =			1,
	format_r32g32b32a32_float =				2,
	format_r32g32b32a32_uint =				3,
	format_r32g32b32a32_sint =				4,
	format_r32g32b32_typeless =				5,
	format_r32g32b32_float =				6,
	format_r32g32b32_uint =					7,
	format_r32g32b32_sint =					8,
	format_r16g16b16a16_typeless =			9,
	format_r16g16b16a16_float =				10,
	format_r16g16b16a16_unorm =				11,
	format_r16g16b16a16_uint =				12,
	format_r16g16b16a16_snorm =				13,
	format_r16g16b16a16_sint =				14,
	format_r32g32_typeless =				15,
	format_r32g32_float =					16,
	format_r32g32_uint =					17,
	format_r32g32_sint =					18,
	format_r32g8x24_typeless =				19,
	format_d32_float_s8x24_uint =			20,
	format_r32_float_x8x24_typeless =		21,
	format_x32_typeless_g8x24_uint =		22,
	format_r10g10b10a2_typeless =			23,
	format_r10g10b10a2_unorm =				24,
	format_r10g10b10a2_uint =				25,
	format_r11g11b10_float =				26,
	format_r8g8b8a8_typeless =				27,
	format_r8g8b8a8_unorm =					28,
	format_r8g8b8a8_unorm_srgb =			29,
	format_r8g8b8a8_uint =					30,
	format_r8g8b8a8_snorm =					31,
	format_r8g8b8a8_sint =					32,
	format_r16g16_typeless =				33,
	format_r16g16_float =					34,
	format_r16g16_unorm =					35,
	format_r16g16_uint =					36,
	format_r16g16_snorm =					37,
	format_r16g16_sint =					38,
	format_r32_typeless =					39,
	format_d32_float =						40,
	format_r32_float =						41,
	format_r32_uint =						42,
	format_r32_sint =						43,
	format_r24g8_typeless =					44,
	format_d24_unorm_s8_uint =				45,
	format_r24_unorm_x8_typeless =			46,
	format_x24_typeless_g8_uint =			47,
	format_r8g8_typeless =					48,
	format_r8g8_unorm =						49,
	format_r8g8_uint =						50,
	format_r8g8_snorm =						51,
	format_r8g8_sint =						52,
	format_r16_typeless =					53,
	format_r16_float =						54,
	format_d16_unorm =						55,
	format_r16_unorm =						56,
	format_r16_uint =						57,
	format_r16_snorm =						58,
	format_r16_sint =						59,
	format_r8_typeless =					60,
	format_r8_unorm =						61,
	format_r8_uint =						62,
	format_r8_snorm =						63,
	format_r8_sint =						64,
	format_a8_unorm =						65,
	format_r1_unorm =						66,
	format_r9g9b9e5_sharedexp =				67,
	format_r8g8_b8g8_unorm =				68,
	format_g8r8_g8b8_unorm =				69,
	format_bc1_typeless =					70,
	format_bc1_unorm =						71,
	format_bc1_unorm_srgb =					72,
	format_bc2_typeless =					73,
	format_bc2_unorm =						74,
	format_bc2_unorm_srgb =					75,
	format_bc3_typeless =					76,
	format_bc3_unorm =						77,
	format_bc3_unorm_srgb =					78,
	format_bc4_typeless =					79,
	format_bc4_unorm =						80,
	format_bc4_snorm =						81,
	format_bc5_typeless =					82,
	format_bc5_unorm =						83,
	format_bc5_snorm =						84,
	format_b5g6r5_unorm =					85,
	format_b5g5r5a1_unorm =					86,
	format_b8g8r8a8_unorm =					87,
	format_b8g8r8x8_unorm =					88,
	format_r10g10b10_xr_bias_a2_unorm =		89,
	format_b8g8r8a8_typeless =				90,
	format_b8g8r8a8_unorm_srgb =			91,
	format_b8g8r8x8_typeless =				92,
	format_b8g8r8x8_unorm_srgb =			93,
	format_bc6h_typeless =					94,
	format_bc6h_uf16 =						95,
	format_bc6h_sf16 =						96,
	format_bc7_typeless =					97,
	format_bc7_unorm =						98,
	format_bc7_unorm_srgb =					99,
	format_ayuv =							100,
	format_y410 =							101,
	format_y416 =							102,
	format_nv12 =							103,
	format_p010 =							104,
	format_p016 =							105,
	format_420_opaque =						106,
	format_yuy2 =							107,
	format_y210 =							108,
	format_y216 =							109,
	format_nv11 =							110,
	format_ai44 =							111,
	format_ia44 =							112,
	format_p8 =								113,
	format_a8p8 =							114,
	format_b4g4r4a4_unorm =					115,
	format_p208 =							130,
	format_v208 =							131,
	format_v408 =							132,

	format_r8g8b8_unorm =					200,
	format_b8g8r8_unorm =					201,
	format_r8g8b8_unorm_srgb =				202,
	format_b8g8r8_unorm_srgb =				203,
	format_r10g10b10a2_snorm =				204,
	format_r10g10b10x2_snorm =				205,
	format_l8_unorm =						206,
	format_l8a8_unorm =						207,
	format_d24_unorm =						208,
	format_b10g10r10a2_unorm =				209,
	format_unknown_210 =					210,
	format_unknown_211 =					211,
	format_pvrtc_2bpp_rgb =					212,
	format_pvrtc_2bpp_rgba =				213,
	format_pvrtc_4bpp_rgb =					214,
	format_pvrtc_4bpp_rgba =				215,
	format_r4g4b4a4_unorm =					216,
	format_r5g6b5_unorm =					217,
	format_b5g5r5x1_unorm =					218,
	format_r8g8b8x8_unorm =					219,
	format_r8g8b8x8_unorm_srgb =			220,
	format_d24_unorm_x8_uint =				221
};

using fs_meta_value_t = u32;

//////////////////////////////////////////////////////////////////////////

constexpr static u32 fs_meta_img_value_count = 2;

using fs_meta_img_value_t = fs_meta_value_t[ fs_meta_img_value_count ];

inline u32 fs_meta_img_get_width( const fs_meta_img_value_t &value )
{
    return 1 + ( value[ 0 ] & 0xFFFF );
}

inline u32 fs_meta_img_get_height( const fs_meta_img_value_t &value )
{
    return 1 + ( value[ 0 ] >> 16 );
}

inline u32 fs_meta_img_get_mipmap_count( const fs_meta_img_value_t &value )
{
    return ( value[ 1 ] & 0xF ) + 1;
}

inline format_t fs_meta_img_get_format( const fs_meta_img_value_t &value )
{
    return static_cast< format_t >( value[ 1 ] >> 4 );
}

inline bool fs_meta_img_is_cube( const fs_meta_img_value_t &value )
{
    return ( value[ 1 ] >> 12 ) & 3;
}

inline u32 fs_meta_img_get_pitch_alignment( const fs_meta_img_value_t &value )
{
    return 1 << ( ( value[ 1 ] >> 20 ) & 0b1111 );
}

inline u32 fs_meta_img_get_image_alignment( const fs_meta_img_value_t &value )
{
    return 1 << ( ( value[ 1 ] >> 24 ) & 0b1111 );
}

inline u32 fs_meta_img_get_count( const fs_meta_img_value_t &value )
{
    return ( ( value[ 1 ] >> 14 ) & 0b111111 ) + 1;
}

//////////////////////////////////////////////////////////////////////////

constexpr static u32 fs_meta_sample_value_count = 1;

using fs_meta_sample_value_t = fs_meta_value_t[ fs_meta_sample_value_count ];

inline mag_filter_t fs_meta_sample_get_mag_filter( const fs_meta_sample_value_t &value )
{
	return static_cast< mag_filter_t >( value[ 0 ] & 0b1 );				// [0] 00000000 00000000 00000000 0000000X
}

inline min_filter_t fs_meta_sample_get_min_filter( const fs_meta_sample_value_t &value )
{
    return static_cast< min_filter_t >( ( value[ 0 ] >> 1 ) & 0b1 );	// [0] 00000000 00000000 00000000 000000X0
}

inline mip_filter_t fs_meta_sample_get_mip_filter( const fs_meta_sample_value_t &value )
{
    return static_cast< mip_filter_t >( ( value[ 0 ] >> 2 ) & 0b11 );	// [0] 00000000 00000000 00000000 0000XX00
}

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

inline tobj_addr_t fs_meta_sample_get_addr_u( const fs_meta_sample_value_t &value )
{
    return fs_meta_addr_mode_to_tobj( ( value[ 0 ] >> 4 ) & 0b111 );	// [0] 00000000 00000000 00000000 0XXX0000
}

inline tobj_addr_t fs_meta_sample_get_addr_v( const fs_meta_sample_value_t &value )
{
    return fs_meta_addr_mode_to_tobj( ( value[ 0 ] >> 7 ) & 0b111 );	// [0] 00000000 00000000 000000XX X0000000
}

inline tobj_addr_t fs_meta_sample_get_addr_w( const fs_meta_sample_value_t &value )
{
    return fs_meta_addr_mode_to_tobj( ( value[ 0 ] >> 10 ) & 0b111 );	// [0] 00000000 00000000 000XXX00 00000000
}

//////////////////////////////////////////////////////////////////////////

constexpr static u32 fs_meta_plain_value_count = 6;

using fs_meta_plain_value_t = fs_meta_value_t[ fs_meta_plain_value_count ];

inline fs_compression_t fs_meta_plain_get_compression( const fs_meta_plain_value_t &value )
{
    return static_cast< fs_compression_t >( value[ 0 ] & 0xF );
}

inline u64 fs_meta_plain_get_compressed_size( const fs_meta_plain_value_t &value )
{
    return value[ 1 ];
}

inline u64 fs_meta_plain_get_size( const fs_meta_plain_value_t &value )
{
    return value[ 3 ];
}

inline u64 fs_meta_plain_get_offset( const fs_meta_plain_value_t &value )
{
    return u64( value[ 4 ] ) | ( u64( value[ 5 ] ) << 32 );
}

//////////////////////////////////////////////////////////////////////////

} // namespace prism

#pragma pack(pop)

/* eof */
