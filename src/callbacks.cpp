/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : callbacks.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

void(*info)(const std::string &level, const std::string &file, const std::string &msg)
	= [](const std::string &level, const std::string &file, const std::string &msg) -> void
	{
		printf("[%s] %s: %s\n", level.c_str(), file.c_str(), msg.c_str());
	};

void(*error)(const std::string &level, const std::string &file, const std::string &msg)
	= [](const std::string &level, const std::string &file, const std::string &msg) -> void
	{
		printf("<error> [%s] %s: %s\n", level.c_str(), file.c_str(), msg.c_str());
	};

void(*warning)(const std::string &level, const std::string &file, const std::string &msg)
	= [](const std::string &level, const std::string &file, const std::string &msg) -> void
	{
		printf("<warning> [%s] %s: %s\n", level.c_str(), file.c_str(), msg.c_str());
	};

/* eof */
