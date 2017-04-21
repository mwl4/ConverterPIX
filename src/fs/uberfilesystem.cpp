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

UniquePtr<File> UberFileSystem::open(const String &filename, FsOpenMode mode)
{
	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
	{
		auto file = (*it).second->open(filename, mode);
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

UniquePtr<List<String>> UberFileSystem::readDir(const String &path, bool absolutePaths, bool recursive)
{
	auto result = std::make_unique<List<String>>();
	for (const auto &fs : m_filesystems)
	{
		auto current = fs.second->readDir(path, absolutePaths, recursive);
		if (current)
		{
			result->insert(result->begin(), current->begin(), current->end());
		}
	}
	return result;
}

FileSystem *UberFileSystem::mount(UniquePtr<FileSystem> fs, int priority)
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
