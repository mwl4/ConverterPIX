/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/texture/texture_object.h
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

public:
	bool load(String filepath);
	bool loadDDS(String filepath);
	bool saveToMidFormats(String exportpath);

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

	friend Model;
};

/* eof */
