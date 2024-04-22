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

bool s_ddsDxt10 = false;

static String resolveTextureFilePath( String textureFilePath, String tobjFilePath )
{
	// basically handles texture file path relative to tobj file path
	return textureFilePath[ 0 ] == '/' ? textureFilePath : directory( tobjFilePath ) + "/" + textureFilePath;
}

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
			if( !extractTextureObject( filepath, metaStat, memFileSystem, false /* cannot be turned on because of cubemap image duplication remover */ ) )
			{
				printf( "Unable to extract tobj: %s\n", filepath.c_str() );
				return false;
			}
			return loadPre( &memFileSystem, filepath );
		}
	}

	return loadPre( getUFS(), filepath );
}

bool TextureObject::loadPre( FileSystem *fs, String filepath )
{
	MemFileSystem memFileSystem;

	UberFileSystem localUfs;
	localUfs.mount( fs, 1 );
	localUfs.mount( &memFileSystem, 2 );

	// Makes sure texture object is in proper format
	if( !convertTextureObjectToOldFormatsIfNeeded( *fs, filepath, memFileSystem, false /* cannot be turned on, because it may overwrite files on disk */ ) )
	{
		printf( "Unable to convert tobj to old formats: %s\n", filepath.c_str() );
		return false;
	}

	return load( &localUfs, filepath );
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

	Array<u8> buffer( static_cast<size_t>( file->size() ) );
	if( !file->blockRead( buffer.data(), 0, buffer.size() ) )
	{
		warning( "tobj", m_filepath, "Unable to read texture object file" );
		return false;
	}
	file.reset();

	const prism::tobj_header_t *const header = &interpretBufferAt<prism::tobj_header_t>( buffer, 0 );
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

	for( uint32_t i = 0, currentTextureOffset = sizeof( prism::tobj_header_t ); currentTextureOffset < buffer.size(); ++i )
	{
		const prism::tobj_texture_t *const texture = &interpretBufferAt<prism::tobj_texture_t>( buffer, currentTextureOffset );

		const char *const textureStringBegin = &interpretBufferAt<char>( buffer, currentTextureOffset + sizeof( prism::tobj_texture_t ), texture->m_length );
		m_textures[ i ] = String( textureStringBegin, texture->m_length );
		++m_texturesCount;

		currentTextureOffset += sizeof( prism::tobj_texture_t ) + texture->m_length;
	}

	loadDDS( fs, m_textures[ 0 ] );

	if( m_mipFilter == NOMIPS && m_nocompress && startsWith( filepath, "/material/ui/" ) )
	{
		m_ui = true;
	}

	return true;
}

bool TextureObject::loadDDS( FileSystem *fs, String filepath )
{
	auto file = fs->open( resolveTextureFilePath( filepath, m_filepath ), FileSystem::read | FileSystem::binary);
	if( !file )
	{
		warning_f( "tobj", m_filepath, "Unable to open file: \'%s\'", filepath );
		return false;
	}
	
	Array<u8> buffer( static_cast< size_t >( sizeof( u32 ) + sizeof( dds::header ) ) ); // magic + header
	if( !file->blockRead( buffer.data(), 0, buffer.size() ) )
	{
		warning_f( "tobj", m_filepath, "DDS file: \'%s\' is corrupted", filepath );
		return false;
	}
	file.reset();

	const u32 magic = interpretBufferAt<u32>( buffer, 0 );
	if( magic != dds::MAGIC )
	{
		error_f( "dds", filepath, "Invalid dds magic: %i expected: %i", magic, dds::MAGIC );
		return false;
	}
	const dds::header *const header = &interpretBufferAt<dds::header>( buffer, sizeof( magic ) );

	if( m_customColorSpace )
	{
		if( ( !!( header->m_pixel_format.m_flags & dds::pixel_flags::four_cc ) && header->m_pixel_format.m_four_cc == dds::COMPRESS_ATI2 )
			|| ( header->m_pixel_format == dds::PIXEL_FORMAT_R16G16 ) )
		{
			m_tsnormal = true;
		}
	}
	return true;
}

bool TextureObject::saveToMidFormats( String exportpath )
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
				return false;
			}
		}
	}

	FileSystem *const inputFileSystem = inputOptionalFileSystem.has_value() ? as<FileSystem>( &inputOptionalFileSystem.value() ) : as<FileSystem>( getUFS() );

	if( !convertTextureObjectToOldFormatsIfNeeded( *inputFileSystem, m_filepath, *inputFileSystem ) )
	{
		printf( "Unable to convert tobj to old formats: %s\n", m_filepath.c_str() );
		return false;
	}

	*file << fmt::sprintf("map %s" SEOL, mapType(m_type).c_str());
	for (uint32_t i = 0; i < m_texturesCount; ++i)
	{
		*file << TAB << m_textures[i].c_str() << SEOL;

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

//////////////////////////////////////////////////////////////////////////

bool extractTextureObject( const String &inputTobjFilePath, const MetaStat &inputTobjMetaStat, FileSystem &fileSystemToWriteTo, const bool ddsOnlyHeader )
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

	const prism::fs_meta_plain_t &plainMetaValue = plainMeta->value<prism::fs_meta_plain_t>();

	const prism::fs_meta_img_t &imgMetaValue = inputTobjMetaStat.get<prism::fs_meta_img_t>();

	const uint32_t imgWidth = imgMetaValue.get_width();
	const uint32_t imgHeight = imgMetaValue.get_height();
	const uint32_t mipmapCount = imgMetaValue.get_mipmap_count();
	const prism::format_t format = imgMetaValue.get_format();
	const bool isCube = imgMetaValue.is_cube();
	const uint32_t imgPitchAlignment = imgMetaValue.get_pitch_alignment();
	const uint32_t imgImageAlignment = imgMetaValue.get_image_alignment();
	const uint32_t imgFaceCount = imgMetaValue.get_count();

	assert( ( imgImageAlignment % TEXTURE_DATA_PLACEMENT_ALIGNMENT ) == 0 );
	assert( ( imgPitchAlignment % TEXTURE_DATA_PITCH_ALIGNMENT ) == 0 );

	const prism::fs_meta_sample_t &sampleMetaValue = inputTobjMetaStat.get< prism::fs_meta_sample_t >();

	const prism::mag_filter_t sampleMagFilter = sampleMetaValue.get_mag_filter();
	const prism::min_filter_t sampleMinFilter = sampleMetaValue.get_min_filter();
	const prism::mip_filter_t sampleMipFilter = sampleMetaValue.get_mip_filter();
	const prism::tobj_addr_t sampleAddrU = sampleMetaValue.get_addr_u();
	const prism::tobj_addr_t sampleAddrV = sampleMetaValue.get_addr_v();
	const prism::tobj_addr_t sampleAddrW = sampleMetaValue.get_addr_w();

	const String outputDDSFilePath = removeExtension( inputTobjFilePath ) + ".dds";

	if( const UniquePtr< File > outputTobjFile = fileSystemToWriteTo.open( inputTobjFilePath, FileSystem::write | FileSystem::binary ) )
	{
		prism::tobj_header_t tobjHeader = {};
		tobjHeader.m_version = prism::tobj_header_t::SUPPORTED_MAGIC;
		tobjHeader.m_unkn4 = 1;
		tobjHeader.m_type = isCube ? prism::tobj_type_t::cubic : prism::tobj_type_t::generic;
		tobjHeader.m_mag_filter = sampleMagFilter;
		tobjHeader.m_min_filter = sampleMinFilter;
		tobjHeader.m_mip_filter = sampleMipFilter;
		tobjHeader.m_addr_u = sampleAddrU;
		tobjHeader.m_addr_v = sampleAddrV;
		tobjHeader.m_addr_w = sampleAddrW;
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
		ddsHeader.m_flags = dds::header_flags::caps | dds::header_flags::height | dds::header_flags::width | dds::header_flags::pixelformat;
		if( mipmapCount > 1 )
		{
			ddsHeader.m_flags |= dds::header_flags::mipmapcount;
		}
		ddsHeader.m_caps = dds::caps::texture;
		if( isCube )
		{
			ddsHeader.m_caps |= dds::caps::complex;
			ddsHeader.m_caps2 = dds::caps2::cubemap |
								dds::caps2::cubemap_positivex | dds::caps2::cubemap_negativex |
								dds::caps2::cubemap_positivey | dds::caps2::cubemap_negativey |
								dds::caps2::cubemap_positivez | dds::caps2::cubemap_negativez;
		}
		if( mipmapCount > 1 )
		{
			ddsHeader.m_caps |= dds::caps::mipmap | dds::caps::complex;
			ddsHeader.m_mip_map_count = mipmapCount;
		}
		ddsHeader.m_width = imgWidth;
		ddsHeader.m_height = imgHeight;
		ddsHeader.m_pixel_format.m_size = sizeof( ddsHeader.m_pixel_format );
		ddsHeader.m_pixel_format.m_flags = dds::pixel_flags::four_cc;
		ddsHeader.m_pixel_format.m_four_cc = dds::DXT10;
		outputDDSFile->write( &ddsHeader, sizeof( ddsHeader ), 1 );

		dds::header_dxt10 ddsHeaderDxt10 = {};
		ddsHeaderDxt10.m_dxgi_format = static_cast< dds::dxgi_format >( format );
		ddsHeaderDxt10.m_array_size = 1;
		ddsHeaderDxt10.m_resource_dimension = dds::resource_dimension::texture2d;
		ddsHeaderDxt10.m_misc_flag = isCube ? dds::misc::texturecube : dds::misc{};
		outputDDSFile->write( &ddsHeaderDxt10, sizeof( ddsHeaderDxt10 ), 1 );

		if( ddsOnlyHeader == false )
		{
			const UniquePtr< File > inputTobjFile = inputTobjMetaStat.m_filesystem->openForReadingWithPlainMeta( inputTobjFilePath, plainMetaValue );
			if( inputTobjFile == nullptr )
			{
				return false;
			}

			Array<u8> inputTobjContent( static_cast<size_t>( inputTobjFile->size() ) );
			if( !inputTobjFile->blockRead( inputTobjContent.data(), 0, inputTobjContent.size() ) )
			{
				return false;
			}
			
			const size_t imgArraySize = ddsHeaderDxt10.m_array_size;
			const size_t imgDepth = 1; // used for volume textures

			SubresourceData subdata[ 32 ] = { 0 };
			assert( CP_ARRAY_SIZE( subdata ) >= mipmapCount * imgArraySize );

			size_t twidth, theight, tdepth, skipMap;
			fillInitData( imgWidth, imgHeight, imgDepth, mipmapCount, imgArraySize, format, 0, inputTobjContent.size(), inputTobjContent.data(), twidth, theight, tdepth, skipMap, subdata );

			u32 currentOffset = 0;

			for( u32 currentFaceIndex = 0; currentFaceIndex < imgFaceCount; ++currentFaceIndex )
			{
				for( u32 mipmapIndex = 0; mipmapIndex < mipmapCount; ++mipmapIndex )
				{
					SubresourceData &mipmapSubdata = subdata[ mipmapIndex ];
					const u32 slicePitch = mipmapSubdata.m_slicePitch;
					const u32 rowPitch = mipmapSubdata.m_rowPitch;

					currentOffset = alignForward( currentOffset, imgImageAlignment );

					for( u32 doneBytes = 0; doneBytes < slicePitch; doneBytes += rowPitch )
					{
						currentOffset = alignForward( currentOffset, imgPitchAlignment );
						assert( currentOffset + rowPitch <= inputTobjContent.size() );
						outputDDSFile->write( inputTobjContent.data() + currentOffset, sizeof( u8 ), rowPitch );
						currentOffset += rowPitch;
					}
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

static Array<u8> convertImageBits_B8G8R8X8_To_B8G8R8( const u8 *bits, u32 bitsLength, u32 width, u32 height, u32 facesCount, u32 imagesCount )
{
	auto bitsConverted = Array<u8>( static_cast< size_t >( bitsLength * 3 / 4 ) );
	u32 currentDestinationOffset = 0, currentSourceOffset = 0;
	u32 currentImageWidth = width, currentImageHeight = height;
	for( u32 faceIndex = 0; faceIndex < facesCount; ++faceIndex )
	{
		for( u32 imageIndex = 0; imageIndex < imagesCount; ++imageIndex )
		{
			const u32 currentImagePixelsCount = currentImageWidth * currentImageHeight;
			for( u32 donePixels = 0; donePixels < currentImagePixelsCount; ++donePixels )
			{
				bitsConverted[ currentDestinationOffset + 0 ] = bits[ currentSourceOffset + 0 ];
				bitsConverted[ currentDestinationOffset + 1 ] = bits[ currentSourceOffset + 1 ];
				bitsConverted[ currentDestinationOffset + 2 ] = bits[ currentSourceOffset + 2 ];
				currentDestinationOffset += 3;
				currentSourceOffset += 4;
			}
			currentImageWidth = ( currentImageWidth > 1 ) ? currentImageWidth / 2 : 1;
			currentImageHeight = ( currentImageHeight > 1 ) ? currentImageHeight / 2 : 1;
		}
	}
	return bitsConverted;
}

bool getDDSBasicFormatInformation( FileSystem &inputFs, const String &textureFilePath, bool &outOldFormat, dds::dxgi_format &outDx10Format )
{
	auto ddsFile = inputFs.open( textureFilePath, FileSystem::read | FileSystem::binary );
	if( ddsFile == nullptr )
	{
		error( "dds", textureFilePath, "Cannot open texture object file" );
		return false;
	}
	u32 magic = 0;
	if( !ddsFile->blockRead( &magic, 0, sizeof( magic ) ) )
	{
		error( "dds", textureFilePath, "File is corrupted" );
		return false;
	}
	if( magic != dds::MAGIC )
	{
		error_f( "dds", textureFilePath, "Invalid DDS magic: %i expected: %i", magic, dds::MAGIC );
		return false;
	}
	dds::header ddsHeader;
	if( !ddsFile->blockRead( &ddsHeader, sizeof( magic ), sizeof( ddsHeader ) ) )
	{
		error( "dds", textureFilePath, "File is corrupted" );
		return false;
	}
	if( !!( ddsHeader.m_pixel_format.m_flags & dds::pixel_flags::four_cc ) && ddsHeader.m_pixel_format.m_four_cc == dds::DXT10 )
	{
		dds::header_dxt10 ddsHeaderDxt10;
		if( !ddsFile->blockRead( &ddsHeaderDxt10, sizeof( magic ) + sizeof( ddsHeader ), sizeof( ddsHeaderDxt10 ) ) )
		{
			error( "dds", textureFilePath, "File is corrupted" );
			return false;
		}
		outOldFormat = false;
		outDx10Format = ddsHeaderDxt10.m_dxgi_format;
	}
	else
	{
		outOldFormat = true;
		outDx10Format = dds::dxgi_format::format_unknown;
	}

	return true;
}

// Returns true if texture file has been converted, or if conversion was not needed
// Returns false on failure
bool convertDDSFromDX10Format( FileSystem &inputFs, const String &textureFilePath, FileSystem &outputFs, bool ddsOnlyHeader )
{
	auto ddsFile = inputFs.open( textureFilePath, FileSystem::read | FileSystem::binary );
	if( !ddsFile )
	{
		error( "dds", textureFilePath, "Cannot open texture object file" );
		return false;
	}
	SizedArray<u8, sizeof( u32 ) + sizeof( dds::header )> ddsBufferHeader; // magic + header
	if( !ddsFile->blockRead( ddsBufferHeader.data(), 0, ddsBufferHeader.size() ) )
	{
		error( "dds", textureFilePath, "File is corrupted" );
		return false;
	}
	const u32 magic = interpretBufferAt<u32>( ddsBufferHeader, 0 );
	if( magic != dds::MAGIC )
	{
		error_f( "dds", textureFilePath, "Invalid DDS magic: %i expected: %i", magic, dds::MAGIC );
		return false;
	}
	const dds::header &ddsHeader = interpretBufferAt<dds::header>( ddsBufferHeader, sizeof( magic ) );
	if( !( ddsHeader.m_pixel_format.m_flags & dds::pixel_flags::four_cc ) )
	{
		return true;
	}
	if( ddsHeader.m_pixel_format.m_four_cc != dds::DXT10 )
	{
		return true;
	}
	Array<u8> ddsBufferHeaderDxt10( static_cast< size_t >( sizeof( dds::header_dxt10 ) ) );
	if( !ddsFile->blockRead( ddsBufferHeaderDxt10.data(), sizeof( u32 ) + sizeof( dds::header ), ddsBufferHeaderDxt10.size() ) )
	{
		error( "dds", textureFilePath, "File is corrupted" );
		return false;
	}
	const dds::header_dxt10 &ddsHeaderDxt10 = interpretBufferAt<dds::header_dxt10>( ddsBufferHeaderDxt10, 0 );

	Array<u8> ddsBufferBits;
	if( ddsOnlyHeader == false )
	{
		const u32 allHeadersLength = sizeof( u32 ) + sizeof( dds::header ) + sizeof( dds::header_dxt10 );
		ddsBufferBits.resize( static_cast< size_t >( ddsFile->size() ) - allHeadersLength );
		if( !ddsFile->blockRead( ddsBufferBits.data(), allHeadersLength, ddsBufferBits.size() ) )
		{
			error( "dds", textureFilePath, "File is corrupted" );
			return false;
		}
	}
	ddsFile.reset();

	dds::header ddsHeaderConverted = ddsHeader;

	ddsHeaderConverted.m_flags |= dds::header_flags::linearsize;
	ddsHeaderConverted.m_pitch_or_linear_size = ddsHeaderConverted.m_width * ddsHeaderConverted.m_height;

	const u32 facesStoredInDDS = !!( ddsHeader.m_caps2 & dds::caps2::cubemap ) ? 6 : 1;
	const u32 imagesStoredInDDS = std::max( 1u, ddsHeader.m_mip_map_count );

	const dds::dxgi_format ddsFormat = ddsHeaderDxt10.m_dxgi_format;

	Optional<Array<u8>> ddsImagesBitsConverted;

	if( ddsFormat == dds::dxgi_format::format_bc1_unorm || ddsFormat == dds::dxgi_format::format_bc1_unorm_srgb || ddsFormat == dds::dxgi_format::format_bc1_typeless )
	{
		ddsHeaderConverted.m_pixel_format.m_four_cc = dds::COMPRESS_DXT1;
	}
	else if( ddsFormat == dds::dxgi_format::format_bc2_unorm ) // ?
	{
		ddsHeaderConverted.m_pixel_format.m_four_cc = dds::COMPRESS_DXT2; // ?
	}
	else if( ddsFormat == dds::dxgi_format::format_bc2_unorm_srgb ) // ?
	{
		ddsHeaderConverted.m_pixel_format.m_four_cc = dds::COMPRESS_DXT3; // ?
	}
	else if( ddsFormat == dds::dxgi_format::format_bc3_unorm ) // ?
	{
		ddsHeaderConverted.m_pixel_format.m_four_cc = dds::COMPRESS_DXT4; // ?
	}
	else if( ddsFormat == dds::dxgi_format::format_bc3_unorm_srgb )
	{
		ddsHeaderConverted.m_pixel_format.m_four_cc = dds::COMPRESS_DXT5;
	}
	else if( ddsFormat == dds::dxgi_format::format_bc4_typeless || ddsFormat == dds::dxgi_format::format_bc4_unorm || ddsFormat == dds::dxgi_format::format_bc4_snorm )
	{
		ddsHeaderConverted.m_pixel_format.m_four_cc = dds::COMPRESS_ATI1;
	}
	else if( ddsFormat == dds::dxgi_format::format_bc5_typeless || ddsFormat == dds::dxgi_format::format_bc5_unorm || ddsFormat == dds::dxgi_format::format_bc5_snorm )
	{
		ddsHeaderConverted.m_pixel_format.m_four_cc = dds::COMPRESS_ATI2;
	}
	else if( ddsFormat == dds::dxgi_format::format_b8g8r8x8_unorm_srgb || ddsFormat == dds::dxgi_format::format_b8g8r8x8_unorm )
	{
		ddsHeaderConverted.m_pixel_format = dds::PIXEL_FORMAT_B8G8R8;
		if( ddsOnlyHeader == false )
		{
			ddsImagesBitsConverted = convertImageBits_B8G8R8X8_To_B8G8R8( ddsBufferBits.data(), ddsBufferBits.size(), ddsHeader.m_width, ddsHeader.m_height, facesStoredInDDS, imagesStoredInDDS );
		}
	}
	else if( ddsFormat == dds::dxgi_format::format_b8g8r8a8_unorm_srgb || ddsFormat == dds::dxgi_format::format_b8g8r8a8_unorm )
	{
		ddsHeaderConverted.m_pixel_format = dds::PIXEL_FORMAT_B8G8R8A8;
	}
	else if( ddsFormat == dds::dxgi_format::format_r8_unorm )
	{
		ddsHeaderConverted.m_pixel_format = dds::PIXEL_FORMAT_R8;
	}
	else
	{
		error_f( "dds", textureFilePath, "File cannot be converted, as format %s(%u) is not supported!", dds::stringize_dxgi_format( ddsFormat ), ( u32 )ddsFormat );
		return false;
	}

	auto ddsFileOutput = outputFs.open( textureFilePath, FileSystem::write | FileSystem::binary );
	if( !ddsFileOutput )
	{
		warning( "dds", textureFilePath, "Cannot open texture for write!" );
		return false;
	}

	if( !ddsFileOutput->blockWrite( &magic, sizeof( magic ) ) ||
		!ddsFileOutput->blockWrite( &ddsHeaderConverted, sizeof( ddsHeaderConverted ) ) )
	{
		warning_f( "dds", textureFilePath, "Write to file failed!" );
		return false;
	}

	if( ddsOnlyHeader == false )
	{
		if( ddsImagesBitsConverted.has_value() )
		{
			if( !ddsFileOutput->blockWrite( ddsImagesBitsConverted.value().data(), ddsImagesBitsConverted.value().size() ) )
			{
				warning_f( "dds", textureFilePath, "Write to file failed!" );
				return false;
			}
		}
		else
		{
			if( !ddsFileOutput->blockWrite( ddsBufferBits.data(), ddsBufferBits.size() ) )
			{
				warning_f( "dds", textureFilePath, "Write to file failed!" );
				return false;
			}
		}
	}

	return true;
}

Array<String> textureObjectReadTextures( const Array<u8> &tobjBuffer )
{
	Array<String> texturesFilePaths;
	for( uint32_t i = 0, currentTextureOffset = sizeof( prism::tobj_header_t ); currentTextureOffset < tobjBuffer.size(); ++i )
	{
		const prism::tobj_texture_t &texture = interpretBufferAt<prism::tobj_texture_t>( tobjBuffer, currentTextureOffset );
		const char *const textureStringBegin = &interpretBufferAt<char>( tobjBuffer, currentTextureOffset + sizeof( prism::tobj_texture_t ), texture.m_length );
		texturesFilePaths.push_back( String( textureStringBegin, texture.m_length ) );
		currentTextureOffset += sizeof( prism::tobj_texture_t ) + texture.m_length;
	}
	return texturesFilePaths;
}

bool disassembleCubemapTextureObject_DDS( FileSystem &inputFs, const String &tobjFilePath, const String &textureFilePath, FileSystem &outputFs, Array<String> &generatedTextureFilePath, bool ddsOnlyHeader )
{
	auto ddsFile = inputFs.open( resolveTextureFilePath( textureFilePath, tobjFilePath ), FileSystem::read | FileSystem::binary );
	if( !ddsFile )
	{
		error_f( "tobj", tobjFilePath, "dds \'%s\': Cannot open texture file!", inputFs.root( textureFilePath ) );
		return false;
	}
	Array<u8> ddsBufferHeader( static_cast< size_t >( sizeof( u32 ) + sizeof( dds::header ) ) ); // magic + header
	if( !ddsFile->blockRead( ddsBufferHeader.data(), 0, ddsBufferHeader.size() ) )
	{
		error_f( "tobj", tobjFilePath, "dds \'%s\': Unable to read DDS header!", textureFilePath );
		return false;
	}
	const u32 magic = interpretBufferAt<u32>( ddsBufferHeader, 0 );
	if( magic != dds::MAGIC )
	{
		error_f( "tobj", tobjFilePath, "dds \'%s\': File has invalid magic: %i expected: %i!", textureFilePath, magic, dds::MAGIC );
		return false;
	}
	const dds::header &ddsHeader = interpretBufferAt<dds::header>( ddsBufferHeader, sizeof( magic ) );
	if( !( ddsHeader.m_caps2 & dds::caps2::cubemap ) )
	{
		error_f( "tobj", tobjFilePath, "dds \'%s\': File is not cubemap!", textureFilePath );
		return false;
	}
	if( ( ddsHeader.m_caps2 & dds::caps2::cubemap_all ) != dds::caps2::cubemap_all )
	{
		error_f( "tobj", tobjFilePath, "dds \'%s\': Cubemap is required to have all 6 faces!", textureFilePath );
		return false;
	}
	if( !( ddsHeader.m_pixel_format.m_flags & dds::pixel_flags::four_cc ) )
	{
		error_f( "tobj", tobjFilePath, "dds \'%s\': Only DX10 cubemap textures can be extracted", textureFilePath );
		return false;
	}
	if( ddsHeader.m_pixel_format.m_four_cc != dds::DXT10 )
	{
		error_f( "tobj", tobjFilePath, "dds \'%s\': Only DX10 cubemap textures can be extracted", textureFilePath );
		return false;
	}
	Array<u8> ddsBufferHeaderDxt10( static_cast< size_t >( sizeof( dds::header_dxt10 ) ) );
	if( !ddsFile->blockRead( ddsBufferHeaderDxt10.data(), sizeof( u32 ) + sizeof( dds::header ), ddsBufferHeaderDxt10.size() ) )
	{
		error_f( "tobj", tobjFilePath, "dds \'%s\': Unable to read DXT10 header!", textureFilePath );
		return false;
	}
	const dds::header_dxt10 &ddsHeaderDxt10 = interpretBufferAt<dds::header_dxt10>( ddsBufferHeaderDxt10, 0 );

	const u32 allHeadersLength = sizeof( u32 ) + sizeof( dds::header ) + sizeof( dds::header_dxt10 );
	Array<u8> ddsBufferBits( static_cast< size_t >( ddsFile->size() ) - allHeadersLength );
	if( !ddsFile->blockRead( ddsBufferBits.data(), allHeadersLength, ddsBufferBits.size() ) )
	{
		error_f( "tobj", tobjFilePath, "dds: \'%s\': Unable to read bits!", textureFilePath );
		return false;
	}
	ddsFile.reset();

	constexpr u32 facesCount = 6;

	if( ( ddsBufferBits.size() % facesCount ) != 0 )
	{
		error_f( "tobj", tobjFilePath, "dds: \'%s\': Bits should be divisible by %u!", textureFilePath, facesCount );
		return false;
	}

	const u32 faceBitsSize = ddsBufferBits.size() / facesCount; // in bytes

	assert( ddsBufferBits.size() > 0 ); // in order to prepare hashes, we need data

	u64 facesBitsHash[ facesCount ] = {};
	for( u32 faceIndex = 0; faceIndex < facesCount; ++faceIndex )
	{
		facesBitsHash[ faceIndex ] = prism::city_hash_64( ddsBufferBits.data() + faceBitsSize * faceIndex, faceBitsSize );
	}

	// following array holds index to other face, if it is duplicate, if not it will be own index
	// for example, let's say there are only 3 unique textures, then array might look like this:
	// [0] = 0 (own index)
	// [1] = 1 (own index)
	// [2] = 1 (using face index 1)
	// [3] = 3 (own index)
	// [4] = 1 (using face index 1)
	// [5] = 3 (using face index 3)
	u32 uniqueTextureIndex[ facesCount ] = {};
	for( u32 faceIndex = 0; faceIndex < facesCount; ++faceIndex )
	{
		uniqueTextureIndex[ faceIndex ] = faceIndex; // initially set own index
		const u64 faceBitsHash = facesBitsHash[ faceIndex ];
		for( u32 innerFaceIndex = 0; innerFaceIndex < faceIndex; ++innerFaceIndex )
		{
			if( uniqueTextureIndex[ innerFaceIndex ] == innerFaceIndex && faceBitsHash == facesBitsHash[ innerFaceIndex ] )
			{
				uniqueTextureIndex[ faceIndex ] = innerFaceIndex;
				break;
			}
		}
	}

	dds::header ddsHeaderConverted = ddsHeader;
	ddsHeaderConverted.m_caps2 = dds::caps2{};
	ddsHeaderConverted.m_caps = dds::caps::texture | ( ddsHeaderConverted.m_mip_map_count > 0 ? dds::caps::complex : dds::caps{} );

	dds::header_dxt10 ddsHeaderDxt10Converted = ddsHeaderDxt10;
	ddsHeaderDxt10Converted.m_misc_flag = dds::misc{};

	for( u32 faceIndex = 0, textureNameIndex = 0; faceIndex < facesCount; ++faceIndex )
	{
		String currentTextureFilePath;
		const bool isUniqueTexture = uniqueTextureIndex[ faceIndex ] == faceIndex;
		if( isUniqueTexture )
		{
			currentTextureFilePath = removeExtension( textureFilePath ) + "___c" + std::to_string( ++textureNameIndex ) + ".dds";
		}
		else
		{
			currentTextureFilePath = generatedTextureFilePath[ uniqueTextureIndex[ faceIndex ] ];
		}
		generatedTextureFilePath.push_back( currentTextureFilePath );

		if( !isUniqueTexture )
		{
			continue;
		}

		auto ddsFileOutput = outputFs.open( resolveTextureFilePath( currentTextureFilePath, tobjFilePath ), FileSystem::write | FileSystem::binary );

		if( !ddsFileOutput->blockWrite( &magic, sizeof( magic ) ) ||
			!ddsFileOutput->blockWrite( &ddsHeaderConverted, sizeof( ddsHeaderConverted ) ) ||
			!ddsFileOutput->blockWrite( &ddsHeaderDxt10Converted, sizeof( ddsHeaderDxt10Converted ) ) )
		{
			warning_f( "tobj", tobjFilePath, "dds: \'%s\': Write to file failed!", currentTextureFilePath );
		}
		if( ddsOnlyHeader == false )
		{
			if( !ddsFileOutput->blockWrite( ddsBufferBits.data() + faceBitsSize * faceIndex, faceBitsSize ) )
			{
				warning_f( "tobj", tobjFilePath, "dds: \'%s\': Write to file failed!", currentTextureFilePath );
			}
		}
	}

	return true;
}

bool disassembleCubemapTextureObject( FileSystem &inputFs, const String &tobjFilePath, const prism::tobj_header_t &tobjHeader, const String &textureFilePath, FileSystem &outputFs,
									  Array<String> &generatedTextureFilePaths, bool ddsOnlyHeader )
{
	bool deassembleTextureSuccess = false;
	if( extractExtension( textureFilePath ).value_or( "" ) == ".dds" )
	{
		deassembleTextureSuccess = disassembleCubemapTextureObject_DDS( inputFs, tobjFilePath, textureFilePath, outputFs, generatedTextureFilePaths, ddsOnlyHeader );
	}
	else
	{
		warning_f( "tobj", tobjFilePath, "Unable to disassemble given texture format: \'%s\'", textureFilePath );
		return false;
	}

	if( !deassembleTextureSuccess )
	{
		return false;
	}

	assert( generatedTextureFilePaths.size() == 6 ); // cubemap should have 6 textures

	UniquePtr<File> tobjFileOutput = outputFs.open( tobjFilePath, FileSystem::write | FileSystem::binary );
	if( tobjFileOutput == nullptr )
	{
		warning_f( "tobj", tobjFilePath, "Cannot open texture object \'%s\' for write!", outputFs.root( tobjFilePath ) );
		return false;
	}
	if( !tobjFileOutput->blockWrite( &tobjHeader, sizeof( tobjHeader ) ) )
	{
		warning_f( "tobj", tobjFilePath, "Write to texture object \'%s\' failed!", outputFs.root( tobjFilePath ) );
		return false;
	}
	for( const String &currentTextureFilePath : generatedTextureFilePaths )
	{
		prism::tobj_texture_t texture = {};
		texture.m_length = currentTextureFilePath.length();
		if( !tobjFileOutput->blockWrite( &texture, sizeof( texture ) ) ||
			!tobjFileOutput->blockWrite( currentTextureFilePath.data(), currentTextureFilePath.length() ) )
		{
			warning_f( "tobj", tobjFilePath, "Write to texture object \'%s\' failed!", outputFs.root( tobjFilePath ) );
			return false;
		}
	}

	return true;
}

// Blender as of 2024 does not really support DDS DX10. Thus we want to convert DDS header to DirectX9 compatible.
// Additionally, we want to disassemble cubemap DDS into multiple simple DDS files, and TOBJ is modified accordingly.
// Returns false if some error occurred.
// Returns true even though conversion has been not made - because tobj+dds is already in the proper format.
bool convertTextureObjectToOldFormatsIfNeeded( FileSystem &fs, const String &tobjFilePath, FileSystem &fileSystemToWriteTo, const bool ddsOnlyHeader )
{
	auto tobjFile = fs.open( tobjFilePath, FileSystem::read | FileSystem::binary );
	if( !tobjFile )
	{
		error( "tobj", tobjFilePath, "Cannot open texture object file" );
		return false;
	}
	Array<u8> tobjBuffer;
	if( !tobjFile->getContents( tobjBuffer ) )
	{
		error( "tobj", tobjFilePath, "Unable to read texture object file" );
		return false;
	}
	tobjFile.reset();

	const prism::tobj_header_t &tobjHeader = interpretBufferAt<prism::tobj_header_t>( tobjBuffer, 0 );
	if( tobjHeader.m_version != prism::tobj_header_t::SUPPORTED_MAGIC )
	{
		error_f( "tobj", tobjFilePath, "Invalid version of tobj file! (have: %i, expected: %i)", tobjHeader.m_version, prism::tobj_header_t::SUPPORTED_MAGIC );
		return false;
	}

	const Array<String> texturesFilePaths = textureObjectReadTextures( tobjBuffer );

	Array<Pair<FileSystem*, String>> texturesToConvert;

	if( tobjHeader.m_type == prism::tobj_type_t::cubic && texturesFilePaths.size() == 1 )
	{
		const String &textureFilePath = texturesFilePaths[ 0 ];

		Array<String> generatedTextureFilePaths;
		if( !disassembleCubemapTextureObject( fs, tobjFilePath, tobjHeader, textureFilePath, fileSystemToWriteTo, generatedTextureFilePaths, ddsOnlyHeader ) )
		{
			return false;
		}

		if( &fs == &fileSystemToWriteTo )
		{
			fs.remove( resolveTextureFilePath( textureFilePath, tobjFilePath ) );
		}

		for( const String &generatedTextureFilePath : generatedTextureFilePaths )
		{
			texturesToConvert.push_back( Pair<FileSystem *, String>( &fileSystemToWriteTo, generatedTextureFilePath ) );
		}
	}
	else
	{
		for( const String &textureFilePath : texturesFilePaths )
		{
			texturesToConvert.push_back( Pair<FileSystem *, String>( &fs, textureFilePath ) );
		}
	}

	for( const Pair<FileSystem *, String> &textureToConvert : texturesToConvert )
	{
		if( extractExtension( textureToConvert.second ).value_or( "" ) == ".dds" )
		{
			bool textureOldFormat = false;
			dds::dxgi_format textureDx10Format = dds::dxgi_format::format_unknown;
			if( !getDDSBasicFormatInformation( *textureToConvert.first, resolveTextureFilePath( textureToConvert.second, tobjFilePath ), textureOldFormat, textureDx10Format ) )
			{
				error_f( "tobj", tobjFilePath, "Unable to get basic information about DDS \'%s\'!", textureToConvert.second );
				return false;
			}

			if( textureOldFormat == false )
			{
				if( s_ddsDxt10 == false )
				{
					if( !convertDDSFromDX10Format( *textureToConvert.first, resolveTextureFilePath( textureToConvert.second, tobjFilePath ), fileSystemToWriteTo, ddsOnlyHeader ) )
					{
						error_f( "tobj", tobjFilePath, "Unable to convert DDS \'%s\' to non-DXT10 format.", textureToConvert.second );
						return false;
					}
				}

				bool noCompressInTobjToSet = false;
				static const dds::dxgi_format noCompressFormats[] =
				{
					dds::dxgi_format::format_b8g8r8x8_unorm_srgb,
					dds::dxgi_format::format_b8g8r8x8_unorm,
					dds::dxgi_format::format_b8g8r8a8_unorm_srgb,
					dds::dxgi_format::format_b8g8r8a8_unorm,
					dds::dxgi_format::format_r8_unorm
				};
				for( dds::dxgi_format noCompressFormat : noCompressFormats )
				{
					if( textureDx10Format == noCompressFormat )
					{
						noCompressInTobjToSet = true;
					}
				}

				bool customColorSpaceInTobjToSet = false;
				static const dds::dxgi_format linearColorSpaceFormats[] =
				{
					dds::dxgi_format::format_bc2_unorm,
					dds::dxgi_format::format_bc3_unorm,
					dds::dxgi_format::format_bc4_unorm,
					dds::dxgi_format::format_bc4_snorm,
					dds::dxgi_format::format_bc5_unorm,
					dds::dxgi_format::format_bc5_snorm,
					dds::dxgi_format::format_b8g8r8x8_unorm,
					dds::dxgi_format::format_b8g8r8a8_unorm,
					dds::dxgi_format::format_r8_unorm
				};
				for( dds::dxgi_format linearColorSpaceFormat : linearColorSpaceFormats )
				{
					if( textureDx10Format == linearColorSpaceFormat )
					{
						customColorSpaceInTobjToSet = true;
					}
				}

				// workaround for SCS packer not writing m_custom_color_space when extracting files. It should be part of tobj extraction.
				if( noCompressInTobjToSet || customColorSpaceInTobjToSet )
				{
					Array<u8> newTobjBuffer = tobjBuffer;
					prism::tobj_header_t &newTobjHeader = *reinterpret_cast< prism::tobj_header_t * >( newTobjBuffer.data() );
					newTobjHeader.m_custom_color_space = customColorSpaceInTobjToSet;
					newTobjHeader.m_nocompress = noCompressInTobjToSet;
					if( const auto newTobjFile = fileSystemToWriteTo.open( tobjFilePath, FileSystem::write | FileSystem::binary ) )
					{
						newTobjFile->blockWrite( newTobjBuffer.data(), newTobjBuffer.size() );
					}
				}
			}
		}
	}

	return true;
}

/* eof */
