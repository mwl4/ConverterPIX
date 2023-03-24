/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/material/material.cpp
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

#include <prerequisites.h>

#include "material.h"
#include "material_converter_147.h"

#include <resource_lib.h>
#include <structs/tobj.h>
#include <texture/texture.h>
#include <texture/texture_object.h>
#include <fs/file.h>
#include <fs/uberfilesystem.h>

bool Material::s_outputMatFormat147Enabled = false;

Material::Attribute::Attribute()
	: m_valueType(FLOAT)
	, m_valueCount(0)
{
}

Material::Attribute::~Attribute()
{
}

void Material::Attribute::clear()
{
	m_valueCount = 0;
	m_value = 0.f;
}

String Material::Attribute::getFormat() const
{
	if (m_valueType == STRING)
	{
		return "STRING";
	}
	else if (m_valueType == FLOAT)
	{
		switch (m_valueCount)
		{
			case 1: return "FLOAT";
			case 2: return "FLOAT2";
			case 3: return "FLOAT3";
			case 4: return "FLOAT4";
		}
	}
	return "UNKNOWN";
}

void Material::destroy()
{
	m_filePath = "";
	m_effect = "";
	m_textures.clear();
	m_attributes.clear();
}

bool Material::load(String filePath)
{
	m_filePath = filePath;
	auto file = getUFS()->open(m_filePath, FileSystem::read | FileSystem::binary);
	if(!file)
	{
		warning_f("material", m_filePath, "Unable to open material!");
		return false;
	}

	size_t fileSize = static_cast<size_t>(file->size());

	uint8_t *buff = new uint8_t[fileSize + 1]; // +1 for null terminator
	file->read((char *)buff, sizeof(uint8_t), fileSize);
	file.reset();

	buff[fileSize] = '\0';
	String buffer = (char *)buff;
	delete[] buff;

	size_t begin_material = buffer.find(':'); // material : "effect" { }
	if (begin_material == String::npos)
	{
		warning("material", m_filePath, "Unable to find \':\' in material!");
		return false;
	}

	String check_mat = removeSpaces(buffer.substr(0, begin_material));
	if (check_mat != "material" && check_mat != "effect") //changed to "effect" since 1.47
	{
		warning("material", m_filePath, "Invalid material format!");
		return false;
	}

	buffer = buffer.substr(begin_material + 1);

	size_t brace_left = buffer.find('{');
	if (brace_left == String::npos)
	{
		warning("material", m_filePath, "Unable to find left brace!");
		return false;
	}
	size_t brace_right = buffer.rfind('}');
	if (brace_right == String::npos)
	{
		warning("material", m_filePath, "Unable to find right brace!");
		return false;
	}

	String effect = betweenQuotes(removeSpaces(buffer.substr(0, brace_left - 1)));
	if (effect == "ERROR")
	{
		warning("material", m_filePath, "Quotes effect error!");
		return false;
	}

	remove(effect, ".rfx");
	remove(effect, ".fx");

	m_effect = effect.c_str();

	buffer = buffer.substr(brace_left + 1, brace_right - brace_left - 1);

	if (check_mat == "effect")
		loadPost147Format(buffer);
	else
		loadPre147Format(buffer);

	for (auto &tex : m_textures)
	{
		if (!tex.load())
		{
			warning("material", m_filePath, "Error in material!");
		}
	}

	return true;
}

String Material::toDeclaration(const String &prefix) const
{
	String result;
	result += prefix + "Material {\n";
	{
		result += prefix + fmt::sprintf(TAB "Alias: \"%s\"\n", alias().c_str());
		result += prefix + fmt::sprintf(TAB "Effect: \"%s\"\n", m_effect.c_str());
	}
	result += prefix + "}\n";
	return result;
}

Pix::Value Material::toPixDefinition() const
{
	if (s_outputMatFormat147Enabled)
	{
		return toPixDefinitionPost147();
	}
	else
	{
		return toPixDefinitionPre147();
	}
}

Pix::Value Material::toPixDeclaration() const
{
	Pix::Value root;
	root["Alias"] = alias();
	root["Effect"] = m_effect;
	return root;
}

String Material::alias() const
{
	return m_alias;
}

void Material::setAlias(String name)
{
	m_alias = name;
}

bool Material::convertTextures(String exportPath) const
{
	for (auto &texture : m_textures)
	{
		if (texture.texobj())
		{
			texture.texobj()->saveToMidFormats(exportPath);
		}
	}
	return true;
}

void Material::setValues(Material::Attribute &attrib, const Array<String> &values, const int startIndex)
{
	/**
	* @brief Color values are converted from srgb to linear
	*/

	static const char *const attributesLinear[] = { "ambient", "diffuse", "specular", "glass_tint_color", "env_factor", 
													"water_near_color", "water_horizon_color", "specular_secondary",
													"paintjob_base_color", "flake_color", "flip_color",
													"paintjob_r_color", "paintjob_g_color", "paintjob_b_color"};

	double maxVal = 0;
	bool convert = false;

	for (const auto &attribb : attributesLinear)
	{
		if (attrib.m_name == attribb)
		{
			convert = true;
			break;
		}
	}

	for (size_t i = 0; i < attrib.m_valueCount; i++)
	{
		attrib.m_value[i] = atof(values[startIndex+i].c_str());
		if (convert)
			maxVal = std::fmax(attrib.m_value[i], maxVal);
	}

	if (!convert)
		return;

	for (size_t i = 0; i < attrib.m_valueCount; i++)
	{
		if (maxVal <= 1.0)
			attrib.m_value[i] = lin2s(attrib.m_value[i]);
		else
			attrib.m_value[i] = lin2s(attrib.m_value[i] / maxVal) * maxVal;
	}
}

/* eof */
