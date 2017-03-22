/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : callbacks.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

extern void(*info)(const std::string &level, const std::string &file, const std::string &msg);
extern void(*error)(const std::string &level, const std::string &file, const std::string &msg);
extern void(*warning)(const std::string &level, const std::string &file, const std::string &msg);

template < typename ...Args >
void info_f(const std::string &level, const std::string &file, const std::string &format, Args ...args)
{
	auto msg = fmt::sprintf(format, args...);
	info(level, file, msg.c_str());
}

template < typename ...Args >
void error_f(const std::string &level, const std::string &file, const std::string &format, Args ...args)
{
	auto msg = fmt::sprintf(format, args...);
	error(level, file, msg.c_str());
}

template < typename ...Args >
void warning_f(const std::string &level, const std::string &file, const std::string &format, Args ...args)
{
	auto msg = fmt::sprintf(format, args...);
	warning(level, file, msg.c_str());
}

/* eof */
