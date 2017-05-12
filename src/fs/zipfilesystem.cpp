/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : zipfilesystem.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "zipfilesystem.h"

#include "sysfilesystem.h"
#include "file.h"

#include <structs/zip.h>

#include <zlib/zlib.h>

ZipFileSystem::ZipFileSystem(const String &root)
{
	m_rootFilename = root;
	m_root = getSFS()->open(root, FileSystem::read | FileSystem::binary);
	if (!m_root)
	{
		error("sysfs", root, "Unable to open root file");
		return;
	}
	readZip();
}

ZipFileSystem::~ZipFileSystem()
{
}

String ZipFileSystem::root() const
{
	return m_rootFilename;
}

String ZipFileSystem::name() const
{
	return "zipfs";
}

UniquePtr<File> ZipFileSystem::open(const String &filename, FsOpenMode mode)
{
	return nullptr;
}

bool ZipFileSystem::mkdir(const String &directory)
{
	return false;
}

bool ZipFileSystem::rmdir(const String &directory)
{
	return false;
}

bool ZipFileSystem::exists(const String &filename)
{
	return false;
}

bool ZipFileSystem::dirExists(const String &dirpath)
{
	return false;
}

auto ZipFileSystem::readDir(const String &path, bool absolutePaths, bool recursive) -> UniquePtr<List<Entry>>
{
	return nullptr;
}

void ZipFileSystem::readZip()
{
	const size_t size = m_root->size();
	if (size <= sizeof(zip::EndOfCentralDirectory))
	{
		error("zipfs", m_rootFilename, "Too short file!");
		return;
	}

	size_t blockSizeToFindCentralDirEnd = ((size < 0x4000) ? size : 0x4000);
	UniquePtr<uint8_t[]> blockToFindCentralDirEnd(new uint8_t[blockSizeToFindCentralDirEnd]);
	if (!m_root->blockRead(blockToFindCentralDirEnd.get(), size - blockSizeToFindCentralDirEnd, blockSizeToFindCentralDirEnd))
	{
		error("zipfs", m_rootFilename, "Failed to read the zip_central_dir_end structure!");
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
			error("zipfs", m_rootFilename, "Cannot find the zip_central_dir_end signature!");
			return;
		}
	}

	if (centralDirEnd->signature != zip::EndOfCentralDirectory::SIGNATURE)
	{
		error("zipfs", m_rootFilename, "Invalid end signature!");
		return;
	}

	const uint16_t numEntriesLimit = 60000;
	if (centralDirEnd->numEntries > numEntriesLimit)
	{
		error_f("zipfs", m_rootFilename, "The number of files(%u) exceeded limits(%u).", centralDirEnd->numEntries, numEntriesLimit);
		return;
	}

	

	// TODO:
}

/* eof */
