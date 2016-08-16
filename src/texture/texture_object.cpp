/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : texture_object.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include "texture_object.h"

#include <file.h>
#include <structs/tobj.h>
#include <structs/dds.h>

bool TextureObject::load(std::string basepath, std::string filepath)
{
	m_filepath = filepath;
	m_basepath = basepath;

	std::string realPath = m_basepath + m_filepath;

	File file;
	if (!file.open(realPath.c_str(), "rb"))
	{
		printf("Cannot open tobj file: \"%s\"! %s\n", m_filepath.c_str(), strerror(errno));
		return false;
	}

	const size_t fileSize = file.getSize();
	std::unique_ptr<uint8_t[]> buffer(new uint8_t[fileSize]);
	file.read((char *)buffer.get(), sizeof(char), fileSize);
	file.close();

	prism::tobj_header *header = (prism::tobj_header *)(buffer.get());
	if (header->m_version != prism::tobj_header::SUPPORTED_MAGIC)
	{
		printf("Invalid version of tobj file! \"%s\" (have: %i, expected: %i\n",
			   m_filepath.c_str(), header->m_version, prism::tobj_header::SUPPORTED_MAGIC);
		return false;
	}

	m_type = (Type)(header->m_type);
	m_magFilter = (Filter)(header->m_mag_filter);
	m_minFilter = (Filter)(header->m_min_filter);
	m_mipFilter = (Filter)(header->m_mip_filter);
	m_addr_u = (Addr)(header->m_addr_u);
	m_addr_v = (Addr)(header->m_addr_v);
	m_addr_w = (Addr)(header->m_addr_w);
	m_bias = header->m_bias;
	m_nocompress = !!header->m_nocompress;
	m_noanisotropic = !!header->m_noanisotropic;
	m_customColorSpace = !!header->m_custom_color_space;

	switch (m_type)
	{
		case _1D_MAP: m_texturesCount = 1; break;
		case _2D_MAP: m_texturesCount = 1; break;
		case _3D_MAP: m_texturesCount = 1; break;
		case _CUBE_MAP: m_texturesCount = 6; break;
	}

	for (uint32_t i = 0, currentTextureOffset = sizeof(prism::tobj_header); i < m_texturesCount; ++i)
	{
		prism::tobj_texture *const texture = (prism::tobj_texture *)(buffer.get() + currentTextureOffset);

		char *texture_str = new char[texture->m_length + 1];
		memcpy(texture_str, (uint8_t *)texture + sizeof(prism::tobj_texture), texture->m_length);
		texture_str[texture->m_length] = '\0';
		m_textures[i] = texture_str;
		delete[] texture_str;

		currentTextureOffset += sizeof(prism::tobj_texture) + texture->m_length;
	}

	loadDDS(m_textures[0]);

	if (!m_tsnormal && m_mipFilter == NOMIPS && m_customColorSpace && m_nocompress)
	{
		m_ui = true;
	}
	return true;
}

bool TextureObject::loadDDS(std::string filepath)
{
	using namespace dds;

	File file;
	if (!file.open((m_basepath + (filepath[0] == '/' ? filepath : directory(m_filepath) + "/" + filepath)).c_str(), "rb"))
	{
		printf("Cannot open file: \"%s\"! %s" SEOL, filepath.c_str(), strerror(errno));
		return false;
	}
	else
	{
		const size_t fileSize = file.getSize();
		std::unique_ptr<uint8_t[]> buffer(new uint8_t[fileSize]);
		file.read((char *)buffer.get(), sizeof(char), fileSize);
		file.close();

		const uint32_t magic = *(uint32_t *)(buffer.get());
		if (magic != dds::MAGIC)
		{
			printf("Invalid dds magic (%s): %i expected: %i\n", filepath.c_str(), magic, dds::MAGIC);
			return false;
		}
		dds::header *header = (dds::header *)(buffer.get() + 4);

		if (m_customColorSpace)
		{
			if (((header->m_pixel_format.m_flags & dds::PF_FOUR_CC) && header->m_pixel_format.m_four_cc == COMPRESS_ATI2)
				|| (header->m_pixel_format == FORMAT_R16G16))
			{
				m_tsnormal = true;
			}
		}
		return true;
	}
}

bool TextureObject::saveToMidFormats(std::string exportpath)
{
	if (m_converted)
		return true;

	File file;
	if (!file.open(exportpath + m_filepath, "wb"))
	{
		printf("Cannot open file: \"%s\"! %s\n" SEOL, m_filepath.c_str(), strerror(errno));
		return false;
	}

	if (m_type < TextureObject::_1D_MAP || m_type > TextureObject::_CUBE_MAP)
	{
		printf("Unsupported tobj type: \"%s\"!\n", m_filepath.c_str());
	}

	auto mapType = [](TextureObject::Type type) -> std::string {
		switch (type)
		{
			case TextureObject::_1D_MAP:	return "1d";
			case TextureObject::_2D_MAP:	return "2d";
			case TextureObject::_3D_MAP:	return "3d";
			case TextureObject::_CUBE_MAP:	return "cube";
		}
		return "UNKNOWN";
	};

	auto addrAttribute = [&](TextureObject::Addr addr) -> std::string {
		switch (addr)
		{
			case TextureObject::REPEAT:				return "repeat";
			case TextureObject::CLAMP:				return "clamp";
			case TextureObject::CLAMP_TO_EDGE:		return "clamp_to_edge";
			case TextureObject::CLAMP_TO_BORDER:	return "clamp_to_border";
			case TextureObject::MIRROR:				return "mirror";
			case TextureObject::MIRROR_CLAMP:		return "mirror_clamp";
			case TextureObject::MIRROR_CLAMP_TO_EDGE:	return "mirror_clamp_to_edge";
			default:
			printf("Unknown addr type of tobj file: \"%s\"!\n", m_filepath.c_str());
		}
		return "UNKNOWN";
	};
	auto filterAttribute = [&](TextureObject::Filter f) -> std::string {
		switch (f)
		{
			case TextureObject::NEAREST:	return "nearest";
			case TextureObject::LINEAR:		return "linear";
			default:
			printf("Unknown filter type of tobj file: \"%s\"!\n", m_filepath.c_str());
		}
		return "UNKNOWN";
	};

	file << fmt::sprintf("map %s" SEOL, mapType(m_type).c_str());
	for (uint32_t i = 0; i < m_texturesCount; ++i)
	{
		copyFile((m_basepath + m_textures[i]).c_str(),
			(exportpath + m_textures[i]).c_str());

		file << TAB << m_textures[i].c_str() << SEOL;
	}

	file << "addr" << SEOL;
	file << TAB << addrAttribute(m_addr_u) << SEOL;
	file << TAB << addrAttribute(m_addr_v) << SEOL;
	if (m_type == TextureObject::_CUBE_MAP)
	{
		file << TAB << addrAttribute(m_addr_w) << SEOL;
	}

	if (m_mipFilter == TextureObject::LINEAR)
	{
		file << "trilinear" << SEOL;
	}
	else
	{
		if (!m_ui && m_mipFilter == TextureObject::NOMIPS)
		{
			file << "nomips" << SEOL;
		}
		if (m_magFilter != TextureObject::DEFAULT || m_minFilter != TextureObject::DEFAULT)
		{
			file << "filter" << TAB << filterAttribute(m_magFilter) << TAB << filterAttribute(m_minFilter) << SEOL;
		}
	}

	if (m_noanisotropic)
	{
		file << "noanisotropic" << SEOL;
	}

	if (!m_ui && m_nocompress)
	{
		file << "nocompress" << SEOL;
	}

	if (!m_tsnormal && !m_ui && m_customColorSpace)
	{
		file << "color_space linear" << SEOL;
	}

	if (m_tsnormal || m_ui)
	{
		file << "usage " << (m_tsnormal ? "tsnormal" : "ui") << SEOL;
	}

	if (m_bias != 0)
	{
		file << fmt::sprintf("bias %i" SEOL, m_bias);
	}

	m_converted = true;
	return true;
}

/* eof */
