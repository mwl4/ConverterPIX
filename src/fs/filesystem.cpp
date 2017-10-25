/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : filesystem.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "filesystem.h"

#include "sysfilesystem.h"
#include "uberfilesystem.h"
#include "hashfilesystem.h"
#include "zipfilesystem.h"

#include "file.h"

FileSystem::FileSystem()
{
}

FileSystem::~FileSystem()
{
}

SysFileSystem *getSFS()
{
	static SysFileSystem fs("");
	return &fs;
}

UberFileSystem *getUFS()
{
	static UberFileSystem fs;
	return &fs;
}

FileSystem *ufsMount(const String &root, scs_bool readOnly, int priority)
{
	if (getSFS()->dirExists(root))
	{
		String rootdirectory = makeSlashAtEnd(root);
		auto fs = std::make_unique<SysFileSystem>(rootdirectory.substr(0, rootdirectory.length() - 1));
		return getUFS()->mount(std::move(fs), priority);
	}
	else if(getSFS()->exists(root))
	{
		auto rootfile = getSFS()->open(root, FileSystem::read | FileSystem::binary);
		char sig[4];
		rootfile->blockRead(&sig, 0, sizeof(sig));
		rootfile.reset();
		if (sig[0] == 'P' && sig[1] == 'K') // zip
		{
			auto fs = std::make_unique<ZipFileSystem>(root);
			return getUFS()->mount(std::move(fs), priority);
		}
		else if (sig[0] == 'S' && sig[1] == 'C' && sig[2] == 'S' && sig[3] == '#') // scs#
		{
			auto fs = std::make_unique<HashFileSystem>(root);
			return getUFS()->mount(std::move(fs), priority);
		}
	}
	warning("system", root, "Unknown filesystem type!");
	return nullptr;
}

void ufsUnmount(FileSystem *fs)
{
	getUFS()->unmount(fs);
}

String directory(const String &filepath)
{
	size_t slashpos = filepath.rfind('/');
	if (slashpos == -1) slashpos = 0;
	size_t backslackpos = filepath.rfind('\\');
	if (backslackpos == -1) backslackpos = 0;
	return filepath.substr(0, std::max(slashpos, backslackpos)).c_str();
}

String relativePath(const String &filepath, const String &directory)
{
	String dir = (directory + "/").c_str();
	String path = filepath.c_str();

	std::replace(dir.begin(), dir.end(), '\\', '/');
	std::replace(path.begin(), path.end(), '\\', '/');

	String result;

	size_t up = 0;
	size_t curpos = 0;
	size_t lastpos = 0;
	while ((curpos = dir.find('/', curpos + 1)) != String::npos)
	{
		if (up == 0)
		{
			if (dir.substr(0, curpos) != path.substr(0, curpos))
			{
				++up;
			}
			else
			{
				lastpos = path[curpos] == '/' ? curpos + 1 : curpos;
			}
		}
		else
		{
			++up;
		}
	}

	for (size_t i = 0; i < up; ++i)
	{
		result += "../";
	}
	result += path.substr(lastpos);
	return result.c_str();
}

/* eof */
