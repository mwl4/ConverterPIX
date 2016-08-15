/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : material.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <structs/pmd.h>
#include <math/vector.h>

class Material
{
public:
	class Attribute
	{
	private:
		std::string m_name;
		enum { FLOAT, STRING } m_valueType;
		uint32_t m_valueCount;
		prism::float4 m_value;
		std::string m_stringValue;
	public:
		Attribute();
		~Attribute();

		void clear();
		const char *getFormat() const;

		friend Material;
	};
private:
	std::string m_effect;
	std::vector<Texture> m_textures;
	std::vector<Attribute> m_attributes;
	std::string m_basePath;
	std::string m_filePath;		// @example: /material/example.mat
	std::string m_alias;
public:
	bool load(std::string basePath, std::string filePath);
	void destroy();

	/**
	 * @brief Creates PIT definition
	 *
	 * @param[in] prefix The prefix for each line of generated definition
	 * @return @c The definition of material
	 */
	std::string toDefinition(const std::string &prefix = "") const;

	std::string toDeclaration(const std::string &prefix = "") const;

	/**
	 * @brief Creates alias of material
	 *
	 * @return @c The alias of material
	 */
	std::string alias() const;

	void setAlias(std::string name);

	bool convertTextures(std::string exportPath) const;

	friend Model;
};

static bool needl2srgb(std::string attrib)
{
	/**
	* @brief The ambient, diffuse, specular, tint, env_factor and water aux are converted from srgb to linear
	*/
	static const char *const attributesLinear[] = { "ambient", "diffuse", "specular", "tint", "env_factor" };

	for (auto& attribb : attributesLinear)
	{
		if (attrib == attribb)
		{
			return true;
		}
	}
	return false;
}

/* eof */
