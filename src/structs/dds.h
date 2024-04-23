/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/structs/dds.h
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

#pragma once
#pragma pack(push, 1)

namespace dds
{
	constexpr inline u32 s2u32( const char( &s )[ 4 + 1 ] )
	{
		return ( ( u32 )( u8 )( s[ 0 ] ) ) | ( ( u32 )( u8 )( s[ 1 ] ) << 8 ) | ( ( u32 )( u8 )( s[ 2 ] ) << 16 ) | ( ( u32 )( u8 )( s[ 3 ] ) << 24 );
	}

	inline String uint2s( const u32 value )
	{
		return String( ( const char * )( &value ), 4 );
	}

	enum class pixel_flags : u32
	{
		alphapixels =					0x1,
		alpha =							0x2,
		four_cc =						0x4,
		rgb =							0x40,
		yuv =							0x200,
		luminance =						0x20000,

		rgba =							alphapixels | rgb
	};

	CP_ENUM_CLASS_BITFIELD( pixel_flags );

	struct pixel_format
	{
		u32 m_size;						// +0
		pixel_flags m_flags;			// +4
		u32 m_four_cc;					// +8
		u32 m_rgb_bit_count;			// +12
		u32 m_r_bit_mask;				// +16
		u32 m_g_bit_mask;				// +20
		u32 m_b_bit_mask;				// +24
		u32 m_a_bit_mask;				// +28
	};
	
	ENSURE_SIZE( pixel_format, 32 );

	inline bool operator==( const pixel_format &a, const pixel_format &b ) { return memcmp( &a, &b, sizeof( pixel_format ) ) == 0; }

	enum class header_flags : u32
	{
		caps =							0x1,
		height =						0x2,
		width =							0x4,
		pitch =							0x8,
		pixelformat =					0x1000,
		mipmapcount =					0x20000,
		linearsize =					0x80000,
		depth =							0x800000
	};

	CP_ENUM_CLASS_BITFIELD( header_flags );

	enum class resource_dimension : u32
	{
		unknown = 0,
		buffer = 1,
		texture1d = 2,
		texture2d = 3,
		texture3d = 4
	};

	enum class dxgi_format : u32
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
		format_sampler_feedback_min_mip_opaque,
		format_sampler_feedback_mip_region_used_opaque,
		format_force_uint =						0xffffffff
	};

	enum class caps : u32
	{
		complex =								0x8,
		mipmap =								0x400000,
		texture =								0x1000
	};

	CP_ENUM_CLASS_BITFIELD( caps );

	enum class caps2 : u32
	{
		cubemap =								0x200,
		cubemap_positivex =						0x400,
		cubemap_negativex =						0x800,
		cubemap_positivey =						0x1000,
		cubemap_negativey =						0x2000,
		cubemap_positivez =						0x4000,
		cubemap_negativez =						0x8000,
		volume =								0x200000,

		cubemap_all =							cubemap_positivex | cubemap_negativex | cubemap_positivey | 
												cubemap_negativey | cubemap_positivez | cubemap_negativez,
	};

	CP_ENUM_CLASS_BITFIELD( caps2 );

	constexpr u32 MAGIC = s2u32( "DDS " );

	struct header
	{
		// notice magic number is not included in this structure, so offset in file is yet +4

		u32 m_size;								// +0
		header_flags m_flags;					// +4
		u32 m_height;							// +8
		u32 m_width;							// +12
		u32 m_pitch_or_linear_size;				// +16
		u32 m_depth;							// +20
		u32 m_mip_map_count;					// +24
		u32 m_reserved[ 11 ];					// +28
		pixel_format m_pixel_format;			// +72
		caps m_caps;							// +104
		caps2 m_caps2;							// +108
		u32 m_caps3;							// +112
		u32 m_caps4;							// +116
		u32 m_reserved2;						// +120
	};
	
	ENSURE_SIZE( header, 124 );
	
	enum class misc : u32
	{
		texturecube =							0x4
	};

	CP_ENUM_CLASS_BITFIELD( misc );

	struct header_dxt10
	{
		dxgi_format m_dxgi_format;					// +0
		resource_dimension m_resource_dimension;	// +4
		misc m_misc_flag;							// +8
		u32 m_array_size;							// +12
		u32 m_misc_flags2;							// +16

	};	ENSURE_SIZE( header_dxt10, 20 );

	constexpr u32 DXT10 = s2u32( "DX10" );

	enum compression_format
	{
		COMPRESS_NONE = 0,
		COMPRESS_DXT1 = s2u32( "DXT1" ), // RGB, 4bpp, no alpha | ARGB, 4bpp, 1 bit alpha
		COMPRESS_DXT2 = s2u32( "DXT2" ), // n/d
		COMPRESS_DXT3 = s2u32( "DXT3" ), // ARGB, 8bpp, explicit alpha
		COMPRESS_DXT4 = s2u32( "DXT4" ), // n/d
		COMPRESS_DXT5 = s2u32( "DXT5" ), // ARGB, 8bpp, interpolated alpha
		COMPRESS_ATI1 = s2u32( "ATI1" ), // n/d
		COMPRESS_ATI2 = s2u32( "ATI2" ), // n/d
	};

	struct named_pixel_format : pixel_format
	{
		const char *m_name;
	};

	static constexpr named_pixel_format PIXEL_FORMAT_B8G8R8A8 =
		{ { sizeof( pixel_format ), pixel_flags::rgba, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 }, "B8G8R8A8" };

	static constexpr named_pixel_format PIXEL_FORMAT_B5G5R5A1 =
		{ { sizeof( pixel_format ), pixel_flags::rgba, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 }, "B5G5R5A1" };

	static constexpr named_pixel_format PIXEL_FORMAT_B4G4R4A4 =
		{ { sizeof( pixel_format ), pixel_flags::rgba, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 }, "B4G4R4A4" };

	static constexpr named_pixel_format PIXEL_FORMAT_B8G8R8 =
		{ { sizeof( pixel_format ), pixel_flags::rgb, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 }, "B8G8R8" };

	static constexpr named_pixel_format PIXEL_FORMAT_B5G6R5 =
		{ { sizeof( pixel_format ), pixel_flags::rgb, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 }, "R5G6B5" };

	static constexpr named_pixel_format PIXEL_FORMAT_R16G16 =
		{ { sizeof( pixel_format ), pixel_flags::rgb, 0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 }, "R16G16" };

	static constexpr named_pixel_format PIXEL_FORMAT_R8 =
		{ { sizeof( pixel_format ), pixel_flags::rgb, 0, 8, 0x000000ff, 0x000000000, 0x00000000, 0x00000000 }, "R8" };

	static constexpr named_pixel_format PIXEL_FORMAT_E5B9G9R9 =
		{ { sizeof( pixel_format ), pixel_flags::rgba, 0, 32, 0xff800000, 0x007fc000, 0x00003fe0, 0x0000001f }, "E5B9G9R9" };

	static constexpr named_pixel_format formats[] =
	{
		PIXEL_FORMAT_B8G8R8A8, PIXEL_FORMAT_B5G5R5A1, PIXEL_FORMAT_B4G4R4A4,
		PIXEL_FORMAT_B8G8R8, PIXEL_FORMAT_B5G6R5, PIXEL_FORMAT_R16G16, PIXEL_FORMAT_R8,
		PIXEL_FORMAT_E5B9G9R9,
	};

	inline const named_pixel_format *recognize_pixel_format( const pixel_format &pf )
	{
		for( const named_pixel_format &f : formats )
		{
			if( pf == f )
			{
				return &f;
			}
		}
		return nullptr;
	}

	const char *stringize_dxgi_format( dxgi_format format );

	void print_debug( const String &filepath );

} // namespace prism

#pragma pack(pop)

/* eof */
