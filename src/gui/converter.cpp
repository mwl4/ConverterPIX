/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : converter.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "converter.h"

Converter::Converter()
{
}

Converter::~Converter()
{
}

bool Converter::Init()
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = ::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hIcon = 0; // LoadIconA(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = 0;// LoadCursorA(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "converter_pix";
	wcex.hIconSm = 0;// LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassExA(&wcex))
	{
		MessageBoxA(NULL, "Call to RegisterClassEx failed!", "Win32 Guided Tour", NULL);
		return false;
	}

	m_window = CreateWindowA(
		"converter_pix",
		"Converter PIX",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 100,
		NULL,
		NULL,
		GetModuleHandleA(NULL),
		NULL
	);

	if (!m_window)
	{
		MessageBoxA(NULL, "Call to CreateWindow failed!", "Win32 Guided Tour", NULL);
		return false;
	}

	SetWindowLongA(m_window, GWL_USERDATA, (LONG)this);

	ShowWindow(m_window, TRUE);
	UpdateWindow(m_window);
	return true;
}

bool Converter::Destroy()
{
	return true;
}

void Converter::Run()
{
	MSG message;
	while (GetMessageA(&message, m_window, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}
}

LRESULT Converter::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PAINT:
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const auto _this = reinterpret_cast<Converter *>(GetWindowLong(hWnd, GWL_USERDATA));
	return _this ? _this->WndProc(hWnd, message, wParam, lParam) : 0;
}

/* eof */
