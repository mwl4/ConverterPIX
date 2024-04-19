/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/utils/string_utils.cpp
 *
 *		  _____                          _            _____ _______   __
 *		 / ____|                        | |          |  __ \_   _\ \ / /
 *		| |     ___  _ ____   _____ _ __| |_ ___ _ __| |__) || |  \ V /
 *		| |    / _ \| '_ \ \ / / _ \ '__| __/ _ \ '__|  ___/ | |   > <
 *		| |___| (_) | | | \ V /  __/ |  | ||  __/ |  | |    _| |_ / . \
 *		 \_____\___/|_| |_|\_/ \___|_|   \__\___|_|  |_|   |_____/_/ \_\
 *
 *
 *  Copyright (C) 2023 Michal Wojtowicz.
 *  All rights reserved.
 *
 *   This software is ditributed WITHOUT ANY WARRANTY; without even
 *   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE. See the copyright file for more information.
 *
 *****************************************************************************/

#include <prerequisites.h>

#include "string_utils.h"

Optional<String> extractExtension( const String &filePath )
{
    const size_t dotPosition = filePath.rfind( '.' );

    if( dotPosition == String::npos )
    {
        return Optional<String>();
    }

    // We need to ensure that dot is part of file name
    const size_t lastBackOrForwardSlashPosition = std::max( filePath.rfind( '/' ), filePath.rfind( '\\' ) );

    if( lastBackOrForwardSlashPosition != String::npos && dotPosition < lastBackOrForwardSlashPosition )
    {
        return Optional<String>();
    }

    return Optional<String>( filePath.substr( dotPosition ) );
}

String removeExtension( const String &filePath )
{
	const size_t dotPosition = filePath.rfind( '.' );

    if( dotPosition == String::npos )
    {
        return filePath;
    }

    // We need to ensure that dot is part of file name
    const size_t lastBackOrForwardSlashPosition = std::max( filePath.rfind( '/' ), filePath.rfind( '\\' ) );

    if( lastBackOrForwardSlashPosition != String::npos && dotPosition < lastBackOrForwardSlashPosition )
    {
        return filePath;
    }

    return filePath.substr( 0, dotPosition );
}

bool startsWith( const String &s, const String &prefix )
{
    if( prefix.length() > s.length() )
    {
        return false;
    }

    return memcmp( s.data(), prefix.data(), prefix.length() ) == 0;
}

/* eof */
