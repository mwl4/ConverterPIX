/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/fs/hashfs_file.h
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

#include "file.h"

#include <structs/hashfs.h>

class HashFsFile : public File
{
public:
	HashFsFile(const String &filepath, HashFileSystem *filesystem, const prism::hashfs_entry_t *header);
	HashFsFile(const HashFsFile &) = delete;
	HashFsFile(HashFsFile &&) = delete;
	virtual ~HashFsFile();

	HashFsFile &operator=(const HashFsFile &) = delete;
	HashFsFile &operator=(HashFsFile &&) = delete;

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
	HashFileSystem *m_filesystem;
	z_stream		m_stream;
	uint64_t		m_position;

	const prism::hashfs_entry_t *m_header;

private:
	void inflateInitialize();
	void inflateDestroy();

	friend class HashFileSystem;
};

/* eof */
