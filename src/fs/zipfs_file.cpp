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

#include "zipfilesystem.h"

ZipFsFile::ZipFsFile(const String &filepath, ZipFileSystem *filesystem, const class ZipEntry *entry)
	: m_filepath(filepath)
	, m_filesystem(filesystem)
	, m_entry(entry)
	, m_position(0)
{
	if (m_entry->m_compressed)
	{
		inflateInitialize();
	}
}

ZipFsFile::~ZipFsFile()
{
	if (m_entry->m_compressed)
	{
		inflateDestroy();
	}
}

size_t ZipFsFile::write(const void *buffer, size_t elementSize, size_t elementCount)
{
	return 0;
}

size_t ZipFsFile::read(void *buffer, size_t elementSize, size_t elementCount)
{
	if (!m_entry->m_compressed)
	{
		if (m_position >= m_entry->m_size)
		{
			return 0;
		}

		if (m_filesystem->ioRead(buffer, elementSize * elementCount, (size_t)(m_entry->m_offset + m_position)))
		{
			size_t result = std::min(elementSize * elementCount, m_entry->m_size - m_position);
			m_position += elementSize * elementCount;
			if (m_position > m_entry->m_size)
			{
				m_position = m_entry->m_size;
			}
			return result;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		const size_t chunk = 1024 * 4;
		uint8_t inbuffer[chunk];
		size_t bufferOffset = 0;

		while (m_position < m_entry->m_compressedSize && bufferOffset < (elementSize * elementCount))
		{
			size_t left = m_entry->m_compressedSize - m_position;
			size_t bytes = std::min(chunk, left);
			if (bytes == 0) {
				break;
			}

			if (!m_filesystem->ioRead(inbuffer, bytes, (size_t)(m_entry->m_offset + m_position)))
			{
				error("zipfs", m_filepath, "Unable to read from filesystem file");
				return 0;
			}

			m_stream.avail_in = bytes;
			m_stream.next_in = inbuffer;

			m_stream.avail_out = (elementSize * elementCount) - bufferOffset;
			m_stream.next_out = (uint8_t *)buffer + bufferOffset;

			int ret = inflate(&m_stream, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);

			if (ret != Z_OK && ret != Z_STREAM_END)
			{
				error_f("zipfs", m_filepath, "zLib error: %s", zError(ret));
				return 0;
			}

			size_t wroteToBuffer = ((elementSize * elementCount) - bufferOffset) - m_stream.avail_out;
			bufferOffset += wroteToBuffer;
			assert(bufferOffset <= (elementSize * elementCount));
			m_position += (bytes - m_stream.avail_in);
		}
		return bufferOffset;
	}
}

size_t ZipFsFile::size() const
{
	return m_entry->m_size;
}

bool ZipFsFile::seek(uint32_t offset, Attrib attr)
{
	if (m_entry->m_compressed)
	{
		if (offset == 0 && attr == SeekSet)
		{
			if (m_position != 0)
			{
				inflateDestroy();
				inflateInitialize();
			}
			return true;
		}
		return false; // random access is not allowed
	}
	else
	{
		if (attr == SeekSet)
		{
			m_position = offset;
		}
		else if (attr == SeekCur)
		{
			m_position += offset;
		}
		else if (attr == SeekEnd)
		{
			m_position = size() - offset;
		}
		return true;
	}
}

void ZipFsFile::rewind()
{
	seek(0, SeekSet);
}

size_t ZipFsFile::tell()
{
	return 0;
}

void ZipFsFile::flush()
{
}

void ZipFsFile::inflateInitialize()
{
	m_stream.zalloc = Z_NULL;
	m_stream.zfree = Z_NULL;
	m_stream.opaque = Z_NULL;
	m_stream.avail_in = 0;
	m_stream.next_in = Z_NULL;
	if (inflateInit2(&m_stream, 0xFFFFFFF1) != Z_OK)
	{
		error("zipfs", "", "Failed to inflate init");
		return;
	}
}

void ZipFsFile::inflateDestroy()
{
	inflateEnd(&m_stream);
}

/* eof */
