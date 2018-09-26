/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/model/piece.h
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
	uint8_t m_boneIndex[BONE_COUNT];
	uint8_t m_boneWeight[BONE_COUNT];
};

struct Triangle
{
	Int3 m_attach;
};

class Piece
{
public:
	Array<uint32_t> texCoords(uint32_t uvChannel) const;

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

	friend Model;
};

/* eof */
