/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/fs/zipfilesystem.h
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

#include <structs/zip.h>

class ZipEntry;

class ZipFileSystem : public FileSystem
{
public:
	ZipFileSystem(const String &root);
	virtual ~ZipFileSystem();

	virtual String root() const override;
	virtual String name() const override;
	virtual UniquePtr<File> open(const String &filename, FsOpenMode mode, bool *outFileExists = nullptr ) override;
	virtual bool remove( const String &filePath ) override;
	virtual bool mkdir(const String &directory) override;
	virtual bool rmdir(const String &directory) override;
	virtual bool exists(const String &filename) override;
	virtual bool dirExists(const String &dirpath) override;
	virtual UniquePtr<List<Entry>> readDir(const String &path, bool absolutePaths, bool recursive) override;
	virtual bool mstat( MetaStat *result, const String &path ) override;

	bool ioRead(void *const buffer, uint64_t bytes, uint64_t offset);

private:
	void readZip();
	void processEntry(const String &name, zip::CentralDirectoryFileHeader *entry);
	ZipEntry *registerEntry(const ZipEntry &entry);
	void link();

	ZipEntry *findEntry(const String &path);

private:
	String m_rootFilename;
	UniquePtr<File> m_root;

	Map<u64, ZipEntry> m_entries;

};

class ZipEntry
{
public:
	ZipEntry();
	~ZipEntry();

	void addChild(ZipEntry *e);
	const String &path() const { return m_path; }

private:
	bool m_directory;

	String m_path;
	String m_name;

	uint32_t m_offset = 0;

	bool m_compressed = false;

	size_t m_size = 0;
	size_t m_compressedSize = 0;

	Array<ZipEntry *> m_children;

	friend class ZipFileSystem;
	friend class ZipFsFile;
};

/* eof */
