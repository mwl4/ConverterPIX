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
#include "zipfs_file.h"

#include <structs/zip.h>

#include <zlib/zlib.h>

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#endif

ZipFileSystem::ZipFileSystem(const String &root)
{
	m_rootFilename = root;
	m_root = getSFS()->open(root, FileSystem::read | FileSystem::binary);
	if (!m_root)
	{
		error("zipfs", root, "Unable to open root file");
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
	ZipEntry *const entry = findEntry(filename);
	if (!entry)
	{
		return UniquePtr<File>();
	}

	return std::make_unique<ZipFsFile>(filename, this, entry);
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
	if (filename.empty())
	{
		return false;
	}

	ZipEntry *const entry = findEntry(filename);
	if (!entry)
	{
		return false;
	}

	if (entry->m_directory)
	{
		return false;
	}

	return true;
}

bool ZipFileSystem::dirExists(const String &dirpath)
{
	if (dirpath.empty())
	{
		return false;
	}

	ZipEntry *const entry = findEntry(dirpath.size() != 1 ? removeSlashAtEnd(dirpath) : dirpath);
	if (!entry)
	{
		return false;
	}

	if (!entry->m_directory)
	{
		return false;
	}

	return true;
}

auto ZipFileSystem::readDir(const String &path, bool absolutePaths, bool recursive) -> UniquePtr<List<Entry>>
{
	if (path.empty())
	{
		error("zipfs", m_rootFilename, "readDir: Path is empty!");
		return UniquePtr<List<Entry>>();
	}

	String dirpath = path.size() != 1 ? removeSlashAtEnd(path) : path;

	ZipEntry *const entry = findEntry(dirpath);
	if (!entry)
	{
		error_f("zipfs", m_rootFilename, "Failed to find entry (%s)", path);
		return UniquePtr<List<Entry>>();
	}

	if (!entry->m_directory)
	{
		error_f("zipfs", m_rootFilename, "Entry is not directory");
		return UniquePtr<List<Entry>>();
	}

	auto result = std::make_unique<List<Entry>>();

	for (const ZipEntry *const e : entry->m_children)
	{
		if (e->m_directory)
		{
			String directorypath;
			if (absolutePaths)
			{
				directorypath = removeSlashAtEnd(dirpath) + "/" + e->m_name;
			}
			else
			{
				directorypath = e->m_name;
			}
			result->push_back(Entry(directorypath, true, false, this));
			if (recursive)
			{
				auto subdir = readDir(directorypath, absolutePaths, recursive);
				if (subdir)
				{
					result->insert(result->end(), subdir->begin(), subdir->end());
				}
			}
		}
		else
		{
			String filepath;
			if (absolutePaths)
			{
				filepath = removeSlashAtEnd(dirpath) + "/" + e->m_name;
			}
			else
			{
				filepath = e->m_name;
			}
			result->push_back(Entry(filepath, false, false, this));
		}
	}

	return result;
}

bool ZipFileSystem::ioRead(void *const buffer, size_t bytes, size_t offset)
{
	return m_root->blockRead(buffer, offset, bytes);
}

void ZipFileSystem::readZip()
{
	ZipEntry rootEntry;
	rootEntry.m_directory = true;
	rootEntry.m_name = "";
	rootEntry.m_path = "/";
	registerEntry(rootEntry);

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
		error("zipfs", m_rootFilename, "Failed to read the zip::EndOfCentralDirectory structure!");
		return;
	}

	zip::EndOfCentralDirectory *centralDirEnd = nullptr;

	for(uint8_t *currentOffset = blockToFindCentralDirEnd.get() + blockSizeToFindCentralDirEnd;;)
	{
		centralDirEnd = reinterpret_cast<zip::EndOfCentralDirectory *>(currentOffset);
		if (centralDirEnd->signature == zip::EndOfCentralDirectory::SIGNATURE)
		{
			break;
		}

		if (--currentOffset < blockToFindCentralDirEnd.get())
		{
			error("zipfs", m_rootFilename, "Cannot find the zip::EndOfCentralDirectory signature!");
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

	for (size_t e = 0, currentOffset = centralDirEnd->offset; e < centralDirEnd->numEntries; ++e)
	{
		zip::CentralDirectoryFileHeader entry;
		if (!m_root->blockRead(&entry, currentOffset, sizeof(zip::CentralDirectoryFileHeader)))
		{
			error_f("zipfs", m_rootFilename, "Failed to read central directory data(%u)!", e);
			return;
		}

		if (entry.signature != zip::CentralDirectoryFileHeader::SIGNATURE)
		{
			error_f("zipfs", m_rootFilename, "Central directory data(%u) has invalid signature!", e);
			return;
		}

		if (entry.filenameLength == 0 || entry.filenameLength > 255)
		{
			error_f("zipfs", m_rootFilename, "Central directory data(%u) has invalid name!", e);
			return;
		}

		char filenameBuffer[256] = { 0 };
		if (!m_root->blockRead(filenameBuffer, currentOffset + sizeof(zip::CentralDirectoryFileHeader), entry.filenameLength))
		{
			error_f("zipfs", m_rootFilename, "Failed to read name of directory data(%u)!", e);
			return;
		}
		const String filename = filenameBuffer;

		if (entry.compressionMethod != zip::COMPRESSION_METHOD::STORED && entry.compressionMethod != zip::COMPRESSION_METHOD::DEFLATED)
		{
			error_f("zipfs", m_rootFilename, "Unsupported compression method(%s : %u)!", filename.c_str(), entry.compressionMethod);
			return;
		}

		processEntry(trimSlashesAtEnd(trimSlashesAtBegin(filename)), &entry);

		currentOffset += sizeof(zip::CentralDirectoryFileHeader)
			+ entry.filenameLength
			+ entry.extrafieldLength
			+ entry.fileCommentLength;
	}

	link();
}

void ZipFileSystem::processEntry(const String &name, zip::CentralDirectoryFileHeader *entry)
{
	ZipEntry zipentry;

	const uint8_t versionMadeBy = entry->versionMadeBy & 0xff00;

	if (versionMadeBy == zip::VERSION_MADE_BY::WINDOWS_NTFS
	 || versionMadeBy == zip::VERSION_MADE_BY::DOS_OS2
	 || versionMadeBy == zip::VERSION_MADE_BY::MVS)
	{
		if (entry->externalFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			zipentry.m_directory = true;
		}
		else
		{
			zipentry.m_directory = false;
			zipentry.m_offset = entry->relOffsetOfLocalHeader;
		}
	}
	else if (versionMadeBy == zip::VERSION_MADE_BY::UNIX)
	{
		if (entry->externalFileAttributes & S_IFDIR)
		{
			zipentry.m_directory = true;
		}
		else
		{
			zipentry.m_directory = false;
			zipentry.m_offset = entry->relOffsetOfLocalHeader;
		}
	}
	else
	{
		error_f("zipfs", m_rootFilename, "Unsupported version of entry: %u!", versionMadeBy);
		return;
	}

	if (zipentry.m_directory)
	{
		String dirname = trimSlashesAtEnd(trimSlashesAtBegin(name));
		zipentry.m_name = dirname.substr(dirname.find_last_of('/') + 1);
		zipentry.m_path = "/" + dirname;
	}
	else
	{
		String filename = trimSlashesAtEnd(trimSlashesAtBegin(name));
		zipentry.m_name = filename.substr(filename.find_last_of('/') + 1);
		zipentry.m_path = "/" + filename;
	}

	if (!zipentry.m_directory)
	{
		zip::LocalFileHeader localEntry;
		if (!m_root->blockRead(&localEntry, zipentry.m_offset, sizeof(zip::LocalFileHeader)))
		{
			error_f("zipfs", m_rootFilename, "Failed to read local file header data!");
			return;
		}

		if (localEntry.signature != zip::LocalFileHeader::SIGNATURE)
		{
			error_f("zipfs", m_rootFilename, "Local file header has invalid signature!");
			return;
		}

		zipentry.m_compressed = entry->compressionMethod == zip::COMPRESSION_METHOD::DEFLATED;
		zipentry.m_size = entry->uncompressedSize;
		zipentry.m_compressedSize = entry->compressedSize;

		zipentry.m_offset = zipentry.m_offset + sizeof(zip::LocalFileHeader) + localEntry.filenameLength + localEntry.extrafieldLength;
	}
	else
	{
		zipentry.m_compressed = false;
		zipentry.m_size = 0;
		zipentry.m_compressedSize = 0;
		zipentry.m_offset = 0;
	}

	registerEntry(zipentry);
}

ZipEntry *ZipFileSystem::registerEntry(const ZipEntry &entry)
{
	if (ZipEntry *existing = findEntry(entry.m_path))
	{
		return existing;
	}
	const uint64_t hash = prism::city_hash_64(entry.m_path.c_str() + 1, entry.m_path.length() - 1);
	ZipEntry *const e = &(m_entries[hash] = entry);
	return e;
}

void ZipFileSystem::link()
{
	Array<ZipEntry> toRegister;

	for (Pair<const u64, ZipEntry> &entry : m_entries)
	{
		ZipEntry *const e = &entry.second;
		if (e->m_path != "/")
		{
			String directory = trimSlashesAtEnd(trimSlashesAtBegin(e->m_path.substr(0, e->m_path.find_last_of('/'))));
			ZipEntry *dir = findEntry("/" + directory);
			if (!dir)
			{
				auto dirit = std::find_if(toRegister.begin(), toRegister.end(),
					[directory](ZipEntry &e)
					{
						return e.path() == ("/" + directory);
					});
				if (dirit == toRegister.end())
				{
					ZipEntry newDirEntry;
					newDirEntry.m_directory = true;
					newDirEntry.m_name = directory.substr(directory.find_last_of('/') + 1);
					newDirEntry.m_path = "/" + directory;
					newDirEntry.m_compressed = false;
					newDirEntry.m_size = 0;
					newDirEntry.m_compressedSize = 0;
					newDirEntry.m_offset = 0;
					toRegister.push_back(newDirEntry);
				}
			}
		}
	}

	for (size_t i = 0; i < toRegister.size(); ++i)
	{
		ZipEntry entry = toRegister[i];
		if (entry.m_path != "/" && !entry.m_path.empty())
		{
			ZipEntry *const e = registerEntry(entry);
			String directory = trimSlashesAtEnd(trimSlashesAtBegin(e->m_path.substr(0, e->m_path.find_last_of('/'))));
			ZipEntry *dir = findEntry("/" + directory);
			if (!dir)
			{
				ZipEntry newDirEntry;
				newDirEntry.m_directory = true;
				newDirEntry.m_name = directory.substr(directory.find_last_of('/') + 1);
				newDirEntry.m_path = "/" + directory;
				newDirEntry.m_compressed = false;
				newDirEntry.m_size = 0;
				newDirEntry.m_compressedSize = 0;
				newDirEntry.m_offset = 0;
				toRegister.push_back(newDirEntry);
			}
		}
	}

	for (Pair<const u64, ZipEntry> &entry : m_entries)
	{
		ZipEntry *const e = &entry.second;
		if (e->m_path != "/")
		{
			String directory = trimSlashesAtEnd(trimSlashesAtBegin(e->m_path.substr(0, e->m_path.find_last_of('/'))));
			ZipEntry *dir = findEntry("/" + directory);
			assert(dir);
			dir->addChild(e);
		}
	}
}

auto ZipFileSystem::findEntry(const String &path) -> ZipEntry *
{
	const u64 hash = prism::city_hash_64(path.c_str() + 1, path.length() - 1);

	auto it = m_entries.find(hash);
	if (it != m_entries.end())
	{
		return &((*it).second);
	}
	return nullptr;
}

/* ------ ZipEntry ------ */

ZipEntry::ZipEntry()
{
}

ZipEntry::~ZipEntry()
{
}

void ZipEntry::addChild(ZipEntry *e)
{
	for (Array<ZipEntry *>::iterator it = m_children.begin();
		 it != m_children.end();
		 ++it)
	{
		ZipEntry *current = (*it);
		if (e->m_name < current->m_name)
		{
			m_children.insert(it, e);
			return;
		}
	}

	m_children.push_back(e);
}

/* eof */
