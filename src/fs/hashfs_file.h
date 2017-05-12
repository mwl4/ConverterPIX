/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : hashfs_file.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include "file.h"

#include <structs/hashfs.h>

class HashFsFile : public File
{
public:
	HashFsFile(const String &filepath, HashFileSystem *filesystem, const prism::hashfs_entry_t *header);
	HashFsFile(const HashFsFile &) = delete;
	HashFsFile(HashFsFile &&) = delete;
	virtual ~HashFsFile();

	HashFsFile &operator=(const HashFsFile &) = delete;
	HashFsFile &operator=(HashFsFile &&) = delete;

	virtual size_t write(const void *buffer, size_t elementSize, size_t elementCount) override;
	virtual size_t read(void *buffer, size_t elementSize, size_t elementCount) override;
	virtual size_t size() const override;
	virtual bool seek(uint32_t offset, Attrib attr) override;
	virtual void rewind() override;
	virtual size_t tell() override;
	virtual void flush() override;

private:
	String			m_filepath;
	HashFileSystem *m_filesystem;
	z_stream		m_stream;
	size_t			m_position;

	const prism::hashfs_entry_t *m_header;

private:
	void inflateInitialize();
	void inflateDestroy();

	friend class HashFileSystem;
};

/* eof */
