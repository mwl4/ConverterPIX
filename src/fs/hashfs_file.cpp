/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : hashfs_file.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "hashfs_file.h"

#include "hashfilesystem.h"

HashFsFile::HashFsFile(const String &filepath, HashFileSystem *filesystem, const prism::hashfs_entry_t *header)
	: m_filepath(filepath)
	, m_filesystem(filesystem)
	, m_header(header)
	, m_position(0)
{
	using namespace prism;

	if (m_header->m_flags & HASHFS_COMPRESSED)
	{
		inflateInitialize();
	}
}

HashFsFile::~HashFsFile()
{
	using namespace prism;

	if (m_header->m_flags & HASHFS_COMPRESSED)
	{
		inflateDestroy();
	}
}

size_t HashFsFile::write(const void *buffer, size_t elementSize, size_t elementCount)
{
	return 0;
}

size_t HashFsFile::read(void *buffer, size_t elementSize, size_t elementCount)
{
	using namespace prism;

	if (!(m_header->m_flags & HASHFS_COMPRESSED))
	{
		if (m_position >= m_header->m_size)
		{
			return 0;
		}

		if (m_filesystem->ioRead(buffer, elementSize * elementCount, (size_t)(m_header->m_offset + m_position)))
		{
			size_t result = std::min(elementSize * elementCount, m_header->m_size - m_position);
			m_position += elementSize * elementCount;
			if (m_position > m_header->m_size)
			{
				m_position = m_header->m_size;
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

		while (m_position < m_header->m_compressed_size && bufferOffset < (elementSize * elementCount))
		{
			size_t left = m_header->m_compressed_size - m_position;
			size_t bytes = std::min(chunk, left);
			if (bytes == 0) {
				break;
			}

			if (!m_filesystem->ioRead(inbuffer, bytes, (size_t)(m_header->m_offset + m_position)))
			{
				error("hashfs", m_filepath, "Unable to read from filesystem file");
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
				error_f("hashfs", m_filepath, "zLib error: %s", zError(ret));
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

size_t HashFsFile::size() const
{
	return m_header->m_size;
}

bool HashFsFile::seek(uint32_t offset, Attrib attr)
{
	if (m_header->m_flags & prism::HASHFS_COMPRESSED)
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

void HashFsFile::rewind()
{
	seek(0, SeekSet);
}

size_t HashFsFile::tell()
{
	return 0;
}

void HashFsFile::flush()
{
}

void HashFsFile::inflateInitialize()
{
	m_stream.zalloc = Z_NULL;
	m_stream.zfree = Z_NULL;
	m_stream.opaque = Z_NULL;
	m_stream.avail_in = 0;
	m_stream.next_in = Z_NULL;
	if (inflateInit(&m_stream) != Z_OK)
	{
		error("hashfs", "", "Failed to inflate init");
		return;
	}
}

void HashFsFile::inflateDestroy()
{
	inflateEnd(&m_stream);
}

/* eof */
