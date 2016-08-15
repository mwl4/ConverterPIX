/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : pmg.h
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
	struct pmg_header // sizeof(116)
	{
		u8 m_version;								// +0
		u8 m_signature[3];							// +1
		i32 m_piece_count;							// +4
		i32 m_part_count;							// +8
		i32 m_bone_count;							// +12
		i32 m_locator_count;						// +16
		float3 m_bb_center;							// +20
		float m_bb_diagonal_size;					// +32
		float3 m_bb_coord_1;						// +36
		float3 m_bb_coord_2;						// +48
		i32 m_bone_offset;							// +60
		i32 m_part_offset;							// +64
		i32 m_locator_offset;						// +68
		i32 m_piece_offset;							// +72
		i32 m_locator_name_offset;					// +76
		i32 m_locators_name_size;					// +80
		i32 m_anims_binds_offset;					// +84
		i32 m_anims_binds_size;						// +88
		i32 m_geometry_offset;						// +92
		i32 m_geometry_size;						// +96
		i32 m_uv_offset;							// +100
		i32 m_uv_size;								// +104
		i32 m_triangle_offset;						// +108
		i32 m_triangles_size;						// +112

		static const u8 SUPPORTED_VERSION = 0x13;
	};	ENSURE_SIZE(pmg_header, 116);

	struct pmg_bone // sizeof(200)
	{
		token_t m_name;								// +0
		mat4 m_transformation;						// +8
		mat4 m_transformation_reversed;				// +72
		quat_t m_stretch;							// +136
		quat_t m_rotation;							// +152
		float3 m_translation;						// +168
		float3 m_scale;								// +180
		float m_sign_of_determinant_of_matrix;		// +192
		i8 m_parent;								// +196
		u8 m_pad[3];								// +197
	};	ENSURE_SIZE(pmg_bone, 200);

	struct pmg_piece // sizeof(104)
	{
		i32 m_triangles;							// +0 edges
		i32 m_verts;								// +4
		u32 m_uv_mask;								// +8
		i32 m_uv_channels;							// +12
		i32 m_bone_count;							// +16
		i32 m_material;								// +20
		float3 m_bb_center;							// +24
		float m_unknown;							// +36
		float3 m_bb_coord1;							// +40
		float3 m_bb_coord2;							// +52
		i32 m_vert_position_offset;					// +64
		i32 m_vert_normal_offset;					// +68
		i32 m_vert_uv_offset;						// +72
		i32 m_vert_rgba_offset;						// +76
		i32 m_vert_rgba2_offset;					// +80
		i32 m_vert_tangent_offset;					// +84
		i32 m_triangle_offset;						// +88
		i32 m_anim_bind_offset;						// +92
		i32 m_anim_bind_bones_offset;				// +96
		i32 m_anim_bind_bones_weight_offset;		// +100
	};	ENSURE_SIZE(pmg_piece, 104);

	struct pmg_part // sizeof(24)
	{
		token_t m_name;								// +0
		i32 m_piece_count;							// +8
		i32 m_pieces_idx;							// +12
		i32 m_locator_count;						// +16
		i32 m_locators_idx;							// +20
	};	ENSURE_SIZE(pmg_part, 24);

	struct pmg_locator // sizeof(44)
	{
		token_t m_name;								// +0
		float3 m_position;							// +8
		float m_scale;								// +20
		quat_t m_rotation;							// +24
		i32 m_name_block_offset;					// +40
	};	ENSURE_SIZE(pmg_locator, 44);

	struct pmg_vert_color // sizeof(4)
	{
		u8 m_r;										// +0
		u8 m_g;										// +1
		u8 m_b;										// +2
		u8 m_a;										// +3
	};	ENSURE_SIZE(pmg_vert_color, 4);

	struct pmg_vert_tangent // sizeof(16)
	{
		float w;									// +0
		float x;									// +4
		float y;									// +8
		float z;									// +12
	};	ENSURE_SIZE(pmg_vert_tangent, 16);

	struct pmg_triangle // sizeof(6)
	{
		u16 a[3];									// +0
	};	ENSURE_SIZE(pmg_triangle, 6);
} // namespace prism

#pragma pack(pop)

/* eof */
