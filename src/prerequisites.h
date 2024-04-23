/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/prerequisites.h
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

#pragma once

//
/// C headers
//
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <cctype>
#include <cassert>
#include <cstdarg>
#include <cwchar>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#include <Windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

//
/// C++ headers
//
#include <vector>
#include <list>
#include <algorithm>
#include <string>
#include <memory>
#include <fstream>
#include <map>
#include <array>
#include <sstream>
#include <unordered_map>
#include <optional>
#include <functional>
#include <type_traits>

//
/// Utils
//
#include "utils/types.h"

#include <version.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <fmt/format.h>
#include <fmt/printf.h>
#include <fmt/posix.h>

#include <zlib/zlib.h>

using String		= std::string;
using WString		= std::wstring;

using StringStream	= std::stringstream;

template < typename T >
using Array			= std::vector<T>;

template < typename T, size_t N >
using SizedArray	= std::array<T, N>;

template < typename T >
using List			= std::list<T>;

template < typename KEY_TYPE, typename VALUE_TYPE >
using Map			= std::map<KEY_TYPE, VALUE_TYPE>;

template < typename KEY_TYPE, typename VALUE_TYPE >
using UnorderedMap	= std::unordered_map<KEY_TYPE, VALUE_TYPE>;

template < typename T >
using SharedPtr		= std::shared_ptr<T>;

template < typename T >
using UniquePtr		= std::unique_ptr<T>;

template < typename FIRST, typename SECOND >
using Pair			= std::pair<FIRST, SECOND>;

template< typename T >
using Optional		= std::optional< T >;

#include <callbacks.h>

#define TAB		"     "
#define SEOL	"\n"
#define FLT_FT	"&%08x"
#define DBL_FT	"&%016x"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define ENABLE_FLAG(variable, flag)		variable |= flag
#define DISABLE_FLAG(variable, flag)	variable &= ~flag
#define CHECK_FLAG(variable, flag)		(variable & flag)

#define STRINGIZE_DETAIL(x)				#x
#define STRINGIZE(x)					STRINGIZE_DETAIL(x)
#define CONCAT2(prefix, suffix)			prefix##suffix
#define CONCAT(prefix, suffix)			CONCAT2(prefix, suffix)
#define STATIC_CHECK(expr)				typedef int CONCAT(some_requirement_failed_at_, __LINE__)[(expr) ? 1 : -1]

#define pad(size)		uint8_t CONCAT(pad_, __LINE__)[size]
#define pad_bool(name)	uint8_t pad_bool_##name[3]

#define bool32(name)	scs_bool name; pad_bool(name)

#define ENSURE_SIZE(structure, expected) static_assert(sizeof(structure) == expected, "Invalid size")
#define COMMA ,

#define CP_ENUM_CLASS_BITFIELD( enumClass ) \
	inline enumClass operator&( enumClass a, enumClass b ) { return enumClass( static_cast<std::underlying_type_t<enumClass>>( a ) & static_cast<std::underlying_type_t<enumClass>>( b ) ); } \
	inline enumClass operator|( enumClass a, enumClass b ) { return enumClass( static_cast<std::underlying_type_t<enumClass>>( a ) | static_cast<std::underlying_type_t<enumClass>>( b ) ); } \
	inline enumClass &operator&=( enumClass &a, enumClass b ) { return a = a & b; } \
	inline enumClass &operator|=( enumClass &a, enumClass b ) { return a = a | b; } \
	inline bool operator!( enumClass a ) { return !static_cast<std::underlying_type_t<enumClass>>( a ); } \
	inline enumClass operator~( enumClass a ) { return enumClass( ~static_cast<std::underlying_type_t<enumClass>>( a ) ); }

namespace prism
{
	template <typename T, size_t N>
	class vec_t;
	template <typename T, size_t N>
	class mat_sq_t;
	class token_t;

	u64 city_hash_64( const void *data, size_t size );
} // namespace prism

class FileSystem;
class SysFileSystem;
class ZipFileSystem;
class HashFileSystem;
class UberFileSystem;

class File;
class SysFsFile;

struct Vertex;
struct Polygon;
class Piece;
class Part;
class Locator;
class Bone;
class Model;
class Variant;

class Animation;

class Prefab;
class Node;
class Curve;
class Sign;
class Semaphore;
class SpawnPoint;
class MapPoint;
class TerrainPointVariant;
class TriggerPoint;
class Intersection;

class Collision;

class Texture;
class TextureObject;
class Material;

class ResourceLibrary;

/**
 * @brief: Converts srgb to linear color space
 *
 * @param[in] x The srgb value
 * @return @c The linear value
*/
double s2lin(double x);

/**
 * @brief: Converts linear to srgb color space
 *
 * @param[in] x The linear value
 * @return @c The srgb value
*/
double lin2s(double x);

inline uint32_t flh(float x)
{
	return *(uint32_t *)(&x);
}

inline uint64_t dlh(double x)
{
	return *(uint64_t*)(&x);
}

String removeSpaces(String str);
String betweenQuotes(String str);
void remove(String &str, const String &substr);

/**
 * Removes exactly one slash or backslash from the end of string
 * 
 * "some_directory"   => "some_directory"
 * "some_directory/"  => "some_directory"
 * "some_directory//" => "some_directory/"
 */
String removeSlashAtEnd( const String &s );

/**
 * Removes exactly one slash or backslash at the beginning of string
 *
 * "some_directory"   => "some_directory"
 * "some_directory/"  => "some_directory"
 * "some_directory//" => "some_directory/"
 */
String removeSlashAtBegin( const String &s );

/**
 * Adds slash at the end of string if it is not there yet
 * 
 * "some_filename"   => "/some_filename"
 * "/some_filename"  => /some_filename"
 */
String makeSlashAtEnd( const String &s );

/**
 * Adds slash at the beginning of string if it is not there yet
 */
String makeSlashAtBegin( const String &s );

/**
 * Removes slashes and backslashes at the beginning of string
 */
String trimSlashesAtBegin( const String &s );

/**
 * Removes slashes and backslashes from the end of string
 */
String trimSlashesAtEnd( const String &s );

/**
 * @brief: Returns directory of the file
 *
 * @param[in] filepath The filepath to process (ex. "/vehicle/truck/mercedes.pmg")
 * @return @c The directory of the file (ex. "/vehicle/truck")
*/
String directory(const String &filepath);

/**
 * @brief: Returns relative path of the file from directory
 *
 * @param[in] filepath The filepath to process (ex. "/model/mover/characters/models/generic/m_asia_01_lod3.pis")
 * @param[in] directory The directory to process (ex. "/model/mover/characters/animations/man")
 * @return @c The relative path (ex. "../../../models/generic/m_asia_01_lod3.pis")
*/
String relativePath(const String &filepath, const String &directory);

bool copyFile(File *const input, File *const output);

void backslashesToSlashes(String &str);

/* float equal */
bool fl_eq(float a, float b);

String valueToQuotedString(const String &value);
String valueToQuotedString(const char *const value);

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
	((unsigned)(unsigned char)(ch0) | ((unsigned)(unsigned char)(ch1) << 8) |   \
	((unsigned)(unsigned char)(ch2) << 16) | ((unsigned)(unsigned char)(ch3) << 24 ))
#endif

template < typename T >
struct IsIntegral {
	enum { value = std::numeric_limits<T>::is_integer };
};

template < typename T >
struct IsFloatingPoint {
	enum {
		value = std::numeric_limits<T>::is_specialized &&
		!std::numeric_limits<T>::is_integer
	};
};

//template < typename T >
//struct IsBoolean {
//	enum {
//		value = st::is_same<
//	};
//};

template < bool C, typename T = void >
struct EnableIf { };

template < typename T >
struct EnableIf<true, T> { typedef T type; };

template <typename T>
struct EnableIfArithmetic : EnableIf<IsIntegral<T>::value || IsFloatingPoint<T>::value, int> {};

template< typename T, unsigned long long N >
char( &ArraySizeHelper( const T( & )[ N ] ) )[ N + 1 ];

#define CP_ARRAY_SIZE( ARRAY ) ( sizeof( ArraySizeHelper( ARRAY ) ) - 1 )

template< typename P, typename A >
P alignForward( P pointer, A alignment )
{
    intptr_t alignment_signed = ( intptr_t )alignment;
    const uintptr_t addr = ( uintptr_t )( pointer );
    const uintptr_t aligned_addr = ( addr + ( alignment_signed - 1 ) ) & -alignment_signed;
    return ( P )( addr + ( aligned_addr - addr ) );
}

const uint32_t TEXTURE_DATA_PITCH_ALIGNMENT =       256; // D3D12_TEXTURE_DATA_PITCH_ALIGNMENT
const uint32_t TEXTURE_DATA_PLACEMENT_ALIGNMENT =   512; // D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT

void extractFile( FileSystem &fileSystem, String filePath, FileSystem &destination );

template< typename T1, typename T2 >
T1 *as( T2 *p )
{
	T1 *const result = p;
	return result;
}

template< typename T, typename Container >
const T &interpretBufferAt( const Container &container, const uint64_t offset, const uint64_t count = 1 )
{
	static_assert( std::is_same_v< typename Container::value_type, u8 >, "value type should be u8!" );

	assert( size_t( offset + count * sizeof( T ) ) <= container.size() );

	return *reinterpret_cast< const T * >( container.data() + offset );
}

template< typename T >
class Span
{
public:
	using element_type = T;
	using value_type = std::remove_cv_t<T>;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using pointer = T *;
	using const_pointer = const T *;
	using reference = T &;
	using const_reference = const T &;

private:
	pointer m_data = nullptr;
	size_type m_size = 0;

public:
	constexpr Span() : m_data( nullptr ), m_size( 0 ) {}

	constexpr Span( T *data, size_type size ) : m_data( data ), m_size( size ) {}

	constexpr T *data() const noexcept { return m_data; }

	constexpr size_type size() const noexcept { return m_size; }

	constexpr size_type size_bytes() const noexcept { return m_size * sizeof( element_type ); }

	[[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }
};

/* eof */
