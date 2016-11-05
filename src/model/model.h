/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : model.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>

#include "bone.h"
#include "piece.h"
#include "part.h"
#include "locator.h"
#include <material/material.h>

class Look
{
	friend Model;
private:
	std::string m_name;
	std::vector<Material> m_materials;
};

class Variant
{
public:
	class Attribute
	{
		friend Variant;
		friend Model;

		std::string m_name;
		enum { INT = 0 } m_type;
		union
		{
			int m_intValue;
			float m_floatValue;
		};
	public:
		Attribute(std::string name) : m_name(name) {}

		/**
		* @brief Creates PIT definition
		*
		* @param[in] prefix The prefix for each line of generated definition
		* @return @c The definition of the attribute
		*/
		std::string toDefinition(const std::string &prefix = "") const;

		std::string getName() const { return m_name; }
		int getInt() const { return m_intValue; }
		float getFloat() const { return m_floatValue; }
	};
	class Part
	{
		friend Variant;
		friend Model;

		const ::Part *m_part = nullptr;
		std::vector<Attribute> m_attributes;
	public:
		const Attribute &operator[](std::string attribute) const;
		const Attribute &operator[](size_t attribute) const;
		Attribute &operator[](std::string attribute);
		Attribute &operator[](size_t attribute);

		const ::Part *part() const { return m_part; }
	};
private:
	std::string m_name;
	std::vector<Part> m_parts;
public:
	void setPartCount(size_t parts);
	const Part &operator[](size_t id) const;
	Part &operator[](size_t id);

	const std::vector<Part> &getParts() const { return m_parts; }

	std::string getName() const { return m_name; }

	friend Model;
};

class Model
{
private:
	std::vector<Bone> m_bones;
	std::vector<Piece> m_pieces;
	std::vector<Part> m_parts;
	std::vector<Locator> m_locators;
	std::vector<Look> m_looks;
	std::vector<Variant> m_variants;

	uint32_t m_vertCount = 0;
	uint32_t m_triangleCount = 0;
	uint32_t m_skinVertCount = 0;
	uint32_t m_materialCount = 0;

	std::shared_ptr<Prefab> m_prefab;
	std::shared_ptr<Collision> m_collision;

	bool m_loaded = false;

	std::string m_basePath;		// @example C:/ets2/base
	std::string m_filePath;		// @example /vehicle/truck/man_tgx/interior/anim
	std::string m_fileName;		// @example anim
	std::string m_directory;	// @example /vehicle/truck/man_tgx/interior
public:
	bool load(std::string basePath, std::string filePath);
	void destroy();

	bool loadModel();
	bool loadDescriptor();
	bool loadCollision();

	bool saveToPim(std::string exportPath) const;
	bool saveToPit(std::string exportPath) const;
	bool saveToPis(std::string exportPath) const;
	void convertTextures(std::string exportPath) const;
	void saveToMidFormat(std::string exportPath, bool convertTexture = true) const;

	bool loaded() const { return m_loaded; }
	std::string basePath() const { return m_basePath; }
	std::string fileName() const { return m_fileName; }
	std::string filePath() const { return m_filePath; }
	std::string fileDirectory() const { return m_directory; }

	uint32_t boneCount() const { return m_bones.size(); }
	Bone *bone(size_t index);

	const std::vector<Part> &getParts() const { return m_parts; }
	const std::vector<Variant> &getVariants() const { return m_variants; }
};

/* eof */
