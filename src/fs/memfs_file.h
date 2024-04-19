#pragma once

#include "file.h"
#include "memfs.h"

class MemFile : public File
{
public:
    MemFile();
    MemFile( MemFileSystem::StoredEntry *entry, bool openedForRead );
    MemFile( const MemFile & ) = delete;
    MemFile( MemFile && ) = delete;
    ~MemFile();

    MemFile &operator=( const MemFile & ) = delete;
    MemFile &operator=( MemFile && ) = delete;

    virtual uint64_t write( const void *buffer, uint64_t elementSize, uint64_t elementCount ) override;
    virtual uint64_t read( void *buffer, uint64_t elementSize, uint64_t elementCount ) override;
    virtual uint64_t size() override;
    virtual bool seek( uint64_t offset, Attrib attr ) override;
    virtual void rewind() override;
    virtual uint64_t tell() const override;
    virtual void flush() override;
    virtual void mstat( MetaStat *result ) override;

    Array<u8> &getContent() { return m_entry ? m_entry->m_content : m_content; }

private:
    uint64_t m_readPosition = 0;

    // When used with MemFileSystem::open
    MemFileSystem::StoredEntry *m_entry = nullptr;
    bool m_openedForRead = false;

    // When used alone
    Array<u8> m_content;
};

/* eof */
