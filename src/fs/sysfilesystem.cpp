/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : sysfilesystem.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "sysfilesystem.h"

#include "sysfs_file.h"

SysFileSystem::SysFileSystem(const String &root)
	: m_root(root)
{
}

SysFileSystem::~SysFileSystem()
{
}

String SysFileSystem::root() const
{
	return m_root;
}

String SysFileSystem::name() const
{
	return "sysfs";
}

UniquePtr<File> SysFileSystem::open(const String &filename, FsOpenMode mode)
{
	const String smode =
		String(mode & read ? "r" : "")
		+ (mode & write ? "w" : "")
		+ (mode & append ? "a" : "")
		+ (mode & binary ? "b" : "")
		+ (mode & update ? "+" : "");

	FILE *fp = fopen((m_root + filename).c_str(), smode.c_str());
	if (!fp)
	{
		if (!dirExists(directory(filename)) && (mode & write))
		{
			if (mkdir(directory(filename)))
			{
				fp = fopen((m_root + filename).c_str(), smode.c_str());
				if (!fp)
				{
					return UniquePtr<File>();
				}
			}
			else
			{
				return UniquePtr<File>();
			}
		}
		else
		{
			return UniquePtr<File>();
		}
	}

	auto file = std::make_unique<SysFsFile>();
	file->m_fp = fp;
	fseek(file->m_fp, 0, SEEK_SET);
	return std::move(file);
}

bool SysFileSystem::mkdir(const String &dir)
{
	String dirr(m_root + dir.c_str());
	std::replace_if(dirr.begin(), dirr.end(), [](char ch)->bool { return ch == '\\'; }, '/');

	if (dirExists(dirr.c_str())) {
		return true;
	}
	dirr += '/';
	for (size_t pos = dirr.find('/', dirr.find('/') + 1); pos != -1; pos = dirr.find('/', pos + 1))
	{
		if (!dirExists(dirr.substr(0, pos).c_str()))
		{
		#ifdef _WIN32
			if (::mkdir(dirr.substr(0, pos).c_str()) != 0)
				return false;
		#else
			if (::mkdir(dirr.substr(0, pos).c_str(), 0775) != 0)
				return false;
		#endif
		}
	}
	return true;
}

bool SysFileSystem::rmdir(const String &directory)
{
	return false;
}

bool SysFileSystem::exists(const String &filename)
{
	FILE *fp = fopen((m_root + filename).c_str(), "rb");
	if (fp)
	{
		fclose(fp);
		return true;
	}
	return false;
}

bool SysFileSystem::dirExists(const String &dirpath)
{
	struct stat buffer;
	return (stat((m_root + dirpath).c_str(), &buffer) == 0 && ((buffer.st_mode & S_IFDIR) != 0));
}

auto SysFileSystem::readDir(const String &directory, bool absolutePaths, bool recursive) -> UniquePtr<List<Entry>>
{
	String directoryNoSlash = removeSlashAtEnd(directory);

#ifdef _WIN32
	HANDLE dir;
	WIN32_FIND_DATA fileData;

	if ((dir = FindFirstFileA((m_root + directoryNoSlash + "/*").c_str(), &fileData)) == INVALID_HANDLE_VALUE)
		return UniquePtr<List<Entry>>();

	auto result = std::make_unique<List<Entry>>();
	do
	{
		const String fileName = fileData.cFileName;
		const String fullFileName = directoryNoSlash + "/" + fileName;

		if (fileName[0] == '.')
			continue;

		const bool isDirectory = !!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		if (isDirectory)
		{
			if (recursive)
			{
				auto subdir = readDir(fullFileName, absolutePaths, recursive);
				if (subdir)
				{
					result->insert(result->end(), subdir->begin(), subdir->end());
				}
			}
		}
		result->push_back(Entry((absolutePaths ? fullFileName : fileName), isDirectory, false, this));
	} while (FindNextFileA(dir, &fileData));
	FindClose(dir);
	return result;
#else
	auto result = std::make_unique<List<Entry>>();
	DIR *dir;
	struct dirent *ent;
	struct stat st;

	dir = opendir(directoryNoSlash.c_str());
	while ((ent = readdir(dir)) != 0)
	{
		const String fileName = ent->d_name;
		const String fullFileName = directoryNoSlash + "/" + fileName;

		if (fileName[0] == '.')
			continue;

		if (stat(fullFileName.c_str(), &st) == -1)
			continue;

		const bool isDirectory = !!(st.st_mode & S_IFDIR);
		if (isDirectory)
		{
			if (recursive)
			{
				auto subdir = readDir(fullFileName, absolutePaths, recursive);
				if (subdir)
				{
					result->insert(result->begin(), subdir->begin(), subdir->end());
				}
			}
		}
		result->push_back(Entry((absolutePaths ? fullFileName : fileName), isDirectory, false, this));
	}
	closedir(dir);
	return result;
#endif
}

String SysFileSystem::getError() const
{
	return strerror(errno);
}

/* eof */
