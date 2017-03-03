/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : texture_object.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>

class TextureObject
{
	friend Model;
private:
	uint32_t m_texturesCount = 0;
	std::string m_textures[6];
	enum Type {
		_1D_MAP = 1,
		_2D_MAP = 2,
		_3D_MAP = 3,
		_CUBE_MAP = 5
	} m_type;
	enum Filter {
		NEAREST = 0,
		LINEAR = 1,
		NOMIPS = 2,
		DEFAULT = 3
	} m_magFilter, m_minFilter, m_mipFilter;
	enum Addr {
		REPEAT = 0,
		CLAMP = 1,
		CLAMP_TO_EDGE = 2,
		CLAMP_TO_BORDER = 3,
		MIRROR = 4,
		MIRROR_CLAMP = 5,
		MIRROR_CLAMP_TO_EDGE = 6
	} m_addr_u, m_addr_v, m_addr_w;
	u8 m_bias = 0;
	bool m_noanisotropic = false;
	bool m_nocompress = false;
	bool m_customColorSpace = false;
	std::string m_filepath; // @example /vehicle/truck/share/glass.tobj
	bool m_converted = false;

	bool m_tsnormal = false;
	bool m_ui = false;
public:
	bool load(std::string filepath);
	bool loadDDS(std::string filepath);
	bool saveToMidFormats(std::string exportpath);
};

/* eof */
