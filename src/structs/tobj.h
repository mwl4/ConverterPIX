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
		trilinear =					1,
		nomips =					2,
		default =					3
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
		mag_filter_t m_mag_filter;	// +26 { nearest = 0, linear = 1, default = 3 }
		min_filter_t m_min_filter;	// +27 { nearest = 0, linear = 1, default = 3 }
		mip_filter_t m_mip_filter;	// +28 { trilinear = 1, nomips = 2, default = 3 }
		u8 m_unkn6;					// +29 always 0?
		u8 m_addr_u;				// +30 { repeat = 0, clamp = 1, clamp_to_edge = 2, clamp_to_border = 3,
		u8 m_addr_v;				// +31  mirror = 4, mirror_clamp = 5, mirror_clamp_to_edge = 6
		u8 m_addr_w;				// +32 }
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
