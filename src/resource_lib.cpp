/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/resource_lib.cpp
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

#include "resource_lib.h"

#include <texture/texture_object.h>
#include <fs/uberfilesystem.h>

auto ResourceLibrary::obtain(String tobjfile) -> Entry
{
	if (m_tobjs.find(tobjfile) == m_tobjs.end())
	{
		Entry texobj = std::make_shared<TextureObject>();
		if (texobj->load(tobjfile))
		{
			m_tobjs.insert({ tobjfile.c_str(), texobj });
		}
		else
		{
			warning("tobj", tobjfile, "Unable to load!");
			return nullptr;
		}
	}
	return m_tobjs[tobjfile];
}

void ResourceLibrary::destroy()
{
	m_tobjs.clear();
}

/* eof */
