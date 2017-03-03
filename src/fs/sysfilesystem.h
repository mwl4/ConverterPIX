/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : sysfilesystem.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include "filesystem.h"

class SysFileSystem : public FileSystem
{
private:
	std::string m_root; // does not contain / at end
public:
	SysFileSystem(const std::string &root);
	virtual ~SysFileSystem();

	virtual std::unique_ptr<File> open(const std::string &filename, FsOpenMode mode) override;
	virtual bool mkdir(const std::string &directory) override;
	virtual bool rmdir(const std::string &directory) override;
	virtual bool exists(const std::string &filename) override;
	virtual bool dirExists(const std::string &dirpath) override;
	virtual std::unique_ptr<std::list<std::string>> readDir(const std::string &path, bool absolutePaths, bool recursive) override;
};

/* eof */
