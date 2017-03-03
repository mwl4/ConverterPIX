/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : zipfilesystem.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include "zipfilesystem.h"
#include "sysfilesystem.h"
#include "file.h"

#include <structs/zip.h>

ZipFileSystem::ZipFileSystem(const std::string &root)
{
	m_rootFilename = root;
	m_root = getSFS()->open(root, FileSystem::read | FileSystem::binary);
	if (!m_root)
	{
		printf("[zipfs] Unable to open root file (%s)!\n", root.c_str());
		return;
	}
	readZip();
}

ZipFileSystem::~ZipFileSystem()
{
}

std::unique_ptr<File> ZipFileSystem::open(const std::string &filename, FsOpenMode mode)
{
	return nullptr;
}

bool ZipFileSystem::mkdir(const std::string &directory)
{
	return false;
}

bool ZipFileSystem::rmdir(const std::string &directory)
{
	return false;
}

bool ZipFileSystem::exists(const std::string &filename)
{
	return false;
}

bool ZipFileSystem::dirExists(const std::string &dirpath)
{
	return false;
}

std::unique_ptr<std::list<std::string>> ZipFileSystem::readDir(const std::string &path, bool absolutePaths, bool recursive)
{
	return nullptr;
}

void ZipFileSystem::readZip()
{
	const size_t size = m_root->getSize();
	if (size <= sizeof(zip::EndOfCentralDirectory))
	{
		printf("[zipfs] Too short file \'%s\'.", m_rootFilename.c_str());
		return;
	}

	size_t blockSizeToFindCentralDirEnd = ((size < 0x4000) ? size : 0x4000);
	std::unique_ptr<uint8_t[]> blockToFindCentralDirEnd(new uint8_t[blockSizeToFindCentralDirEnd]);
	if (!m_root->blockRead(blockToFindCentralDirEnd.get(), size - blockSizeToFindCentralDirEnd, blockSizeToFindCentralDirEnd))
	{
		printf("[zipfs] Failed to read the zip_central_dir_end structure in \'%s\'.", m_rootFilename.c_str());
		return;
	}

	uint8_t *currentOffset = blockToFindCentralDirEnd.get() + blockSizeToFindCentralDirEnd;
	auto centralDirEnd = reinterpret_cast<zip::EndOfCentralDirectory *>(currentOffset);
	while (true)
	{
		if (centralDirEnd->signature == zip::EndOfCentralDirectory::SIGNATURE)
		{
			break;
		}

		if (--currentOffset < blockToFindCentralDirEnd.get())
		{
			printf("[zipfs] Cannot find the zip_central_dir_end signature in \'%s\'.", m_rootFilename.c_str());
			return;
		}
	}

	if (centralDirEnd->signature != zip::EndOfCentralDirectory::SIGNATURE)
	{
		printf("[zipfs] Invalid end signature in \'%s\'.", m_rootFilename.c_str());
		return;
	}

	const uint16_t numEntriesLimit = 60000;
	if (centralDirEnd->numEntries > numEntriesLimit)
	{
		printf("[zipfs] The number of files (%u) in file '%s' exceeded limits (%u).", centralDirEnd->numEntries, m_rootFilename.c_str(), numEntriesLimit);
		return;
	}


}

/* eof */
