/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/fs/uberfilesystem.cpp
 *
 *		  _____                          _            _____ _______   __
 *		 / ____|                        | |          |  __ \_   _\ \ / /
 *		| |     ___  _ ____   _____ _ __| |_ ___ _ __| |__) || |  \ V /
 *		| |    / _ \| '_ \ \ / / _ \ '__| __/ _ \ '__|  ___/ | |   > <
 *		| |___| (_) | | | \ V /  __/ |  | ||  __/ |  | |    _| |_ / . \
 *		 \_____\___/|_| |_|\_/ \___|_|   \__\___|_|  |_|   |_____/_/ \_\
 *
 *
 *  Copyright (C) 2017 Michal Wojtowicz.
 *  All rights reserved.
 *
 *   This software is ditributed WITHOUT ANY WARRANTY; without even
 *   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE. See the copyright file for more information.
 *
 *****************************************************************************/

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
	Map<String, int> aux;

	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
	{
		const auto &fs = (*it);
		if(!fs.second->dirExists(path))
		{
			continue;
		}

		auto current = fs.second->readDir(path, absolutePaths, recursive);
		if (!current)
		{
			continue;
		}

		if (!result)
		{
			result = std::make_unique<List<Entry>>();
		}

		for (const FileSystem::Entry &entry : (*current))
		{
			int &auxValue = aux[entry.GetPath()];
			if (auxValue == 1) { continue; }
			auxValue = 1;
			result->push_back(entry);
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
