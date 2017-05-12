/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : hashfilesystem.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

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

	bool ioRead(void *const buffer, size_t bytes, size_t offset);

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
