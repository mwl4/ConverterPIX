/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/callbacks.cpp
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
