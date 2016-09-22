/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : pmc.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <math/vector.h>
#include <math/matrix.h>
#include <math/quaternion.h>
#include <utils/token.h>

#pragma pack(push, 1)

namespace prism
{
	struct pmc_header // sizeof(40)
	{
		u32 m_version;					// +0

		u32 m_look_count;				// +4
		u32 m_variant_count;			// +8
		u32 m_material_count;			// +12
		u32 m_piece_count;				// +16

		u32 m_look_offset;				// +20
		u32 m_variant_offset;			// +24
		u32 m_material_offset;			// +28
		u32 m_piece_offset;				// +32
		u32 m_variant_def_offset;		// +36

		static const u8 SUPPORTED_VERSION = 0x6;
	};	ENSURE_SIZE(pmc_header, 40);

	struct pmc_variant // sizeof(8)
	{
		token_t m_name;					// +0
	};	ENSURE_SIZE(pmc_variant, 8);

	struct pmc_variant_def // sizeof(4)
	{
		u32 m_offset;					// +0
	};	ENSURE_SIZE(pmc_variant_def, 4);

	struct pmc_piece // sizeof(16)
	{
		u32 m_edges;					// +0
		u32 m_verts;					// +4
		u32 m_vert_offset;				// +8
		u32 m_face_offset;				// +12
	};	ENSURE_SIZE(pmc_piece, 16);

	struct pmc_locator // sizeof(36)
	{
		s32 m_unknown;					// +0 ; 8 or 1
		s32 m_data_size;				// +4
		u32 m_alias;					// +8
		float m_weight;					// +12
		token_t m_name;					// +16
		float3 m_position;				// +24
	};	ENSURE_SIZE(pmc_locator, 36);

	struct pmc_locator_rotated : pmc_locator // sizeof(52)
	{
		quat_t m_rotation;				// +36
	};	ENSURE_SIZE(pmc_locator_rotated, 52);

	struct pmc_locator_convex : pmc_locator_rotated // sizeof(56)
	{
		u32 m_convex_piece;				// +52
	};	ENSURE_SIZE(pmc_locator_convex, 56);

	struct pmc_locator_cylinder : pmc_locator_rotated // sizeof(60)
	{
		float m_radius;					// +52
		float m_depth;					// +56
	};	ENSURE_SIZE(pmc_locator_cylinder, 60);

	struct pmc_locator_box : pmc_locator_rotated // sizeof(64)
	{
		float3 m_scale;					// +52
	};	ENSURE_SIZE(pmc_locator_box, 64);

	struct pmc_locator_sphere : pmc_locator // sizeof(40)
	{
		float m_radius;					// +36
	};	ENSURE_SIZE(pmc_locator_sphere, 40);

	struct pmc_triangle // sizeof(6)
	{
		u16 a[3];									// +0
	};	ENSURE_SIZE(pmc_triangle, 6);
} // namespace prism

#pragma pack(pop)

/* eof */
