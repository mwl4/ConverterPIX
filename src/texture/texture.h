/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : texture.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

class Texture
{
	friend Material;
private:
	String m_texture;
	String m_textureName;

	SharedPtr<TextureObject> m_texObj;
public:
	bool load();
	String texture() const { return m_texture; }
	auto texobj() const -> const decltype(m_texObj) { return m_texObj; }
};

/* eof */
