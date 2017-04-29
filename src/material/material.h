/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : material.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <structs/pmd.h>

#include <math/vector.h>

#include <pix/pix.h>

class Material
{
public:
	class Attribute
	{
	private:
		String m_name;
		enum { FLOAT, STRING } m_valueType;
		uint32_t m_valueCount;
		Double4 m_value;
		String m_stringValue;
	public:
		Attribute();
		~Attribute();

		void clear();
		String getFormat() const;

		friend Material;
	};
private:
	String m_effect;
	Array<Texture> m_textures;
	Array<Attribute> m_attributes;
	String m_filePath;		// @example: /material/example.mat
	String m_alias;
public:
	bool load(String filePath);
	void destroy();

	/**
	 * @brief Creates PIT definition
	 *
	 * @param[in] prefix The prefix for each line of generated definition
	 * @return @c The definition of material
	 */
	String toDefinition(const String &prefix = "") const;

	String toDeclaration(const String &prefix = "") const;

	Pix::Value toPixDefinition() const;
	Pix::Value toPixDeclaration() const;

	/**
	 * @brief Creates alias of material
	 *
	 * @return @c The alias of material
	 */
	String alias() const;

	void setAlias(String name);

	bool convertTextures(String exportPath) const;

	friend Model;
};

static bool needl2srgb(String attrib)
{
	/**
	* @brief The ambient, diffuse, specular, tint, env_factor and water aux are converted from srgb to linear
	*/
	static const char *const attributesLinear[] = { "ambient", "diffuse", "specular", "tint", "env_factor", "aux" };

	for (const auto &attribb : attributesLinear)
	{
		if (attrib == attribb)
		{
			return true;
		}
	}
	return false;
}

/* eof */
