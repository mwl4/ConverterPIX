#include "prerequisites.h"

#include "memfs_file.h"

MemFile::MemFile( MemFileSystem::StoredEntry *entry, bool openedForRead )
    : m_entry( entry )
    , m_openedForRead( openedForRead )
{
    if( m_openedForRead )
    {
        ++m_entry->m_openedForReadCount;
    }
    else
    {
        m_entry->m_openedForWrite = true;
    }
}

MemFile::MemFile() = default;

MemFile::~MemFile()
{
    if( m_entry )
    {
        if( m_openedForRead )
        {
            --m_entry->m_openedForReadCount;
        }
        else
        {
            m_entry->m_openedForWrite = false;
        }
        m_entry = nullptr;
    }
}

uint64_t MemFile::write( const void *buffer, uint64_t elementSize, uint64_t elementCount )
{
    Array<u8> &content = getContent();
    const size_t bytesToWrite = static_cast<size_t>( elementSize * elementCount );
    const size_t offsetToWrite = content.size();
    content.resize( offsetToWrite + bytesToWrite );
    memcpy( content.data() + offsetToWrite, buffer, bytesToWrite );
    return bytesToWrite;
}

uint64_t MemFile::read( void *buffer, uint64_t elementSize, uint64_t elementCount )
{
    Array<u8> &content = getContent();
    const size_t bytesToRead = static_cast<size_t>( elementSize * elementCount );
    const size_t bytesLeftInContent = content.size() - static_cast<size_t>( m_readPosition );
    const size_t bytesActuallyRead = std::min( bytesToRead, bytesLeftInContent );
    memcpy( buffer, content.data() + m_readPosition, bytesActuallyRead );
    m_readPosition += bytesActuallyRead;
    return bytesActuallyRead;
}

uint64_t MemFile::size()
{
    return getContent().size();
}

bool MemFile::seek( uint64_t offset, Attrib attr )
{
    switch( attr )
    {
        case SeekCur:
        {
            if( m_readPosition + offset <= getContent().size() )
            {
                m_readPosition += offset;
                return true;
            }
            else return false;
        }
        case SeekSet:
        {
            if( offset <= getContent().size() )
            {
                m_readPosition = offset;
                return true;
            }
            else return false;
        }
        case SeekEnd:
        {
            if( offset <= getContent().size() )
            {
                m_readPosition = getContent().size() - offset;
                return true;
            }
            else return false;
        }
    }
    return false;
}

void MemFile::rewind()
{
    m_readPosition = 0;
}

uint64_t MemFile::tell() const
{
    return m_readPosition;
}

void MemFile::flush()
{
}

void MemFile::mstat( MetaStat *result )
{
}

/* eof */
