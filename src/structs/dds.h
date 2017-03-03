/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : dds.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>

#pragma pack(push, 1)

namespace dds
{
	enum pixel_flags
	{
		PF_ALPHAPIXELS					= 0x1,
		PF_ALPHA						= 0x2,
		PF_FOUR_CC						= 0x4,
		PF_RGB							= 0x40,
		PF_YUV							= 0x200,
		PF_LUMINANCE					= 0x20000,

		PF_RGBA							= PF_ALPHAPIXELS | PF_RGB
	};

	struct pixel_format
	{
		u32 m_size;						// +0
		u32 m_flags;					// +4
		u32 m_four_cc;					// +8
		u32 m_rgb_bit_count;			// +12
		u32 m_r_bit_mask;				// +16
		u32 m_g_bit_mask;				// +20
		u32 m_b_bit_mask;				// +24
		u32 m_a_bit_mask;				// +28
	};	ENSURE_SIZE(pixel_format, 32);

	enum header_flags
	{
		HF_CAPS							= 0x1,
		HF_HEIGHT						= 0x2,
		HF_WIDTH						= 0x4,
		HF_PITCH						= 0x8,
		HF_PIXELFORMAT					= 0x1000,
		HF_MIPMAPCOUNT					= 0x20000,
		HF_LINEARSIZE					= 0x80000,
		HF_DEPTH						= 0x800000
	};

	struct header
	{
		u32 m_size;						// +0
		u32 m_flags;					// +4
		u32 m_height;					// +8
		u32 m_width;					// +12
		u32 m_pitch_or_linear_size;		// +16
		u32 m_depth;					// +20
		u32 m_mip_map_count;			// +24
		u32 m_reserved[11];				// +28
		pixel_format m_pixel_format;	// +72
		u32 m_caps;						// +104
		u32 m_caps2;					// +108
		u32 m_caps3;					// +112
		u32 m_caps4;					// +116
		u32 m_reserved2;				// +120
	};	ENSURE_SIZE(header, 124);

	constexpr inline u32 s2u32(const char(&s)[4 + 1])
	{
		return ((u32)(u8)(s[0])) | ((u32)(u8)(s[1]) << 8) | ((u32)(u8)(s[2]) << 16) | ((u32)(u8)(s[3]) << 24);
	}

	inline std::string uint2s(const u32 value)
	{
		return std::string((const char *)(&value), 4);
	}

	constexpr u32 MAGIC = s2u32("DDS ");

	enum compression_format
	{
		COMPRESS_NONE = 0,
		COMPRESS_DXT1 = s2u32("DXT1"), // RGB, 4bpp, no alpha | ARGB, 4bpp, 1 bit alpha
		COMPRESS_DXT2 = s2u32("DXT2"), // n/d
		COMPRESS_DXT3 = s2u32("DXT3"), // ARGB, 8bpp, explicit alpha
		COMPRESS_DXT4 = s2u32("DXT4"), // n/d
		COMPRESS_DXT5 = s2u32("DXT5"), // ARGB, 8bpp, interpolated alpha
		COMPRESS_ATI2 = s2u32("ATI2"), // n/d
	};

	struct named_pixel_format
	{
		const char *m_name;
		pixel_format m_data;
	};

	const named_pixel_format FORMAT_A8R8G8B8 =
		{ "A8R8G8B8", { sizeof(pixel_format), PF_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 } };

	const named_pixel_format FORMAT_A1R5G5B5 =
		{ "A1R5G5B5", { sizeof(pixel_format), PF_RGBA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 } };

	const named_pixel_format FORMAT_A4R4G4B4 =
		{ "A4R4G4B4", { sizeof(pixel_format), PF_RGBA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 } };

	const named_pixel_format FORMAT_R8G8B8 =
		{ "R8G8B8", { sizeof(pixel_format), PF_RGB, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 } };

	const named_pixel_format FORMAT_R5G6B5 =
		{ "R5G6B5", { sizeof(pixel_format), PF_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 } };

	const named_pixel_format FORMAT_R16G16 =
		{ "R16G16", { sizeof(pixel_format), PF_RGB, 0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 } };

	const std::vector<named_pixel_format> formats = {
		FORMAT_A8R8G8B8, FORMAT_A1R5G5B5, FORMAT_A4R4G4B4, FORMAT_R8G8B8, FORMAT_R5G6B5, FORMAT_R16G16
	};

	inline bool operator==(const pixel_format &lhs, const named_pixel_format &rhs)
	{
		return lhs.m_flags == rhs.m_data.m_flags
			&& lhs.m_rgb_bit_count == rhs.m_data.m_rgb_bit_count
			&& lhs.m_r_bit_mask == rhs.m_data.m_r_bit_mask
			&& lhs.m_g_bit_mask == rhs.m_data.m_g_bit_mask
			&& lhs.m_b_bit_mask == rhs.m_data.m_b_bit_mask
			&& lhs.m_a_bit_mask == rhs.m_data.m_a_bit_mask;
	}

	inline const named_pixel_format *recognize_pixel_format(const pixel_format *pf)
	{
		for (const auto &f : formats)
		{
			if ((*pf) == f)
			{
				return &f;
			}
		}
		return nullptr;
	}

	void print_debug(std::string filepath);
} // namespace prism

#pragma pack(pop)

/* eof */
