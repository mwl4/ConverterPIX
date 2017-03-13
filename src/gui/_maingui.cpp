/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : _maingui.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include <resource_lib.h>
#include <model/model.h>
#include <model/animation.h>
#include <texture/texture_object.h>
#include <texture/texture.h>

#include <structs/dds.h>
#include <fs/file.h>
#include <fs/sysfilesystem.h>
#include <fs/uberfilesystem.h>

#include <converter.h>

#ifdef _WIN32

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	Converter converter;
	if (converter.Init())
	{
		converter.Run();
	}
	return converter.Destroy() ? 0 : 1;
}

#endif

/* eof */
