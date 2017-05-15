/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : uberfilesystem.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include "filesystem.h"

class UberFileSystem : public FileSystem
{
public:
	using Priority = int;

public:
	UberFileSystem();
	virtual ~UberFileSystem();

	virtual String root() const override;
	virtual String name() const override;
	virtual UniquePtr<File> open(const String &filename, FsOpenMode mode) override;
	virtual bool mkdir(const String &directory) override;
	virtual bool rmdir(const String &directory) override;
	virtual bool exists(const String &filename) override;
	virtual bool dirExists(const String &dirpath) override;
	virtual UniquePtr<List<Entry>> readDir(const String &path, bool absolutePaths, bool recursive) override;

	FileSystem *mount(UniquePtr<FileSystem> fs, Priority priority);
	void unmount(FileSystem *fs);

private:
	std::map<Priority, UniquePtr<FileSystem>> m_filesystems;
};

/* eof */
