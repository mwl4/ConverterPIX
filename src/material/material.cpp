/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : material.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

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

const char *Material::Attribute::getFormat() const
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

bool Material::load(std::string filePath)
{
	m_filePath = filePath;
	auto file = getUFS()->open(m_filePath, FileSystem::read | FileSystem::binary);
	if(!file)
	{
		printf("Cannot open material file: \"%s\"! %s\n", m_filePath.c_str(), strerror(errno));
		return false;
	}

	size_t fileSize = file->getSize();

	uint8_t *buff = new uint8_t[fileSize + 1]; // +1 for null terminator
	file->read((char *)buff, sizeof(uint8_t), fileSize);
	file.reset();

	buff[fileSize] = '\0';
	std::string buffer = (char *)buff;
	delete[] buff;

	size_t begin_material = buffer.find(':'); // material : "effect" { }
	if (begin_material == std::string::npos)
	{
		printf("[mat] Unable to find \':\' in buffer! (%s)\n", m_filePath.c_str());
		return false;
	}

	std::string check_mat = removeSpaces(buffer.substr(0, begin_material));
	if (check_mat != "material")
	{
		printf("[mat] Invalid material format! (%s)\n", m_filePath.c_str());
		return false;
	}

	buffer = buffer.substr(begin_material + 1);

	size_t brace_left = buffer.find('{');
	if (brace_left == std::string::npos)
	{
		printf("[mat] Unable to find left brace! (%s)\n", m_filePath.c_str());
		return false;
	}
	size_t brace_right = buffer.rfind('}');
	if (brace_right == std::string::npos)
	{
		printf("[mat] Unable to find right brace! (%s)\n", m_filePath.c_str());
		return false;
	}

	std::string effect = betweenQuotes(removeSpaces(buffer.substr(0, brace_left - 1)));
	if (effect == "ERROR")
	{
		printf("[mat] Quotes effect error! (%s)\n", m_filePath.c_str());
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
		if (middle != std::string::npos)
		{
			std::string name = removeSpaces(buffer.substr(0, middle));
			std::string value = removeSpaces(buffer.substr(middle + 1));

			std::vector<std::string> values;

			size_t braceLeft = value.find('{');
			size_t quoteLeft = value.find('\"');

			if (braceLeft != std::string::npos)
			{
				size_t braceRight = value.find('}');
				if (braceRight == std::string::npos)
				{
					printf("[mat] Unable to find closing brace! (%s)\n", m_filePath.c_str());
					continue;
				}

				std::string valuesArray = value.substr(braceLeft + 1, braceRight - braceLeft - 1);
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
			else if (quoteLeft != std::string::npos)
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
				if (indexBraceLeft != std::string::npos && indexBraceRight != std::string::npos)
				{
					indexTexture = atoi(name.substr(indexBraceLeft + 1, indexBraceRight - 1).c_str());
				}
				if (indexTexture >= (int)m_textures.size())
				{
					m_textures.resize(indexTexture + 1);
				}
				return indexTexture;
			};

			if (name.substr(0, sizeof("texture_name") - 1) == "texture_name")
			{
				m_textures[textureProperty()].m_textureName = value.c_str();
			}
			else if (name.substr(0, sizeof("texture") - 1) == "texture")
			{
				m_textures[textureProperty()].m_texture = value[0] == '/' ? value.c_str() : directory(m_filePath) + "/" + value.c_str();
			}
			else if (name != "queue_bias")
			{
				Attribute attrib;
				attrib.m_name = name.c_str();
				if (values.size() > 0)
				{
					if (values.size() > 4)
					{
						printf("[mat] Too many values in the attribute! (%s)\n", m_filePath.c_str());
						continue;
					}
					attrib.m_valueType = Attribute::FLOAT;
					attrib.m_valueCount = values.size();
					for (uint32_t i = 0; i < attrib.m_valueCount; ++i)
					{
						attrib.m_value[i] = needl2srgb(attrib.m_name) ? lin2s((float)atof(values[i].c_str())) : (float)atof(values[i].c_str());
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

	for (auto& tex : m_textures)
	{
		if (!tex.load())
		{
			printf("Error in %s material!\n", m_filePath.c_str());
		}
	}

	return true;
}

std::string Material::toDefinition(const std::string &prefix) const
{
	std::string result;
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
				result += prefix + fmt::sprintf(TAB TAB "Value: \"%s\"\n", std::string(tex->m_texture.c_str()).substr(0, tex->m_texture.length() - 5).c_str());
			}
			result += prefix + TAB + "}\n";
		}
	}
	result += prefix + "}\n";
	return result;
}

std::string Material::toDeclaration(const std::string &prefix) const
{
	std::string result;
	result += prefix + "Material {\n";
	{
		result += prefix + fmt::sprintf(TAB "Alias: \"%s\"\n", alias().c_str());
		result += prefix + fmt::sprintf(TAB "Effect: \"%s\"\n", m_effect.c_str());
	}
	result += prefix + "}\n";
	return result;
}

std::string Material::alias() const
{
	return m_alias;
}

void Material::setAlias(std::string name)
{
	m_alias = name;
}

bool Material::convertTextures(std::string exportPath) const
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
