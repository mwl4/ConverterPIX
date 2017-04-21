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

class ZipFileSystem : public FileSystem
{
private:
	String m_rootFilename;
	UniquePtr<File> m_root;
public:
	ZipFileSystem(const String &root);
	virtual ~ZipFileSystem();

	virtual UniquePtr<File> open(const String &filename, FsOpenMode mode) override;
	virtual bool mkdir(const String &directory) override;
	virtual bool rmdir(const String &directory) override;
	virtual bool exists(const String &filename) override;
	virtual bool dirExists(const String &dirpath) override;
	virtual UniquePtr<List<String>> readDir(const String &path, bool absolutePaths, bool recursive) override;

	void readZip();
};

/* eof */
