/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/fs/hashfilesystem_v2.h
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

#include "filesystem.h"

#include "structs/hashfs_0x02.h"

class HashFsV2 final : public FileSystem
{
public:
	HashFsV2( const String &root );
	HashFsV2( const HashFsV2 & ) = delete;
	HashFsV2( HashFsV2 &&rhs ) = delete;
	virtual ~HashFsV2();

	HashFsV2 &operator=( HashFsV2 & ) = delete;
	HashFsV2 &operator=( HashFsV2 &&rhs ) = delete;

	virtual String root() const override;
	virtual String name() const override;
	virtual UniquePtr<File> open( const String &filename, FsOpenMode mode, bool *outFileExists = nullptr ) override;
	virtual bool remove( const String &filePath ) override;
	virtual bool mkdir( const String &directory ) override;
	virtual bool rmdir( const String &directory ) override;
	virtual bool exists( const String &filename ) override;
	virtual bool dirExists( const String &dirpath ) override;
	virtual UniquePtr<List<Entry>> readDir( const String &path, bool absolutePaths, bool recursive ) override;
	virtual bool mstat( MetaStat *result, const String &path ) override;

	virtual UniquePtr<File> openForReadingWithPlainMeta( const String &filename, const prism::fs_meta_plain_t &plainMetaValues, bool *outFileExists = nullptr ) override;

	bool ioRead( void *const buffer, uint64_t bytes, uint64_t offset );

	const u32 *findMetadata( const prism::hashfs_v2_entry_t *entry, prism::hashfs_v2_meta_t meta );
	void walkMetadata( const prism::hashfs_v2_entry_t *entry, std::function< void( prism::hashfs_v2_meta_t meta, const uint32_t *metadata ) > f );

	void mstatEntry( MetaStat *result, const prism::hashfs_v2_entry_t *entry );

	static prism::token_t getMetaTokenName( prism::hashfs_v2_meta_t meta );

private:
	bool readHashFS();
	prism::hashfs_v2_entry_t *findEntry( const String &path );

private:
	String m_rootFilename;
	UniquePtr<File> m_root;

	prism::hashfs_v2_header_t m_header;
	Array<prism::hashfs_v2_entry_t> m_entryTable;
	Array<u32> m_metadataTable;
};

/* eof */
