/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : sysfs_file.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "sysfs_file.h"

SysFsFile::SysFsFile()
{
}

SysFsFile::~SysFsFile()
{
	if (m_fp)
	{
		::fclose(m_fp);
	}
}

uint64_t SysFsFile::write(const void *buffer, uint64_t elementSize, uint64_t elementCount)
{
	return ::fwrite(buffer, static_cast<size_t>(elementSize), static_cast<size_t>(elementCount), m_fp);
}

uint64_t SysFsFile::read(void *buffer, uint64_t elementSize, uint64_t elementCount)
{
	return ::fread(buffer, static_cast<size_t>(elementSize), static_cast<size_t>(elementCount), m_fp);
}

uint64_t SysFsFile::size()
{
	uint64_t current = tell();
	seek(0, SeekEnd);
	uint64_t result = tell();
	seek(current, SeekSet);
	return result;
}

bool SysFsFile::seek(uint64_t offset, Attrib attr)
{
#ifdef _WIN32
	return ::_fseeki64(m_fp, static_cast<long long>(offset), static_cast<int>(attr)) == 0;
#else
	return ::fseeko(m_fp, static_cast<off_t>(offset), static_cast<int>(attr)) == 0;
#endif
}

void SysFsFile::rewind()
{
	::rewind(m_fp);
}

uint64_t SysFsFile::tell() const
{
#ifdef _WIN32
	return static_cast<uint64_t>(::_ftelli64(m_fp));
#else
	return static_cast<uint64_t>(::ftello(m_fp));
#endif
}

void SysFsFile::flush()
{
	::fflush(m_fp);
}

/* eof */
