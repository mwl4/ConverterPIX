/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : material.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

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

	size_t fileSize = file->size();

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
	if (check_mat != "material")
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
		if (middle != String::npos)
		{
			String name = removeSpaces(buffer.substr(0, middle));
			String nameWithoutIndex = removeSpaces(name.substr(0, name.find('[')));
			String value = removeSpaces(buffer.substr(middle + 1));

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
				const char *valuesArrayptr = valuesArray2;
				while(sscanf(valuesArrayptr, "%s", tempValue) != EOF)
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
			else if (name != "queue_bias")
			{
				Attribute attrib;
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
					for (uint32_t i = 0; i < attrib.m_valueCount; ++i)
					{
						attrib.m_value[i] = needl2srgb(nameWithoutIndex) ? lin2s((float)atof(values[i].c_str())) : (float)atof(values[i].c_str());
					}
				}
				else
				{
					attrib.m_valueType = Attribute::STRING;
					attrib.m_stringValue = value.c_str();
				}
				m_attributes.push_back(attrib);
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
		for (size_t i = 0; i < m_attributes.size(); ++i)
		{
			const Attribute *const attr = &m_attributes[i];
			result += prefix + TAB + "Attribute {\n";
			{
				result += prefix + fmt::sprintf(TAB TAB "Format: %s" SEOL, attr->getFormat());
				result += prefix + fmt::sprintf(TAB TAB "Tag: \"%s\"" SEOL, attr->m_name.c_str());
				result += prefix + fmt::sprintf(TAB TAB "Value: ( ");
				if (attr->m_valueType == Attribute::FLOAT)
				{
					for (uint32_t j = 0; j < attr->m_valueCount; ++j)
					{
						result += fmt::sprintf("%f ", attr->m_value[j]);
					}
				}
				else
				{
					result += "\"" + attr->m_stringValue + "\" ";
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

	for (size_t i = 0; i < m_attributes.size(); ++i)
	{
		const Attribute *const attr = &m_attributes[i];
		Pix::Value &attribute = root["Attribute"];
		attribute["Format"] = Pix::Value::Enumeration(attr->getFormat());
		attribute["Tag"] = attr->m_name;
		if (attr->m_valueType == Attribute::FLOAT)
		{
			attribute["Value"] = Pix::Value(attr->m_value, attr->m_valueCount);
		}
		else
		{
			attribute["Value"] = Pix::Value::Enumeration("( \"" + attr->m_stringValue + "\" )");
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

/* eof */
