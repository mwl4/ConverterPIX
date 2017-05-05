 /******************************************************************************
 *
 *  Project:	mbd_reader @ core
 *  File:		/structs/hashfs.hxx
 *
 *  Copyright (C) 2013 - 2017, TruckersMP Team.
 *  All rights reserved.
 *
 *   This software is ditributed WITHOUT ANY WARRANTY; without even
 *   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE. See the copyright file for more information.
 *
 *****************************************************************************/

#pragma once
#pragma pack(push, 1)

namespace prism
{
	struct hashfs_header_t
	{
		u32 m_magic;			// +0
		u16 m_version;			// +4
		u16 m_salt;				// +6
		u32 m_hash_method;		// +8
		u32 m_entries_count;	// +12
		u32 m_start_offset;		// +16
		pad(12);				// +20
		// +32 --
		static constexpr u32 SUPPORTED_VERSION = 0x01;
	};	ENSURE_SIZE(hashfs_header_t, 32);

	enum hashfs_entry_flags_t : u32
	{
		HASHFS_DIR			= (1 << 0),
		HASHFS_COMPRESSED	= (1 << 1),
		HASHFS_VERIFY		= (1 << 2),
		HASHFS_ENCRYPTED	= (1 << 3)
	};

	struct hashfs_entry_t
	{
		u64 m_hash;				// +0
		u64 m_offset;			// +8
		u32 m_flags;			// +16
		u32 m_crc;				// +20
		u32 m_size;				// +24
		u32 m_compressed_size;	// +28
		// +32 --
	};	ENSURE_SIZE(hashfs_entry_t, 32);
} // namespace prism

#pragma pack(pop)

/* eof */
