/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : texture.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include "texture.h"

#include <resource_lib.h>

bool Texture::load()
{
	return (m_texObj = ResourceLibrary::Get()->obtain(m_texture)) != nullptr;
}

/* eof */
