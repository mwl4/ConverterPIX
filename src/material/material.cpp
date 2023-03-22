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

#include <resource_lib.h>
#include <structs/tobj.h>
#include <texture/texture.h>
#include <texture/texture_object.h>
#include <fs/file.h>
#include <fs/uberfilesystem.h>

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

Material::AttributeConvert::AttributeConvert(String oldName, int valueCount, int startIndex)
	: m_oldName(oldName)
	, m_valueCount(valueCount)
	, m_startIndex(startIndex)
{
}

Material::AttributeConvertMap Material::m_convertMap = {
	{ "additional_ambient", Material::AttributeConvert("add_ambient", 1, 0) },
	{ "glass_tint_color", Material::AttributeConvert("tint", 3, 0) },
	{ "glass_tint_opacity", Material::AttributeConvert("tint_opacity", 1, 0) },
	{ "shadowmap_bias", Material::AttributeConvert("shadow_bias", 1, 0) },
	{ "paintjob_base_color", Material::AttributeConvert("aux[8]", 3, 0) },
	{ "specular_secondary", Material::AttributeConvert("aux[3]", 4, 0) },
	{ "shininess_secondary", Material::AttributeConvert("aux[3]", 4, 3) },
	{ "reflection_secondary", Material::AttributeConvert("reflection2", 1, 0) },
	{ "lod_selector", Material::AttributeConvert("aux[1]", 1, 0) },
	{ "shadow_offset", Material::AttributeConvert("aux[0]", 1, 0) },
	{ "amod_decal_blending_factors", Material::AttributeConvert("amod_decal_blending_factors", 2, 0) }, //not present in materials until 1.47
	{ "texgen_0_gen", Material::AttributeConvert("aux[0]", 2, 0) },
	{ "texgen_0_rot", Material::AttributeConvert("texgen_0_rot", 1, 0) }, //not present in materials until 1.47
	{ "texgen_1_gen", Material::AttributeConvert("aux[1]", 2, 0) },
	{ "texgen_1_rot", Material::AttributeConvert("texgen_1_rot", 1, 0) }, //not present in materials until 1.47
	{ "depth_bias", Material::AttributeConvert("aux[0]", 1, 0) },
	{ "luminance_output", Material::AttributeConvert("aux[5]", 2, 0) },
	{ "luminance_night", Material::AttributeConvert("aux[5]", 2, 1) },
	{ "water_distances", Material::AttributeConvert("aux[0]", 3, 0) },
	{ "water_near_color", Material::AttributeConvert("aux[1]", 3, 0) },
	{ "water_horizon_color", Material::AttributeConvert("aux[2]", 3, 0) },
	{ "water_layer_0_yaw", Material::AttributeConvert("aux[3]", 4, 0) },
	{ "water_layer_0_speed", Material::AttributeConvert("aux[3]", 4, 1) },
	{ "water_layer_0_scale", Material::AttributeConvert("aux[3]", 4, 2) },
	{ "water_layer_1_yaw", Material::AttributeConvert("aux[4]", 4, 0) },
	{ "water_layer_1_speed", Material::AttributeConvert("aux[4]", 4, 1) },
	{ "water_layer_1_scale", Material::AttributeConvert("aux[4]", 4, 2) },
	{ "water_mirror", Material::AttributeConvert("aux[5]", 1, 0) },
	{ "animsheet_cfg_fps", Material::AttributeConvert("aux[0]", 3, 0) },
	{ "animsheet_cfg_frames_row", Material::AttributeConvert("aux[0]", 3, 1) },
	{ "animsheet_cfg_frames_total", Material::AttributeConvert("aux[0]", 3, 2) },
	{ "animsheet_frame_width", Material::AttributeConvert("aux[1]", 2, 0) },
	{ "animsheet_frame_height", Material::AttributeConvert("aux[1]", 2, 1) },
	{ "detail_fadeout_from", Material::AttributeConvert("aux[5]", 4, 0) },
	{ "detail_fadeout_range", Material::AttributeConvert("aux[5]", 4, 1) },
	{ "detail_blend_bias", Material::AttributeConvert("aux[5]", 4, 2) },
	{ "detail_uv_scale", Material::AttributeConvert("aux[5]", 4, 3) },
	{ "animation_speed", Material::AttributeConvert("aux[0]", 1, 0) },
	{ "showroom_r_color", Material::AttributeConvert("aux[0]", 1, 0) }, //was single float, since 1.47 is 3d vector
	{ "showroom_speed", Material::AttributeConvert("aux[4]", 3, 0) },
	{ "flake_uvscale", Material::AttributeConvert("aux[5]", 4, 0) },
	{ "flake_shininess", Material::AttributeConvert("aux[5]", 4, 1) },
	{ "flake_clearcoat_rolloff", Material::AttributeConvert("aux[5]", 4, 2) },
	{ "flake_vratio", Material::AttributeConvert("aux[5]", 4, 3) },
	{ "flake_color", Material::AttributeConvert("aux[6]", 4, 0) },
	{ "flake_density", Material::AttributeConvert("aux[6]", 4, 3) },
	{ "flip_color", Material::AttributeConvert("aux[7]", 4, 0) },
	{ "flip_strength", Material::AttributeConvert("aux[7]", 4, 3) },
	{ "mix00_diffuse_secondary", Material::AttributeConvert("mix00_diffuse_secondary", 3, 0) }, //not present in materials until 1.47
	{ "mult_uvscale", Material::AttributeConvert("aux[5]", 4, 0) },
	{ "mult_uvscale_secondary", Material::AttributeConvert("aux[5]", 4, 2) },
	{ "sheet_frame_size_r", Material::AttributeConvert("aux[0]", 4, 0) },
	{ "sheet_frame_size_g", Material::AttributeConvert("aux[0]", 4, 2) },
	{ "sheet_frame_size_b", Material::AttributeConvert("aux[1]", 4, 0) },
	{ "sheet_frame_size_a", Material::AttributeConvert("aux[1]", 4, 2) },
	{ "paintjob_r_color", Material::AttributeConvert("aux[5]", 3, 0) },
	{ "paintjob_g_color", Material::AttributeConvert("aux[6]", 3, 0) },
	{ "paintjob_b_color", Material::AttributeConvert("aux[7]", 3, 0) }
};

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

	std::stringstream ssbuffer(buffer);

	while (std::getline(ssbuffer, buffer))
	{
		size_t middle = buffer.find(':');
		if (middle == String::npos)
		{
			continue;
		}

		const String name = removeSpaces(buffer.substr(0, middle));
		const String nameWithoutIndex = removeSpaces(name.substr(0, name.find('[')));
		String value = removeSpaces(buffer.substr(middle + 1));

		Array<String> values;

		size_t braceLeft = value.find('{');
		size_t quoteLeft = value.find('\"');

		if (braceLeft != String::npos && name != "texture")
		{
			size_t braceRight = value.find('}');
			if (braceRight == String::npos)
			{
				warning("material", m_filePath, "Unable to find closing brace!");
				continue;
			}
			String valuesArray = value.substr(braceLeft + 1, braceRight - braceLeft - 1);
			std::replace_if(valuesArray.begin(), valuesArray.end(), [](char ch)->bool { return ch == ','; }, ' ');

			char valuesArray2[128] = { 0 }, tempValue[128] = { 0 };
			strcpy(valuesArray2, valuesArray.c_str());
			const char *valuesArrayptr = valuesArray2;
			while (sscanf(valuesArrayptr, "%s", tempValue) != EOF)
			{
				values.push_back(tempValue);
				valuesArrayptr += strlen(tempValue) + 1;
			}
		}
		else if (braceLeft != String::npos && name == "texture")
		{
			/* handles the new format of texture attribute since 1.47
			* texture : "texture_name" {
			*	source : "texture_path"
			* }
			*/

			String textureType = betweenQuotes(value);
			if (textureType == "ERROR")
			{
				warning("material", m_filePath, "Unable to parse texture type!");
				continue;
			}
			std::getline(ssbuffer, buffer); //assuming the "source" attribute is in the next line
			middle = buffer.find(':');
			if (removeSpaces(buffer.substr(0, middle)) != "source")
			{
				warning("material", m_filePath, "Unexpected texture attribute: " + removeSpaces(buffer.substr(0, middle)));
				continue;
			}
			value = removeSpaces(buffer.substr(middle + 1));
			quoteLeft = value.find('\"');
			if (quoteLeft != String::npos)
			{
				value = betweenQuotes(value);
			}

			Texture newTexture;
			newTexture.m_texture = value[0] == '/' ? value.c_str() : directory(m_filePath) + "/" + value.c_str();
			newTexture.m_textureName = textureType;
			m_textures.push_back(newTexture);

			std::getline(ssbuffer, buffer); //read the closing brace
			continue;
		}
		else if (quoteLeft != String::npos)
		{
			value = betweenQuotes(value);
		}
		else
		{
			values.push_back(value);
		}

		if ((nameWithoutIndex == "texture" || nameWithoutIndex == "texture_name") && name != nameWithoutIndex)
		{
			//handles the old format of texture attribute
			auto textureProperty = [&]()->int {
				int indexTexture = 0;
				size_t indexBraceLeft = name.find('[');
				size_t indexBraceRight = name.find(']');
				if (indexBraceLeft != String::npos && indexBraceRight != String::npos)
				{
					indexTexture = atoi(name.substr(indexBraceLeft + 1, indexBraceRight - 1).c_str());
				}
				if (indexTexture >= (int)m_textures.size())
				{
					m_textures.resize(indexTexture + 1);
				}
				return indexTexture;
			};

			if (nameWithoutIndex == "texture_name")
			{
				m_textures[textureProperty()].m_textureName = value.c_str();
			}
			else if (nameWithoutIndex == "texture")
			{
				m_textures[textureProperty()].m_texture = value[0] == '/' ? value.c_str() : directory(m_filePath) + "/" + value.c_str();
			}
		}
		else if (name != "queue_bias" && name != "texture")
		{
			AttributeConvertMap::iterator convertRule = m_convertMap.find(name);
			bool needConversion = (convertRule != m_convertMap.end());
			String nameAfterConversion;

			if (needConversion)
				nameAfterConversion = convertRule->second.m_oldName;
			else
				nameAfterConversion = name;

			Attribute& attrib = m_attributes[nameAfterConversion];
			attrib.m_name = nameAfterConversion;

			if (values.size() > 0)
			{
				if (values.size() > 4)
				{
					warning("material", m_filePath, "Too many values in the attribute!");
					continue;
				}
				attrib.m_valueType = Attribute::FLOAT;
				if (needConversion)
					attrib.m_valueCount = convertRule->second.m_valueCount;
				else
					attrib.m_valueCount = values.size();

				convertAttribIfNeeded(attrib, effect, name, values, needConversion ? convertRule->second.m_startIndex : 0);
			}
			else
			{
				attrib.m_valueType = Attribute::STRING;
				attrib.m_stringValue = value.c_str();
			}
		}
	}

	for (auto &tex : m_textures)
	{
		if (!tex.load())
		{
			warning("material", m_filePath, "Error in material!");
		}
	}

	return true;
}

String Material::toDefinition(const String &prefix) const
{
	String result;
	result += prefix + "Material {\n";
	{
		result += prefix + fmt::sprintf(TAB "Alias: \"%s\"\n", alias().c_str());
		result += prefix + fmt::sprintf(TAB "Effect: \"%s\"\n", m_effect.c_str());
		result += prefix + fmt::sprintf(TAB "Flags: %i\n", 0);
		result += prefix + fmt::sprintf(TAB "AttributeCount: %i\n", (int32_t)m_attributes.size());
		result += prefix + fmt::sprintf(TAB "TextureCount: %i\n", (int32_t)m_textures.size());
		for (AttributesMap::const_iterator it = m_attributes.begin(); it != m_attributes.end(); ++it)
		{
			result += prefix + TAB + "Attribute {\n";
			{
				result += prefix + fmt::sprintf(TAB TAB "Format: %s" SEOL, it->second.getFormat());
				result += prefix + fmt::sprintf(TAB TAB "Tag: \"%s\"" SEOL, it->second.m_name.c_str());
				result += prefix + fmt::sprintf(TAB TAB "Value: ( ");
				if (it->second.m_valueType == Attribute::FLOAT)
				{
					for (uint32_t j = 0; j < it->second.m_valueCount; ++j)
					{
						result += fmt::sprintf("%f ", it->second.m_value[j]);
					}
				}
				else
				{
					result += "\"" + it->second.m_stringValue + "\" ";
				}
				result += ")" SEOL;
			}
			result += prefix + TAB + "}\n";
		}
		for (size_t i = 0; i < m_textures.size(); ++i)
		{
			const Texture *const tex = &m_textures[i];
			result += prefix + TAB + "Texture {\n";
			{
				result += prefix + fmt::sprintf(TAB TAB "Tag: \"texture[%i]:%s\"\n", (int)i, tex->m_textureName.c_str());
				result += prefix + fmt::sprintf(TAB TAB "Value: \"%s\"\n", String(tex->m_texture.c_str()).substr(0, tex->m_texture.length() - 5).c_str());
			}
			result += prefix + TAB + "}\n";
		}
	}
	result += prefix + "}\n";
	return result;
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
	Pix::Value root;
	root["Alias"] = alias();
	root["Effect"] = m_effect;
	root["Flags"] = 0;
	root["AttributeCount"] = m_attributes.size();
	root["TextureCount"] = m_textures.size();

	for (AttributesMap::const_iterator it = m_attributes.begin(); it != m_attributes.end(); ++it)
	{
		Pix::Value &attribute = root["Attribute"];
		attribute["Format"] = Pix::Value::Enumeration(it->second.getFormat());
		attribute["Tag"] = it->second.m_name;
		if (it->second.m_valueType == Attribute::FLOAT)
		{
			attribute["Value"] = Pix::Value(it->second.m_value, it->second.m_valueCount);
		}
		else
		{
			attribute["Value"] = Pix::Value::Enumeration("( \"" + it->second.m_stringValue + "\" )");
		}
	}

	for (size_t i = 0; i < m_textures.size(); ++i)
	{
		const Texture *const tex = &m_textures[i];
		Pix::Value &texture = root["Texture"];
		texture["Tag"] = fmt::sprintf("texture[%i]:%s", (int)i, tex->m_textureName);
		texture["Value"] = tex->m_texture.substr(0, tex->m_texture.length() - 5); // -5 -> .tobj removing
	}
	return root;
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

void Material::convertAttribIfNeeded(Material::Attribute &attrib, const String &effect, const String &attribName, const Array<String> &values, const int startIndex)
{
	/**
	* @brief The ambient, diffuse, specular, tint, env_factor and water aux are converted from srgb to linear
	*/

	static const char *const attributesLinear[] = { "ambient", "diffuse", "specular", "tint", "env_factor" };

	double maxVal = 0;
	bool convert = false;

	for (const auto &attribb : attributesLinear)
	{
		if (attribName == attribb)
		{
			convert = true;
			break;
		}
	}

	if (effect == "eut2.water" && (attribName == "aux[1]" || attribName == "aux[2]"))
		convert = true;

	for (size_t i = 0; i < values.size(); i++)
	{
		attrib.m_value[startIndex+i] = atof(values[i].c_str());
		if (convert)
			maxVal = std::fmax(attrib.m_value[startIndex+i], maxVal);
	}

	if (!convert)
		return;

	for (size_t i = 0; i < values.size(); i++)
	{
		if (maxVal <= 1.0)
			attrib.m_value[startIndex+i] = lin2s(attrib.m_value[startIndex+i]);
		else
			attrib.m_value[startIndex+i] = lin2s(attrib.m_value[startIndex+i] / maxVal) * maxVal;
	}
}

/* eof */
