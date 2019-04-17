/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/structs/pmg_0x15.h
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

#include <math/vector.h>
#include <math/matrix.h>
#include <math/quaternion.h>
#include <math/aabox.h>
#include <utils/token.h>

#pragma pack(push, 1)

namespace prism
{
	namespace pmg_0x15
	{
		struct pmg_header_t // sizeof(112)
		{
			u8 m_version;								// +0
			u8 m_signature[3];							// +1
			i32 m_piece_count;							// +4
			i32 m_part_count;							// +8
			i32 m_bone_count;							// +12
			i32 m_weight_width;							// +16
			i32 m_locator_count;						// +20
			token_t m_skeleton_hash;					// +24
			float3 m_bb_center;							// +32
			float m_bb_diagonal_size;					// +44
			aabox_t m_bb;								// +48
			i32 m_skeleton_offset;						// +72
			i32 m_parts_offset;							// +76
			i32 m_locators_offset;						// +80
			i32 m_pieces_offset;						// +84
			i32 m_string_pool_offset;					// +88
			i32 m_string_pool_size;						// +92
			i32 m_vertex_pool_offset;					// +96
			i32 m_vertex_pool_size;						// +100
			i32 m_index_pool_offset;					// +104
			i32 m_index_pool_size;						// +108

			static const u8 SUPPORTED_VERSION = 0x15;
		};	ENSURE_SIZE(pmg_header_t, 112);

		struct pmg_bone_data_t // sizeof(200)
		{
			token_t m_name;								// +0
			mat4 m_transformation;						// +8
			mat4 m_transformation_reversed;				// +72
			quat_t m_stretch;							// +136
			quat_t m_rotation;							// +152
			float3 m_translation;						// +168
			float3 m_scale;								// +180
			float m_sign_of_determinant_of_matrix;		// +192
			u8 m_parent;								// +196
			u8 m_pad[3];								// +197
		};	ENSURE_SIZE(pmg_bone_data_t, 200);

		struct pmg_piece_t // sizeof(100)
		{
			i32 m_edges;								// +0
			i32 m_verts;								// +4
			u32 m_texcoord_mask;						// +8
			i32 m_texcoord_width;						// +12
			i32 m_material;								// +16
			float3 m_bb_center;							// +20
			float m_bb_diagonal_size;					// +32
			aabox_t m_bb;								// +36
			i32 m_skeleton_offset;						// +60
			i32 m_vert_position_offset;					// +64
			i32 m_vert_normal_offset;					// +68
			i32 m_vert_texcoord_offset;					// +72
			i32 m_vert_color_offset;					// +76
			i32 m_vert_color2_offset;					// +80
			i32 m_vert_tangent_offset;					// +84
			i32 m_vert_bone_index_offset;				// +92
			i32 m_vert_bone_weight_offset;				// +96
			i32 m_index_offset;							// +88
		};	ENSURE_SIZE(pmg_piece_t, 100);

		struct pmg_part_t // sizeof(24)
		{
			token_t m_name;								// +0
			i32 m_piece_count;							// +8
			i32 m_pieces_idx;							// +12
			i32 m_locator_count;						// +16
			i32 m_locators_idx;							// +20
		};	ENSURE_SIZE(pmg_part_t, 24);

		struct pmg_locator_t // sizeof(44)
		{
			token_t m_name;								// +0
			float3 m_position;							// +8
			float m_scale;								// +20
			quat_t m_rotation;							// +24
			i32 m_hookup_offset;						// +40
		};	ENSURE_SIZE(pmg_locator_t, 44);

		struct pmg_vert_color_t // sizeof(4)
		{
			u8 r;										// +0
			u8 g;										// +1
			u8 b;										// +2
			u8 a;										// +3
		};	ENSURE_SIZE(pmg_vert_color_t, 4);

		struct pmg_vert_tangent_t // sizeof(16)
		{
			float w;									// +0
			float x;									// +4
			float y;									// +8
			float z;									// +12
		};	ENSURE_SIZE(pmg_vert_tangent_t, 16);

		struct pmg_index_t // sizeof(6)
		{
			u16 a[3];									// +0
		};	ENSURE_SIZE(pmg_index_t, 6);

		//
		// File structure:
		//  __________________________________________________________
		// | offset               | structure                         |
		// |==========================================================|
		// | 0                    | pmg_header_t                      |
		// | m_skeleton_offset    | pmg_bone_data_t[m_bone_count]     |
		// | m_parts_offset       | pmg_part_t[m_part_count]          |
		// | m_locators_offset    | pmg_locator_t[m_locator_count]    |
		// | m_pieces_offset      | pmg_piece_t[m_piece_count]        |
		// | m_string_pool_offset | byte[m_string_pool_size]          |
		// | m_vertex_pool_offset | byte[m_vertex_pool_size]          |
		// | m_index_pool_offset  | byte[m_index_pool_size]           |
		//  ----------------------------------------------------------

		//
		// Vertex attributes:
		//  _________________________________________________________
		// | float3        position                                  |
		// | float3        normal                                    |
		// | [opt] float4  tangent                                   |
		// | u32           vertex color                              |
		// | [opt] u32     secondary vertex color                    |
		// | [opt] float2[m_texcoord_width] texture coordinate       |
		// | [opt] u32     4 x 8 bit bone indexes if(bone_count > 0) |
		// | [opt] u32     4 x 8 bit bone weights if(bone_count > 0) |
		//  ---------------------------------------------------------
	} // namespace pmg_0x15
} // namespace prism

#pragma pack(pop)

/* eof */
