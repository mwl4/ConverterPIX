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

public:
	bool load(String filePath);
	void loadPre147Format(String &content);
	void loadPost147Format(String &content);
	void destroy();

	String toDeclaration(const String &prefix = "") const;

	Pix::Value toPixDefinition() const;
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

	using AttributesMap = Map<String, Attribute>;

	static bool s_outputMatFormat147Enabled;

private:
	String m_effect;
	Array<Texture> m_textures;
	AttributesMap m_attributes;
	String m_filePath;		// @example: /material/example.mat
	String m_alias;

	friend Model;
};

/* eof */
