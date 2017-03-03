/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : tobj.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>

#pragma pack(push, 1)

namespace prism
{
	struct tobj_header_t
	{
		u32 m_version;
		u32 m_unkn0;
		u32 m_unkn1;
		u32 m_unkn2;
		u32 m_unkn3;
		u16 m_unkn4;
		u8 m_bias;
		u8 m_unkn4_0;
		u8 m_type;			// 0x2 - generic, 0x5 - cubic
		u8 m_unkn5;			// 2 or 0
		u8 m_mag_filter;	// { nearest = 0, linear = 1, default = 3 }
		u8 m_min_filter;	// { nearest = 0, linear = 1, default = 3 }
		u8 m_mip_filter;	// { trilinear = 1, nomips = 2, default = 3 }
		u8 m_unkn6;			// always 0?
		u8 m_addr_u;		// { repeat = 0, clamp = 1, clamp_to_edge = 2, clamp_to_border = 3, 
		u8 m_addr_v;		//   mirror = 4, mirror_clamp = 5, mirror_clamp_to_edge = 6
		u8 m_addr_w;		// }
		u8 m_nocompress;
		u8 m_unkn7;
		u8 m_noanisotropic;
		u8 m_unkn9;
		u8 m_unkn10;
		u8 m_custom_color_space;	// color_space srgb(0[default]), tsnormal(1), color_space linear(1)
		u8 m_unkn11;

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
