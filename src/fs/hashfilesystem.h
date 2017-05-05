 /******************************************************************************
 *
 *  Project:	mbd_reader @ core
 *  File:		/fs/hashfilesystem.hxx
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
	virtual UniquePtr<File> open(const String &filename, FsOpenMode mode) override;
	virtual bool mkdir(const String &directory) override;
	virtual bool rmdir(const String &directory) override;
	virtual bool exists(const String &filename) override;
	virtual bool dirExists(const String &dirpath) override;
	virtual UniquePtr<List<Entry>> readDir(const String &path, bool absolutePaths, bool recursive) override;

	bool ioRead(void *const buffer, size_t bytes, size_t offset);

private:
	String m_rootFilename;
	UniquePtr<File> m_root;

	prism::hashfs_header_t m_header;
	Array<prism::hashfs_entry_t> m_entries;

private:
	bool readHashFS();
	prism::hashfs_entry_t *findEntry(String path);
};

/* eof */
