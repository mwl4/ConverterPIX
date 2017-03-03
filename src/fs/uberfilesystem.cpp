/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : uberfilesystem.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include "uberfilesystem.h"
#include "file.h"

UberFileSystem::UberFileSystem()
{
}

UberFileSystem::~UberFileSystem()
{
}

std::unique_ptr<File> UberFileSystem::open(const std::string &filename, FsOpenMode mode)
{
	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
	{
		auto file = (*it).second->open(filename, mode);
		if (file)
		{
			return file;
		}
	}
	return std::unique_ptr<File>();
}

bool UberFileSystem::mkdir(const std::string &directory)
{
	return false;
}

bool UberFileSystem::rmdir(const std::string &directory)
{
	return false;
}

bool UberFileSystem::exists(const std::string &filename)
{
	for (const auto &fs : m_filesystems)
	{
		if (fs.second->exists(filename))
			return true;
	}
	return false;
}

bool UberFileSystem::dirExists(const std::string &dirpath)
{
	for (const auto &fs : m_filesystems)
	{
		if (fs.second->dirExists(dirpath))
			return true;
	}
	return false;
}

std::unique_ptr<std::list<std::string>> UberFileSystem::readDir(const std::string &path, bool absolutePaths, bool recursive)
{
	auto result = std::make_unique<std::list<std::string>>();
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

FileSystem *UberFileSystem::mount(std::unique_ptr<FileSystem> fs, int priority)
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
