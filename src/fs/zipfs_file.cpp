/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : zipfs_file.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "zipfs_file.h"

ZipFsFile::ZipFsFile()
{
}

ZipFsFile::~ZipFsFile()
{
}

size_t ZipFsFile::write(const void *buffer, size_t elementSize, size_t elementCount)
{
	return 0;
}

size_t ZipFsFile::read(void *buffer, size_t elementSize, size_t elementCount)
{
	return 0;
}

size_t ZipFsFile::getSize() const
{
	return 0;
}

const char *ZipFsFile::getLine(String &out)
{
	return nullptr;
}

int ZipFsFile::seek(uint32_t offset, Attrib attr)
{
	return 0;
}

void ZipFsFile::rewind()
{
}

size_t ZipFsFile::tell()
{
	return 0;
}

/* eof */
