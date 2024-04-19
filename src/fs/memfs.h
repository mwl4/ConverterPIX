#pragma once

#include "filesystem.h"

class MemFileSystem : public FileSystem
{
public:
    MemFileSystem();
    MemFileSystem( const MemFileSystem & ) = delete;
    MemFileSystem( MemFileSystem &&rhs ) = delete;
    virtual ~MemFileSystem();

    MemFileSystem &operator=( MemFileSystem & ) = delete;
    MemFileSystem &operator=( MemFileSystem &&rhs ) = delete;

    virtual String root() const override;
    virtual String name() const override;
    virtual UniquePtr<File> open( const String &filename, FsOpenMode mode, bool *outFileExists = nullptr ) override;
    virtual bool remove( const String &filePath ) override;
    virtual bool mkdir( const String &directory ) override;
    virtual bool rmdir( const String &directory ) override;
    virtual bool exists( const String &filename ) override;
    virtual bool dirExists( const String &dirpath ) override;
    virtual UniquePtr<List<Entry>> readDir( const String &path, bool absolutePaths, bool recursive ) override;
    virtual bool mstat( MetaStat *result, const String &path ) override;

private:
    struct StoredEntry;

private:
    StoredEntry *findEntry( const String &path ) const;
    StoredEntry *findOrCreateEntry( const String &path, bool *outOptCreated = nullptr );

    StoredEntry *findFileEntry( const String &path ) const;
    StoredEntry *findOrCreateFileEntry( const String &path, bool *outOptCreated = nullptr );

private:
    Array<UniquePtr<StoredEntry>> m_storedEntries;

    friend class MemFile;
};

struct MemFileSystem::StoredEntry
{
    bool m_isDirectory = false;
    u32 m_openedForReadCount = 0;
    bool m_openedForWrite = false;
    String m_path;
    Array<u8> m_content;
};

/* eof */
