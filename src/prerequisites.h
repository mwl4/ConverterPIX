/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : prerequisites.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

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

//
/// Utils
//
#include "utils/types.h"

#include <version.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <fmt/format.h>
#include <fmt/string.h>
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

#include <callbacks.h>

#define TAB		"     "
#define SEOL	"\n"
#define FLT_FT	"&%08x"

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

namespace prism
{
	template <typename T, size_t N>
	class vec_t;
	template <typename T, size_t N>
	class mat_sq_t;
	class token_t;

	u64 city_hash_64(const char *const data, size_t size);
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
float s2lin(float x);

/**
 * @brief: Converts linear to srgb color space
 *
 * @param[in] x The linear value
 * @return @c The srgb value
*/
float lin2s(float x);

inline uint32_t flh(float x)
{
	return *(uint32_t *)(&x);
}

String removeSpaces(String str);
String betweenQuotes(String str);
void remove(String &str, const String &substr);

String removeSlashAtEnd(const String &s);
String removeSlashAtBegin(const String &s);
String makeSlashAtEnd(const String &s);

String trimSlashesAtBegin(const String &s);
String trimSlashesAtEnd(const String &s);

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

/* eof */
