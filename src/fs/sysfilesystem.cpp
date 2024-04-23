/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/fs/sysfilesystem.cpp
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

#include "sysfilesystem.h"

#include "sysfs_file.h"

#include "utils/string_utils.h"

SysFileSystem::SysFileSystem( const String &root )
	: m_root( root )
{
	if( !m_root.empty() )
	{
		m_root = makeSlashAtEnd( m_root );
	}
}

SysFileSystem::~SysFileSystem() = default;

String SysFileSystem::root() const
{
	return m_root;
}

String SysFileSystem::name() const
{
	return "sysfs";
}

UniquePtr<File> SysFileSystem::open( const String &filePath, FsOpenMode mode, bool *outFileExists )
{
	const String builtFilePath = buildPath( filePath );

	if( outFileExists )
	{
		if( fileExistsStatic( builtFilePath ) )
		{
			*outFileExists = true;
		}
	}

	const String smode =
		String( mode & read ? "r" : "" )
		+ ( mode & write ? "w" : "" )
		+ ( mode & append ? "a" : "" )
		+ ( mode & binary ? "b" : "" )
		+ ( mode & update ? "+" : "" );

	FILE *fp = fopen( builtFilePath.c_str(), smode.c_str() );
	if( ( mode & write ) && fp == nullptr )
	{
		if( !dirExists( directory( filePath ) ) )
		{
			if( mkdir( directory( filePath ) ) )
			{
				fp = fopen( builtFilePath.c_str(), smode.c_str() );
				if( fp == nullptr )
				{
					return nullptr;
				}
			}
			else
			{
				return nullptr;
			}
		}
		else
		{
			return nullptr;
		}
	}

	auto file = std::make_unique<SysFsFile>();
	file->m_fp = fp;
	fseek( file->m_fp, 0, SEEK_SET );
	return std::move( file );
}

bool SysFileSystem::remove( const String &filePath )
{
	return std::remove( buildPath( filePath ).c_str() ) == 0;
}

bool SysFileSystem::mkdir(const String &dir)
{
	String dirr( buildPath( dir ) );
	std::replace_if( dirr.begin(), dirr.end(), []( char ch )->bool { return ch == '\\'; }, '/' );

	if( dirExistsStatic( dirr.c_str() ) )
	{
		return true;
	}
	dirr += '/';
	for( size_t pos = dirr.find( '/', 0 ); pos != -1; pos = dirr.find( '/', pos + 1 ) )
	{
		if( !dirExistsStatic( dirr.substr( 0, pos ).c_str() ) )
		{
		#ifdef _WIN32
			if( ::mkdir( dirr.substr( 0, pos ).c_str() ) != 0 )
				return false;
		#else
			if( ::mkdir( dirr.substr( 0, pos ).c_str(), 0775 ) != 0 )
				return false;
		#endif
		}
	}
	return true;
}

bool SysFileSystem::rmdir(const String &directory)
{
	return false;
}

bool SysFileSystem::exists( const String &filename )
{
	return fileExistsStatic( buildPath( filename ) );
}

bool SysFileSystem::dirExists( const String &dirpath )
{
	return dirExistsStatic( buildPath( dirpath ) );
}

auto SysFileSystem::readDir(const String &directory, bool absolutePaths, bool recursive) -> UniquePtr<List<Entry>>
{
	const String directoryNoSlash = trimSlashesAtEnd( directory );

#ifdef _WIN32
	HANDLE dir;
	WIN32_FIND_DATA fileData;

	if ((dir = FindFirstFileA((buildPath(directoryNoSlash) + "/*").c_str(), &fileData)) == INVALID_HANDLE_VALUE)
		return UniquePtr<List<Entry>>();

	auto result = std::make_unique<List<Entry>>();
	do
	{
		const String fileName = fileData.cFileName;
		const String fullFileName = directoryNoSlash + "/" + fileName;

		if (fileName[0] == '.')
			continue;

		const bool isDirectory = !!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		if (isDirectory)
		{
			if (recursive)
			{
				auto subdir = readDir(fullFileName, absolutePaths, recursive);
				if (subdir)
				{
					result->insert(result->end(), subdir->begin(), subdir->end());
				}
			}
		}
		result->push_back(Entry((absolutePaths ? fullFileName : fileName), isDirectory, false, this));
	} while (FindNextFileA(dir, &fileData));
	FindClose(dir);
	return result;
#else
	auto result = std::make_unique<List<Entry>>();
	DIR *dir;
	struct dirent *ent;
	struct stat st;

	dir = opendir(buildPath(directoryNoSlash).c_str());
	while ((ent = readdir(dir)) != 0)
	{
		const String fileName = ent->d_name;
		const String fullFileName = directoryNoSlash + "/" + fileName;

		if (fileName[0] == '.')
			continue;

		if (stat(buildPath(fullFileName).c_str(), &st) == -1)
			continue;

		const bool isDirectory = !!(st.st_mode & S_IFDIR);
		if (isDirectory)
		{
			if (recursive)
			{
				auto subdir = readDir(fullFileName, absolutePaths, recursive);
				if (subdir)
				{
					result->insert(result->begin(), subdir->begin(), subdir->end());
				}
			}
		}
		result->push_back(Entry((absolutePaths ? fullFileName : fileName), isDirectory, false, this));
	}
	closedir(dir);
	return result;
#endif
}

bool SysFileSystem::mstat( MetaStat *result, const String &path )
{
	// We must return true if such path exists in the filesystem
	if( exists( path ) || dirExists( path ) )
	{
		result->m_filesystem = this;
		return true;
	}
	else return false;
}

String SysFileSystem::getError() const
{
	return strerror(errno);
}

/**
 * Be aware that filePath might be not absolute path.
 * Potentially, it can be absolute path, like:
 *   (on linux) /home/some_directory/some_file
 *   (on windows) C:\some_directory\some_file
 * But it can be also relative path to current directory (cd):
 *   some_directory/some_file
 */
bool SysFileSystem::fileExistsStatic( const String &builtFilePath )
{
	String filePathFinal = builtFilePath;
#ifdef _WIN32
	std::replace_if( filePathFinal.begin(), filePathFinal.end(), []( char ch )->bool { return ch == '/'; }, '\\' );
	const DWORD attr = GetFileAttributesA( builtFilePath.c_str() );
	return attr != INVALID_FILE_ATTRIBUTES && !( attr & FILE_ATTRIBUTE_DIRECTORY );
#else
	std::replace_if( filePathFinal.begin(), filePathFinal.end(), []( char ch )->bool { return ch == '\\'; }, '/' );
    struct stat buffer;
    return ( stat( filePathFinal.c_str(), &buffer ) == 0 && ( ( buffer.st_mode & S_IFDIR ) == 0 ) );
#endif
}

/**
 * Be aware that dirPath might be not absolute path.
 * Potentially, it can be absolute path, like:
 *   (on linux) /home/some_directory/some_file
 *   (on windows) C:\some_directory\some_file
 * But it can be also relative path to current directory (cd):
 *   some_directory/some_file
 */
bool SysFileSystem::dirExistsStatic( const String &builtDirPath )
{
    String dirPathFinal = builtDirPath;
#ifdef _WIN32
	std::replace_if( dirPathFinal.begin(), dirPathFinal.end(), []( char ch )->bool { return ch == '/'; }, '\\' );
	const DWORD attr = GetFileAttributesA( dirPathFinal.c_str() );
	return attr != INVALID_FILE_ATTRIBUTES && !!( attr & FILE_ATTRIBUTE_DIRECTORY );
#else
    std::replace_if( dirPathFinal.begin(), dirPathFinal.end(), []( char ch )->bool { return ch == '\\'; }, '/' );
    struct stat buffer;
    return ( stat( builtDirPath.c_str(), &buffer ) == 0 && ( ( buffer.st_mode & S_IFDIR ) != 0 ) );
#endif
}

String SysFileSystem::buildPath( const String &path ) const
{
	// "" +	"some_file"  => "some_file"
	// "" + "/some_file" => "/some_file"
	// "" + "C:/some_file" => "C:/some_file"

    // Special handling of global sys fs
    if( m_root.empty() )
    {
        return path;
    }

	// "/" + "some_file"				=> "/some_file"
	// "/" + "/some_file"				=> "/some_file"
	// "some_dir/" + "some_file"		=> "some_dir/some_file"
	// "some_dir/" + "/some_file"		=> "some_dir/some_file"
	// "C:/some_dir/" + "some_file"		=> "C:/some_dir/some_file"
	// "C:/some_dir/" + "/some_file"	=> "C:/some_dir/some_file"
	return m_root + trimSlashesAtBegin( path );
}

/* eof */
