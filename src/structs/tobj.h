/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/structs/tobj.h
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

namespace prism
{
	enum class tobj_type_t : u8
	{
		generic =					2,
		cubic =						5,
	};

	enum class mag_filter_t : u8
	{
		nearest =					0,
		linear =					1,
		default =					3
	};

	enum class min_filter_t : u8
	{
		nearest =					0,
		linear =					1,
		default =					3
	};

	enum class mip_filter_t : u8
	{
		nearest =					0,
		trilinear =					1,
		nomips =					2,
		default =					3
	};

	enum class tobj_addr_t : u8
	{
		repeat =					0,
		clamp =						1,
		clamp_to_edge =				2,
		clamp_to_border =			3,
		mirror =					4,
		mirror_clamp =				5,
		mirror_clamp_to_edge =		6,
	};

	struct tobj_header_t
	{
		u32 m_version;				// +0
		u32 m_unkn1;				// +8
		u32 m_unkn0;				// +4
		u32 m_unkn2;				// +12
		u32 m_unkn3;				// +16
		u16 m_unkn4;				// +20
		u8 m_bias;					// +22
		u8 m_unkn4_0;				// +23
		tobj_type_t m_type;			// +24 0x2 - generic, 0x5 - cubic
		u8 m_unkn5;					// +25 2 or 0
		mag_filter_t m_mag_filter;	// +26
		min_filter_t m_min_filter;	// +27
		mip_filter_t m_mip_filter;	// +28
		u8 m_unkn6;					// +29 always 0?
		tobj_addr_t m_addr_u;		// +30
		tobj_addr_t m_addr_v;		// +31
		tobj_addr_t m_addr_w;		// +32
		u8 m_nocompress;			// +33
		u8 m_unkn7;					// +34
		u8 m_noanisotropic;			// +35
		u8 m_unkn9;					// +36
		u8 m_unkn10;				// +37
		u8 m_custom_color_space;	// +38 color_space srgb(0[default]), tsnormal(1), color_space linear(1)
		u8 m_unkn11;				// +39

		static const u32 SUPPORTED_MAGIC = 1890650625;
	};	ENSURE_SIZE(tobj_header_t, 40);

	struct tobj_texture_t
	{
		u32 m_length;
		u32 m_unknown;
	};	ENSURE_SIZE(tobj_texture_t, 8);

	enum class format_t : u8 // surface format
	{
		format_unknown = 0,
		format_r32g32b32a32_typeless = 1,
		format_r32g32b32a32_float = 2,
		format_r32g32b32a32_uint = 3,
		format_r32g32b32a32_sint = 4,
		format_r32g32b32_typeless = 5,
		format_r32g32b32_float = 6,
		format_r32g32b32_uint = 7,
		format_r32g32b32_sint = 8,
		format_r16g16b16a16_typeless = 9,
		format_r16g16b16a16_float = 10,
		format_r16g16b16a16_unorm = 11,
		format_r16g16b16a16_uint = 12,
		format_r16g16b16a16_snorm = 13,
		format_r16g16b16a16_sint = 14,
		format_r32g32_typeless = 15,
		format_r32g32_float = 16,
		format_r32g32_uint = 17,
		format_r32g32_sint = 18,
		format_r32g8x24_typeless = 19,
		format_d32_float_s8x24_uint = 20,
		format_r32_float_x8x24_typeless = 21,
		format_x32_typeless_g8x24_uint = 22,
		format_r10g10b10a2_typeless = 23,
		format_r10g10b10a2_unorm = 24,
		format_r10g10b10a2_uint = 25,
		format_r11g11b10_float = 26,
		format_r8g8b8a8_typeless = 27,
		format_r8g8b8a8_unorm = 28,
		format_r8g8b8a8_unorm_srgb = 29,
		format_r8g8b8a8_uint = 30,
		format_r8g8b8a8_snorm = 31,
		format_r8g8b8a8_sint = 32,
		format_r16g16_typeless = 33,
		format_r16g16_float = 34,
		format_r16g16_unorm = 35,
		format_r16g16_uint = 36,
		format_r16g16_snorm = 37,
		format_r16g16_sint = 38,
		format_r32_typeless = 39,
		format_d32_float = 40,
		format_r32_float = 41,
		format_r32_uint = 42,
		format_r32_sint = 43,
		format_r24g8_typeless = 44,
		format_d24_unorm_s8_uint = 45,
		format_r24_unorm_x8_typeless = 46,
		format_x24_typeless_g8_uint = 47,
		format_r8g8_typeless = 48,
		format_r8g8_unorm = 49,
		format_r8g8_uint = 50,
		format_r8g8_snorm = 51,
		format_r8g8_sint = 52,
		format_r16_typeless = 53,
		format_r16_float = 54,
		format_d16_unorm = 55,
		format_r16_unorm = 56,
		format_r16_uint = 57,
		format_r16_snorm = 58,
		format_r16_sint = 59,
		format_r8_typeless = 60,
		format_r8_unorm = 61,
		format_r8_uint = 62,
		format_r8_snorm = 63,
		format_r8_sint = 64,
		format_a8_unorm = 65,
		format_r1_unorm = 66,
		format_r9g9b9e5_sharedexp = 67,
		format_r8g8_b8g8_unorm = 68,
		format_g8r8_g8b8_unorm = 69,
		format_bc1_typeless = 70,
		format_bc1_unorm = 71,
		format_bc1_unorm_srgb = 72,
		format_bc2_typeless = 73,
		format_bc2_unorm = 74,
		format_bc2_unorm_srgb = 75,
		format_bc3_typeless = 76,
		format_bc3_unorm = 77,
		format_bc3_unorm_srgb = 78,
		format_bc4_typeless = 79,
		format_bc4_unorm = 80,
		format_bc4_snorm = 81,
		format_bc5_typeless = 82,
		format_bc5_unorm = 83,
		format_bc5_snorm = 84,
		format_b5g6r5_unorm = 85,
		format_b5g5r5a1_unorm = 86,
		format_b8g8r8a8_unorm = 87,
		format_b8g8r8x8_unorm = 88,
		format_r10g10b10_xr_bias_a2_unorm = 89,
		format_b8g8r8a8_typeless = 90,
		format_b8g8r8a8_unorm_srgb = 91,
		format_b8g8r8x8_typeless = 92,
		format_b8g8r8x8_unorm_srgb = 93,
		format_bc6h_typeless = 94,
		format_bc6h_uf16 = 95,
		format_bc6h_sf16 = 96,
		format_bc7_typeless = 97,
		format_bc7_unorm = 98,
		format_bc7_unorm_srgb = 99,
		format_ayuv = 100,
		format_y410 = 101,
		format_y416 = 102,
		format_nv12 = 103,
		format_p010 = 104,
		format_p016 = 105,
		format_420_opaque = 106,
		format_yuy2 = 107,
		format_y210 = 108,
		format_y216 = 109,
		format_nv11 = 110,
		format_ai44 = 111,
		format_ia44 = 112,
		format_p8 = 113,
		format_a8p8 = 114,
		format_b4g4r4a4_unorm = 115,
		format_p208 = 130,
		format_v208 = 131,
		format_v408 = 132,

		format_r8g8b8_unorm = 200,
		format_b8g8r8_unorm = 201,
		format_r8g8b8_unorm_srgb = 202,
		format_b8g8r8_unorm_srgb = 203,
		format_r10g10b10a2_snorm = 204,
		format_r10g10b10x2_snorm = 205,
		format_l8_unorm = 206,
		format_l8a8_unorm = 207,
		format_d24_unorm = 208,
		format_b10g10r10a2_unorm = 209,
		format_unknown_210 = 210,
		format_unknown_211 = 211,
		format_pvrtc_2bpp_rgb = 212,
		format_pvrtc_2bpp_rgba = 213,
		format_pvrtc_4bpp_rgb = 214,
		format_pvrtc_4bpp_rgba = 215,
		format_r4g4b4a4_unorm = 216,
		format_r5g6b5_unorm = 217,
		format_b5g5r5x1_unorm = 218,
		format_r8g8b8x8_unorm = 219,
		format_r8g8b8x8_unorm_srgb = 220,
		format_d24_unorm_x8_uint = 221
	};

	/*
		default:
		 - tga (32 bits) -> DXT5
		 - tga (24 bits) -> DXT1

		usage tsnormal:
		 - tga -> ATI2
		 - png (nocompress) -> R16G16    # png is required

		nomips:
		 - tga -> DXT5/DXT1

		color_space linear:
		 - tga -> DXT5/DXT1

		nocompress:
		 - tga (32 bits) -> A8R8G8B8
		 - tga (24 bits) -> R8G8B8

		usage ui = nomips + color_space linear + nocompress

		usage ui    # (nomips + color_space linear + nocompress)
		 - tga (32 bits) -> A8R8G8B8
		 - tga (24 bits) -> R8G8B8
	*/
} // namespace prism

#pragma pack(pop)

/* eof */
