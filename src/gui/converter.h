/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : converter.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <utils/explicit_singleton.h>

class Converter : public ExplicitSingleton<Converter>
{
private:
	HWND m_window;

public:
	Converter();
	~Converter();

	bool Init();
	bool Destroy();

	void Run();

	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/* eof */
