/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/material/material.h
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

#include <structs/pmd.h>

#include <math/vector.h>

#include <pix/pix.h>

class Material
{
public:
	class Attribute
	{
	public:
		Attribute();
		~Attribute();

		void clear();
		String getFormat() const;

		String m_name;
		enum { FLOAT, STRING } m_valueType;
		uint32_t m_valueCount;
		Double4 m_value;
		String m_stringValue;
	};

	class AttributeConvert
	{
	public:
		AttributeConvert(String oldName, int valueCount, int startIndex);
	private:
		String m_oldName;
		uint32_t m_valueCount;
		/* some new attributes map to a single old attribute
		* example:
		* detail_fadeout_from -> aux[5][0]
		* detail_fadeout_range -> aux[5][1]
		* detail_blend_bias -> aux[5][2]
		* detail_uv_scale -> aux[5][3]
		* that's why we need the startIndex to insert the value at specified offset
		*/
		uint32_t m_startIndex;

		friend Material;
	};

public:
	bool load(String filePath);
	void loadPre147Format(String &content);
	void loadPost147Format(String &content);
	void destroy();

	/**
	 * @brief Creates PIT definition
	 *
	 * @param[in] prefix The prefix for each line of generated definition
	 * @return @c The definition of material
	 */
	//String toDefinition(const String &prefix = "") const;

	String toDeclaration(const String &prefix = "") const;

	Pix::Value toPixDefinitionPre147() const;
	Pix::Value toPixDefinitionPost147() const;
	Pix::Value toPixDeclaration() const;

	/**
	 * @brief Creates alias of material
	 *
	 * @return @c The alias of material
	 */
	String alias() const;

	void setAlias(String name);

	bool convertTextures(String exportPath) const;

	static void setValues(Material::Attribute &attrib, const Array<String> &values, const int startIndex = 0);

	typedef Map<String, Attribute> AttributesMap; //changed to map since 1.47 to easily insert values to a single old attribute from multiple new attributes

private:
	String m_effect;
	Array<Texture> m_textures;
	AttributesMap m_attributes;
	String m_filePath;		// @example: /material/example.mat
	String m_alias;

	friend Model;
};

/* eof */
