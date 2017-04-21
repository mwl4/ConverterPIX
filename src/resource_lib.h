/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : resource_lib.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <material/material.h>
#include <utils/explicit_singleton.h>

class ResourceLibrary : public ExplicitSingleton<ResourceLibrary>
{
public:
	using Entry = SharedPtr<TextureObject>;
private:
	UnorderedMap<String, Entry> m_tobjs;
public:
	Entry obtain(String tobjfile);
	void destroy();
};

/* eof */
