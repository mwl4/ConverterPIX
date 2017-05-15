/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : sysfs_file.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include "file.h"

class SysFsFile : public File
{
public:
	SysFsFile();
	SysFsFile(const SysFsFile &) = delete;
	SysFsFile(SysFsFile &&) = delete;
	virtual ~SysFsFile();

	SysFsFile &operator=(const SysFsFile &) = delete;
	SysFsFile &operator=(SysFsFile &&) = delete;

	virtual size_t write(const void *buffer, size_t elementSize, size_t elementCount) override;
	virtual size_t read(void *buffer, size_t elementSize, size_t elementCount) override;
	virtual size_t size() const override;
	virtual bool seek(uint32_t offset, Attrib attr) override;
	virtual void rewind() override;
	virtual size_t tell() override;
	virtual void flush() override;

private:
	FILE *m_fp = nullptr;

	friend class SysFileSystem;
};

/* eof */
