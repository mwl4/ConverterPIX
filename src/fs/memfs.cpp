#include "prerequisites.h"

#include "memfs.h"

#include "memfs_file.h"

MemFileSystem::MemFileSystem() = default;

MemFileSystem::~MemFileSystem() = default;

String MemFileSystem::root() const
{
    return fmt::sprintf( "memfs_%p", ( void* )this );
}

String MemFileSystem::name() const
{
    return "memfs";
}

UniquePtr<File> MemFileSystem::open( const String &filename, FsOpenMode mode )
{
    assert( mode & binary ); // only binary mode is supported now
    assert( !( mode & update ) ); // update mode is not supported

    if( mode & read )
    {
        if( StoredEntry *const entry = findFileEntry( filename ) )
        {
            if( entry->m_openedForWrite == false )
            {
                return std::make_unique<MemFile>( entry, true );
            }
        }
    }
    else if( mode & write )
    {
        bool created = false;
        if( StoredEntry *const entry = findOrCreateFileEntry( filename, &created ) )
        {
            if( entry->m_openedForReadCount == 0 && entry->m_openedForWrite == false )
            {
                if( created == false ) // clear file content if file was already there
                {
                    entry->m_content.clear();
                    entry->m_content.shrink_to_fit();
                }
                return std::make_unique<MemFile>( entry, false );
            }
        }
    }
    else if( mode & append )
    {
        if( StoredEntry *const entry = findFileEntry( filename ) )
        {
            if( entry->m_openedForReadCount == 0 && entry->m_openedForWrite == false )
            {
                return std::make_unique<MemFile>( entry, false );
            }
        }
    }
    return nullptr;
}

bool MemFileSystem::mkdir( const String &directory )
{
    assert( false ); // not supported
    return false;
}

bool MemFileSystem::rmdir( const String &directory )
{
    assert( false ); // not supported
    return false;
}

bool MemFileSystem::exists( const String &filename )
{
    if( StoredEntry *const entry = findEntry( filename ) )
    {
        return entry->m_isDirectory == false;
    }
    else return false;
}

bool MemFileSystem::dirExists( const String &dirpath )
{
    if( StoredEntry *const entry = findEntry( dirpath ) )
    {
        return entry->m_isDirectory == true;
    }
    else return false;
}

auto MemFileSystem::readDir( const String &path, bool absolutePaths, bool recursive ) -> UniquePtr<List<Entry>>
{
    assert( false ); // not supported
    return nullptr;
}

bool MemFileSystem::mstat( MetaStat *result, const String &path )
{
    if( findEntry( path ) )
    {
        result->m_filesystem = this;
        return true;
    }
    else return false;
}

auto MemFileSystem::findEntry( const String &path ) const -> StoredEntry *
{
    for( const UniquePtr<StoredEntry> &entry : m_storedEntries )
    {
        if( entry->m_path == path )
        {
            return entry.get();
        }
    }
    return nullptr;
}

auto MemFileSystem::findOrCreateEntry( const String &path, bool *outOptCreated ) -> StoredEntry *
{
    if( StoredEntry *const entry = findEntry( path ) )
    {
        return entry;
    }

    UniquePtr<StoredEntry> entry = std::make_unique<StoredEntry>();
    StoredEntry *const entryPointer = entry.get();
    entry->m_path = path;
    m_storedEntries.push_back( std::move( entry ) );
    if( outOptCreated ) *outOptCreated = true;
    return entryPointer;
}

auto MemFileSystem::findFileEntry( const String &path ) const -> StoredEntry *
{
    if( StoredEntry *const entry = findEntry( path ) )
    {
        if( entry->m_isDirectory == false )
        {
            return entry;
        }
    }
    return false;
}

auto MemFileSystem::findOrCreateFileEntry( const String &path, bool *outOptCreated ) -> StoredEntry *
{
    bool createdEntry = false;
    if( StoredEntry *const entry = findOrCreateEntry( path, &createdEntry ) )
    {
        if( createdEntry )
        {
            entry->m_isDirectory = false;
            if( outOptCreated ) *outOptCreated = true;
            return entry;
        }
        else
        {
            if( entry->m_isDirectory == false )
            {
                return entry;
            }
        }
    }
    return false;
}

/* eof */
