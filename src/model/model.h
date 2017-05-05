/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : model.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include "bone.h"
#include "piece.h"
#include "part.h"
#include "locator.h"

#include <material/material.h>

class Look
{
private:
	String m_name;
	Array<Material> m_materials;

	friend Model;
};

class Variant
{
public:
	class Attribute
	{
		friend Variant;
		friend Model;

		String m_name;
		enum { INT = 0 } m_type;
		union
		{
			int m_intValue;
			float m_floatValue;
		};

	public:
		Attribute(String name) : m_name(name) {}

		/**
		* @brief Creates PIT definition
		*
		* @param[in] prefix The prefix for each line of generated definition
		* @return @c The definition of the attribute
		*/
		String toDefinition(const String &prefix = "") const;

		/**
		 * @brief 
		 */
		Pix::Value toPixDefinition() const;

		String getName() const { return m_name; }
		int getInt() const { return m_intValue; }
		float getFloat() const { return m_floatValue; }
	};

	class Part
	{
		friend Variant;
		friend Model;

		const ::Part *m_part = nullptr;
		Array<Attribute> m_attributes;

	public:
		const Attribute &operator[](String attribute) const;
		const Attribute &operator[](size_t attribute) const;
		Attribute &operator[](String attribute);
		Attribute &operator[](size_t attribute);

		const ::Part *part() const { return m_part; }
	};

private:
	String m_name;
	Array<Part> m_parts;

public:
	void setPartCount(size_t parts);
	const Part &operator[](size_t id) const;
	Part &operator[](size_t id);

	const Array<Part> &getParts() const { return m_parts; }

	String getName() const { return m_name; }

	friend Model;
};

class Model
{
private:
	Array<Bone> m_bones;
	Array<Piece> m_pieces;
	Array<Part> m_parts;
	Array<Locator> m_locators;
	Array<Look> m_looks;
	Array<Variant> m_variants;

	uint32_t m_vertCount = 0;
	uint32_t m_triangleCount = 0;
	uint32_t m_skinVertCount = 0;
	uint32_t m_materialCount = 0;

	UniquePtr<Prefab> m_prefab;
	UniquePtr<Collision> m_collision;

	bool m_loaded = false;

	String m_filePath;		// @example /vehicle/truck/man_tgx/interior/anim
	String m_fileName;		// @example anim
	String m_directory;		// @example /vehicle/truck/man_tgx/interior

public:
	Model();
	~Model();

	bool load(String filePath);
	void destroy();

	bool loadModel();
	bool loadDescriptor();
	bool loadPrefab();
	bool loadCollision();

	bool saveToPim(String exportPath) const;
	bool saveToPit(String exportPath) const;
	bool saveToPis(String exportPath) const;
	void convertTextures(String exportPath) const;
	void saveToMidFormat(String exportPath, bool convertTexture = true) const;

	bool loaded() const { return m_loaded; }
	String fileName() const { return m_fileName; }
	String filePath() const { return m_filePath; }
	String fileDirectory() const { return m_directory; }

	uint32_t boneCount() const { return m_bones.size(); }
	Bone *bone(size_t index);

	const Array<Part> &getParts() const { return m_parts; }
	const Array<Variant> &getVariants() const { return m_variants; }

private:
	bool loadModel0x13(const uint8_t *const buffer, const size_t size);
	bool loadModel0x14(const uint8_t *const buffer, const size_t size);
};

/* eof */
