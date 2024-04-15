/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/structs/hashfs_0x02.h
 *
 *		  _____                          _            _____ _______   __
 *		 / ____|                        | |          |  __ \_   _\ \ / /
 *		| |     ___  _ ____   _____ _ __| |_ ___ _ __| |__) || |  \ V /
 *		| |    / _ \| '_ \ \ / / _ \ '__| __/ _ \ '__|  ___/ | |   > <
 *		| |___| (_) | | | \ V /  __/ |  | ||  __/ |  | |    _| |_ / . \
 *		 \_____\___/|_| |_|\_/ \___|_|   \__\___|_|  |_|   |_____/_/ \_\
 *
 *
 *  Copyright (C) 2024 Michal Wojtowicz.
 *  All rights reserved.
 *
 *   This software is ditributed WITHOUT ANY WARRANTY; without even
 *   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE. See the copyright file for more information.
 *
 *****************************************************************************/

#pragma once

#include "fs.h"

#pragma pack(push, 1)

namespace prism
{
	enum class hashfs_v2_platform_t : u8
	{
		pc =								0,
		xbone =								1,
		xs =								2,
		ps4 =								3,
		ps5 =								4,
	
	};	ENSURE_SIZE( hashfs_v2_platform_t, 1 );

	struct hashfs_v2_header_t
	{
		u32									m_magic;										// +0
		u16									m_version;										// +4
		u16									m_salt;											// +6
		u32									m_hash_method;									// +8
		u32									m_entry_table_count;							// +12
		u32									m_entry_table_compressed_size;					// +16
		u32									m_metadata_table_count;							// +20
		u32									m_metadata_table_compressed_size;				// +24
		u64									m_entry_table_offset;							// +28
		u64									m_metadata_table_offset;						// +36
		u64									m_security_descriptor_offset;					// +44
		hashfs_v2_platform_t				m_platform;										// +52

		static constexpr u16				SUPPORTED_VERSION = 2;

	};	ENSURE_SIZE( hashfs_v2_header_t, 53 );

	enum class hashfs_v2_entry_flags_t : u8
	{
		directory =							1 << 0,
	};

	CP_ENUM_CLASS_BITFIELD( hashfs_v2_entry_flags_t );

	struct hashfs_v2_entry_t
	{
		u64									m_hash;											// +0
		u32									m_metadata_index;								// +8
		u16									m_metadata_count;								// +12
		hashfs_v2_entry_flags_t				m_flags;										// +14
		u8									m_some_byte;									// +15

	};	ENSURE_SIZE( hashfs_v2_entry_t, 16 );

	enum class hashfs_v2_meta_t : u8
	{
		img =								1,
		sample =							2,
		mipproxy =							3,
		inline_directory =					4,
		plain =								1 << 7,			// 128
		directory =							plain | 1,		// 129
		mip0 =								plain | 2,		// 130
		mip1 =								plain | 3,		// 131
		miptail =							plain | 4,		// 132

	};	ENSURE_SIZE( hashfs_v2_meta_t, 1 );

	CP_ENUM_CLASS_BITFIELD( hashfs_v2_meta_t );

	inline void hashfs_v2_meta_img_get_value( const u32 *metadata, fs_meta_img_value_t &out_value )
	{
		static_assert( CP_ARRAY_SIZE( out_value ) == 2, "Fix code below." );
		out_value[ 0 ] = metadata[ 0 ];
		out_value[ 1 ] = metadata[ 1 ];
	}

	inline void hashfs_v2_meta_sample_get_value( const u32 *metadata, fs_meta_sample_value_t &out_value )
	{
		static_assert( CP_ARRAY_SIZE( out_value ) == 1, "Fix code below." );
		out_value[ 0 ] = metadata[ 0 ];
	}

	inline void hashfs_v2_meta_plain_get_value( const u32 *metadata, fs_meta_plain_value_t &out_value )
	{
		static_assert( CP_ARRAY_SIZE( out_value ) == 6, "Fix code below." );
		out_value[ 0 ] = ( ( metadata[ 0 ] >> 4 ) | ( metadata[ 1 ] & 0xF0FFFFFF ) ) >> 24;
		out_value[ 1 ] = metadata[ 0 ] & 0xFFFFFFF;
		out_value[ 2 ] = metadata[ 2 ] & 0xFFFFFFF;
		out_value[ 3 ] = metadata[ 1 ] & 0xFFFFFFF;
		out_value[ 4 ] = 16 * metadata[ 3 ];
		out_value[ 5 ] = static_cast< u32 >( ( 16ull * static_cast< u64 >( metadata[ 3 ] ) ) >> 32 );
	}
} // namespace prism

#pragma pack(pop)

/* eof */
