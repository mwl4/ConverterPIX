/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/resource_lib.h
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

#include <utils/explicit_singleton.h>
#include <material/material.h>

class ResourceLibrary : public ExplicitSingleton<ResourceLibrary>
{
public:
	using Entry = SharedPtr<TextureObject>;

public:
	Entry obtain(String tobjfile);
	void destroy();

private:
	UnorderedMap<String, Entry> m_tobjs;
};

/* eof */
