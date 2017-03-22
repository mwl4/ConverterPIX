/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
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
	struct pmc_header_t // sizeof(40)
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

		static const u32 SUPPORTED_VERSION = 0x6;
	};	ENSURE_SIZE(pmc_header_t, 40);

	struct pmc_variant_t // sizeof(8)
	{
		token_t m_name;					// +0
	};	ENSURE_SIZE(pmc_variant_t, 8);

	struct pmc_variant_def_t // sizeof(4)
	{
		u32 m_offset;					// +0
	};	ENSURE_SIZE(pmc_variant_def_t, 4);

	struct pmc_piece_t // sizeof(16)
	{
		u32 m_edges;					// +0
		u32 m_verts;					// +4
		u32 m_vert_offset;				// +8
		u32 m_face_offset;				// +12
	};	ENSURE_SIZE(pmc_piece_t, 16);

	struct pmc_locator_t // sizeof(36)
	{
		s32 m_type;						// +0 ; 1 = box, 2 = ..., 4 = ..., 8 = convex
		s32 m_data_size;				// +4
		u32 m_alias;					// +8
		float m_weight;					// +12
		token_t m_name;					// +16
		float3 m_position;				// +24
	};	ENSURE_SIZE(pmc_locator_t, 36);

	struct pmc_locator_rotated_t : pmc_locator_t // sizeof(52)
	{
		quat_t m_rotation;				// +36
	};	ENSURE_SIZE(pmc_locator_rotated_t, 52);

	struct pmc_locator_convex_t : pmc_locator_rotated_t // sizeof(56)
	{
		u32 m_convex_piece;				// +52
	};	ENSURE_SIZE(pmc_locator_convex_t, 56);

	struct pmc_locator_cylinder_t : pmc_locator_rotated_t // sizeof(60)
	{
		float m_radius;					// +52
		float m_depth;					// +56
	};	ENSURE_SIZE(pmc_locator_cylinder_t, 60);

	struct pmc_locator_box_t : pmc_locator_rotated_t // sizeof(64)
	{
		float3 m_scale;					// +52
	};	ENSURE_SIZE(pmc_locator_box_t, 64);

	struct pmc_locator_sphere_t : pmc_locator_t // sizeof(40)
	{
		float m_radius;					// +36
	};	ENSURE_SIZE(pmc_locator_sphere_t, 40);

	struct pmc_triangle_t // sizeof(6)
	{
		u16 a[3];									// +0
	};	ENSURE_SIZE(pmc_triangle_t, 6);
} // namespace prism

#pragma pack(pop)

/* eof */
