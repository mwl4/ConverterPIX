/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/model/animation.cpp
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

#include "animation.h"

#include <fs/file.h>
#include <fs/sysfilesystem.h>
#include <fs/uberfilesystem.h>
#include <structs/pma_0x03.h>
#include <structs/pma_0x04.h>
#include <structs/pma_0x05.h>
#include <model/model.h>
#include <pix/pix.h>

#include <glm/gtx/transform.hpp>

using namespace prism;

inline static u64 getSkeletonHashFromHeader( const pma_0x03::pma_header_t &header ) { return 0; }
inline static u64 getSkeletonHashFromHeader( const pma_0x04::pma_header_t &header ) { return header.m_skeleton_hash; }
inline static u64 getSkeletonHashFromHeader( const pma_0x05::pma_header_t &header ) { return header.m_skeleton_hash; }

template<typename pma_header_t, typename pma_frame_t>
bool Animation::loadAnim( Span<u8> buffer )
{
	using pma_flags_t = decltype( pma_header_t::m_flags );

	pma_header_t &header = interpretBufferAt<pma_header_t>( buffer, 0 );

	m_skeletonHash = getSkeletonHashFromHeader( header );

	m_totalLength = header.m_anim_length;

	m_bones.resize( header.m_bones );
	m_frames.resize( header.m_bones );
	m_timeframes.resize( header.m_frames );

	for( u32 frameIndex = 0; frameIndex < header.m_frames; ++frameIndex )
	{
		m_timeframes[ frameIndex ] = interpretBufferAt<float>( buffer, header.m_lengths_offset + frameIndex * sizeof( float ) );
	}

	for( u32 i = 0; i < m_bones.size(); ++i )
	{
		m_bones[ i ] = interpretBufferAt<BoneIndex>( buffer, header.m_bones_offset + i * sizeof( u8 ) );

		Array<Frame> &destBoneFrameArray = m_frames[ i ];
		destBoneFrameArray.resize( header.m_frames );

		for( u32 frameIndex = 0; frameIndex < header.m_frames; ++frameIndex )
		{
			pma_frame_t &frame = interpretBufferAt<pma_frame_t>( buffer, header.m_frames_offset + i * sizeof( pma_frame_t ) + frameIndex * m_bones.size() * sizeof( pma_frame_t ) );
			Frame &destBoneFrame = destBoneFrameArray[ frameIndex ];
			destBoneFrame.m_scaleOrientation = frame.m_scale_orient;
			destBoneFrame.m_rotation = frame.m_rot;
			destBoneFrame.m_translation = frame.m_trans;
			destBoneFrame.m_scale = frame.m_scale;
		}
	}

	if( !!( header.m_flags & pma_flags_t::movement ) )
	{
		Array<Float3> movement( static_cast<size_t>( header.m_frames ) );
		for( u32 frameIndex = 0; frameIndex < header.m_frames; ++frameIndex )
		{
			movement[ frameIndex ] = interpretBufferAt<Float3>( buffer, header.m_delta_trans_offset + frameIndex * sizeof( Float3 ) );
		}
		m_movement = std::make_unique<Array<Float3>>( std::move( movement ) );
	}

	return true;
}

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
	UniquePtr<File> file = getUFS()->open(pmaFilepath, FileSystem::read | FileSystem::binary);
	if (!file)
	{
		error("animation", m_filePath, "Cannot open animation file!");
		return false;
	}

	const size_t fileSize = static_cast<size_t>( file->size() );
	UniquePtr<u8[]> buffer( new u8[ fileSize ] );
	file->read( buffer.get(), sizeof( u8 ), fileSize );
	file.reset();

	const Span<u8> bufferSpan( buffer.get(), fileSize );

	const u32 pmaVersion = interpretBufferAt<u32>( bufferSpan, 0 );

	switch( pmaVersion )
	{
		case pma_0x03::pma_header_t::SUPPORTED_VERSION: return loadAnim<pma_0x03::pma_header_t, pma_0x03::pma_frame_t>( bufferSpan );
		case pma_0x04::pma_header_t::SUPPORTED_VERSION: return loadAnim<pma_0x04::pma_header_t, pma_0x04::pma_frame_t>( bufferSpan );
		case pma_0x05::pma_header_t::SUPPORTED_VERSION: return loadAnim<pma_0x05::pma_header_t, pma_0x05::pma_frame_t>( bufferSpan );
	}

	error_f( "animation", m_filePath, "Invalid version of the file (have: %u, expected: %u, %u or %u)",
			 pmaVersion, pma_0x03::pma_header_t::SUPPORTED_VERSION, pma_0x04::pma_header_t::SUPPORTED_VERSION, pma_0x05::pma_header_t::SUPPORTED_VERSION );

	return false;
}

void Animation::saveToPia(String exportPath) const
{
	const String piafile = exportPath + m_filePath + ".pia";
	UniquePtr<File> file = getSFS()->open(piafile, FileSystem::write | FileSystem::binary);
	if (!file)
	{
		error_f("animation", piafile, "Unable to save file (%s)", getSFS()->getError());
		return;
	}

	Pix::Value root;

	Pix::Value &header = root["Header"];
	header["FormatVersion"] = 3;
	header["Source"] = STRING_VERSION;
	header["Type"] = "Animation";
	header["Name"] = m_filePath.substr(m_filePath.rfind('/') + 1);

	Pix::Value &global = root["Global"];
	global["Skeleton"] = relativePath((m_model->filePath() + ".pis"), directory(m_filePath));
	global["TotalTime"] = double{ m_totalLength };
	global["BoneChannelCount"] = (int)m_bones.size();
	global["CustomChannelCount"] = m_movement ? 1 : 0;

	if (m_movement)
	{
		Pix::Value &channel = root["CustomChannel"];
		channel["Name"] = "Prism Movement";
		channel["StreamCount"] = 2;
		channel["KeyframeCount"] = m_timeframes.size();

		{
			Pix::Value &stream = channel["Stream"];
			stream["Format"] = Pix::Value::Enumeration("FLOAT");
			stream["Tag"] = "_TIME";
			stream.allocateIndexedObjects(m_timeframes.size());
			for (size_t timeframe = 0; timeframe < m_timeframes.size(); ++timeframe)
			{
				stream[timeframe] = Float1(m_timeframes[timeframe]);
			}
		}
		{
			Pix::Value &stream = channel["Stream"];
			stream["Format"] = Pix::Value::Enumeration("FLOAT3");
			stream["Tag"] = "_MOVEMENT";
			stream.allocateIndexedObjects(m_timeframes.size());
			for (size_t keyframe = 0; keyframe < m_timeframes.size(); ++keyframe)
			{
				stream[keyframe] = (*m_movement)[keyframe];
			}
		}
	}

	for (size_t boneIndex = 0; boneIndex < m_bones.size(); ++boneIndex)
	{
		if (m_bones[boneIndex] >= m_model->boneCount())
		{
			warning_f("animation", m_filePath, "Bone index outside bones array! [%i/%i]", (int)m_bones[boneIndex], m_model->boneCount());
			return;
		}

		const auto bone = m_model->bone(m_bones[boneIndex]);

		Pix::Value &channel = root["BoneChannel"];
		channel["Name"] = bone->m_name;
		channel["StreamCount"] = 2;
		channel["KeyframeCount"] = m_timeframes.size();

		{
			Pix::Value &stream = channel["Stream"];
			stream["Format"] = Pix::Value::Enumeration("FLOAT");
			stream["Tag"] = "_TIME";
			stream.allocateIndexedObjects(m_timeframes.size());
			for (size_t timeframe = 0; timeframe < m_timeframes.size(); ++timeframe)
			{
				stream[timeframe] = Float1(m_timeframes[timeframe]);
			}
		}
		{
			Pix::Value &stream = channel["Stream"];
			stream["Format"] = Pix::Value::Enumeration("FLOAT4x4");
			stream["Tag"] = "_MATRIX";
			stream.allocateIndexedObjects(m_timeframes.size());
			for (size_t keyframe = 0; keyframe < m_timeframes.size(); ++keyframe)
			{
				const Frame *frame = &m_frames[boneIndex][keyframe];
				const glm::vec3 trans = glm_cast(frame->m_translation);
				const glm::quat rot = glm_cast(frame->m_rotation);
				const glm::vec3 scale = glm_cast(frame->m_scale) * bone->m_signOfDeterminantOfMatrix;
				const prism::mat4 mat = glm::translate(trans) * glm::mat4_cast(rot) * glm::scale(scale);
				stream[keyframe] = mat;
			}
		}
	}

	Pix::StyledFileWriter writer;
	writer.write(file.get(), root);
	file->flush();
}

/* eof */
