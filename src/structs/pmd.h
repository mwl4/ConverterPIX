/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : pmd.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <utils/token.h>

#pragma pack(push, 1)

namespace prism
{
	struct pmd_attrib_link_t
	{
		s32 m_from;							// +0
		s32 m_to;							// +4
	};	ENSURE_SIZE(pmd_attrib_link_t, 8);

	struct pmd_attrib_def_t
	{
		token_t m_name;						// +0
		s32 m_type;							// +8
		s32 m_offset;						// +12
	};	ENSURE_SIZE(pmd_attrib_def_t, 16);

	struct pmd_attrib_value_t
	{
		union
		{
			int m_int_value;
			float m_float_value;
		};
	};	ENSURE_SIZE(pmd_attrib_value_t, 4);

	struct pmd_header_t // sizeof(64)
	{
		u32 m_version;						// +0

		u32 m_material_count;				// +4
		u32 m_look_count;					// +8
		u32 m_piece_count;					// +12
		u32 m_variant_count;				// +16
		u32 m_part_count;					// +20
		u32 m_attribs_count;				// +24

		u32 m_attribs_values_size;			// +28
		u32 m_material_block_size;			// +32

		u32 m_look_offset;					// +36
		u32 m_variant_offset;				// +40
		u32 m_part_attribs_offset;			// +44
		u32 m_attribs_value_offset;			// +48
		u32 m_attribs_offset;				// +52
		u32 m_material_offset;				// +56
		u32 m_material_data_offset;			// +60

		/**
			token_t looks[m_look_count];
			token_t variants[m_variant_count];
			struct pmd_attrib_link attribs_link[m_part_count];
			struct pmd_attrib_def attribs_def[m_attribs_count];
			u8 attribs_value_block[m_variant_count * m_attribs_values_size];
			u32 materials_offset[m_look_count * m_material_count];
			u8 materials_data[m_material_block_size];
		*/

		static const u32 SUPPORTED_VERSION = 0x04;
	};	ENSURE_SIZE(pmd_header_t, 64);
} // namespace prism

#pragma pack(pop)

/* eof */
