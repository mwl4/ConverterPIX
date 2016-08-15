/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : piece.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <structs/pmg.h>
#include <math/vector.h>

struct Vertex
{
	Float3 m_position;
	Float3 m_normal;
	Float2 m_uv[4];
	Float4 m_color;
	Float4 m_color2;
	Float4 m_tangent;
	uint16_t m_animBind;
};

struct Triangle
{
	uint16_t m_a[3];
};

class AnimBind
{
private:
	struct Bind
	{
		int8_t m_bone;
		uint8_t m_weight;
	};
	std::vector<Bind> m_binds;
public:
	void setBoneCount(size_t bones);
	int8_t &bone(size_t idx);
	const int8_t &bone(size_t idx) const;
	uint8_t &weight(size_t idx);
	const uint8_t &weight(size_t idx) const;
};

class Piece
{
	friend Model;
private:
	uint32_t m_index = 0;

	uint32_t m_maskUV = 0;
	uint32_t m_UVs = 0;
	uint32_t m_bones = 0;
	int32_t m_material = 0;

	uint32_t m_streamCount = 0;
	bool m_position = false;
	bool m_normal = false;
	bool m_uv = false;
	bool m_color = false;
	bool m_color2 = false;
	bool m_tangent = false;

	std::vector<Vertex> m_vertices;
	std::vector<Triangle> m_triangles;
	std::vector<AnimBind> m_animBinds;
public:
	std::vector<uint32_t> texCoords(uint32_t uvChannel) const;
};

/* eof */
