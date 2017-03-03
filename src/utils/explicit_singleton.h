/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : explicit_singleton.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

template < typename T >
class ExplicitSingleton
{
private:
	static T *s_instance;
public:
	ExplicitSingleton()
	{
		s_instance = static_cast<T *>(this);
	}
	~ExplicitSingleton()
	{
		if (s_instance == this)
		{
			s_instance = nullptr;
		}
	}
	static T *Get()
	{
		return s_instance;
	}
};

template < typename T >
T *ExplicitSingleton<T>::s_instance = nullptr;

/* eof */
