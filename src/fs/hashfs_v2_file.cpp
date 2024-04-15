/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/fs/hashfs_v2_file.cpp
 *
 *		  _____                          _            _____ _______   __
 *		 / ____|                        | |          |  __ \_   _\ \ / /
 *		| |     ___  _ ____   _____ _ __| |_ ___ _ __| |__) || |  \ V /
 *		| |    / _ \| '_ \ \ / / _ \ '__| __/ _ \ '__|  ___/ | |   > <
 *		| |___| (_) | | | \ V /  __/ |  | ||  __/ |  | |    _| |_ / . \
 *		 \_____\___/|_| |_|\_/ \___|_|   \__\___|_|  |_|   |_____/_/ \_\
 *
 *
 *  Copyright (C) 2024 Michal Wojtowicz.
 *  All rights reserved.
 *
 *   This software is ditributed WITHOUT ANY WARRANTY; without even
 *   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE. See the copyright file for more information.
 *
 *****************************************************************************/

#include <prerequisites.h>

#include "hashfs_v2_file.h"

#include "hashfs_v2.h"

HashFsV2File::HashFsV2File( const String &filepath, HashFsV2 *filesystem, const prism::hashfs_v2_entry_t *entry, const prism::fs_meta_plain_value_t &plainMetaValues )
    : m_filepath( filepath )
    , m_filesystem( filesystem )
    , m_entry( entry )
{
    m_compression = prism::fs_meta_plain_get_compression( plainMetaValues );

    if( m_compression == prism::fs_compression_t::nocompress )
    {
        // do nothing
    }
    else if( m_compression == prism::fs_compression_t::zlib )
    {
        zlibInflateInitialize();
    }
    else if( m_compression == prism::fs_compression_t::gdeflate )
    {
        gdeflateDecompressorInitialize();
    }
    else
    {
        assert( false );
    }

    m_compressedSize = prism::fs_meta_plain_get_compressed_size( plainMetaValues );
    m_size = prism::fs_meta_plain_get_size( plainMetaValues );
    m_deviceOffset = prism::fs_meta_plain_get_offset( plainMetaValues );
}

HashFsV2File::~HashFsV2File()
{
	if( m_compression == prism::fs_compression_t::nocompress )
	{
		// do nothing
	}
	else if( m_compression == prism::fs_compression_t::zlib )
	{
		zlibInflateDestroy();
	}
	else if( m_compression == prism::fs_compression_t::gdeflate )
	{
        gdeflateDecompressorDestroy();
	}
	else
	{
		assert( false );
	}
}

uint64_t HashFsV2File::write( const void *buffer, uint64_t elementSize, uint64_t elementCount )
{
    return 0;
}

uint64_t HashFsV2File::read(void *buffer, uint64_t elementSize, uint64_t elementCount)
{
	using namespace prism;

	if( m_compression == prism::fs_compression_t::nocompress )
	{
		if (m_position >= m_size)
		{
			return 0;
		}
	
		if (m_filesystem->ioRead(buffer, elementSize * elementCount, m_deviceOffset + m_position))
		{
			uint64_t result = std::min(elementSize * elementCount, m_size - m_position);
			m_position += elementSize * elementCount;
			if (m_position > m_size )
			{
				m_position = m_size;
			}
			return result;
		}
		else
		{
			return 0;
		}
	}
    else if( m_compression == prism::fs_compression_t::zlib )
    {
        const uint64_t chunk = 1024 * 4;
        uint8_t inbuffer[ chunk ];
        uint64_t bufferOffset = 0;

        while( m_position < m_compressedSize && bufferOffset < ( elementSize * elementCount ) )
        {
            uint64_t left = m_compressedSize - m_position;
            uint64_t bytes = std::min( chunk, left );
            if( bytes == 0 )
			{
                break;
            }

            if( !m_filesystem->ioRead( inbuffer, bytes, m_deviceOffset + m_position ) )
            {
                error( "hashfs", m_filepath, "Unable to read from filesystem file" );
                return 0;
            }

            m_zlibStream->avail_in = static_cast< unsigned int >( bytes );
            m_zlibStream->next_in = inbuffer;

            m_zlibStream->avail_out = static_cast< unsigned int >( ( elementSize * elementCount ) - bufferOffset );
            m_zlibStream->next_out = ( uint8_t * )buffer + bufferOffset;

            int ret = inflate( m_zlibStream, Z_NO_FLUSH );
            assert( ret != Z_STREAM_ERROR );

            if( ret != Z_OK && ret != Z_STREAM_END )
            {
                error_f( "hashfs", m_filepath, "zLib error: %s", zError( ret ) );
                return 0;
            }

            uint64_t wroteToBuffer = ( ( elementSize * elementCount ) - bufferOffset ) - m_zlibStream->avail_out;
            bufferOffset += wroteToBuffer;
            assert( bufferOffset <= ( elementSize * elementCount ) );
            m_position += ( bytes - m_zlibStream->avail_in );
        }
        return bufferOffset;
    }
	else if( m_compression == prism::fs_compression_t::gdeflate )
	{
		assert( m_position == 0 );
		assert( elementSize * elementCount == m_size );

        Array< uint8_t > compressedBuffer;
		compressedBuffer.resize( size_t( m_compressedSize ) );
		if( !m_filesystem->ioRead( compressedBuffer.data(), m_compressedSize, m_deviceOffset ) )
		{
            error( "hashfs", m_filepath, "Unable to read from filesystem file" );
            return 0;
		}

        bool result = GDeflate::Decompress( ( uint8_t *)buffer, size_t( elementSize * elementCount ), compressedBuffer.data(), compressedBuffer.size(), 1 );
		assert( result == true );

		return elementSize * elementCount;
	}
	else
	{
		assert( false );
		return 0;
	}
}

uint64_t HashFsV2File::size()
{
	return m_size;
}

bool HashFsV2File::seek(uint64_t offset, Attrib attr)
{
	if( m_compression == prism::fs_compression_t::nocompress )
	{
        if( attr == SeekSet )
        {
            m_position = offset;
        }
        else if( attr == SeekCur )
        {
            m_position += offset;
        }
        else if( attr == SeekEnd )
        {
            m_position = size() - offset;
        }
        return true;
	}
	else if( m_compression == prism::fs_compression_t::zlib )
	{
        if( offset == 0 && attr == SeekSet )
        {
            if( m_position != 0 )
            {
				zlibInflateDestroy();
				zlibInflateInitialize();
            }
            return true;
        }
        return false; // random access is not allowed
	}
	else if( m_compression == prism::fs_compression_t::gdeflate )
	{
        if( offset == 0 && attr == SeekSet )
        {
            if( m_position != 0 )
            {
				gdeflateDecompressorDestroy();
				gdeflateDecompressorInitialize();
            }
            return true;
        }
        return false; // random access is not allowed
	}
	else
	{
		assert( false );
		return false;
	}
}

void HashFsV2File::rewind()
{
	seek(0, SeekSet);
}

uint64_t HashFsV2File::tell() const
{
	return m_position;
}

void HashFsV2File::flush()
{
}

void HashFsV2File::mstat( MetaStat *result )
{
    m_filesystem->mstatEntry( result, m_entry );
}

void HashFsV2File::zlibInflateInitialize()
{
	assert( m_zlibStream == nullptr );
    m_zlibStream = new z_stream;
    m_zlibStream->zalloc = Z_NULL;
    m_zlibStream->zfree = Z_NULL;
    m_zlibStream->opaque = Z_NULL;
    m_zlibStream->avail_in = 0;
    m_zlibStream->next_in = Z_NULL;
    if( inflateInit( m_zlibStream ) != Z_OK )
    {
        error( "hashfs_v2", "", "Failed to inflate init" );
        return;
    }
}

void HashFsV2File::zlibInflateDestroy()
{
	if( m_zlibStream )
	{
        inflateEnd( m_zlibStream );
        delete m_zlibStream;
        m_zlibStream = nullptr;
	}
}





/* eof */
