/******************************************************************************
 *
 *  Project:	ConverterPIX @ GUI Application
 *  File:		/gui/_maingui.cpp
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
