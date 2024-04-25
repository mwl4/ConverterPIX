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

// Loads .tobj file from UFS, and prepares object to be written to mid formats (along with texture).
// It is now pretty complicated, because:
// Prior 1.50 tobj files are stored in HashFS_v1 or ZipFS or SysFS, where TOBJ and DDS files are simply stored as a files, and DDS is in non-DXT10 format, cubemap is individual 6 DDS files.
// Since 1.50 tobj files can be stored in all file systems listed above + in HashFS_v2.
// Texture object in HashFS_v2 is stored as single file, with extension .tobj, but it does not contain .tobj binary file anymore. It contains DDS bits only. All other properties are stored as metadata.
// Additionally, cubemaps are packed together to just one DDS. In order to be able to load it all to Blender, we have to disassemble them into 6 individual DDS files.
// We must also handle case, when files are extracted from HashFS_v2 with SCS Extractor, and loaded here from SysFS.
bool TextureObject::load( String filepath )
{
	const Optional<String > extension = extractExtension( filepath );
	if( !extension.has_value() || extension.value() != ".tobj" )
	{
		warning( "tobj", filepath, "This loader supports only .tobj files!" );
		return false;
	}

	return loadPreFirstStep( getUFS(), filepath );
}

// In first step, we make sure TOBJ is extracted along with DDS files.
bool TextureObject::loadPreFirstStep( FileSystem *fs, String filepath )
{
	// extract and load from memory if needed
	MetaStat metaStat;
	if( !fs->mstat( &metaStat, filepath ) )
	{
		warning_f( "tobj", filepath, "Unable to mstat file!" );
		return false;
	}
	if( metaStat.m_meta.size() > 0 )
	{
		MemFileSystem memFileSystem;
		if( !extractTextureObject( filepath, metaStat, memFileSystem, false /* cannot be turned on because of cubemap image duplication remover */ ) )
		{
			warning_f( "tobj", filepath, "Unable to extract file!" );
			return false;
		}
		return loadPreSecondStep( &memFileSystem, filepath );
	}

	return loadPreSecondStep( fs, filepath );
}

bool TextureObject::loadPreSecondStep( FileSystem *fs, String filepath )
{
	UberFileSystem localUfs;
	localUfs.mount( fs, 1 );

	{
		// mount temporary device to which we will write potential results
		FileSystem &fsToWriteTo = *localUfs.mount( std::make_unique< MemFileSystem >(), 2 );

		// Makes sure texture object is in proper format
		if( !convertTextureObjectToOldFormatsIfNeeded( *fs, filepath, fsToWriteTo ) )
		{
			printf( "Unable to convert tobj to old formats: %s\n", filepath.c_str() );
			return false;
		}
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
	m_nocompress = header->m_compress == prism::tobj_compress_t::no;
	m_noanisotropic = header->m_anisotropic == prism::tobj_anisotropic_t::no;
	m_linearColorSpace = header->m_color_space == prism::tobj_color_space_t::linear;

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
		m_usage = Usage::ui;
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
	const dds::header &header = interpretBufferAt<dds::header>( buffer, sizeof( magic ) );

	if( m_linearColorSpace )
	{
		if( ( !!( header.m_pixel_format.m_flags & dds::pixel_flags::four_cc ) && header.m_pixel_format.m_four_cc == dds::COMPRESS_ATI2 )
			|| ( header.m_pixel_format == dds::PIXEL_FORMAT_R16G16 ) )
		{
			m_usage = Usage::tsnormal;
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

	Optional< MemFileSystem > optionalExtractedTobjFs;
	{
		MetaStat metaStat;
		if( !getUFS()->mstat( &metaStat, m_filepath ) )
		{
			warning_f( "tobj", m_filepath, "Unable to mstat file!" );
			return false;
		}
		if( metaStat.m_meta.size() > 0 )
		{
			optionalExtractedTobjFs.emplace();
			if( !extractTextureObject( m_filepath, metaStat, optionalExtractedTobjFs.value() ) )
			{
				printf( "Unable to extract tobj: %s\n", m_filepath.c_str() );
				return false;
			}
		}
	}

	FileSystem *const inputFileSystem = optionalExtractedTobjFs.has_value() ? as<FileSystem>( &optionalExtractedTobjFs.value() ) : as<FileSystem>( getUFS() );

	UberFileSystem conversionLocalUfs;
	conversionLocalUfs.mount( inputFileSystem, 1 );
	{
		// mount temporary device to which we will write potential results
		FileSystem &conversionFsToWriteTo = *conversionLocalUfs.mount( std::make_unique< MemFileSystem >(), 2 );
		if( !convertTextureObjectToOldFormatsIfNeeded( *inputFileSystem, m_filepath, conversionFsToWriteTo ) )
		{
			printf( "Unable to convert tobj to old formats: %s\n", m_filepath.c_str() );
			return false;
		}
	}

	*file << fmt::sprintf("map %s" SEOL, mapType(m_type).c_str());
	for (uint32_t i = 0; i < m_texturesCount; ++i)
	{
		*file << TAB << m_textures[i].c_str() << SEOL;

		auto inputf = conversionLocalUfs.open(m_textures[i], FileSystem::read | FileSystem::binary);
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
		if ( m_usage != Usage::ui && m_mipFilter == TextureObject::NOMIPS)
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

	if ( m_usage != Usage::ui && m_nocompress)
	{
		*file << "nocompress" << SEOL;
	}

	if (m_usage == Usage::none && m_linearColorSpace)
	{
		*file << "color_space linear" << SEOL;
	}

	if (m_usage != Usage::none)
	{
		*file << "usage " << ( m_usage == Usage::tsnormal ? "tsnormal" : "ui") << SEOL;
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

static Array<u8> convertImageBits_R9G9B9E5_SHAREDEXP_To_E5B9G9R9( const u8 *bits, u32 bitsLength, u32 width, u32 height, u32 facesCount, u32 imagesCount )
{
	auto bitsConverted = Array<u8>( static_cast< size_t >( bitsLength ) );
	u32 currentOffset = 0;
	u32 currentImageWidth = width, currentImageHeight = height;
	for( u32 faceIndex = 0; faceIndex < facesCount; ++faceIndex )
	{
		for( u32 imageIndex = 0; imageIndex < imagesCount; ++imageIndex )
		{
			const u32 currentImagePixelsCount = currentImageWidth * currentImageHeight;
			for( u32 donePixels = 0; donePixels < currentImagePixelsCount; ++donePixels )
			{
				const u32 bitsU32 = *reinterpret_cast<const u32 *>( &bits[ currentOffset ] );
				u32 &bitsConvertedU32 = *reinterpret_cast<u32 *>( &bitsConverted[ currentOffset ] );
				
				//bitsConvertedU32 = ( ( ( bitsU32 >> 27 ) & 0b11111 )     << 0  ) | // E
				//				     ( ( ( bitsU32 >> 18 ) & 0b111111111 ) << 5  ) | // B
				//				     ( ( ( bitsU32 >> 9  ) & 0b111111111 ) << 14 ) | // G
				//				     ( ( ( bitsU32 >> 0  ) & 0b111111111 ) << 23 );  // R

				// Okay, that is really weird. Basing on all my experience, I should move bits around here.
				// But basing on /asset/skybox/shared/grayscale.dds it is not really the case.
				bitsConvertedU32 = bitsU32;

				currentOffset += sizeof( u32 );
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
	SizedArray<u8, sizeof( u32 ) + sizeof( dds::header ) + sizeof( dds::header_dxt10 )> ddsHeaderBuffer;
	const u64 ddsHeaderBufferSize = ddsFile->read( ddsHeaderBuffer.data(), sizeof( u8 ), ddsHeaderBuffer.size() );
	if( ddsHeaderBufferSize < sizeof( u32 ) + sizeof( dds::header ) )
	{
		error( "dds", textureFilePath, "File is corrupted: unable to read DDS magic + header!" );
		return false;
	}

	const u32 magic = interpretBufferAt<u32>( ddsHeaderBuffer, 0 );
	if( magic != dds::MAGIC )
	{
		error_f( "dds", textureFilePath, "Invalid DDS magic: %i expected: %i", magic, dds::MAGIC );
		return false;
	}
	const dds::header &ddsHeader = interpretBufferAt<dds::header>( ddsHeaderBuffer, sizeof( magic ) );
	if( !!( ddsHeader.m_pixel_format.m_flags & dds::pixel_flags::four_cc ) && ddsHeader.m_pixel_format.m_four_cc == dds::DXT10 )
	{
		if( ddsHeaderBufferSize < sizeof( u32 ) + sizeof( dds::header ) + sizeof( dds::header_dxt10 ) )
		{
			error( "dds", textureFilePath, "File is corrupted: unable to read DDS DXT10 header!" );
			return false;
		}
		const dds::header_dxt10 &ddsHeaderDxt10 = interpretBufferAt<dds::header_dxt10>( ddsHeaderBuffer, sizeof( u32 ) + sizeof( dds::header ) );

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
bool convertDDSFromDX10Format( FileSystem &inputFs, const String &textureFilePath, FileSystem &outputFs )
{
	auto ddsFile = inputFs.open( textureFilePath, FileSystem::read | FileSystem::binary );
	if( !ddsFile )
	{
		error( "dds", textureFilePath, "Cannot open texture object file" );
		return false;
	}

	constexpr static u32 allHeadersLength = sizeof( u32 ) + sizeof( dds::header ) + sizeof( dds::header_dxt10 );

	Array<u8> ddsBuffer( static_cast<size_t>( ddsFile->size() ) );
	if( !ddsFile->blockRead( ddsBuffer.data(), 0, ddsBuffer.size() ) )
	{
		error( "dds", textureFilePath, "Unable to read file!" );
		return false;
	}

	const u32 magic = interpretBufferAt<u32>( ddsBuffer, 0 );
	if( magic != dds::MAGIC )
	{
		error_f( "dds", textureFilePath, "Invalid DDS magic: %i expected: %i", magic, dds::MAGIC );
		return false;
	}
	const dds::header &ddsHeader = interpretBufferAt<dds::header>( ddsBuffer, sizeof( magic ) );
	if( !( ddsHeader.m_pixel_format.m_flags & dds::pixel_flags::four_cc ) )
	{
		return true;
	}
	if( ddsHeader.m_pixel_format.m_four_cc != dds::DXT10 )
	{
		return true;
	}
	const dds::header_dxt10 &ddsHeaderDxt10 = interpretBufferAt<dds::header_dxt10>( ddsBuffer, sizeof( u32 ) + sizeof( dds::header ) );

	const Span<u8> ddsBufferBits = Span<u8>( ddsBuffer.data() + allHeadersLength, static_cast<size_t>( ddsBuffer.size() - allHeadersLength ) );

	ddsFile.reset();

	dds::header ddsHeaderConverted = ddsHeader;

	ddsHeaderConverted.m_flags &= ~dds::header_flags::pitch;
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
		ddsImagesBitsConverted = convertImageBits_B8G8R8X8_To_B8G8R8( ddsBufferBits.data(), ddsBufferBits.size(), ddsHeader.m_width, ddsHeader.m_height, facesStoredInDDS, imagesStoredInDDS );
	}
	else if( ddsFormat == dds::dxgi_format::format_b8g8r8a8_unorm_srgb || ddsFormat == dds::dxgi_format::format_b8g8r8a8_unorm )
	{
		ddsHeaderConverted.m_pixel_format = dds::PIXEL_FORMAT_B8G8R8A8;
	}
	else if( ddsFormat == dds::dxgi_format::format_r8_unorm )
	{
		ddsHeaderConverted.m_pixel_format = dds::PIXEL_FORMAT_R8;
	}
	else if( ddsFormat == dds::dxgi_format::format_r9g9b9e5_sharedexp )
	{
		ddsHeaderConverted.m_pixel_format = dds::PIXEL_FORMAT_E5B9G9R9;
		ddsImagesBitsConverted = convertImageBits_R9G9B9E5_SHAREDEXP_To_E5B9G9R9( ddsBufferBits.data(), ddsBufferBits.size(), ddsHeader.m_width, ddsHeader.m_height, facesStoredInDDS, imagesStoredInDDS );
	}
	else
	{
		error_f( "dds", textureFilePath, "File cannot be converted, as format %s(%u) is not supported!", dds::stringize_dxgi_format( ddsFormat ), ( u32 )ddsFormat );
		return false;
	}

	auto ddsFileOutput = outputFs.open( textureFilePath, FileSystem::write | FileSystem::binary );
	if( !ddsFileOutput )
	{
		warning_f( "dds", textureFilePath, "Cannot open texture \'%s\' for write!", outputFs.root( textureFilePath ) );
		return false;
	}

	if( !ddsFileOutput->blockWrite( &magic, sizeof( magic ) ) ||
		!ddsFileOutput->blockWrite( &ddsHeaderConverted, sizeof( ddsHeaderConverted ) ) )
	{
		warning_f( "dds", textureFilePath, "Write to file failed!" );
		return false;
	}

	const Span<u8> ddsImagesBitsToWrite = ddsImagesBitsConverted.has_value()
		? Span<u8>( ddsImagesBitsConverted.value().data(), ddsImagesBitsConverted.value().size() )
		: Span<u8>( ddsBufferBits.data(), ddsBufferBits.size() );

	if( !ddsFileOutput->blockWrite( ddsImagesBitsToWrite.data(), ddsImagesBitsToWrite.size() ) )
	{
		warning_f( "dds", textureFilePath, "Write to file failed!" );
		return false;
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

bool disassembleCubemapTextureObject_DDS( FileSystem &inputFs, const String &tobjFilePath, const String &textureFilePath, FileSystem &outputFs, Array<String> &generatedTextureFilePath )
{
	auto ddsFile = inputFs.open( resolveTextureFilePath( textureFilePath, tobjFilePath ), FileSystem::read | FileSystem::binary );
	if( !ddsFile )
	{
		error_f( "tobj", tobjFilePath, "dds \'%s\': Cannot open texture file!", inputFs.root( textureFilePath ) );
		return false;
	}

	constexpr static u32 allHeadersLength = sizeof( u32 ) + sizeof( dds::header ) + sizeof( dds::header_dxt10 );

	Array<u8> ddsBuffer( static_cast<size_t>( ddsFile->size() ) );
	if( !ddsFile->blockRead( ddsBuffer.data(), 0, ddsBuffer.size() ) )
	{
		error( "dds", textureFilePath, "Unable to read file!" );
		return false;
	}
	const u32 magic = interpretBufferAt<u32>( ddsBuffer, 0 );
	if( magic != dds::MAGIC )
	{
		error_f( "tobj", tobjFilePath, "dds \'%s\': File has invalid magic: %i expected: %i!", textureFilePath, magic, dds::MAGIC );
		return false;
	}
	const dds::header &ddsHeader = interpretBufferAt<dds::header>( ddsBuffer, sizeof( magic ) );
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
	const dds::header_dxt10 &ddsHeaderDxt10 = interpretBufferAt<dds::header_dxt10>( ddsBuffer, sizeof( magic ) + sizeof( ddsHeader ) );

	const Span<u8> ddsBufferBits = Span<u8>( ddsBuffer.data() + allHeadersLength, static_cast< size_t >( ddsBuffer.size() - allHeadersLength ) );

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
			warning_f( "tobj", tobjFilePath, "dds: \'%s\': Write to file failed (headers)!", currentTextureFilePath );
			return false;
		}
		if( !ddsFileOutput->blockWrite( ddsBufferBits.data() + faceBitsSize * faceIndex, faceBitsSize ) )
		{
			warning_f( "tobj", tobjFilePath, "dds: \'%s\': Write to file failed (bits)!", currentTextureFilePath );
			return false;
		}
	}

	return true;
}

bool disassembleCubemapTextureObject( FileSystem &inputFs, const String &tobjFilePath, const prism::tobj_header_t &tobjHeader, const String &textureFilePath,
									  FileSystem &outputFs, Array<String> &generatedTextureFilePaths )
{
	bool deassembleTextureSuccess = false;
	if( extractExtension( textureFilePath ).value_or( "" ) == ".dds" )
	{
		deassembleTextureSuccess = disassembleCubemapTextureObject_DDS( inputFs, tobjFilePath, textureFilePath, outputFs, generatedTextureFilePaths );
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
bool convertTextureObjectToOldFormatsIfNeeded( FileSystem &fs, const String &tobjFilePath, FileSystem &fileSystemToWriteTo )
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
		if( !disassembleCubemapTextureObject( fs, tobjFilePath, tobjHeader, textureFilePath, fileSystemToWriteTo, generatedTextureFilePaths ) )
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
					if( !convertDDSFromDX10Format( *textureToConvert.first, resolveTextureFilePath( textureToConvert.second, tobjFilePath ), fileSystemToWriteTo ) )
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
					dds::dxgi_format::format_r8_unorm,
					dds::dxgi_format::format_r9g9b9e5_sharedexp,
				};
				for( dds::dxgi_format noCompressFormat : noCompressFormats )
				{
					if( textureDx10Format == noCompressFormat )
					{
						noCompressInTobjToSet = true;
					}
				}

				bool linearColorSpaceInTobjToSet = false;
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
					dds::dxgi_format::format_r8_unorm,
					dds::dxgi_format::format_r9g9b9e5_sharedexp,
				};
				for( dds::dxgi_format linearColorSpaceFormat : linearColorSpaceFormats )
				{
					if( textureDx10Format == linearColorSpaceFormat )
					{
						linearColorSpaceInTobjToSet = true;
					}
				}

				// workaround for SCS packer not writing m_custom_color_space when extracting files. It should be part of tobj extraction.
				if( noCompressInTobjToSet || linearColorSpaceInTobjToSet )
				{
					Array<u8> newTobjBuffer = tobjBuffer;
					prism::tobj_header_t &newTobjHeader = interpretBufferAt<prism::tobj_header_t>( newTobjBuffer, 0 );
					newTobjHeader.m_color_space = linearColorSpaceInTobjToSet ? prism::tobj_color_space_t::linear : prism::tobj_color_space_t::srgb;
					newTobjHeader.m_compress = noCompressInTobjToSet ? prism::tobj_compress_t::no : prism::tobj_compress_t::yes;
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
