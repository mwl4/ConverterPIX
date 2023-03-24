/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/material/material_pre_147.cpp
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

#include <texture/texture.h>

void Material::loadPre147Format(String& content)
{
	std::stringstream ssbuffer(content);

	while (std::getline(ssbuffer, content))
	{
		size_t middle = content.find(':');
		if (middle == String::npos)
		{
			continue;
		}

		const String name = removeSpaces(content.substr(0, middle));
		const String nameWithoutIndex = removeSpaces(name.substr(0, name.find('[')));
		String value = removeSpaces(content.substr(middle + 1));

		Array<String> values;

		size_t braceLeft = value.find('{');
		size_t quoteLeft = value.find('\"');

		if (braceLeft != String::npos)
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
			const char* valuesArrayptr = valuesArray2;
			while (sscanf(valuesArrayptr, "%s", tempValue) != EOF)
			{
				values.push_back(tempValue);
				valuesArrayptr += strlen(tempValue) + 1;
			}
		}
		else if (quoteLeft != String::npos)
		{
			value = betweenQuotes(value);
		}
		else
		{
			values.push_back(value);
		}

		if ((nameWithoutIndex == "texture" || nameWithoutIndex == "texture_name"))
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
		else if (name != "queue_bias")
		{
			if (values.size() > 0)
			{
				if (values.size() > 4)
				{
					warning("material", m_filePath, "Too many values in the attribute!");
					continue;
				}

				if (!MaterialConverter147::convertAttributesToPost147Format(m_effect, name, values, m_attributes))
				{
					//attribute is in common format
					Attribute& attrib = m_attributes[name];
					attrib.m_name = name;
					attrib.m_valueType = Attribute::FLOAT;
					attrib.m_valueCount = values.size();

					setValues(attrib, values, 0);
				}
			}
			else
			{
				Attribute& attrib = m_attributes[name];
				attrib.m_name = name;
				attrib.m_valueType = Attribute::STRING;
				attrib.m_stringValue = value.c_str();
			}
		}
	}
}

Pix::Value Material::toPixDefinitionPre147() const
{
	AttributesMap pre147Attributes;
	MaterialConverter147::convertAttributesToPre147Format(m_effect, m_attributes, pre147Attributes);

	Pix::Value root;
	root["Alias"] = alias();
	root["Effect"] = m_effect;
	root["Flags"] = 0;
	root["AttributeCount"] = pre147Attributes.size();
	root["TextureCount"] = m_textures.size();

	for (AttributesMap::const_iterator it = pre147Attributes.begin(); it != pre147Attributes.end(); ++it)
	{
		Pix::Value& attribute = root["Attribute"];
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
		const Texture* const tex = &m_textures[i];
		Pix::Value& texture = root["Texture"];
		texture["Tag"] = fmt::sprintf("texture[%i]:%s", (int)i, tex->m_textureName);
		texture["Value"] = tex->m_texture.substr(0, tex->m_texture.length() - 5); // -5 -> .tobj removing
	}
	return root;
}