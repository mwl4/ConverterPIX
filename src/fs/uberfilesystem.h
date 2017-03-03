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
private:
	std::map<int, std::unique_ptr<FileSystem>> m_filesystems;
public:
	UberFileSystem();
	virtual ~UberFileSystem();

	virtual std::unique_ptr<File> open(const std::string &filename, FsOpenMode mode) override;
	virtual bool mkdir(const std::string &directory) override;
	virtual bool rmdir(const std::string &directory) override;
	virtual bool exists(const std::string &filename) override;
	virtual bool dirExists(const std::string &dirpath) override;
	virtual std::unique_ptr<std::list<std::string>> readDir(const std::string &path, bool absolutePaths, bool recursive) override;

	FileSystem *mount(std::unique_ptr<FileSystem> fs, int priority);
	void unmount(FileSystem *fs);
};

/* eof */
