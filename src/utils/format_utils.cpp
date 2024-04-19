/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/utils/format_utils.cpp
 *
 *		  _____                          _            _____ _______   __
 *		 / ____|                        | |          |  __ \_   _\ \ / /
 *		| |     ___  _ ____   _____ _ __| |_ ___ _ __| |__) || |  \ V /
 *		| |    / _ \| '_ \ \ / / _ \ '__| __/ _ \ '__|  ___/ | |   > <
 *		| |___| (_) | | | \ V /  __/ |  | ||  __/ |  | |    _| |_ / . \
 *		 \_____\___/|_| |_|\_/ \___|_|   \__\___|_|  |_|   |_____/_/ \_\
 *
 *
 *  Copyright (C) 2024 Michal Wojtowicz.
 *  All rights reserved.
 *
 *   This software is ditributed WITHOUT ANY WARRANTY; without even
 *   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE. See the copyright file for more information.
 *
 *****************************************************************************/

#include "prerequisites.h"

#include "format_utils.h"

#include "structs/fs.h"

// https://github.com/microsoft/DirectStorage/blob/60a909f351d18293aeae1af7e24fc38519ebe118/Samples/BulkLoadDemo/Core/DDSTextureLoader.cpp#L162
u32 bitsPerPixel( prism::format_t fmt )
{
    switch( fmt )
    {
        case prism::format_t::format_r32g32b32a32_typeless:
        case prism::format_t::format_r32g32b32a32_float:
        case prism::format_t::format_r32g32b32a32_uint:
        case prism::format_t::format_r32g32b32a32_sint:
            return 128;

        case prism::format_t::format_r32g32b32_typeless:
        case prism::format_t::format_r32g32b32_float:
        case prism::format_t::format_r32g32b32_uint:
        case prism::format_t::format_r32g32b32_sint:
            return 96;

        case prism::format_t::format_r16g16b16a16_typeless:
        case prism::format_t::format_r16g16b16a16_float:
        case prism::format_t::format_r16g16b16a16_unorm:
        case prism::format_t::format_r16g16b16a16_uint:
        case prism::format_t::format_r16g16b16a16_snorm:
        case prism::format_t::format_r16g16b16a16_sint:
        case prism::format_t::format_r32g32_typeless:
        case prism::format_t::format_r32g32_float:
        case prism::format_t::format_r32g32_uint:
        case prism::format_t::format_r32g32_sint:
        case prism::format_t::format_r32g8x24_typeless:
        case prism::format_t::format_d32_float_s8x24_uint:
        case prism::format_t::format_r32_float_x8x24_typeless:
        case prism::format_t::format_x32_typeless_g8x24_uint:
        case prism::format_t::format_y416:
        case prism::format_t::format_y210:
        case prism::format_t::format_y216:
            return 64;

        case prism::format_t::format_r10g10b10a2_typeless:
        case prism::format_t::format_r10g10b10a2_unorm:
        case prism::format_t::format_r10g10b10a2_uint:
        case prism::format_t::format_r11g11b10_float:
        case prism::format_t::format_r8g8b8a8_typeless:
        case prism::format_t::format_r8g8b8a8_unorm:
        case prism::format_t::format_r8g8b8a8_unorm_srgb:
        case prism::format_t::format_r8g8b8a8_uint:
        case prism::format_t::format_r8g8b8a8_snorm:
        case prism::format_t::format_r8g8b8a8_sint:
        case prism::format_t::format_r16g16_typeless:
        case prism::format_t::format_r16g16_float:
        case prism::format_t::format_r16g16_unorm:
        case prism::format_t::format_r16g16_uint:
        case prism::format_t::format_r16g16_snorm:
        case prism::format_t::format_r16g16_sint:
        case prism::format_t::format_r32_typeless:
        case prism::format_t::format_d32_float:
        case prism::format_t::format_r32_float:
        case prism::format_t::format_r32_uint:
        case prism::format_t::format_r32_sint:
        case prism::format_t::format_r24g8_typeless:
        case prism::format_t::format_d24_unorm_s8_uint:
        case prism::format_t::format_r24_unorm_x8_typeless:
        case prism::format_t::format_x24_typeless_g8_uint:
        case prism::format_t::format_r9g9b9e5_sharedexp:
        case prism::format_t::format_r8g8_b8g8_unorm:
        case prism::format_t::format_g8r8_g8b8_unorm:
        case prism::format_t::format_b8g8r8a8_unorm:
        case prism::format_t::format_b8g8r8x8_unorm:
        case prism::format_t::format_r10g10b10_xr_bias_a2_unorm:
        case prism::format_t::format_b8g8r8a8_typeless:
        case prism::format_t::format_b8g8r8a8_unorm_srgb:
        case prism::format_t::format_b8g8r8x8_typeless:
        case prism::format_t::format_b8g8r8x8_unorm_srgb:
        case prism::format_t::format_ayuv:
        case prism::format_t::format_y410:
        case prism::format_t::format_yuy2:
            return 32;

        case prism::format_t::format_p010:
        case prism::format_t::format_p016:
            return 24;

        case prism::format_t::format_r8g8_typeless:
        case prism::format_t::format_r8g8_unorm:
        case prism::format_t::format_r8g8_uint:
        case prism::format_t::format_r8g8_snorm:
        case prism::format_t::format_r8g8_sint:
        case prism::format_t::format_r16_typeless:
        case prism::format_t::format_r16_float:
        case prism::format_t::format_d16_unorm:
        case prism::format_t::format_r16_unorm:
        case prism::format_t::format_r16_uint:
        case prism::format_t::format_r16_snorm:
        case prism::format_t::format_r16_sint:
        case prism::format_t::format_b5g6r5_unorm:
        case prism::format_t::format_b5g5r5a1_unorm:
        case prism::format_t::format_a8p8:
        case prism::format_t::format_b4g4r4a4_unorm:
            return 16;

        case prism::format_t::format_nv12:
        case prism::format_t::format_420_opaque:
        case prism::format_t::format_nv11:
            return 12;

        case prism::format_t::format_r8_typeless:
        case prism::format_t::format_r8_unorm:
        case prism::format_t::format_r8_uint:
        case prism::format_t::format_r8_snorm:
        case prism::format_t::format_r8_sint:
        case prism::format_t::format_a8_unorm:
        case prism::format_t::format_ai44:
        case prism::format_t::format_ia44:
        case prism::format_t::format_p8:
            return 8;

        case prism::format_t::format_r1_unorm:
            return 1;

        case prism::format_t::format_bc1_typeless:
        case prism::format_t::format_bc1_unorm:
        case prism::format_t::format_bc1_unorm_srgb:
        case prism::format_t::format_bc4_typeless:
        case prism::format_t::format_bc4_unorm:
        case prism::format_t::format_bc4_snorm:
            return 4;

        case prism::format_t::format_bc2_typeless:
        case prism::format_t::format_bc2_unorm:
        case prism::format_t::format_bc2_unorm_srgb:
        case prism::format_t::format_bc3_typeless:
        case prism::format_t::format_bc3_unorm:
        case prism::format_t::format_bc3_unorm_srgb:
        case prism::format_t::format_bc5_typeless:
        case prism::format_t::format_bc5_unorm:
        case prism::format_t::format_bc5_snorm:
        case prism::format_t::format_bc6h_typeless:
        case prism::format_t::format_bc6h_uf16:
        case prism::format_t::format_bc6h_sf16:
        case prism::format_t::format_bc7_typeless:
        case prism::format_t::format_bc7_unorm:
        case prism::format_t::format_bc7_unorm_srgb:
            return 8;

        default:
            return 0;
    }
}

// https://github.com/microsoft/DirectStorage/blob/60a909f351d18293aeae1af7e24fc38519ebe118/Samples/BulkLoadDemo/Core/DDSTextureLoader.cpp#L312
void getSurfaceInfo( size_t width, size_t height, prism::format_t fmt, size_t *outNumBytes, size_t *outRowBytes, size_t *outNumRows )
{
    size_t numBytes = 0;
    size_t rowBytes = 0;
    size_t numRows = 0;

    bool bc = false;
    bool packed = false;
    bool planar = false;
    size_t bpe = 0;
    switch( fmt )
    {
        case prism::format_t::format_bc1_typeless:
        case prism::format_t::format_bc1_unorm:
        case prism::format_t::format_bc1_unorm_srgb:
        case prism::format_t::format_bc4_typeless:
        case prism::format_t::format_bc4_unorm:
        case prism::format_t::format_bc4_snorm:
            bc = true;
            bpe = 8;
            break;

        case prism::format_t::format_bc2_typeless:
        case prism::format_t::format_bc2_unorm:
        case prism::format_t::format_bc2_unorm_srgb:
        case prism::format_t::format_bc3_typeless:
        case prism::format_t::format_bc3_unorm:
        case prism::format_t::format_bc3_unorm_srgb:
        case prism::format_t::format_bc5_typeless:
        case prism::format_t::format_bc5_unorm:
        case prism::format_t::format_bc5_snorm:
        case prism::format_t::format_bc6h_typeless:
        case prism::format_t::format_bc6h_uf16:
        case prism::format_t::format_bc6h_sf16:
        case prism::format_t::format_bc7_typeless:
        case prism::format_t::format_bc7_unorm:
        case prism::format_t::format_bc7_unorm_srgb:
            bc = true;
            bpe = 16;
            break;

        case prism::format_t::format_r8g8_b8g8_unorm:
        case prism::format_t::format_g8r8_g8b8_unorm:
        case prism::format_t::format_yuy2:
            packed = true;
            bpe = 4;
            break;

        case prism::format_t::format_y210:
        case prism::format_t::format_y216:
            packed = true;
            bpe = 8;
            break;

        case prism::format_t::format_nv12:
        case prism::format_t::format_420_opaque:
            planar = true;
            bpe = 2;
            break;

        case prism::format_t::format_p010:
        case prism::format_t::format_p016:
            planar = true;
            bpe = 4;
            break;
    }

    if( bc )
    {
        size_t numBlocksWide = 0;
        if( width > 0 )
        {
            numBlocksWide = std::max<size_t>( 1, ( width + 3 ) / 4 );
        }
        size_t numBlocksHigh = 0;
        if( height > 0 )
        {
            numBlocksHigh = std::max<size_t>( 1, ( height + 3 ) / 4 );
        }
        rowBytes = numBlocksWide * bpe;
        numRows = numBlocksHigh;
        numBytes = rowBytes * numBlocksHigh;
    }
    else if( packed )
    {
        rowBytes = ( ( width + 1 ) >> 1 ) * bpe;
        numRows = height;
        numBytes = rowBytes * height;
    }
    else if( fmt == prism::format_t::format_nv11 )
    {
        rowBytes = ( ( width + 3 ) >> 2 ) * 4;
        numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
        numBytes = rowBytes * numRows;
    }
    else if( planar )
    {
        rowBytes = ( ( width + 1 ) >> 1 ) * bpe;
        numBytes = ( rowBytes * height ) + ( ( rowBytes * height + 1 ) >> 1 );
        numRows = height + ( ( height + 1 ) >> 1 );
    }
    else
    {
        size_t bpp = bitsPerPixel( fmt );
        rowBytes = ( width * bpp + 7 ) / 8; // round up to nearest byte
        numRows = height;
        numBytes = rowBytes * height;
    }

    if( outNumBytes )
    {
        *outNumBytes = numBytes;
    }
    if( outRowBytes )
    {
        *outRowBytes = rowBytes;
    }
    if( outNumRows )
    {
        *outNumRows = numRows;
    }
}

// https://github.com/microsoft/DirectStorage/blob/60a909f351d18293aeae1af7e24fc38519ebe118/Samples/BulkLoadDemo/Core/DDSTextureLoader.cpp#L312
bool fillInitData( size_t width,
                   size_t height,
                   size_t depth,
                   size_t mipCount,
                   size_t arraySize,
                   prism::format_t format,
                   size_t maxsize,
                   size_t bitSize,
                   const uint8_t *bitData,
                   size_t &twidth,
                   size_t &theight,
                   size_t &tdepth,
                   size_t &skipMip,
                   /* [mipCount * arraySize] */ SubresourceData *initData )
{
    if( !bitData || !initData )
    {
        return false;
    }

    skipMip = 0;
    twidth = 0;
    theight = 0;
    tdepth = 0;

    size_t NumBytes = 0;
    size_t RowBytes = 0;
    size_t NumRows = 0;
    const uint8_t *pSrcBits = bitData;
    const uint8_t *pEndBits = bitData + bitSize;

    size_t index = 0;
    for( size_t j = 0; j < arraySize; j++ )
    {
        size_t w = width;
        size_t h = height;
        size_t d = depth;
        for( size_t i = 0; i < mipCount; i++ )
        {
            getSurfaceInfo( w, h, format, &NumBytes, &RowBytes, &NumRows );

            if( ( mipCount <= 1 ) || !maxsize || ( w <= maxsize && h <= maxsize && d <= maxsize ) )
            {
                if( !twidth )
                {
                    twidth = w;
                    theight = h;
                    tdepth = d;
                }

                assert( index < mipCount * arraySize );
                initData[ index ].m_data = ( const void * )pSrcBits;
                initData[ index ].m_rowPitch = static_cast< u32 >( RowBytes );
                initData[ index ].m_slicePitch = static_cast< u32 >( NumBytes );
                ++index;
            }
            else if( !j )
            {
                // Count number of skipped mipmaps (first item only)
                ++skipMip;
            }

            if( pSrcBits + ( NumBytes * d ) > pEndBits )
            {
                return false; // eof
            }

            pSrcBits += NumBytes * d;

            w = w >> 1;
            h = h >> 1;
            d = d >> 1;
            if( w == 0 )
            {
                w = 1;
            }
            if( h == 0 )
            {
                h = 1;
            }
            if( d == 0 )
            {
                d = 1;
            }
        }
    }

    return ( index > 0 );
}

/* eof */
