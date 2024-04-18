/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/structs/dds.cpp
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

#include <prerequisites.h>

#include "dds.h"

#include <fs/file.h>
#include <fs/sysfilesystem.h>

namespace dds
{

const char *stringize_dxgi_format( dxgi_format format )
{
    switch( format )
    {
        case dxgi_format::format_unknown: return "UNKNOWN";
        case dxgi_format::format_r32g32b32a32_typeless: return "R32G32B32A32_TYPELESS";
        case dxgi_format::format_r32g32b32a32_float: return "R32G32B32A32_FLOAT";
        case dxgi_format::format_r32g32b32a32_uint: return "R32G32B32A32_UINT";
        case dxgi_format::format_r32g32b32a32_sint: return "R32G32B32A32_SINT";
        case dxgi_format::format_r32g32b32_typeless: return "R32G32B32_TYPELESS";
        case dxgi_format::format_r32g32b32_float: return "R32G32B32_FLOAT";
        case dxgi_format::format_r32g32b32_uint: return "R32G32B32_UINT";
        case dxgi_format::format_r32g32b32_sint: return "R32G32B32_SINT";
        case dxgi_format::format_r16g16b16a16_typeless: return "R16G16B16A16_TYPELESS";
        case dxgi_format::format_r16g16b16a16_float: return "R16G16B16A16_FLOAT";
        case dxgi_format::format_r16g16b16a16_unorm: return "R16G16B16A16_UNORM";
        case dxgi_format::format_r16g16b16a16_uint: return "R16G16B16A16_UINT";
        case dxgi_format::format_r16g16b16a16_snorm: return "R16G16B16A16_SNORM";
        case dxgi_format::format_r16g16b16a16_sint: return "R16G16B16A16_SINT";
        case dxgi_format::format_r32g32_typeless: return "R32G32_TYPELESS";
        case dxgi_format::format_r32g32_float: return "R32G32_FLOAT";
        case dxgi_format::format_r32g32_uint: return "R32G32_UINT";
        case dxgi_format::format_r32g32_sint: return "R32G32_SINT";
        case dxgi_format::format_r32g8x24_typeless: return "R32G8X24_TYPELESS";
        case dxgi_format::format_d32_float_s8x24_uint: return "D32_FLOAT_S8X24_UINT";
        case dxgi_format::format_r32_float_x8x24_typeless: return "R32_FLOAT_X8X24_TYPELESS";
        case dxgi_format::format_x32_typeless_g8x24_uint: return "X32_TYPELESS_G8X24_UINT";
        case dxgi_format::format_r10g10b10a2_typeless: return "R10G10B10A2_TYPELESS";
        case dxgi_format::format_r10g10b10a2_unorm: return "R10G10B10A2_UNORM";
        case dxgi_format::format_r10g10b10a2_uint: return "R10G10B10A2_UINT";
        case dxgi_format::format_r11g11b10_float: return "R11G11B10_FLOAT";
        case dxgi_format::format_r8g8b8a8_typeless: return "R8G8B8A8_TYPELESS";
        case dxgi_format::format_r8g8b8a8_unorm: return "R8G8B8A8_UNORM";
        case dxgi_format::format_r8g8b8a8_unorm_srgb: return "R8G8B8A8_UNORM_SRGB";
        case dxgi_format::format_r8g8b8a8_uint: return "R8G8B8A8_UINT";
        case dxgi_format::format_r8g8b8a8_snorm: return "R8G8B8A8_SNORM";
        case dxgi_format::format_r8g8b8a8_sint: return "R8G8B8A8_SINT";
        case dxgi_format::format_r16g16_typeless: return "R16G16_TYPELESS";
        case dxgi_format::format_r16g16_float: return "R16G16_FLOAT";
        case dxgi_format::format_r16g16_unorm: return "R16G16_UNORM";
        case dxgi_format::format_r16g16_uint: return "R16G16_UINT";
        case dxgi_format::format_r16g16_snorm: return "R16G16_SNORM";
        case dxgi_format::format_r16g16_sint: return "R16G16_SINT";
        case dxgi_format::format_r32_typeless: return "R32_TYPELESS";
        case dxgi_format::format_d32_float: return "D32_FLOAT";
        case dxgi_format::format_r32_float: return "R32_FLOAT";
        case dxgi_format::format_r32_uint: return "R32_UINT";
        case dxgi_format::format_r32_sint: return "R32_SINT";
        case dxgi_format::format_r24g8_typeless: return "R24G8_TYPELESS";
        case dxgi_format::format_d24_unorm_s8_uint: return "D24_UNORM_S8_UINT";
        case dxgi_format::format_r24_unorm_x8_typeless: return "R24_UNORM_X8_TYPELESS";
        case dxgi_format::format_x24_typeless_g8_uint: return "X24_TYPELESS_G8_UINT";
        case dxgi_format::format_r8g8_typeless: return "R8G8_TYPELESS";
        case dxgi_format::format_r8g8_unorm: return "R8G8_UNORM";
        case dxgi_format::format_r8g8_uint: return "R8G8_UINT";
        case dxgi_format::format_r8g8_snorm: return "R8G8_SNORM";
        case dxgi_format::format_r8g8_sint: return "R8G8_SINT";
        case dxgi_format::format_r16_typeless: return "R16_TYPELESS";
        case dxgi_format::format_r16_float: return "R16_FLOAT";
        case dxgi_format::format_d16_unorm: return "D16_UNORM";
        case dxgi_format::format_r16_unorm: return "R16_UNORM";
        case dxgi_format::format_r16_uint: return "R16_UINT";
        case dxgi_format::format_r16_snorm: return "R16_SNORM";
        case dxgi_format::format_r16_sint: return "R16_SINT";
        case dxgi_format::format_r8_typeless: return "R8_TYPELESS";
        case dxgi_format::format_r8_unorm: return "R8_UNORM";
        case dxgi_format::format_r8_uint: return "R8_UINT";
        case dxgi_format::format_r8_snorm: return "R8_SNORM";
        case dxgi_format::format_r8_sint: return "R8_SINT";
        case dxgi_format::format_a8_unorm: return "A8_UNORM";
        case dxgi_format::format_r1_unorm: return "R1_UNORM";
        case dxgi_format::format_r9g9b9e5_sharedexp: return "R9G9B9E5_SHAREDEXP";
        case dxgi_format::format_r8g8_b8g8_unorm: return "R8G8_B8G8_UNORM";
        case dxgi_format::format_g8r8_g8b8_unorm: return "G8R8_G8B8_UNORM";
        case dxgi_format::format_bc1_typeless: return "BC1_TYPELESS";
        case dxgi_format::format_bc1_unorm: return "BC1_UNORM";
        case dxgi_format::format_bc1_unorm_srgb: return "BC1_UNORM_SRGB";
        case dxgi_format::format_bc2_typeless: return "BC2_TYPELESS";
        case dxgi_format::format_bc2_unorm: return "BC2_UNORM";
        case dxgi_format::format_bc2_unorm_srgb: return "BC2_UNORM_SRGB";
        case dxgi_format::format_bc3_typeless: return "BC3_TYPELESS";
        case dxgi_format::format_bc3_unorm: return "BC3_UNORM";
        case dxgi_format::format_bc3_unorm_srgb: return "BC3_UNORM_SRGB";
        case dxgi_format::format_bc4_typeless: return "BC4_TYPELESS";
        case dxgi_format::format_bc4_unorm: return "BC4_UNORM";
        case dxgi_format::format_bc4_snorm: return "BC4_SNORM";
        case dxgi_format::format_bc5_typeless: return "BC5_TYPELESS";
        case dxgi_format::format_bc5_unorm: return "BC5_UNORM";
        case dxgi_format::format_bc5_snorm: return "BC5_SNORM";
        case dxgi_format::format_b5g6r5_unorm: return "B5G6R5_UNORM";
        case dxgi_format::format_b5g5r5a1_unorm: return "B5G5R5A1_UNORM";
        case dxgi_format::format_b8g8r8a8_unorm: return "B8G8R8A8_UNORM";
        case dxgi_format::format_b8g8r8x8_unorm: return "B8G8R8X8_UNORM";
        case dxgi_format::format_r10g10b10_xr_bias_a2_unorm: return "R10G10B10_XR_BIAS_A2_UNORM";
        case dxgi_format::format_b8g8r8a8_typeless: return "B8G8R8A8_TYPELESS";
        case dxgi_format::format_b8g8r8a8_unorm_srgb: return "B8G8R8A8_UNORM_SRGB";
        case dxgi_format::format_b8g8r8x8_typeless: return "B8G8R8X8_TYPELESS";
        case dxgi_format::format_b8g8r8x8_unorm_srgb: return "B8G8R8X8_UNORM_SRGB";
        case dxgi_format::format_bc6h_typeless: return "BC6H_TYPELESS";
        case dxgi_format::format_bc6h_uf16: return "BC6H_UF16";
        case dxgi_format::format_bc6h_sf16: return "BC6H_SF16";
        case dxgi_format::format_bc7_typeless: return "BC7_TYPELESS";
        case dxgi_format::format_bc7_unorm: return "BC7_UNORM";
        case dxgi_format::format_bc7_unorm_srgb: return "BC7_UNORM_SRGB";
        case dxgi_format::format_ayuv: return "AYUV";
        case dxgi_format::format_y410: return "Y410";
        case dxgi_format::format_y416: return "Y416";
        case dxgi_format::format_nv12: return "NV12";
        case dxgi_format::format_p010: return "P010";
        case dxgi_format::format_p016: return "P016";
        case dxgi_format::format_420_opaque: return "420_OPAQUE";
        case dxgi_format::format_yuy2: return "YUY2";
        case dxgi_format::format_y210: return "Y210";
        case dxgi_format::format_y216: return "Y216";
        case dxgi_format::format_nv11: return "NV11";
        case dxgi_format::format_ai44: return "AI44";
        case dxgi_format::format_ia44: return "IA44";
        case dxgi_format::format_p8: return "P8";
        case dxgi_format::format_a8p8: return "A8P8";
        case dxgi_format::format_b4g4r4a4_unorm: return "B4G4R4A4_UNORM";
        case dxgi_format::format_p208: return "P208";
        case dxgi_format::format_v208: return "V208";
        case dxgi_format::format_v408: return "V408";
        default: return "UNKNOWN";
    }
}

void print_debug( const String &filepath )
{
    auto file = getSFS()->open( filepath, FileSystem::read | FileSystem::binary );
    if( file == nullptr )
    {
        printf( "Cannot open dds file: \"%s\"! %s" SEOL, filepath.c_str(), strerror( errno ) );
        return;
    }

    Array<u8> headerBuffer( static_cast< size_t >( sizeof( u32 ) + sizeof( dds::header ) ) );
    if( !file->blockRead( headerBuffer.data(), 0, headerBuffer.size() ) )
    {
        printf( "Unable to read file!\n" );
        return;
    }

    const u32 magic = interpretBufferAt<u32>( headerBuffer, 0 );
    if( magic != dds::MAGIC )
    {
        printf( "Invalid dds magic: %i expected: %i\n", magic, dds::MAGIC );
        return;
    }

    const dds::header &header = interpretBufferAt<dds::header>( headerBuffer, sizeof( magic ) );

    if( !!( header.m_pixel_format.m_flags & dds::pixel_flags::four_cc ) )
    {
        if( header.m_pixel_format.m_four_cc == dds::DXT10 )
        {
            Array<u8> dxt10HeaderBuffer( static_cast< size_t >( sizeof( dds::header_dxt10 ) ) );
            if( !file->blockRead( dxt10HeaderBuffer.data(), headerBuffer.size(), dxt10HeaderBuffer.size() ) )
            {
                printf( "Unable to read file!\n" );
                return;
            }

            const dds::header_dxt10 &dxt10Header = interpretBufferAt<dds::header_dxt10>( dxt10HeaderBuffer, 0 );

            printf( "dxt10\n" );

            if( const char *const format = stringize_dxgi_format( dxt10Header.m_dxgi_format ) )
            {
                printf( "pixel format = %s\n", format );
            }
            else
            {
                printf( "pixel format not known\n" );
            }
        }
        else
        {
            printf( "compression: %s\n", dds::uint2s( header.m_pixel_format.m_four_cc ).c_str() );
        }
    }
    else
    {
        if( const auto pixelFormat = dds::recognize_pixel_format( header.m_pixel_format ) )
        {
            printf( "pixel format = %s\n", pixelFormat->m_name );
        }
        else
        {
            printf( "pixel format not known\n" );
        }
        printf( "bits: %i masks(rgba): %X/%X/%X/%X flags: %X\n",
                header.m_pixel_format.m_rgb_bit_count,
                header.m_pixel_format.m_r_bit_mask,
                header.m_pixel_format.m_g_bit_mask,
                header.m_pixel_format.m_b_bit_mask,
                header.m_pixel_format.m_a_bit_mask,
                header.m_pixel_format.m_flags );
    }
    
    printf( "mipmaps count: %u\n", header.m_mip_map_count );

    printf( "cubemap: %s\n", !!( header.m_caps2 & dds::caps2::cubemap ) ? "yes" : "no" );
}
}

/* eof */
