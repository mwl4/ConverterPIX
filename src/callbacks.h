/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : callbacks.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

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
