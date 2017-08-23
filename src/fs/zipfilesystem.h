/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : zipfilesystem.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

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
	virtual UniquePtr<File> open(const String &filename, FsOpenMode mode) override;
	virtual bool mkdir(const String &directory) override;
	virtual bool rmdir(const String &directory) override;
	virtual bool exists(const String &filename) override;
	virtual bool dirExists(const String &dirpath) override;
	virtual UniquePtr<List<Entry>> readDir(const String &path, bool absolutePaths, bool recursive) override;

	bool ioRead(void *const buffer, size_t bytes, size_t offset);

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
