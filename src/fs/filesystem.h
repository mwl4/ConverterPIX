/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : filesystem.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

class FileSystem
{
public:
	class Entry;

	enum FsOpenMode
	{
		OpenModeNone = 0
	};
	static constexpr FsOpenMode read	= (FsOpenMode)(1 << 0);
	static constexpr FsOpenMode write	= (FsOpenMode)(1 << 1);
	static constexpr FsOpenMode append	= (FsOpenMode)(1 << 2);
	static constexpr FsOpenMode update	= (FsOpenMode)(1 << 3);
	static constexpr FsOpenMode binary	= (FsOpenMode)(1 << 4);

public:
	FileSystem();
	FileSystem(const FileSystem&) = delete;
	FileSystem(FileSystem &&) = delete;
	virtual ~FileSystem();

	FileSystem &operator=(FileSystem &) = delete;
	FileSystem &operator=(FileSystem &&) = delete;

	virtual String root() const = 0;
	virtual String name() const = 0;
	virtual UniquePtr<File> open(const String &filename, FsOpenMode mode) = 0;
	virtual bool mkdir(const String &directory) = 0;
	virtual bool rmdir(const String &directory) = 0;
	virtual bool exists(const String &filename) = 0;
	virtual bool dirExists(const String &dirpath) = 0;
	virtual UniquePtr<List<Entry>> readDir(const String &path, bool absolutePaths, bool recursive) = 0;
};

class FileSystem::Entry
{
public:
	Entry()
	{
	}

	Entry(String path, bool directory, bool encrypted, FileSystem *filesystem)
		: m_path(path)
		, m_directory(directory)
		, m_encrypted(encrypted)
		, m_filesystem(filesystem)
	{
	}

	inline const String &GetPath() const { return m_path; }
	inline void SetPath(const String path) { m_path = path; }

	inline bool IsDirectory() const { return m_directory; }
	inline bool IsEncrypted() const { return m_encrypted; }
	inline FileSystem *GetFileSystem() const { return m_filesystem; }

private:
	String m_path;
	bool m_directory = false;
	bool m_encrypted = false;
	FileSystem *m_filesystem = nullptr;
};

constexpr FileSystem::FsOpenMode operator|(const FileSystem::FsOpenMode t, const FileSystem::FsOpenMode f)
{
	return static_cast<FileSystem::FsOpenMode>((unsigned)t | (unsigned)f);
}

SysFileSystem *getSFS();
UberFileSystem *getUFS();

FileSystem *ufsMount(const String &root, scs_bool readOnly, int priority);
void ufsUnmount(FileSystem *fs);

/* eof */
