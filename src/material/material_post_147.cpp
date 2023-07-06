/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/material/material_post_147.cpp
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

#include <texture/texture.h>

void Material::loadPost147Format(String& content)
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
			if (name == "texture")
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

				Texture newTexture;
				newTexture.m_textureName = textureType;

				do
				{
					std::getline(ssbuffer, content);
					if (content.find('}') != String::npos)
						break;

					middle = content.find(':');
					String attr = removeSpaces(content.substr(0, middle));
					value = removeSpaces(content.substr(middle + 1));
					quoteLeft = value.find('\"');
					if (quoteLeft != String::npos)
					{
						value = betweenQuotes(value);
					}

					if (attr == "source")
					{
						newTexture.m_texture = value[0] == '/' ? value.c_str() : directory(m_filePath) + "/" + value.c_str();
					}
					else
					{
						Attribute attrib;
						attrib.m_name = attr;
						attrib.m_valueType = Attribute::STRING;
						attrib.m_stringValue = value;
						newTexture.m_attributes.push_back(attrib);
					}
				} while (value.find('}') == String::npos);

				m_textures.push_back(newTexture);
				continue;
			}
			else
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
		}
		else if (quoteLeft != String::npos)
		{
			value = betweenQuotes(value);
		}
		else
		{
			values.push_back(value);
		}

		if (name != "queue_bias" && name != "texture")
		{
			Attribute& attrib = m_attributes[name];
			attrib.m_name = name;

			if (values.size() > 0)
			{
				if (values.size() > 4)
				{
					warning("material", m_filePath, "Too many values in the attribute!");
					continue;
				}
				attrib.m_valueType = Attribute::FLOAT;
				attrib.m_valueCount = values.size();

				setValues(attrib, values, 0);
			}
			else
			{
				attrib.m_valueType = Attribute::STRING;
				attrib.m_stringValue = value.c_str();
			}
		}
	}
}

Pix::Value Material::toPixDefinitionPost147() const
{
	Pix::Value root;
	root["Alias"] = alias();
	root["Effect"] = m_effect;
	root["Flags"] = 0;
	root["AttributeCount"] = m_attributes.size();
	root["TextureCount"] = m_textures.size();

	for (AttributesMap::const_iterator it = m_attributes.begin(); it != m_attributes.end(); ++it)
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