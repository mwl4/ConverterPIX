/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : texture.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>

class Texture
{
	friend Material;
private:
	std::string m_texture;
	std::string m_textureName;

	std::shared_ptr<TextureObject> m_texObj;
public:
	bool load();
	std::string texture() const { return m_texture; }
	auto texobj() const -> const decltype(m_texObj) { return m_texObj; }
};

/* eof */
