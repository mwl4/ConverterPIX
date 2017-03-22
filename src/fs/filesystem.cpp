/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : filesystem.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include "filesystem.h"
#include "sysfilesystem.h"
#include "uberfilesystem.h"

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

FileSystem *ufsMount(const std::string &root, scs_bool readOnly, int priority)
{
	if (root.substr(root.length() - 4) == ".zip")
	{
		/* TODO: Create zip file system */
		error("system", root, "Zip filesystem is not supported yet!");
		return nullptr;
	}
	else if (root.substr(root.length() - 1) == "/")
	{
		auto fs = std::make_unique<SysFileSystem>(root.substr(0, root.length() - 1));
		return getUFS()->mount(std::move(fs), priority);
	}
	error("system", root, "nknown filesystem type!");
	return nullptr;
}

void ufsUnmount(FileSystem *fs)
{
	getUFS()->unmount(fs);
}

std::string directory(const std::string &filepath)
{
	size_t slashpos = filepath.rfind('/');
	if (slashpos == -1) slashpos = 0;
	size_t backslackpos = filepath.rfind('\\');
	if (backslackpos == -1) backslackpos = 0;
	return filepath.substr(0, std::max(slashpos, backslackpos)).c_str();
}

std::string relativePath(const std::string &filepath, const std::string &directory)
{
	std::string dir = (directory + "/").c_str();
	std::string path = filepath.c_str();

	std::replace(dir.begin(), dir.end(), '\\', '/');
	std::replace(path.begin(), path.end(), '\\', '/');

	std::string result;

	size_t up = 0;
	size_t curpos = 0;
	size_t lastpos = 0;
	while ((curpos = dir.find('/', curpos + 1)) != std::string::npos)
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
