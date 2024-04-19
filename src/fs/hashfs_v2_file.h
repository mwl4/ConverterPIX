/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/fs/hashfs_v2_file.h
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

#include "file.h"

#include <structs/hashfs_0x02.h>

#include "GDeflate.h"

class HashFsV2;

class HashFsV2File : public File
{
public:
	HashFsV2File( const String &filepath, HashFsV2 *filesystem, const prism::hashfs_v2_entry_t *entry, const prism::fs_meta_plain_t &plainMetaValues );
	HashFsV2File( const HashFsV2File & ) = delete;
	HashFsV2File( HashFsV2File && ) = delete;
	virtual ~HashFsV2File();

	HashFsV2File &operator=(const HashFsV2File &) = delete;
	HashFsV2File &operator=(HashFsV2File &&) = delete;

	virtual uint64_t write(const void *buffer, uint64_t elementSize, uint64_t elementCount) override;
	virtual uint64_t read(void *buffer, uint64_t elementSize, uint64_t elementCount) override;
	virtual uint64_t size() override;
	virtual bool seek(uint64_t offset, Attrib attr) override;
	virtual void rewind() override;
	virtual uint64_t tell() const override;
	virtual void flush() override;
	virtual void mstat( MetaStat *result ) override;

private:
	String			m_filepath;
	HashFsV2 *		m_filesystem;

	const prism::hashfs_v2_entry_t *m_entry;
	prism::fs_compression_t m_compression = prism::fs_compression_t::nocompress;

	uint64_t		m_position = 0;

	uint64_t m_compressedSize = 0;
	uint64_t m_size = 0;
	uint64_t m_deviceOffset = 0;

	z_stream *m_zlibStream = nullptr;

private:
	void zlibInflateInitialize();
	void zlibInflateDestroy();

	inline void gdeflateDecompressorInitialize() {}
	inline void gdeflateDecompressorDestroy() {}

	//friend class HashFsV2;
};

/* eof */
