/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/fs/hashfilesystem.h
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

#include <structs/hashfs.h>

class HashFileSystem : public FileSystem
{
public:
	HashFileSystem(const String &root);
	HashFileSystem(const HashFileSystem&) = delete;
	HashFileSystem(HashFileSystem &&rhs) = delete;
	virtual ~HashFileSystem();

	HashFileSystem &operator=(HashFileSystem &) = delete;
	HashFileSystem &operator=(HashFileSystem &&rhs) = delete;

	virtual String root() const override;
	virtual String name() const override;
	virtual UniquePtr<File> open(const String &filename, FsOpenMode mode) override;
	virtual bool mkdir(const String &directory) override;
	virtual bool rmdir(const String &directory) override;
	virtual bool exists(const String &filename) override;
	virtual bool dirExists(const String &dirpath) override;
	virtual UniquePtr<List<Entry>> readDir(const String &path, bool absolutePaths, bool recursive) override;
	virtual bool mstat( MetaStat *result, const String &path ) override;

	bool ioRead(void *const buffer, uint64_t bytes, uint64_t offset);

private:
	String m_rootFilename;
	UniquePtr<File> m_root;

	prism::hashfs_header_t m_header;
	Array<prism::hashfs_entry_t> m_entries;

private:
	bool readHashFS();
	prism::hashfs_entry_t *findEntry(const String &path);
};

/* eof */
