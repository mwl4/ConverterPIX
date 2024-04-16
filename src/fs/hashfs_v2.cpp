/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/fs/hashfilesystem_v2.cpp
 *
 *		  _____                          _            _____ _______   __
 *		 / ____|                        | |          |  __ \_   _\ \ / /
 *		| |     ___  _ ____   _____ _ __| |_ ___ _ __| |__) || |  \ V /
 *		| |    / _ \| '_ \ \ / / _ \ '__| __/ _ \ '__|  ___/ | |   > <
 *		| |___| (_) | | | \ V /  __/ |  | ||  __/ |  | |    _| |_ / . \
 *		 \_____\___/|_| |_|\_/ \___|_|   \__\___|_|  |_|   |_____/_/ \_\
 *
 *
 *  Copyright (C) 2017 Michal Wojtowicz.
 *  All rights reserved.
 *
 *   This software is ditributed WITHOUT ANY WARRANTY; without even
 *   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE. See the copyright file for more information.
 *
 *****************************************************************************/

#include <prerequisites.h>

#include "hashfs_v2.h"

#include "hashfs_v2_file.h"
#include "sysfilesystem.h"
#include "file.h"

#include "utils/string_tokenizer.h"
#include "utils/compression.h"
#include "utils/token.h"

HashFsV2::HashFsV2( const String &root )
{
	m_rootFilename = root;
	m_root = getSFS()->open( root, FileSystem::read | FileSystem::binary );
	if( !m_root )
	{
		error( "hashfs_v2", root, "Unable to open root file" );
		return;
	}
	if( !readHashFS() )
	{
		assert( false );
	}
}

HashFsV2::~HashFsV2() = default;

String HashFsV2::root() const
{
	return m_rootFilename;
}

String HashFsV2::name() const
{
	return "hashfs_v2";
}

UniquePtr<File> HashFsV2::open( const String &filename, FsOpenMode mode )
{
	prism::hashfs_v2_entry_t *const entry = findEntry( filename );
	if( !entry )
	{
		return nullptr;
	}

	//if (entry->m_flags & HASHFS_ENCRYPTED)
	//{
	//	error("hashfs", filename, "Encrypted files are not supported!");
	//	return UniquePtr<File>();
	//}

	const prism::hashfs_v2_meta_t meta = !!( entry->m_flags & prism::hashfs_v2_entry_flags_t::directory ) ? prism::hashfs_v2_meta_t::directory : prism::hashfs_v2_meta_t::plain;
	const u32 *const plainMetadata = findMetadata( entry, meta );

	if( plainMetadata == nullptr )
	{
		return nullptr;
	}

	prism::fs_meta_plain_t plainMetaValues = { 0 };
	prism::hashfs_v2_meta_plain_get_value( plainMetadata, plainMetaValues );

	return std::make_unique<HashFsV2File>( filename, this, entry, plainMetaValues );
}

bool HashFsV2::mkdir( const String &directory )
{
	return false;
}

bool HashFsV2::rmdir( const String &directory )
{
	return false;
}

bool HashFsV2::exists( const String &filename )
{
	if( filename.empty() )
	{
		return false;
	}

	prism::hashfs_v2_entry_t *const entry = findEntry( filename );
	if( !entry )
	{
		return false;
	}

	if( !!( entry->m_flags & prism::hashfs_v2_entry_flags_t::directory ) )
	{
		return false;
	}

	return true;
}

bool HashFsV2::dirExists( const String &dirpath )
{
	if( dirpath.empty() )
	{
		return false;
	}

	prism::hashfs_v2_entry_t *const entry = findEntry( dirpath.size() != 1 ? removeSlashAtEnd( dirpath ) : dirpath );
	if( !entry )
	{
		return false;
	}

	if( !( entry->m_flags & prism::hashfs_v2_entry_flags_t::directory ) )
	{
		return false;
	}

	return true;
}

auto HashFsV2::readDir( const String &path, bool absolutePaths, bool recursive ) -> UniquePtr<List<Entry>>
{
	if( path.empty() )
	{
		error( "hashfs_v2", m_rootFilename, "readDir: Path is empty!" );
		return nullptr;
	}

	String dirpath = path.size() != 1 ? removeSlashAtEnd( path ) : path;

	prism::hashfs_v2_entry_t *const entry = findEntry( dirpath );
	if( entry == nullptr )
	{
		error_f( "hashfs_v2", m_rootFilename, "Failed to open dirlist entry (%s)!", path );
		return nullptr;
	}

	if( !( entry->m_flags & prism::hashfs_v2_entry_flags_t::directory ) )
	{
		error_f( "hashfs_v2", m_rootFilename, "Entry is not directory!" );
		return nullptr;
	}

	const u32 *const plainMetadata = findMetadata( entry, prism::hashfs_v2_meta_t::directory );
	if( plainMetadata == nullptr )
	{
		error_f( "hashfs_v2", m_rootFilename, "Entry metadata is not available!" );
		return nullptr;
	}

	prism::fs_meta_plain_t plainMetaValues = { 0 };
	prism::hashfs_v2_meta_plain_get_value( plainMetadata, plainMetaValues );

	HashFsV2File directoryFile( path, this, entry, plainMetaValues );

	Array<u8> buffer( size_t( directoryFile.size() ) );
	if( !directoryFile.blockRead( buffer.data(), 0, buffer.size() ) )
	{
		error_f( "hashfs_v2", m_rootFilename, "Entry could not be read!" );
		return nullptr;
	}

	const uint32_t countOfItems = interpretBufferAt<uint32_t>( buffer, 0 );

	auto result = std::make_unique<List<Entry>>();

	uint32_t currentLengthOffset = sizeof( uint32_t );
	uint32_t currentStringOffset = currentLengthOffset + countOfItems * sizeof( uint8_t );

	for( uint32_t i = 0; i < countOfItems; ++i )
	{
		const uint8_t pathLength = interpretBufferAt<uint8_t>( buffer, currentLengthOffset );
		currentLengthOffset += sizeof( uint8_t );

		const String path( &interpretBufferAt<char>( buffer, currentStringOffset, pathLength ), size_t( pathLength ) );
		currentStringOffset += pathLength;

		if( path[ 0 ] == '/' ) // directory
		{
			String directorypath;
			if( absolutePaths )
			{
				directorypath = removeSlashAtEnd( dirpath ) + "/" + String( path.c_str() + 1 );
			}
			else
			{
				directorypath = String( path.c_str() + 1 );
			}
			result->push_back( Entry( directorypath, true, false, this ) );

			if( recursive )
			{
				auto subdir = readDir( directorypath, absolutePaths, recursive );
				if( subdir )
				{
					result->insert( result->end(), subdir->begin(), subdir->end() );
				}
			}
		}
		else
		{
			String filepath;
			if( absolutePaths )
			{
				filepath = removeSlashAtEnd( dirpath ) + "/" + path.c_str();
			}
			else
			{
				filepath = path;
			}

			bool encrypted = false;
			prism::hashfs_v2_entry_t *const entry = findEntry( removeSlashAtEnd( dirpath ) + "/" + path.c_str() );
			if( entry )
			{
				//encrypted = !!(entry->m_flags & HASHFS_ENCRYPTED);
			}
			result->push_back( Entry( filepath, false, encrypted, this ) );
		}
	}

	return result;
}

bool HashFsV2::mstat( MetaStat *result, const String &path )
{
	prism::hashfs_v2_entry_t *const entry = findEntry( path );
	if( !entry )
	{
		return false;
	}

	result->m_filesystem = this;
	mstatEntry( result, entry );
	return true;
}

UniquePtr<File> HashFsV2::openForReadingWithPlainMeta( const String &filename, const prism::fs_meta_plain_t &plainMetaValues )
{
	prism::hashfs_v2_entry_t *const entry = findEntry( filename );
	if( !entry )
	{
		return nullptr;
	}

	return std::make_unique<HashFsV2File>( filename, this, entry, plainMetaValues );
}

void HashFsV2::mstatEntry( MetaStat *result, const prism::hashfs_v2_entry_t *entry )
{
	walkMetadata( entry, [ result ]( prism::hashfs_v2_meta_t meta, const u32 *metadata )
	{
		MetaStat::Meta metaToAdd;
		if( meta == prism::hashfs_v2_meta_t::img )
		{
			prism::fs_meta_img_t value;
			prism::hashfs_v2_meta_img_get_value( metadata, value );
			metaToAdd.setValue( value );
		}
		else if( meta == prism::hashfs_v2_meta_t::sample )
		{
			prism::fs_meta_sample_t value;
			prism::hashfs_v2_meta_sample_get_value( metadata, value );
			metaToAdd.setValue( value );
		}
		else if( meta == prism::hashfs_v2_meta_t::mipproxy || meta == prism::hashfs_v2_meta_t::inline_directory || meta == prism::hashfs_v2_meta_t::directory )
		{
			return; // skip it
		}
		else if( !!( meta & prism::hashfs_v2_meta_t::plain ) )
		{
			prism::fs_meta_plain_t value;
			prism::hashfs_v2_meta_plain_get_value( metadata, value );
			metaToAdd.setValue( value );
		}
		else
		{
			assert( false );
		}
		metaToAdd.m_name = getMetaTokenName( meta );
		result->m_meta.push_back( metaToAdd );
	} );
}

bool HashFsV2::ioRead( void *const buffer, uint64_t bytes, uint64_t offset )
{
	return m_root->blockRead( buffer, offset, bytes );
}

bool HashFsV2::readHashFS()
{
	if( !m_root->blockRead( &m_header, 0, sizeof( prism::hashfs_v2_header_t ) ) )
	{
		error( "hashfs_v2", m_rootFilename, "Failed to read header!" );
		return false;
	}

	if( m_header.m_version != prism::hashfs_v2_header_t::SUPPORTED_VERSION )
	{
		error_f( "hashfs_v2", m_rootFilename, "Unsupported version (%u)", m_header.m_version );
		return false;
	}

	if( m_header.m_hash_method != MAKEFOURCC( 'C', 'I', 'T', 'Y' ) )
	{
		error_f( "hashfs_v2", m_rootFilename, "Unsupported hash method (%08X)", m_header.m_hash_method );
		return false;
	}

	m_entryTable.resize( m_header.m_entry_table_count );

	const u32 entryTableSize = m_header.m_entry_table_count * sizeof( prism::hashfs_v2_entry_t );

	static_assert( std::is_same_v< decltype( m_entryTable )::value_type, prism::hashfs_v2_entry_t>, "" ); // following code assumes that m_entryTable is simple container for entries

	if( entryTableSize == m_header.m_entry_table_compressed_size ) // entry table is not compressed
	{
		if( !m_root->blockRead( m_entryTable.data(), m_header.m_entry_table_offset, entryTableSize ) )
		{
			error( "hashfs_v2", m_rootFilename, "Failed to read entry table!" );
			return false;
		}
	}
	else
	{
		Array<u8> compressedEntryTable( size_t( m_header.m_entry_table_compressed_size ) );
		if( !m_root->blockRead( compressedEntryTable.data(), m_header.m_entry_table_offset, compressedEntryTable.size() ) )
		{
			error( "hashfs_v2", m_rootFilename, "Failed to read entry table!" );
			return false;
		}
		if( !unCompress_zlib( m_entryTable.data(), m_entryTable.size() * sizeof( prism::hashfs_v2_entry_t ), compressedEntryTable.data(), compressedEntryTable.size() ) )
		{
			error( "hashfs_v2", m_rootFilename, "Failed to uncompress entry table!" );
			return false;
		}
	}

	m_metadataTable.resize( m_header.m_metadata_table_count );

	const u32 metadataTableSize = m_header.m_metadata_table_count * sizeof( u32 );

	if( metadataTableSize == m_header.m_metadata_table_compressed_size )
	{
		if( !m_root->blockRead( m_metadataTable.data(), m_header.m_metadata_table_offset, m_metadataTable.size() ) )
		{
			error( "hashfs_v2", m_rootFilename, "Failed to read metadata table!" );
			return false;
		}
	}
	else
	{
		Array<u8> compressedMetadataTable( static_cast<size_t>( m_header.m_metadata_table_compressed_size ) );
		if( !m_root->blockRead( compressedMetadataTable.data(), m_header.m_metadata_table_offset, compressedMetadataTable.size() ) )
		{
			error( "hashfs_v2", m_rootFilename, "Failed to read metadata table!" );
			return false;
		}
		if( !unCompress_zlib( m_metadataTable.data(), m_metadataTable.size() * sizeof( u32 ), compressedMetadataTable.data(), compressedMetadataTable.size() ) )
		{
			error( "hashfs_v2", m_rootFilename, "Failed to uncompress metadata table!" );
			return false;
		}
	}

	return true;
}

const u32 *HashFsV2::findMetadata( const prism::hashfs_v2_entry_t *entry, prism::hashfs_v2_meta_t meta )
{
	const u32 metadataIndex = entry->m_metadata_index;
	for( u32 i = 0; i < entry->m_metadata_count; ++i )
	{
		const u32 metadata = m_metadataTable.at( metadataIndex + i );
		if( static_cast< prism::hashfs_v2_meta_t >( static_cast< u8 >( metadata >> 24 ) ) == meta )
		{
			return &m_metadataTable.at( metadata & 0xFFFFFF );
		}
	}
	return nullptr;
}

void HashFsV2::walkMetadata( const prism::hashfs_v2_entry_t *entry, std::function< void( prism::hashfs_v2_meta_t meta, const u32 *metadata ) > f )
{
	const u32 metadataIndex = entry->m_metadata_index;
	for( u32 i = 0; i < entry->m_metadata_count; ++i )
	{
		const u32 metadata = m_metadataTable.at( metadataIndex + i );
		const prism::hashfs_v2_meta_t meta = static_cast< prism::hashfs_v2_meta_t >( static_cast< u8 >( metadata >> 24 ) );
		f( meta, &m_metadataTable.at( metadata & 0xFFFFFF ) );
	}
}

prism::hashfs_v2_entry_t *HashFsV2::findEntry( const String &path )
{
	if( m_entryTable.empty() )
	{
		return nullptr;
	}

	String pathToFind;
	if( m_header.m_salt != 0 )
	{
		pathToFind = fmt::sprintf( "%u%s", m_header.m_salt, ( path.c_str() + 1 ) );
	}
	else
	{
		pathToFind = path.c_str() + 1;
	}

	const u64 hash = prism::city_hash_64( pathToFind.c_str(), pathToFind.length() );

	for( s64 index, l = 0, r = m_entryTable.size() - 1; l <= r;) // binary search
	{
		index = l + ( r - l ) / 2;

		if( m_entryTable[ static_cast< size_t >( index ) ].m_hash == hash )
		{
			return &m_entryTable[ static_cast< size_t >( index ) ];
		}

		if( m_entryTable[ static_cast< size_t >( index ) ].m_hash < hash )
		{
			l = index + 1;
		}
		else
		{
			r = index - 1;
		}
	}

	return nullptr;
}

prism::token_t HashFsV2::getMetaTokenName( prism::hashfs_v2_meta_t meta )
{
	switch( meta )
	{
	case prism::hashfs_v2_meta_t::img:		return tn( "img" );
	case prism::hashfs_v2_meta_t::sample:	return tn( "sample" );
	case prism::hashfs_v2_meta_t::mipproxy:	return tn( "mipproxy" );
	case prism::hashfs_v2_meta_t::plain:	return tn( "plain" );
	case prism::hashfs_v2_meta_t::mip0:		return tn( "mip0" );
	case prism::hashfs_v2_meta_t::mip1:		return tn( "mip1" );
	case prism::hashfs_v2_meta_t::miptail:	return tn( "miptail" );
	default:
		assert( false );
		return prism::token_t();
	}
}

/* eof */
