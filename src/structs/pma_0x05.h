/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/structs/pma_0x05.h
 *
 *		  _____                          _            _____ _______   __
 *		 / ____|                        | |          |  __ \_   _\ \ / /
 *		| |     ___  _ ____   _____ _ __| |_ ___ _ __| |__) || |  \ V /
 *		| |    / _ \| '_ \ \ / / _ \ '__| __/ _ \ '__|  ___/ | |   > <
 *		| |___| (_) | | | \ V /  __/ |  | ||  __/ |  | |    _| |_ / . \
 *		 \_____\___/|_| |_|\_/ \___|_|   \__\___|_|  |_|   |_____/_/ \_\
 *
 *
 *  Copyright (C) 2022 Michal Wojtowicz.
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
#include <utils/token.h>

#pragma pack(push, 1)

namespace prism
{
	namespace pma_0x05
	{
		enum class pma_flags_t : u16
		{
			movement =		( 1 << 1 ),
			rotation =		( 1 << 2 ),
		};

		CP_ENUM_CLASS_BITFIELD( pma_flags_t );

		struct pma_header_t
		{
			u32 m_version;				// +0
			u16 m_frames;				// +4
			pma_flags_t m_flags;		// +6
			u32 m_bones;				// +8
			u64 m_skeleton_hash;		// +12 cityhash64 of skeleton file full path, for example: /vehicle/some_skeleton.pis
			float3 m_bsphere_org;		// +20
			float m_bsphere_rad;		// +32
			float3 m_bbox_min;			// +36
			float3 m_bbox_max;			// +48
			float m_anim_length;		// +60
			i32 m_lengths_offset;		// +64
			i32 m_bones_offset;			// +68
			i32 m_frames_offset;		// +72
			i32 m_delta_trans_offset;	// +76
			i32 m_delta_rot_offset;		// +80

			static const u32 SUPPORTED_VERSION = 0x05;

		};	ENSURE_SIZE( pma_header_t, 84 );

		struct pma_frame_t
		{
			quat_t m_scale_orient;		// +0
			quat_t m_rot;				// +16
			float3 m_trans;				// +32
			float3 m_scale;				// +44

		};	ENSURE_SIZE( pma_frame_t, 56 );

	}; // namespace pma_0x05
} // namespace prism

#pragma pack(pop)

/* eof */
