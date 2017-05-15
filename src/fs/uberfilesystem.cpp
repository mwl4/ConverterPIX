/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : uberfilesystem.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "uberfilesystem.h"

#include "file.h"

UberFileSystem::UberFileSystem()
{
}

UberFileSystem::~UberFileSystem()
{
}

String UberFileSystem::root() const
{
	return "/";
}

String UberFileSystem::name() const
{
	return "uberfs";
}

UniquePtr<File> UberFileSystem::open(const String &filename, FsOpenMode mode)
{
	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
	{
		UniquePtr<File> file = (*it).second->open(filename, mode);
		if (file)
		{
			return file;
		}
	}
	return UniquePtr<File>();
}

bool UberFileSystem::mkdir(const String &directory)
{
	return false;
}

bool UberFileSystem::rmdir(const String &directory)
{
	return false;
}

bool UberFileSystem::exists(const String &filename)
{
	for (const auto &fs : m_filesystems)
	{
		if (fs.second->exists(filename))
			return true;
	}
	return false;
}

bool UberFileSystem::dirExists(const String &dirpath)
{
	for (const auto &fs : m_filesystems)
	{
		if (fs.second->dirExists(dirpath))
			return true;
	}
	return false;
}

auto UberFileSystem::readDir(const String &path, bool absolutePaths, bool recursive) -> UniquePtr<List<Entry>>
{
	UniquePtr<List<Entry>> result;
	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
	{
		const auto &fs = (*it);
		if(fs.second->dirExists(path))
		{
			auto current = fs.second->readDir(path, absolutePaths, recursive);
			if (current)
			{
				if (!result)
				{
					result = std::make_unique<List<Entry>>();
				}
				for (const auto &c : (*current))
				{
					bool existsAlready = false;
					for (const auto &r : (*result))
					{
						if (c.GetPath() == r.GetPath())
						{
							existsAlready = true;
							break;
						}
					}
					if (!existsAlready)
					{
						result->push_back(c);
					}
				}
			}
		}
	}
	return result;
}

FileSystem *UberFileSystem::mount(UniquePtr<FileSystem> fs, Priority priority)
{
	m_filesystems[priority] = std::move(fs);
	return m_filesystems[priority].get();
}

void UberFileSystem::unmount(FileSystem *filesystem)
{
	for (const auto &fs : m_filesystems)
	{
		if (fs.second.get() == filesystem)
		{
			m_filesystems.erase(fs.first);
			return;
		}
	}
}

/* eof */
