/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : texture_object.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

class TextureObject
{
public:
	enum Type
	{
		_1D_MAP = 1,
		_2D_MAP = 2,
		_3D_MAP = 3,
		_CUBE_MAP = 5
	};

	enum Filter
	{
		NEAREST = 0,
		LINEAR = 1,
		NOMIPS = 2,
		DEFAULT = 3
	};

	enum Addr
	{
		REPEAT = 0,
		CLAMP = 1,
		CLAMP_TO_EDGE = 2,
		CLAMP_TO_BORDER = 3,
		MIRROR = 4,
		MIRROR_CLAMP = 5,
		MIRROR_CLAMP_TO_EDGE = 6
	};

private:
	uint32_t m_texturesCount = 0;
	String m_textures[6];

	Type m_type;

	Filter m_magFilter;
	Filter m_minFilter;
	Filter m_mipFilter;

	Addr m_addr_u;
	Addr m_addr_v;
	Addr m_addr_w;

	u8 m_bias = 0;
	bool m_noanisotropic = false;
	bool m_nocompress = false;
	bool m_customColorSpace = false;

	String m_filepath; // @example /vehicle/truck/share/glass.tobj
	bool m_converted = false;

	bool m_tsnormal = false;
	bool m_ui = false;

public:
	bool load(String filepath);
	bool loadDDS(String filepath);
	bool saveToMidFormats(String exportpath);

	friend Model;
};

/* eof */
