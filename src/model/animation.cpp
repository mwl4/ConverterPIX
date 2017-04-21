/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : animation.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "animation.h"

#include <fs/file.h>
#include <fs/sysfilesystem.h>
#include <fs/uberfilesystem.h>
#include <structs/pma.h>
#include <model/model.h>

#include <glm/gtx/transform.hpp>

using namespace prism;

bool Animation::load(SharedPtr<Model> model, String filePath)
{
	if (!model || !model->loaded())
	{
		error_f("animation", filePath, "Model (%s) is not loaded!", model->filePath());
		return false;
	}

	m_model = model;
	if (filePath[0] == '/')
	{
		m_filePath = filePath;
	}
	else
	{
		m_filePath = m_model->fileDirectory() + "/" + filePath;
	}

	const String pmaFilepath = m_filePath + ".pma";
	auto file = getUFS()->open(pmaFilepath, FileSystem::read | FileSystem::binary);
	if (!file)
	{
		error("animation", m_filePath, "Cannot open animation file!");
		return false;
	}

	size_t fileSize = file->getSize();
	UniquePtr<uint8_t[]> buffer(new uint8_t[fileSize]);
	file->read((char *)buffer.get(), sizeof(uint8_t), fileSize);
	file.reset();

	pma_header_t *header = (pma_header_t *)(buffer.get());
	if (header->m_version != pma_header_t::SUPPORTED_VERSION)
	{
		error_f("animation", m_filePath, "Invalid version of the file! (have: %i, expected: %i)", header->m_version, pma_header_t::SUPPORTED_VERSION);
		return false;
	}

	m_name = token_to_string(header->m_name);
	m_totalLength = header->m_anim_length;

	m_bones.resize(header->m_bones);
	m_frames.resize(header->m_bones);
	m_timeframes.resize(header->m_frames);

	for (uint32_t i = 0; i < m_bones.size(); ++i)
	{
		m_bones[i] = *(uint8_t *)(buffer.get() + header->m_bones_offset + i*sizeof(uint8_t));
		m_frames[i].resize(header->m_frames);
		for (uint32_t j = 0; j < header->m_frames; ++j)
		{
			pma_frame_t *frame = (pma_frame_t *)(buffer.get() + header->m_frames_offset + i*sizeof(pma_frame_t) + j*m_bones.size()*sizeof(pma_frame_t));
			if(i == 0) m_timeframes[j] = *(float *)(buffer.get() + header->m_lengths_offset + j * sizeof(float));
			m_frames[i][j].m_scaleOrientation = frame->m_scale_orient;
			m_frames[i][j].m_rotation = frame->m_rot;
			m_frames[i][j].m_translation = frame->m_trans;
			m_frames[i][j].m_scale = frame->m_scale;
		}
	}

	if (header->m_flags == 2)
	{
		m_movement = std::make_unique<Array<Float3>>(header->m_frames);
		for (u32 i = 0; i < header->m_frames; ++i)
		{
			(*m_movement)[i] = *((float3 *)(buffer.get() + header->m_delta_trans_offset) + i);
		}
	}

	return true;
}

void Animation::saveToPia(String exportPath) const
{
	const String filename = m_filePath.substr(m_filePath.rfind('/') + 1);
	const String piafile = exportPath + m_filePath + ".pia";
	auto file = getSFS()->open(piafile, FileSystem::write | FileSystem::binary);
	if (!file)
	{
		error_f("animation", piafile, "Unable to save file (%s)", getSFS()->getError());
		return;
	}

	*file << fmt::sprintf(
		"Header {"						SEOL
		TAB "FormatVersion: 3"			SEOL
		TAB "Source: \"%s\""			SEOL
		TAB "Type: \"Animation\""		SEOL
		TAB "Name: \"%s\""				SEOL
		"}"								SEOL,
			STRING_VERSION,
			filename.c_str()
		);

	*file << fmt::sprintf(
		"Global {"						SEOL
		TAB "Skeleton: \"%s\""			SEOL
		TAB "TotalTime: %f"				SEOL
		TAB "BoneChannelCount: %i"		SEOL
		TAB "CustomChannelCount: %i"	SEOL
		"}"								SEOL,
			relativePath((m_model->filePath() + ".pis"), directory(m_filePath)).c_str(),
			m_totalLength,
			(int)m_bones.size(),
			0
		);

	if (m_movement)
	{
		*file << "CustomChannel {"			SEOL;
		*file << fmt::sprintf(
			TAB "Name: \"%s\""				SEOL
			TAB "StreamCount: %i"			SEOL
			TAB "KeyframeCount : %i"		SEOL,
				"Prism Movement",
				2,
				(int)m_timeframes.size()
			);
		*file << TAB "Stream {"		SEOL;
		{
			*file << fmt::sprintf(
				TAB TAB "Format: FLOAT"		SEOL
				TAB TAB "Tag: \"_TIME\""	SEOL
				);

			for (uint32_t j = 0; j < m_timeframes.size(); ++j)
			{
				*file << fmt::sprintf(
					TAB TAB "%-5i( " FLT_FT " )" SEOL,
						j, flh(m_timeframes[j])
					);
			}
		}
		*file << TAB "}"					SEOL;
		*file << TAB "Stream {"				SEOL;
		{
			*file << fmt::sprintf(
				TAB TAB "Format: FLOAT3"	SEOL
				TAB TAB "Tag: \"_MOVEMENT\"" SEOL
				);

			for (uint32_t j = 0; j < m_timeframes.size(); ++j)
			{
				*file << fmt::sprintf(
					TAB TAB "%-5i( " FLT_FT "  " FLT_FT "  " FLT_FT " )" SEOL,
						j, flh((*m_movement)[j][0]), flh((*m_movement)[j][1]), flh((*m_movement)[j][2])
					);
			}
		}
		*file << TAB "}"					SEOL;
		*file << "}"						SEOL;
	}

	for (uint32_t i = 0; i < m_bones.size(); ++i)
	{
		if (m_bones[i] >= m_model->boneCount())
		{
			error_f("animation", m_filePath, "Bone index outside bones array! [%i/%i]", (int)m_bones[i], m_model->boneCount());
			return;
		}

		const auto bone = m_model->bone(m_bones[i]);

		*file << "BoneChannel {"		SEOL;
		{
			*file << fmt::sprintf(
				TAB "Name: \"%s\""		SEOL
				TAB "StreamCount: %i"	SEOL
				TAB "KeyframeCount: %i"	SEOL,
					bone->m_name.c_str(),
					2,
					m_timeframes.size()
				);
			*file << TAB "Stream {"		SEOL;
			{
				*file << fmt::sprintf(
					TAB TAB "Format: FLOAT"		SEOL
					TAB TAB "Tag: \"_TIME\""	SEOL
					);

				for (uint32_t j = 0; j < m_timeframes.size(); ++j)
				{
					*file << fmt::sprintf(
						TAB TAB "%-5i( " FLT_FT " )" SEOL,
							j, flh(m_timeframes[j])
						);
				}
			}
			*file << TAB "}"			SEOL;

			*file << TAB "Stream {"		SEOL;
			{
				*file << fmt::sprintf(
					TAB TAB "Format: FLOAT4x4"	SEOL
					TAB TAB "Tag: \"_MATRIX\""	SEOL
					);

				for (uint32_t j = 0; j < m_timeframes.size(); ++j)
				{
					const Frame *frame = &m_frames[i][j];
					const glm::vec3 trans = glm_cast(frame->m_translation);
					const glm::quat rot = glm_cast(frame->m_rotation);
					const glm::vec3 scale = glm_cast(frame->m_scale) * bone->m_signOfDeterminantOfMatrix;
					const prism::mat4 mat = glm::translate(trans) * glm::mat4_cast(rot) * glm::scale(scale);

					*file << fmt::sprintf(
						TAB TAB  "%-5i(  &%08x  &%08x  &%08x  &%08x"	SEOL
						TAB TAB "        &%08x  &%08x  &%08x  &%08x"	SEOL
						TAB TAB "        &%08x  &%08x  &%08x  &%08x"	SEOL
						TAB TAB "        &%08x  &%08x  &%08x  &%08x )"	SEOL,
							j,  flh(mat[0][0]), flh(mat[1][0]), flh(mat[2][0]), flh(mat[3][0]),
								flh(mat[0][1]), flh(mat[1][1]), flh(mat[2][1]), flh(mat[3][1]),
								flh(mat[0][2]), flh(mat[1][2]), flh(mat[2][2]), flh(mat[3][2]),
								flh(mat[0][3]), flh(mat[1][3]), flh(mat[2][3]), flh(mat[3][3])
						);
				}
			}
			*file << TAB "}"			SEOL;
		}
		*file << "}"					SEOL;
	}
	file.reset();
}

/* eof */
