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

#define TAB		"     "
#define SEOL	"\n"
#define FLT_FT	"&%08x"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define ENSURE_SIZE(structure, expected) static_assert(sizeof(structure) == expected, "Invalid size")
#define COMMA ,

namespace prism
{
	template <typename T, size_t N>
	class vec_t;
	template <typename T, size_t N>
	class mat_sq_t;
	class token_t;
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
static float s2lin(float x)
{
	const float a = 0.055f;
	return ((x <= 0.04045f) ? (x * (1.f / 12.92f)) : (pow((x + a) * (1.f / (1.f + a)), 2.4f)));
}

/**
 * @brief: Converts linear to srgb color space
 *
 * @param[in] x The linear value
 * @return @c The srgb value
*/
static float lin2s(float x)
{
	const float a = 0.055f;
	return ((x <= 0.0031308f) ? (x * 12.92f) : ((1.f + a) * pow(x, 1.f / 2.4f) - a));
}

static uint32_t flh(float x)
{
	return *(uint32_t *)(&x);
}

static std::string removeSpaces(std::string str)
{
	str.erase(std::remove_if(str.begin(), str.end(), [](char c)->bool { return !!isspace(c); }), str.end());
	return str;
}

static std::string betweenQuotes(std::string str)
{
	size_t left = str.find('\"');
	if (left != std::string::npos)
	{
		size_t right = str.find('\"', left + 1);
		if (right != std::string::npos)
		{
			return str.substr(left + 1, right - left - 1);
		}
	}
	return "ERROR";
}

static void remove(std::string &str, const std::string &substr)
{
	for (size_t pos = std::string::npos; (pos = str.find(substr)) != std::string::npos;)
	{
		str.erase(pos, substr.length());
	}
}

/**
 * @brief: Returns directory of the file
 * 
 * @param[in] filepath The filepath to process (ex. "/vehicle/truck/mercedes.pmg")
 * @return @c The directory of the file (ex. "/vehicle/truck")
*/
std::string directory(const std::string &filepath);

/**
 * @brief: Returns relative path of the file from directory
 *
 * @param[in] filepath The filepath to process (ex. "/model/mover/characters/models/generic/m_asia_01_lod3.pis")
 * @param[in] directory The directory to process (ex. "/model/mover/characters/animations/man")
 * @return @c The relative path (ex. "../../../models/generic/m_asia_01_lod3.pis")
*/
std::string relativePath(const std::string &filepath, const std::string &directory);

bool copyFile(File *const input, File *const output);

static void backslashesToSlashes(std::string &str)
{
	std::replace(str.begin(), str.end(), '\\', '/');
}

/* float equal */
static bool fl_eq(float a, float b)
{
	return fabs(a - b) < FLT_EPSILON;
}

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
	((unsigned)(unsigned char)(ch0) | ((unsigned)(unsigned char)(ch1) << 8) |   \
	((unsigned)(unsigned char)(ch2) << 16) | ((unsigned)(unsigned char)(ch3) << 24 ))
#endif

/* eof */
