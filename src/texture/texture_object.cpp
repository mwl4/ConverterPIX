/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/texture/texture_object.cpp
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

#include "texture_object.h"

#include <fs/file.h>
#include <fs/uberfilesystem.h>
#include <fs/sysfilesystem.h>
#include <structs/tobj.h>
#include <structs/dds.h>

#include "fs/filesystem.h"
#include "fs/uberfilesystem.h"
#include "fs/sysfilesystem.h"
#include "fs/memfs.h"
#include "structs/dds.h"
#include "utils/format_utils.h"
#include "utils/string_utils.h"

bool TextureObject::load( String filepath )
{
    const Optional<String > extension = extractExtension( filepath );
    assert( extension.has_value() && extension.value() == ".tobj" );

    // extract and load from memory if needed
    {
        MetaStat metaStat;
        if( !getUFS()->mstat( &metaStat, filepath ) )
        {
            printf( "Unable to mstat file: %s\n", filepath.c_str() );
            return false;
        }
        if( metaStat.m_meta.size() > 0 )
        {
            MemFileSystem memFileSystem;
            if( !extractTextureObject( filepath, metaStat, memFileSystem, true ) )
            {
                printf( "Unable to extract tobj: %s\n", filepath.c_str() );
                return false;
            }
            return load( &memFileSystem, filepath );
        }
    }

    return load( getUFS(), filepath );
}

bool TextureObject::load( FileSystem *fs, String filepath )
{
    m_filepath = filepath;
    auto file = fs->open( m_filepath, FileSystem::read | FileSystem::binary );
    if( !file )
    {
        warning( "tobj", m_filepath, "Cannot open texture object file" );
        return false;
    }

    const size_t fileSize = static_cast< size_t >( file->size() );
    UniquePtr<uint8_t[]> buffer( new uint8_t[ fileSize ] );
    if( !file->blockRead( ( char * )buffer.get(), 0, fileSize ) )
    {
        warning( "tobj", m_filepath, "Unable to read texture object file" );
        return false;
    }
    file.reset();

    prism::tobj_header_t *header = ( prism::tobj_header_t * )( buffer.get() );
    if( header->m_version != prism::tobj_header_t::SUPPORTED_MAGIC )
    {
        error_f( "tobj", m_filepath, "Invalid version of tobj file! (have: %i, expected: %i)", header->m_version, prism::tobj_header_t::SUPPORTED_MAGIC );
        return false;
    }

    m_type = ( Type )( header->m_type );
    m_magFilter = ( Filter )( header->m_mag_filter );
    m_minFilter = ( Filter )( header->m_min_filter );
    m_mipFilter = ( Filter )( header->m_mip_filter );
    m_addr_u = ( Addr )( header->m_addr_u );
    m_addr_v = ( Addr )( header->m_addr_v );
    m_addr_w = ( Addr )( header->m_addr_w );
    m_bias = header->m_bias;
    m_nocompress = !!header->m_nocompress;
    m_noanisotropic = !!header->m_noanisotropic;
    m_customColorSpace = !!header->m_custom_color_space;

    m_texturesCount = 0;

    for( uint32_t i = 0, currentTextureOffset = sizeof( prism::tobj_header_t ); ; ++i )
    {
        prism::tobj_texture_t *const texture = ( prism::tobj_texture_t * )( buffer.get() + currentTextureOffset );
        m_textures[ i ] = String( ( char * )( ( uint8_t * )texture + sizeof( prism::tobj_texture_t ) ), texture->m_length );

        ++m_texturesCount;

        currentTextureOffset += sizeof( prism::tobj_texture_t ) + texture->m_length;

        if( currentTextureOffset == fileSize )
        {
            break;
        }
    }

    loadDDS( fs, m_textures[ 0 ] );

    if( !m_tsnormal && m_mipFilter == NOMIPS && m_customColorSpace && m_nocompress )
    {
        m_ui = true;
    }
    return true;
}

bool TextureObject::loadDDS( FileSystem *fs, String filepath )
{
	using namespace dds;

	auto file = fs->open(filepath[0] == '/' ? filepath : directory(m_filepath) + "/" + filepath, FileSystem::read | FileSystem::binary);
	if (!file)
	{
		warning_f("tobj", m_filepath, "Unable to open file: \'%s\'", filepath);
		return false;
	}
	else
	{
        const size_t fileSize = static_cast< size_t >( file->size() );

        const size_t bytesToReadNow = sizeof( u32 ) + sizeof( dds::header ); // magic + header
        if( fileSize < bytesToReadNow )
        {
            warning_f( "tobj", m_filepath, "DDS file: \'%s\' is corrupted", filepath );
            return false;
        }

        UniquePtr<uint8_t[]> buffer( new uint8_t[ bytesToReadNow ] );
        file->read( ( char * )buffer.get(), sizeof( char ), bytesToReadNow );
        file.reset();

        const u32 magic = *( u32 * )( buffer.get() );
        if( magic != dds::MAGIC )
        {
            error_f( "dds", filepath, "Invalid dds magic: %i expected: %i", magic, dds::MAGIC );
            return false;
        }
        dds::header *header = ( dds::header * )( buffer.get() + sizeof( u32 ) );

        if( m_customColorSpace )
        {
            if( ( ( header->m_pixel_format.m_flags & dds::PF_FOUR_CC ) && header->m_pixel_format.m_four_cc == COMPRESS_ATI2 )
               || ( header->m_pixel_format == FORMAT_R16G16 ) )
            {
                m_tsnormal = true;
            }
        }
        return true;
	}
}

bool TextureObject::saveToMidFormats(String exportpath)
{
	if (m_converted)
		return true;

	auto file = getSFS()->open(exportpath + m_filepath, FileSystem::write | FileSystem::binary);
	if (!file)
	{
		printf("Cannot open file: \"%s\"! %s\n" SEOL, m_filepath.c_str(), strerror(errno));
		return false;
	}

	if (m_type < TextureObject::_1D_MAP || m_type > TextureObject::_CUBE_MAP)
	{
		printf("Unsupported tobj type: \"%s\"!\n", m_filepath.c_str());
	}

	auto mapType = [](TextureObject::Type type) -> String {
		switch (type)
		{
			case TextureObject::_1D_MAP:	return "1d";
			case TextureObject::_2D_MAP:	return "2d";
			case TextureObject::_3D_MAP:	return "3d";
			case TextureObject::_CUBE_MAP:	return "cube";
		}
		return "UNKNOWN";
	};

	auto addrAttribute = [&](TextureObject::Addr addr) -> String {
		switch (addr)
		{
			case TextureObject::REPEAT:				return "repeat";
			case TextureObject::CLAMP:				return "clamp";
			case TextureObject::CLAMP_TO_EDGE:		return "clamp_to_edge";
			case TextureObject::CLAMP_TO_BORDER:	return "clamp_to_border";
			case TextureObject::MIRROR:				return "mirror";
			case TextureObject::MIRROR_CLAMP:		return "mirror_clamp";
			case TextureObject::MIRROR_CLAMP_TO_EDGE:	return "mirror_clamp_to_edge";
			default: printf("Unknown addr type of tobj file: \"%s\"!\n", m_filepath.c_str());
		}
		return "UNKNOWN";
	};
	auto filterAttribute = [&](TextureObject::Filter f) -> String {
		switch (f)
		{
			case TextureObject::NEAREST:	return "nearest";
			case TextureObject::LINEAR:		return "linear";
			default:
				printf("Unknown filter type of tobj file: \"%s\"!\n", m_filepath.c_str());
		}
		return "UNKNOWN";
	};

    Optional< MemFileSystem > inputOptionalFileSystem;
    {
        MetaStat metaStat;
        if( !getUFS()->mstat( &metaStat, m_filepath ) )
        {
            return false;
        }
        if( metaStat.m_meta.size() > 0 )
        {
            inputOptionalFileSystem.emplace();
            if( !extractTextureObject( m_filepath, metaStat, inputOptionalFileSystem.value() ) )
            {
                printf( "Unable to extract tobj: %s\n", m_filepath.c_str() );
            }
        }
    }

    FileSystem *const inputFileSystem = inputOptionalFileSystem.has_value() ? as<FileSystem>( &inputOptionalFileSystem.value() ) : as<FileSystem>( getUFS() );

	*file << fmt::sprintf("map %s" SEOL, mapType(m_type).c_str());
	for (uint32_t i = 0; i < m_texturesCount; ++i)
	{
		*file << TAB << m_textures[i].c_str() << SEOL;

		if (getSFS()->exists(exportpath + m_textures[i]))
			continue;

		auto inputf = inputFileSystem->open(m_textures[i], FileSystem::read | FileSystem::binary);
		if (!inputf)
		{
			printf("Could not open file: \"%s\" to copy-read!\n", m_textures[i].c_str());
			continue;
		}
		auto outputf = getSFS()->open(exportpath + m_textures[i], FileSystem::write | FileSystem::binary);
		if (!outputf)
		{
			printf("Could not open file: \"%s\" to copy-read!\n", (exportpath + m_textures[i]).c_str());
			continue;
		}
		copyFile(inputf.get(), outputf.get());
	}

	*file << "addr" << SEOL;
	*file << TAB << addrAttribute(m_addr_u) << SEOL;
	*file << TAB << addrAttribute(m_addr_v) << SEOL;
	if (m_type == TextureObject::_CUBE_MAP)
	{
		*file << TAB << addrAttribute(m_addr_w) << SEOL;
	}

	if (m_mipFilter == TextureObject::LINEAR)
	{
		*file << "trilinear" << SEOL;
	}
	else
	{
		if (!m_ui && m_mipFilter == TextureObject::NOMIPS)
		{
			*file << "nomips" << SEOL;
		}
		if (m_magFilter != TextureObject::DEFAULT || m_minFilter != TextureObject::DEFAULT)
		{
			*file << "filter" << TAB << filterAttribute(m_magFilter) << TAB << filterAttribute(m_minFilter) << SEOL;
		}
	}

	if (m_noanisotropic)
	{
		*file << "noanisotropic" << SEOL;
	}

	if (!m_ui && m_nocompress)
	{
		*file << "nocompress" << SEOL;
	}

	if (!m_tsnormal && !m_ui && m_customColorSpace)
	{
		*file << "color_space linear" << SEOL;
	}

	if (m_tsnormal || m_ui)
	{
		*file << "usage " << (m_tsnormal ? "tsnormal" : "ui") << SEOL;
	}

	if (m_bias != 0)
	{
		*file << fmt::sprintf("bias %i" SEOL, m_bias);
	}

	m_converted = true;
	return true;
}

bool extractTextureObject( const String &inputTobjFilePath, const MetaStat &inputTobjMetaStat, FileSystem &fileSystemToWriteTo, bool ddsOnlyHeader )
{
	const Optional<String> inputTobjFilePathExtension = extractExtension( inputTobjFilePath );
	if( inputTobjFilePathExtension == std::nullopt || inputTobjFilePathExtension.value() != ".tobj" )
	{
		return false;
	}

    const MetaStat::Meta *plainMeta = nullptr;
    const prism::token_t plainMetaNamesToTry[] = { tn( "plain" ), tn( "mip0" ), tn( "mip1" ), tn( "miptail" ) };
    for( prism::token_t plainMetaNameToTry : plainMetaNamesToTry )
    {
        if( plainMeta = inputTobjMetaStat.find( plainMetaNameToTry ) )
        {
            break;
        }
    }

    if( plainMeta == nullptr )
    {
        assert( false );
        return false;
    }

    const prism::fs_meta_plain_value_t &plainMetaValue = plainMeta->value< prism::fs_meta_plain_value_t >();

    const MetaStat::Meta *const imgMeta = inputTobjMetaStat.find( tn( "img" ) );
    assert( imgMeta );

    const prism::fs_meta_img_value_t &imgMetaValue = imgMeta->value< prism::fs_meta_img_value_t >();

    const uint32_t imgWidth = prism::fs_meta_img_get_width( imgMetaValue );
    const uint32_t imgHeight = prism::fs_meta_img_get_height( imgMetaValue );
    const uint32_t mipmapCount = prism::fs_meta_img_get_mipmap_count( imgMetaValue );
    const prism::format_t format = prism::fs_meta_img_get_format( imgMetaValue );
    const bool isCube = prism::fs_meta_img_is_cube( imgMetaValue );
    const uint32_t imgSomething4 = prism::fs_meta_img_get_something4( imgMetaValue );

    const String outputDDSFilePath = inputTobjFilePath.substr( 0, inputTobjFilePath.length() - ( sizeof( "tobj" ) - 1 ) ) + "dds";

    if( const UniquePtr< File > outputTobjFile = fileSystemToWriteTo.open( inputTobjFilePath, FileSystem::write | FileSystem::binary ) )
    {
        prism::tobj_header_t tobjHeader = {};
        tobjHeader.m_version = prism::tobj_header_t::SUPPORTED_MAGIC;
        tobjHeader.m_unkn4 = 1;
        tobjHeader.m_type = isCube ? prism::tobj_type_t::cubic : prism::tobj_type_t::generic;
        tobjHeader.m_mag_filter = prism::mag_filter_t::default;
        tobjHeader.m_min_filter = prism::min_filter_t::default;
        tobjHeader.m_mip_filter = prism::mip_filter_t::default;
        tobjHeader.m_unkn10 = 1;
        outputTobjFile->write( &tobjHeader, sizeof( tobjHeader ), 1 );

        prism::tobj_texture_t tobjTexture = {};
        tobjTexture.m_length = outputDDSFilePath.length();
        outputTobjFile->write( &tobjTexture, sizeof( tobjTexture ), 1 );
        outputTobjFile->write( outputDDSFilePath.c_str(), sizeof( char ), tobjTexture.m_length );
    }

    if( const UniquePtr< File > outputDDSFile = fileSystemToWriteTo.open( outputDDSFilePath, FileSystem::write | FileSystem::binary ) )
    {
        uint32_t ddsMagic = dds::MAGIC;
        outputDDSFile->write( &ddsMagic, sizeof( ddsMagic ), 1 );

        dds::header ddsHeader = {};
        ddsHeader.m_size = sizeof( ddsHeader );
        if( mipmapCount > 1 )
        {
            ddsHeader.m_flags = dds::HF_MIPMAPCOUNT;
        }
        else
        {
            ddsHeader.m_flags = dds::HF_CAPS | dds::HF_HEIGHT | dds::HF_WIDTH | dds::HF_PIXELFORMAT;
        }
        ddsHeader.m_caps = dds::caps::texture;
        if( isCube )
        {
            ddsHeader.m_caps = dds::caps::complex;
            ddsHeader.m_caps2 = dds::caps2::cubemap | dds::caps2::cubemap_positivex | dds::caps2::cubemap_negativex |
                dds::caps2::cubemap_positivey | dds::caps2::cubemap_negativey | dds::caps2::cubemap_positivez | dds::caps2::cubemap_negativez;
        }
        if( mipmapCount > 1 )
        {
            ddsHeader.m_caps = dds::caps::complex | dds::caps::mipmap;
            ddsHeader.m_mip_map_count = mipmapCount;
        }
        ddsHeader.m_width = imgWidth;
        ddsHeader.m_height = imgHeight;
        ddsHeader.m_pixel_format.m_size = sizeof( ddsHeader.m_pixel_format );
        ddsHeader.m_pixel_format.m_flags = dds::PF_FOUR_CC;
        ddsHeader.m_pixel_format.m_four_cc = dds::s2u32( "DX10" );
        outputDDSFile->write( &ddsHeader, sizeof( ddsHeader ), 1 );

        dds::header_dxt10 ddsHeaderDxt10 = {};
        ddsHeaderDxt10.m_dxgi_format = static_cast< dds::dxgi_format >( format );
        ddsHeaderDxt10.m_array_size = 1;
        ddsHeaderDxt10.m_resource_dimension = dds::resource_dimension::texture2d;
        ddsHeaderDxt10.m_misc_flag = isCube ? dds::misc::texturecube : dds::misc::none;
        outputDDSFile->write( &ddsHeaderDxt10, sizeof( ddsHeaderDxt10 ), 1 );

        if( ddsOnlyHeader == false )
        {
            const UniquePtr< File > inputTobjFile = inputTobjMetaStat.m_filesystem->openForReadingWithPlainMeta( inputTobjFilePath, plainMetaValue );

            if( inputTobjFile == nullptr )
            {
                return false;
            }

            Array< uint8_t > inputTobjContent;
            inputTobjContent.resize( size_t( inputTobjFile->size() ) );
            inputTobjFile->blockRead( inputTobjContent.data(), 0, inputTobjContent.size() );

            SubresourceData subdata[ 32 ] = { 0 };

            size_t twidth, theight, tdepth, skipMap;
            fillInitData( imgWidth, imgHeight, 1, mipmapCount, 1, format, 0, inputTobjContent.size(), inputTobjContent.data(), twidth, theight, tdepth, skipMap, subdata );

            uint32_t currentOffset = 0;

            for( uint32_t i = 0; i < mipmapCount; ++i )
            {
                SubresourceData &mipmapSubdata = subdata[ i ];

                currentOffset = alignForward( currentOffset, TEXTURE_DATA_PLACEMENT_ALIGNMENT );

                for( uint32_t doneBytes = 0; doneBytes < mipmapSubdata.m_slicePitch; )
                {
                    currentOffset = alignForward( currentOffset, TEXTURE_DATA_PITCH_ALIGNMENT );
                    outputDDSFile->write( inputTobjContent.data() + currentOffset, sizeof( uint8_t ), mipmapSubdata.m_rowPitch );
                    currentOffset += mipmapSubdata.m_rowPitch;
                    doneBytes += mipmapSubdata.m_rowPitch;
                }
            }
        }
    }
    else
    {
        printf( "Unable to open file for write!" );
    }

    return true;
}

/* eof */
