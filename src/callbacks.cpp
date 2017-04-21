/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : callbacks.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "callbacks.h"

void(*info)(const String &level, const String &file, const String &msg)
	= [](const String &level, const String &file, const String &msg) -> void
	{
		printf("[%s] %s: %s\n", level.c_str(), file.c_str(), msg.c_str());
	};

void(*error)(const String &level, const String &file, const String &msg)
	= [](const String &level, const String &file, const String &msg) -> void
	{
		printf("<error> [%s] %s: %s\n", level.c_str(), file.c_str(), msg.c_str());
	};

void(*warning)(const String &level, const String &file, const String &msg)
	= [](const String &level, const String &file, const String &msg) -> void
	{
		printf("<warning> [%s] %s: %s\n", level.c_str(), file.c_str(), msg.c_str());
	};

/* eof */
