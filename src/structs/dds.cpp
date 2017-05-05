/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : dds.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>

#include "dds.h"

#include <fs/file.h>
#include <fs/sysfilesystem.h>

namespace dds
{
	void print_debug(String filepath)
	{
		auto file = getSFS()->open(filepath, FileSystem::read | FileSystem::binary);
		if (!file)
		{
			printf("Cannot open dds file: \"%s\"! %s" SEOL, filepath.c_str(), strerror(errno));
			return;
		}
		else
		{
			const size_t fileSize = file->size();
			UniquePtr<uint8_t[]> buffer(new uint8_t[fileSize]);
			file->read((char *)buffer.get(), sizeof(char), fileSize);
			file.reset();

			const uint32_t magic = *(uint32_t *)(buffer.get());
			if (magic != dds::MAGIC)
			{
				printf("Invalid dds magic: %i expected: %i\n", magic, dds::MAGIC);
				return;
			}
			dds::header *header = (dds::header *)(buffer.get() + 4);
			if (header->m_pixel_format.m_flags & dds::PF_FOUR_CC)
			{
				printf("compression: %s\n", dds::uint2s(header->m_pixel_format.m_four_cc).c_str());
			}
			else
			{
				const auto pxformat = dds::recognize_pixel_format(&header->m_pixel_format);
				if (pxformat)
				{
					printf("pixel format = %s\n", pxformat->m_name);
				}
				else
				{
					printf("pixel format not known\n");
				}
				printf("bits: %i masks(rgba): %X/%X/%X/%X flags: %X\n",
					   header->m_pixel_format.m_rgb_bit_count,
					   header->m_pixel_format.m_r_bit_mask,
					   header->m_pixel_format.m_g_bit_mask,
					   header->m_pixel_format.m_b_bit_mask,
					   header->m_pixel_format.m_a_bit_mask,
					   header->m_pixel_format.m_flags);
			}
			printf("mipmaps count: %u\n", header->m_mip_map_count);
		}
	}
}

/* eof */
