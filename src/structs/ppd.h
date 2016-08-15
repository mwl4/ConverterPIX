/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : ppd.h
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
	struct ppd_node_t // sizeof(104)
	{
		u32 m_terrain_point_idx;
		u32 m_terrain_point_count;
		u32 m_variant_idx;
		u32 m_variant_count;
		float3 m_pos;
		float3 m_dir;
		i32 m_input_lines[8];
		i32 m_output_lines[8];
	};	ENSURE_SIZE(ppd_node_t, 104);

	struct ppd_curve_t // sizeof(128)
	{
		token_t m_name;
		u32 m_flags;
		u32 m_leads_to_nodes;
		float3 m_start_pos;
		float3 m_end_pos;
		quat_t m_start_rot;
		quat_t m_end_rot;
		float m_length;
		i32 m_next_lines[4];
		i32 m_prev_lines[4];
		u32 m_count_next;
		u32 m_count_prev;
		i32 m_semaphore_id;
		token_t m_traffic_rule;
	};	ENSURE_SIZE(ppd_curve_t, 128);

	struct ppd_sign_t // sizeof(52)
	{
		token_t m_name;
		float3 m_position;
		quat_t m_rotation;
		token_t m_model;
		token_t m_part;
	};	ENSURE_SIZE(ppd_sign_t, 52);

	struct ppd_semaphore_t // sizeof(68)
	{
		float3 m_position;
		quat_t m_rotation;
		u32 m_type;
		u32 m_semaphore_id;
		float4 m_intervals;
		float m_cycle;
		token_t m_profile;
		u32 m_unknown;
	};	ENSURE_SIZE(ppd_semaphore_t, 68);

	struct ppd_spawn_point_t // sizeof(32)
	{
		float3 m_position;
		quat_t m_rotation;
		u32 m_type;
	};	ENSURE_SIZE(ppd_spawn_point_t, 32);

	struct ppd_map_point_t // sizeof(48)
	{
		u32 m_map_visual_flags;
		u32 m_map_nav_flags;
		float3 m_position;
		i32 m_neighbours[6];
		u32 m_neighbour_count;
	};	ENSURE_SIZE(ppd_map_point_t, 48);

	struct ppd_terrain_point_variant_t
	{
		u32 m_attach0;
		u32 m_attach1;
	};	ENSURE_SIZE(ppd_terrain_point_variant_t, 8);

	struct ppd_trigger_point_t // sizeof(48)
	{
		u32 m_trigger_id;
		token_t m_trigger_action;
		float m_trigger_range;
		float m_trigger_reset_delay;
		float m_trigger_reset_dist;
		u32 m_flags;
		float3 m_position;
		s32 m_neighbours[2];
	};	ENSURE_SIZE(ppd_trigger_point_t, 48);

	struct ppd_intersection_t // sizeof(16)
	{
		u32 m_inter_curve_id;
		float m_inter_position;
		float m_inter_radius;
		u32 m_flags;
	};	ENSURE_SIZE(ppd_intersection_t, 16);

	struct ppd_header_t
	{
		u32 m_version;                      // +0

		u32 m_node_count;                   // +4
		u32 m_nav_curve_count;              // +8
		u32 m_sign_count;                   // +12
		u32 m_semaphore_count;              // +16
		u32 m_spawn_point_count;            // +20
		u32 m_terrain_point_count;          // +24
		u32 m_terrain_point_variant_count;  // +28
		u32 m_map_point_count;              // +32
		u32 m_trigger_point_count;          // +36
		u32 m_intersection_count;           // +40

		u32 m_node_offset;                  // +44
		u32 m_nav_curve_offset;             // +48
		u32 m_sign_offset;                  // +52
		u32 m_semaphore_offset;             // +56
		u32 m_spawn_point_offset;           // +60
		u32 m_terrain_point_pos_offset;     // +64
		u32 m_terrain_point_normal_offset;  // +72
		u32 m_terrain_point_variant_offset; // +68
		u32 m_map_point_offset;             // +76
		u32 m_trigger_point_offset;         // +80
		u32 m_intersection_offset;          // +84

		static const u32 SUPPORTED_VERSION = 0x15;
	};	ENSURE_SIZE(ppd_header_t, 88);
} // namespace prism

#pragma pack(pop)

/* eof */
