/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : resource_lib.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

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
