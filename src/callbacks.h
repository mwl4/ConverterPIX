/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/callbacks.h
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

#pragma once

extern void(*info)(const String &level, const String &file, const String &msg);
extern void(*error)(const String &level, const String &file, const String &msg);
extern void(*warning)(const String &level, const String &file, const String &msg);

template < typename ...Args >
void info_f(const String &level, const String &file, const String &format, Args ...args)
{
	auto msg = fmt::sprintf(format, args...);
	info(level, file, msg.c_str());
}

template < typename ...Args >
void error_f(const String &level, const String &file, const String &format, Args ...args)
{
	auto msg = fmt::sprintf(format, args...);
	error(level, file, msg.c_str());
}

template < typename ...Args >
void warning_f(const String &level, const String &file, const String &format, Args ...args)
{
	auto msg = fmt::sprintf(format, args...);
	warning(level, file, msg.c_str());
}

/* eof */
