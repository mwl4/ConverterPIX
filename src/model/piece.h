/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : piece.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <math/vector.h>

struct Vertex
{
	static const size_t TEXCOORD_COUNT = 4;
	static const size_t BONE_COUNT = 8;

	Float3 m_position;
	Float3 m_normal;
	Float4 m_tangent;
	Float2 m_texcoords[TEXCOORD_COUNT];
	Float4 m_color;
	Float4 m_color2;
	int8_t m_boneIndex[BONE_COUNT];
	uint8_t m_boneWeight[BONE_COUNT];
};

struct Triangle
{
	Int3 m_attach;
};

class Piece
{
	friend Model;
private:
	uint32_t m_index = 0;

	uint32_t m_texcoordMask = 0;
	uint32_t m_texcoordCount = 0;
	uint32_t m_bones = 0;
	int32_t m_material = 0;

	uint32_t m_streamCount = 0;
	bool m_position = false;
	bool m_normal = false;
	bool m_tangent = false;
	bool m_texcoord = false;
	bool m_color = false;
	bool m_color2 = false;

	Array<Vertex> m_vertices;
	Array<Triangle> m_triangles;
public:
	Array<uint32_t> texCoords(uint32_t uvChannel) const;
};

/* eof */
