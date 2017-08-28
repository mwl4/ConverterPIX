/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : model.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "model.h"

#include <fs/file.h>
#include <fs/uberfilesystem.h>
#include <fs/sysfilesystem.h>

#include <pix/pix.h>
#include <resource_lib.h>
#include <texture/texture.h>
#include <prefab/prefab.h>
#include <model/collision.h>

#include <structs/pmg_0x13.h>
#include <structs/pmg_0x14.h>

#include <glm/gtx/transform.hpp>

using namespace prism;

Model::Model()
{
}

Model::~Model()
{
}

bool Model::load(String filePath)
{
	if (m_loaded)
		destroy();

	m_filePath = filePath;
	m_directory = directory(filePath);
	m_fileName = filePath.substr(m_directory.length() + 1);

	if (!loadDescriptor()) return false;
	if (!loadModel()) return false;

	loadPrefab();
	loadCollision();

	m_loaded = true;
	return true;
}

void Model::destroy()
{
	m_bones.clear();
	m_locators.clear();
	m_parts.clear();
	m_pieces.clear();
	m_looks.clear();
	m_variants.clear();

	m_vertCount = 0;
	m_triangleCount = 0;
	m_skinVertCount = 0;
	m_materialCount = 0;

	m_loaded = false;
	m_filePath = "";
	m_fileName = "";
}

bool Model::loadModel()
{
	String pmgPath = m_filePath + ".pmg";
	auto file = getUFS()->open(pmgPath, FileSystem::read | FileSystem::binary);
	if (!file)
	{
		error_f("model", m_filePath, "Unable to open geometry file [.pmg] (%s)!", strerror(errno));
		return false;
	}

	const size_t fileSize = file->size();
	UniquePtr<uint8_t[]> buffer(new uint8_t[fileSize]);
	file->read((char *)buffer.get(), sizeof(char), fileSize);
	file.reset();

	const auto version = *(const u32 *)(buffer.get());
	switch (version)
	{
		case MAKEFOURCC(0x13, 'g', 'm', 'P'): return loadModel0x13(buffer.get(), fileSize);
		case MAKEFOURCC(0x14, 'g', 'm', 'P'): return loadModel0x14(buffer.get(), fileSize);
	}

	error_f("model", m_filePath, "Invalid version of geometry file (have: %i signature: %c%c%c, expected: %i or %i)",
			buffer.get()[0], buffer.get()[3], buffer.get()[2], buffer.get()[1],
			pmg_0x13::pmg_header_t::SUPPORTED_VERSION, pmg_0x14::pmg_header_t::SUPPORTED_VERSION);

	return false;
}

bool Model::loadModel0x13(const uint8_t *const buffer, const size_t size)
{
	using namespace prism::pmg_0x13;

	if (size < sizeof(pmg_header_t))
	{
		error("model", m_filePath, "Geometry file is malformed!");
		return false;
	}

	const auto header = (const pmg_header_t *)(buffer);

	m_pieces.resize(header->m_piece_count);
	m_bones.resize(header->m_bone_count);
	m_locators.resize(header->m_locator_count);

	auto bone = (const pmg_bone_t *)(buffer + header->m_bone_offset);
	for (int32_t i = 0; i < header->m_bone_count; ++i, ++bone)
	{
		Bone *const currentBone = &m_bones[i];
		currentBone->m_index = i;
		currentBone->m_name = token_to_string(bone->m_name);
		currentBone->m_transReversed = bone->m_transformation_reversed;
		currentBone->m_transformation = bone->m_transformation;
		currentBone->m_stretch = bone->m_stretch;
		currentBone->m_rotation = bone->m_rotation;
		currentBone->m_translation = bone->m_translation;
		currentBone->m_scale = bone->m_scale;
		currentBone->m_signOfDeterminantOfMatrix = bone->m_sign_of_determinant_of_matrix;
		currentBone->m_parent = (bone->m_parent != i) ? bone->m_parent : -1;

		if (currentBone->m_name.empty())
		{
			currentBone->m_name = "noname";
		}

		String name = currentBone->m_name;
		for (int j = 0;
			 std::find_if(
				m_bones.begin(),
				m_bones.begin() + i,
				[&](const Bone &bone) -> bool
				{
					return bone.m_name == currentBone->m_name;
				}) != (m_bones.begin() + i);
			 ++j)
		{
			const auto id = std::to_string(j);
			currentBone->m_name = name.substr(0, 12 - id.length()) + id;
		}
	}

	auto part = (const pmg_part_t *)(buffer + header->m_part_offset);
	for (int32_t i = 0; i < header->m_part_count; ++i, ++part)
	{
		Part *const currentPart = &m_parts[i];
		currentPart->m_name = token_to_string(part->m_name);
		currentPart->m_locatorCount = part->m_locator_count;
		currentPart->m_locatorId = part->m_locators_idx;
		currentPart->m_pieceCount = part->m_piece_count;
		currentPart->m_pieceId = part->m_pieces_idx;
	}

	auto locator = (const pmg_locator_t *)(buffer + header->m_locator_offset);
	for (int32_t i = 0; i < header->m_locator_count; ++i, ++locator)
	{
		Locator *const currentLocator = &m_locators[i];
		currentLocator->m_index = i;
		currentLocator->m_position = locator->m_position;
		currentLocator->m_rotation = locator->m_rotation;
		currentLocator->m_scale = locator->m_scale;
		currentLocator->m_name = token_to_string(locator->m_name);

		if (locator->m_name_block_offset != -1) {
			currentLocator->m_hookup = String(
				(const char *)(buffer + header->m_locator_name_offset + locator->m_name_block_offset)
			).substr(0, header->m_locators_name_size - locator->m_name_block_offset);
		}
		else {
			currentLocator->m_hookup = "";
		}
	}

	auto piece = (const pmg_piece_t *)(buffer + header->m_piece_offset);
	for (int32_t i = 0; i < header->m_piece_count; ++i, ++piece)
	{
		Piece *const currentPiece = &m_pieces[i];
		currentPiece->m_index = i;
		currentPiece->m_texcoordMask = piece->m_uv_mask;
		currentPiece->m_texcoordCount = piece->m_uv_channels;
		currentPiece->m_bones = piece->m_bone_count;
		currentPiece->m_material = piece->m_material;

		if (piece->m_bone_count > Vertex::BONE_COUNT)
		{
			warning_f("model", m_filePath,
					"Bone count in piece: %i exceeds maximum bone count (%i/%i)! "
					"To fix it increase Vertex::BONE_COUNT constant, and recompile software.",
					i, piece->m_bone_count, Vertex::BONE_COUNT);
		}

		currentPiece->m_vertices.resize(piece->m_verts);
		m_vertCount += piece->m_verts;

		currentPiece->m_triangles.resize(piece->m_edges / 3);
		m_triangleCount += (piece->m_edges / 3);

		m_skinVertCount += currentPiece->m_bones > 0 ? currentPiece->m_vertices.size() : 0;

		uint32_t poolSizeStatic = 0;
		uint32_t poolSizeDynamic = 0;

		if (piece->m_vert_position_offset != -1)
		{
			currentPiece->m_position = true;
			++currentPiece->m_streamCount;
			poolSizeStatic += sizeof(float3);
		}
		if (piece->m_vert_normal_offset != -1)
		{
			currentPiece->m_normal = true;
			++currentPiece->m_streamCount;
			poolSizeStatic += sizeof(float3);
		}
		if (piece->m_vert_tangent_offset != -1)
		{
			currentPiece->m_tangent = true;
			++currentPiece->m_streamCount;
			poolSizeStatic += sizeof(pmg_vert_tangent_t);
		}
		if (piece->m_vert_uv_offset != -1)
		{
			currentPiece->m_texcoord = true;
			currentPiece->m_streamCount += piece->m_uv_channels;
			poolSizeDynamic += sizeof(float2)*piece->m_uv_channels;
		}
		if (piece->m_vert_rgba_offset != -1)
		{
			currentPiece->m_color = true;
			++currentPiece->m_streamCount;
			poolSizeDynamic += sizeof(uint32_t);
		}
		if (piece->m_vert_rgba2_offset != -1)
		{
			currentPiece->m_color2 = true;
			++currentPiece->m_streamCount;
			poolSizeDynamic += sizeof(uint32_t);
		}

		if (piece->m_bone_count == 0)
		{
			poolSizeStatic += poolSizeDynamic;
			poolSizeDynamic = poolSizeStatic;
		}

		for (int32_t j = 0; j < piece->m_verts; ++j)
		{
			Vertex *const vert = &currentPiece->m_vertices[j];

			if (currentPiece->m_position)
			{
				vert->m_position = *(const float3 *)(buffer + piece->m_vert_position_offset + poolSizeStatic*j);
			}
			if (currentPiece->m_normal)
			{
				vert->m_normal = *(const float3 *)(buffer + piece->m_vert_normal_offset + poolSizeStatic*j);
			}
			if (currentPiece->m_tangent)
			{
				const auto vertTangent = (const pmg_vert_tangent_t *)(buffer + piece->m_vert_tangent_offset + poolSizeStatic*j);
				vert->m_tangent[0] = vertTangent->w;
				vert->m_tangent[1] = vertTangent->x;
				vert->m_tangent[2] = vertTangent->y;
				vert->m_tangent[3] = vertTangent->z;
			}
			if (currentPiece->m_texcoord)
			{
				for (int32_t k = 0; k < piece->m_uv_channels; ++k)
				{
					vert->m_texcoords[k] = *(float2 *)(buffer + piece->m_vert_uv_offset + poolSizeDynamic*j + sizeof(float2)*k);
				}
			}
			if (currentPiece->m_color)
			{
				const auto vertRgba = (const pmg_vert_color_t *)(buffer + piece->m_vert_rgba_offset + poolSizeDynamic*j);
				vert->m_color[0] = 2.f * vertRgba->m_r / 255.f;
				vert->m_color[1] = 2.f * vertRgba->m_g / 255.f;
				vert->m_color[2] = 2.f * vertRgba->m_b / 255.f;
				vert->m_color[3] = 2.f * vertRgba->m_a / 255.f;
			}
			if (currentPiece->m_color2)
			{
				const auto vertRgba = (const pmg_vert_color_t *)(buffer + piece->m_vert_rgba2_offset + poolSizeDynamic*j);
				vert->m_color2[0] = 2.f * vertRgba->m_r / 255.f;
				vert->m_color2[1] = 2.f * vertRgba->m_g / 255.f;
				vert->m_color2[2] = 2.f * vertRgba->m_b / 255.f;
				vert->m_color2[3] = 2.f * vertRgba->m_a / 255.f;
			}
			if (piece->m_anim_bind_offset != -1)
			{
				const auto animBind = *(const uint16_t *)(buffer + piece->m_anim_bind_offset + j * sizeof(uint16_t));
				for (int k = 0; k < std::min(piece->m_bone_count, (i32)Vertex::BONE_COUNT); ++k)
				{
					vert->m_boneIndex[k] = *(const int8_t *)(buffer + piece->m_anim_bind_bones_offset + (animBind * piece->m_bone_count) + k);
					vert->m_boneWeight[k] = *(const uint8_t *)(buffer + piece->m_anim_bind_bones_weight_offset + (animBind * piece->m_bone_count) + k);
				}
				for (int k = std::min(piece->m_bone_count, (i32)Vertex::BONE_COUNT); k < Vertex::BONE_COUNT; ++k)
				{
					vert->m_boneIndex[k] = -1;
					vert->m_boneWeight[k] = 0;
				}
			}
		}

		auto triangle = (const pmg_triangle_t *)(buffer + piece->m_triangle_offset);
		for (int32_t j = 0; j < (piece->m_edges / 3); ++j, ++triangle)
		{
			currentPiece->m_triangles[j].m_attach[0] = triangle->a[0];
			currentPiece->m_triangles[j].m_attach[1] = triangle->a[1];
			currentPiece->m_triangles[j].m_attach[2] = triangle->a[2];
		}
	}
	return true;
}

bool Model::loadModel0x14(const uint8_t *const buffer, const size_t size)
{
	using namespace prism::pmg_0x14;

	if (size < sizeof(pmg_header_t))
	{
		error("model", m_filePath, "Geometry file is malformed!");
		return false;
	}

	const auto header = (const pmg_header_t *)(buffer);

	m_pieces.resize(header->m_piece_count);
	m_bones.resize(header->m_bone_count);
	m_locators.resize(header->m_locator_count);
	m_parts.resize(header->m_part_count);

	auto bone = (const pmg_bone_data_t *)(buffer + header->m_skeleton_offset);
	for (int32_t i = 0; i < header->m_bone_count; ++i, ++bone)
	{
		Bone *const currentBone = &m_bones[i];
		currentBone->m_index = i;
		currentBone->m_name = token_to_string(bone->m_name);
		currentBone->m_transReversed = bone->m_transformation_reversed;
		currentBone->m_transformation = bone->m_transformation;
		currentBone->m_stretch = bone->m_stretch;
		currentBone->m_rotation = bone->m_rotation;
		currentBone->m_translation = bone->m_translation;
		currentBone->m_scale = bone->m_scale;
		currentBone->m_signOfDeterminantOfMatrix = bone->m_sign_of_determinant_of_matrix;
		currentBone->m_parent = (bone->m_parent != i) ? bone->m_parent : -1;

		if (currentBone->m_name.empty())
		{
			currentBone->m_name = "noname";
		}

		String name = currentBone->m_name;
		for (int j = 0;
			 std::find_if(
				m_bones.begin(),
				m_bones.begin() + i,
				[&](const Bone &bone) -> bool
				{
					return bone.m_name == currentBone->m_name;
				}) != (m_bones.begin() + i);
			 ++j)
		{
			const auto id = std::to_string(j);
			currentBone->m_name = name.substr(0, 12 - id.length()) + id;
		}
	}

	auto part = (const pmg_part_t *)(buffer + header->m_parts_offset);
	for (int32_t i = 0; i < header->m_part_count; ++i, ++part)
	{
		Part *const currentPart = &m_parts[i];
		currentPart->m_name = token_to_string(part->m_name);
		currentPart->m_locatorCount = part->m_locator_count;
		currentPart->m_locatorId = part->m_locators_idx;
		currentPart->m_pieceCount = part->m_piece_count;
		currentPart->m_pieceId = part->m_pieces_idx;
	}

	auto locator = (const pmg_locator_t *)(buffer + header->m_locators_offset);
	for (int32_t i = 0; i < header->m_locator_count; ++i, ++locator)
	{
		Locator *const currentLocator = &m_locators[i];
		currentLocator->m_index = i;
		currentLocator->m_position = locator->m_position;
		currentLocator->m_rotation = locator->m_rotation;
		currentLocator->m_scale = locator->m_scale;
		currentLocator->m_name = token_to_string(locator->m_name);

		if (locator->m_hookup_offset != -1) {
			currentLocator->m_hookup = String(
				(const char *)(buffer + header->m_string_pool_offset + locator->m_hookup_offset)
			).substr(0, header->m_string_pool_size - locator->m_hookup_offset);
		}
		else {
			currentLocator->m_hookup = "";
		}
	}

	auto piece = (const pmg_piece_t *)(buffer + header->m_pieces_offset);
	for (int32_t i = 0; i < header->m_piece_count; ++i, ++piece)
	{
		Piece *const currentPiece = &m_pieces[i];
		currentPiece->m_index = i;
		currentPiece->m_texcoordMask = piece->m_texcoord_mask;
		currentPiece->m_texcoordCount = piece->m_texcoord_width;
		currentPiece->m_bones = header->m_weight_width;
		currentPiece->m_material = piece->m_material;

		currentPiece->m_vertices.resize(piece->m_verts);
		m_vertCount += piece->m_verts;

		currentPiece->m_triangles.resize(piece->m_edges / 3);
		m_triangleCount += (piece->m_edges / 3);

		m_skinVertCount += currentPiece->m_bones > 0 ? currentPiece->m_vertices.size() : 0;

		uint32_t poolSize = 0;

		if (piece->m_vert_position_offset != -1)
		{
			currentPiece->m_position = true;
			++currentPiece->m_streamCount;
			poolSize += sizeof(float3);
		}
		if (piece->m_vert_normal_offset != -1)
		{
			currentPiece->m_normal = true;
			++currentPiece->m_streamCount;
			poolSize += sizeof(float3);
		}
		if (piece->m_vert_tangent_offset != -1)
		{
			currentPiece->m_tangent = true;
			++currentPiece->m_streamCount;
			poolSize += sizeof(pmg_vert_tangent_t);
		}
		if (piece->m_vert_texcoord_offset != -1)
		{
			currentPiece->m_texcoord = true;
			currentPiece->m_streamCount += piece->m_texcoord_width;
			poolSize += sizeof(float2) * piece->m_texcoord_width;
		}
		if (piece->m_vert_color_offset != -1)
		{
			currentPiece->m_color = true;
			++currentPiece->m_streamCount;
			poolSize += sizeof(uint32_t);
		}
		if (piece->m_vert_color2_offset != -1)
		{
			currentPiece->m_color2 = true;
			++currentPiece->m_streamCount;
			poolSize += sizeof(uint32_t);
		}
		if (piece->m_vert_bone_index_offset != -1)
		{
			poolSize += 2 * sizeof(uint32_t);
		}

		for (int32_t j = 0; j < piece->m_verts; ++j)
		{
			Vertex *vert = &currentPiece->m_vertices[j];

			if (currentPiece->m_position)
			{
				vert->m_position = *(const float3 *)(buffer + piece->m_vert_position_offset + poolSize*j);
			}
			if (currentPiece->m_normal)
			{
				vert->m_normal = *(const float3 *)(buffer + piece->m_vert_normal_offset + poolSize*j);
			}
			if (currentPiece->m_tangent)
			{
				const auto vertTangent = (const pmg_vert_tangent_t *)(buffer + piece->m_vert_tangent_offset + poolSize*j);
				vert->m_tangent[0] = vertTangent->w;
				vert->m_tangent[1] = vertTangent->x;
				vert->m_tangent[2] = vertTangent->y;
				vert->m_tangent[3] = vertTangent->z;
			}
			if (currentPiece->m_texcoord)
			{
				for (int32_t k = 0; k < piece->m_texcoord_width; ++k)
				{
					vert->m_texcoords[k] = *(float2 *)(buffer + piece->m_vert_texcoord_offset + poolSize*j + sizeof(float2)*k);
				}
			}
			if (currentPiece->m_color)
			{
				const auto vertRgba = (const pmg_vert_color_t *)(buffer + piece->m_vert_color_offset + poolSize*j);
				vert->m_color[0] = 2.f * vertRgba->r / 255.f;
				vert->m_color[1] = 2.f * vertRgba->g / 255.f;
				vert->m_color[2] = 2.f * vertRgba->b / 255.f;
				vert->m_color[3] = 2.f * vertRgba->a / 255.f;
			}
			if (currentPiece->m_color2)
			{
				const auto vertRgba = (const pmg_vert_color_t *)(buffer + piece->m_vert_color_offset + poolSize*j);
				vert->m_color2[0] = 2.f * vertRgba->r / 255.f;
				vert->m_color2[1] = 2.f * vertRgba->g / 255.f;
				vert->m_color2[2] = 2.f * vertRgba->b / 255.f;
				vert->m_color2[3] = 2.f * vertRgba->a / 255.f;
			}
			if (piece->m_vert_bone_index_offset != -1 && piece->m_vert_bone_weight_offset != -1)
			{
				for (int bone = 0; bone < 4; ++bone)
				{
					const uint32_t indexes = *(const uint32_t *)(buffer + piece->m_vert_bone_index_offset + poolSize*j);
					vert->m_boneIndex[bone] = (indexes >> (8 * bone)) & 0xff;

					const uint32_t weights = *(const uint32_t *)(buffer + piece->m_vert_bone_weight_offset + poolSize*j);
					vert->m_boneWeight[bone] = (weights >> (8 * bone)) & 0xff;
				}
				for (int bone = 4; bone < Vertex::BONE_COUNT; ++bone)
				{
					vert->m_boneIndex[bone] = -1;
					vert->m_boneWeight[bone] = 0;
				}
			}
		}

		auto triangle = (const pmg_index_t *)(buffer + piece->m_index_offset);
		for (int32_t j = 0; j < (piece->m_edges / 3); ++j, ++triangle)
		{
			currentPiece->m_triangles[j].m_attach[0] = triangle->a[0];
			currentPiece->m_triangles[j].m_attach[1] = triangle->a[1];
			currentPiece->m_triangles[j].m_attach[2] = triangle->a[2];
		}
	}
	return true;
}

bool Model::loadDescriptor()
{
	const String pmdPath = m_filePath + ".pmd";

	auto file = getUFS()->open(pmdPath, FileSystem::read | FileSystem::binary);
	if(!file)
	{
		error_f("model", m_filePath, "Unable to open descriptor file [.pmd] (%s)!", strerror(errno));
		return false;
	}

	size_t fileSize = file->size();
	UniquePtr<uint8_t[]> buffer(new uint8_t[fileSize]);
	file->read((char *)buffer.get(), sizeof(uint8_t), fileSize);
	file.reset();

	if (fileSize < sizeof(pmd_header_t))
	{
		error("model", m_filePath, "Desciptor file is malformed!");
		return false;
	}

	const auto header = (pmd_header_t *)(buffer.get());
	if (header->m_version != pmd_header_t::SUPPORTED_VERSION)
	{
		error_f("model", m_filePath, "Invalid version of descriptor file! (have: %i, expected: %i)", header->m_version, pmd_header_t::SUPPORTED_VERSION);
		return false;
	}

	m_materialCount = header->m_material_count;
	m_looks.resize(header->m_look_count);

	for (uint32_t i = 0; i < m_looks.size(); ++i)
	{
		Look *currentLook = &m_looks[i];
		token_t currentNameLook = *(token_t *)(buffer.get() + header->m_look_offset + i*sizeof(token_t));

		currentLook->m_name = token_to_string(currentNameLook);
		currentLook->m_materials.resize(header->m_material_count);
		for (uint32_t j = 0; j < header->m_material_count; ++j)
		{
			uint32_t currentOffsetMat = ((i*header->m_material_count) + j)*sizeof(uint32_t);
			uint32_t offsetMaterial = *(uint32_t *)(buffer.get() + header->m_material_offset + currentOffsetMat);
			const char *materialPath = (const char *)(buffer.get() + offsetMaterial);
			currentLook->m_materials[j].load(materialPath[0] == '/' ? materialPath : (m_directory + "/" + materialPath));
			if (i == 0)
			{
				if (currentLook->m_materials[j].m_textures.size() > 0)
				{
					String textureName = String(currentLook->m_materials[j].m_textures[0].texture().c_str());
					textureName = textureName.substr(0, textureName.size() - 5);
					size_t lastSlash = textureName.rfind('/');
					if (lastSlash != String::npos)
					{
						textureName = textureName.substr(lastSlash + 1);
					}
					currentLook->m_materials[j].setAlias(fmt::sprintf("mat_%04i_%s", j, textureName.c_str()).c_str());
				}
				else
				{
					currentLook->m_materials[j].setAlias(fmt::sprintf("mat_%04i", j).c_str());
				}
			}
			else
			{
				currentLook->m_materials[j].setAlias(m_looks[0].m_materials[j].alias());
			}
		}
	}

	m_parts.resize(header->m_part_count);
	m_variants.resize(header->m_variant_count);

	for (uint32_t i = 0; i < m_variants.size(); ++i)
	{
		Variant *variant = &m_variants[i];
		token_t variantName = *((token_t *)(buffer.get() + header->m_variant_offset) + i);

		variant->m_name = variantName.to_string();
		variant->setPartCount(header->m_part_count);

		for (uint32_t j = 0; j < header->m_part_count; ++j)
		{
			(*variant)[j].m_part = &m_parts[j];
			const auto attribLink = (pmd_attrib_link_t *)(buffer.get() + header->m_part_attribs_offset) + j;
			for (int32_t k = attribLink->m_from; k < attribLink->m_to; ++k)
			{
				const auto attribDef = (pmd_attrib_def_t *)(buffer.get() + header->m_attribs_offset) + k;
				const auto attribValue = (pmd_attrib_value_t *)(buffer.get() + header->m_attribs_value_offset + attribDef->m_offset + i*header->m_attribs_values_size);
				Variant::Attribute attrib(attribDef->m_name.to_string());
				switch (attribDef->m_type)
				{
					case 0:
					{
						attrib.m_type = Variant::Attribute::INT;
						attrib.m_intValue = attribValue->m_int_value;
					} break;
					// TODO: More attributes
					default: warning_f("model", m_filePath, "Invalid attribute type in descriptor (%i)!", attribDef->m_type);
				}
				(*variant)[j].m_attributes.push_back(attrib);
			}
		}
	}
	return true;
}

bool Model::loadPrefab()
{
	if (getUFS()->exists(m_filePath + ".ppd"))
	{
		m_prefab = std::make_unique<Prefab>();
		if (!m_prefab->load(m_filePath))
		{
			m_prefab.reset();
			return false;
		}
		return true;
	}
	return false;
}

bool Model::loadCollision()
{
	if (getUFS()->exists(m_filePath + ".pmc"))
	{
		m_collision = std::make_unique<Collision>();
		if (!m_collision->load(this, m_filePath))
		{
			m_collision.reset();
			return false;
		}
		return true;
	}
	return false;
}

bool Model::saveToPim(String exportPath) const
{
	const String pimFilePath = exportPath + m_filePath + ".pim";
	auto file = getSFS()->open(pimFilePath, FileSystem::write | FileSystem::binary);
	if (!file)
	{
		error_f("model", m_filePath, "Unable to save model file [%s] (%s)!", pimFilePath, strerror(errno));
		return false;
	}

	//Pix::Value root;

	//root.allocateNamedObjects( // optimization stuff
	//	  1 // header
	//	+ 1 // global
	//	+ (m_looks.size() > 0 ? m_looks[0].m_materials.size() : 0)
	//	+ m_pieces.size()
	//	+ m_parts.size()
	//	+ m_locators.size()
	//	+ m_bones.size()
	//	+ 1 // skin
	//);

	//Pix::Value &header = root["Header"];
	//header["FormatVersion"] = 5;
	//header["Source"] = STRING_VERSION;
	//header["Type"] = "Model";
	//header["Name"] = m_fileName;

	//Pix::Value &global = root["Global"];
	//global["VertexCount"] = m_vertCount;
	//global["TriangleCount"] = m_triangleCount;
	//global["MaterialCount"] = m_materialCount;
	//global["PieceCount"] = m_pieces.size();
	//global["PartCount"] = m_parts.size();
	//global["BoneCount"] = m_bones.size();
	//global["LocatorCount"] = m_locators.size();
	//global["Skeleton"] = m_fileName + ".pis";

	//if (m_looks.size() > 0)
	//{
	//	for (const auto &mat : m_looks[0].m_materials)
	//	{
	//		root["Material"] = mat.toPixDeclaration();
	//	}
	//}

	//for (const auto &piece : m_pieces)
	//{
	//	Pix::Value &p = root["Piece"];
	//	p["Index"] = piece.m_index;
	//	p["Material"] = piece.m_material;
	//	p["VertexCount"] = piece.m_vertices.size();
	//	p["TriangleCount"] = piece.m_triangles.size();
	//	p["StreamCount"] = piece.m_streamCount;
	//	p.allocateNamedObjects(10); // optimization stuff

	//	if (piece.m_position)
	//	{
	//		Pix::Value &stream = p["Stream"];
	//		stream["Format"] = Pix::Value::Enumeration("FLOAT3");
	//		stream["Tag"] = "_POSITION";
	//		stream.allocateIndexedObjects(piece.m_vertices.size());
	//		for (size_t i = 0; i < piece.m_vertices.size(); ++i)
	//		{
	//			stream[i] = piece.m_vertices[i].m_position;
	//		}
	//	}

	//	if (piece.m_normal)
	//	{
	//		Pix::Value &stream = p["Stream"];
	//		stream["Format"] = Pix::Value::Enumeration("FLOAT3");
	//		stream["Tag"] = "_NORMAL";
	//		stream.allocateIndexedObjects(piece.m_vertices.size());
	//		for (size_t i = 0; i < piece.m_vertices.size(); ++i)
	//		{
	//			stream[i] = piece.m_vertices[i].m_normal;
	//		}
	//	}

	//	if (piece.m_tangent)
	//	{
	//		Pix::Value &stream = p["Stream"];
	//		stream["Format"] = Pix::Value::Enumeration("FLOAT3");
	//		stream["Tag"] = "_TANGENT";
	//		stream.allocateIndexedObjects(piece.m_vertices.size());
	//		for (size_t i = 0; i < piece.m_vertices.size(); ++i)
	//		{
	//			stream[i] = piece.m_vertices[i].m_tangent;
	//		}
	//	}

	//	if (piece.m_texcoord)
	//	{
	//		for (uint32_t texcoord = 0; texcoord < piece.m_texcoordCount; ++texcoord)
	//		{
	//			Pix::Value &stream = p["Stream"];
	//			stream["Format"] = Pix::Value::Enumeration("FLOAT2");
	//			stream["Tag"] = fmt::sprintf("_UV%i", texcoord);

	//			Array<uint32_t> texCoords = piece.texCoords(texcoord);
	//			stream["AliasCount"] = texCoords.size();

	//			Array<String> texCoordsString;
	//			for (const uint32_t &tex : texCoords)
	//			{
	//				texCoordsString.push_back(fmt::sprintf("_TEXCOORD%i", tex));
	//			}
	//			stream["Aliases"] = texCoordsString;

	//			stream.allocateIndexedObjects(piece.m_vertices.size());
	//			for (size_t i = 0; i < piece.m_vertices.size(); ++i)
	//			{
	//				stream[i] = piece.m_vertices[i].m_texcoords[texcoord];
	//			}
	//		}
	//	}

	//	if (piece.m_color)
	//	{
	//		Pix::Value &stream = p["Stream"];
	//		stream["Format"] = Pix::Value::Enumeration("FLOAT4");
	//		stream["Tag"] = "_RGBA";
	//		stream.allocateIndexedObjects(piece.m_vertices.size());
	//		for (size_t i = 0; i < piece.m_vertices.size(); ++i)
	//		{
	//			stream[i] = piece.m_vertices[i].m_color;
	//		}
	//	}

	//	Pix::Value &triangles = p["Triangles"];
	//	triangles.allocateIndexedObjects(piece.m_triangles.size());
	//	for (size_t i = 0; i < piece.m_triangles.size(); ++i)
	//	{
	//		triangles[i] = piece.m_triangles[i].m_attach;
	//	}
	//}

	//for (const auto &part : m_parts)
	//{
	//	Pix::Value &p = root["Part"];
	//	p["Name"] = part.m_name;
	//	p["PieceCount"] = part.m_pieceCount;
	//	p["LocatorCount"] = part.m_locatorCount;

	//	Array<int> pieces;
	//	for (uint32_t i = 0; i < part.m_pieceCount; ++i)
	//	{
	//		pieces.push_back(part.m_pieceId + i);
	//	}
	//	p["Pieces"] = pieces;

	//	Array<int> locators;
	//	for (uint32_t i = 0; i < part.m_locatorCount; ++i)
	//	{
	//		locators.push_back(part.m_locatorId + i);
	//	}
	//	p["Locators"] = locators;
	//}

	//for (const auto &locator : m_locators)
	//{
	//	Pix::Value &l = root["Locator"];
	//	l["Name"] = locator.m_name;
	//	if (!locator.m_hookup.empty())
	//	{
	//		l["Hookup"] = locator.m_hookup;
	//	}
	//	l["Index"] = locator.m_index;
	//	l["Position"] = locator.m_position;
	//	l["Rotation"] = locator.m_rotation;
	//	l["Scale"] = locator.m_scale;
	//}

	//if (!m_bones.empty())
	//{
	//	Pix::Value &bones = root["Bones"];
	//	bones.allocateIndexedObjects(m_bones.size());
	//	for (size_t i = 0; i < m_bones.size(); ++i)
	//	{
	//		bones[i] = m_bones[i].m_name;
	//	}
	//}

	//if (m_skinVertCount > 0)
	//{
	//	Pix::Value &skin = root["Skin"];
	//	skin["StreamCount"] = 1;
	//	Pix::Value &skinStream = skin["SkinStream"];
	//	skinStream["Format"] = Pix::Value::Enumeration("FLOAT3");
	//	skinStream["Tag"] = "_POSITION";
	//	skinStream["ItemCount"] = 1;
	//	skinStream["TotalWeightCount"] = 1;
	//	skinStream["TotalCloneCount"] = 1;
	//	skinStream.allocateIndexedObjects(m_skinVertCount);
	//	for (size_t i = 0; i < m_skinVertCount; ++i)
	//	{
	//		skinStream[i] =
	//	}
	//}

	//Pix::StyledFileWriter writer;
	//writer.write(file.get(), root);

	*file << fmt::sprintf(
		"Header {"							SEOL
		TAB "FormatVersion: 5"				SEOL
		TAB "Source: \"%s\""				SEOL
		TAB "Type: \"Model\""				SEOL
		TAB "Name: \"%s\""					SEOL
		"}"									SEOL,
			STRING_VERSION,
			m_fileName.c_str()
		);

	*file << fmt::sprintf(
		"Global {"							SEOL
		TAB "VertexCount: %i"				SEOL
		TAB "TriangleCount: %i"				SEOL
		TAB "MaterialCount: %i"				SEOL
		TAB "PieceCount: %i"				SEOL
		TAB "PartCount: %i"					SEOL
		TAB "BoneCount: %i"					SEOL
		TAB "LocatorCount: %i"				SEOL
		TAB "Skeleton: \"%s\""				SEOL
		"}"									SEOL,
			m_vertCount,
			m_triangleCount,
			m_materialCount,
			(int)m_pieces.size(),
			(int)m_parts.size(),
			(int)m_bones.size(),
			(int)m_locators.size(),
			(m_fileName + ".pis").c_str()
		);

	if(m_looks.size() > 0)
	{
		for (uint32_t i = 0; i < m_materialCount; ++i)
		{
			*file << m_looks[0].m_materials[i].toDeclaration();
		}
	}

	for (uint32_t i = 0; i < m_pieces.size(); ++i)
	{
		const Piece *currentPiece = &m_pieces[i];

		*file << fmt::sprintf(
			"Piece {"						SEOL
			TAB "Index: %i"					SEOL
			TAB "Material: %i"				SEOL
			TAB "VertexCount: %i"			SEOL
			TAB "TriangleCount: %i"			SEOL
			TAB "StreamCount: %i"			SEOL,
				currentPiece->m_index,
				currentPiece->m_material,
				(int)currentPiece->m_vertices.size(),
				(int)currentPiece->m_triangles.size(),
				currentPiece->m_streamCount
			);

		if (currentPiece->m_position)
		{
			*file << fmt::sprintf(
				TAB "Stream {"				SEOL
				TAB TAB "Format: %s"		SEOL
				TAB TAB "Tag: \"%s\""		SEOL,
					"FLOAT3",
					"_POSITION"
				);

			for (uint32_t j = 0; j < currentPiece->m_vertices.size(); ++j)
			{
				*file << fmt::sprintf(
					TAB TAB "%-5i( %s )" SEOL,
						j, to_string(currentPiece->m_vertices[j].m_position).c_str()
					);
			}

			*file << TAB "}" SEOL;
		}
		if (currentPiece->m_normal)
		{
			*file << fmt::sprintf(
				TAB "Stream {"				SEOL
				TAB TAB "Format: %s"		SEOL
				TAB TAB "Tag: \"%s\""		SEOL,
					"FLOAT3",
					"_NORMAL"
				);

			for (uint32_t j = 0; j < currentPiece->m_vertices.size(); ++j)
			{
				*file << fmt::sprintf(
					TAB TAB "%-5i( %s )" SEOL,
						j, to_string(currentPiece->m_vertices[j].m_normal).c_str()
					);
			}

			*file << TAB "}" SEOL;
		}
		if (currentPiece->m_tangent)
		{
			*file << fmt::sprintf(
				TAB "Stream {"				SEOL
				TAB TAB "Format: %s"		SEOL
				TAB TAB "Tag: \"%s\""		SEOL,
					"FLOAT4",
					"_TANGENT"
				);

			for (uint32_t j = 0; j < currentPiece->m_vertices.size(); ++j)
			{
				*file << fmt::sprintf(
					TAB TAB "%-5i( %s )" SEOL,
						j, to_string(currentPiece->m_vertices[j].m_tangent).c_str()
					);
			}

			*file << TAB "}" SEOL;
		}
		if (currentPiece->m_texcoord)
		{
			for (uint32_t j = 0; j < currentPiece->m_texcoordCount; ++j)
			{
				Array<uint32_t> texCoords = currentPiece->texCoords(j);

				*file << fmt::sprintf(
					TAB "Stream {"				SEOL
					TAB TAB "Format: FLOAT2"	SEOL
					TAB TAB "Tag: \"_UV%i\""	SEOL
					TAB TAB "AliasCount: %i"	SEOL
					TAB TAB "Aliases: " ,
						j, texCoords.size()
					);

				for (const uint32_t& texCoord : texCoords)
				{
					*file << fmt::sprintf("\"_TEXCOORD%i\" ", texCoord);
				}
				*file << SEOL;

				for (uint32_t k = 0; k < currentPiece->m_vertices.size(); ++k)
				{
					*file << fmt::sprintf(
						TAB TAB "%-5i( %s )" SEOL,
							k, to_string(currentPiece->m_vertices[k].m_texcoords[j]).c_str()
						);
				}

				*file << TAB "}" SEOL;
			}

		}
		if (currentPiece->m_color)
		{
			*file << fmt::sprintf(
				TAB "Stream {" SEOL
				TAB TAB "Format: %s" SEOL
				TAB TAB "Tag: \"%s\"" SEOL,
					"FLOAT4",
					"_RGBA"
				);

			for (uint32_t j = 0; j < currentPiece->m_vertices.size(); ++j)
			{
				*file << fmt::sprintf(
					TAB TAB "%-5i( %s )" SEOL,
						j, to_string(currentPiece->m_vertices[j].m_color).c_str()
					);
			}

			*file << TAB "}" SEOL;
		}

		{ // triangles
			*file << fmt::sprintf(
				TAB "%s {" SEOL,
					"Triangles"
				);

			for (uint32_t j = 0; j < currentPiece->m_triangles.size(); ++j)
			{
				*file << fmt::sprintf(
					TAB TAB "%-5i( %-5i %-5i %-5i )" SEOL,
						j, currentPiece->m_triangles[j].m_attach[0],
						   currentPiece->m_triangles[j].m_attach[1],
						   currentPiece->m_triangles[j].m_attach[2]
					);
			}

			*file << TAB "}" SEOL;
		}
		*file << "}" SEOL; // piece
	}

	for (uint32_t i = 0; i < m_parts.size(); ++i)
	{
		const Part *currentPart = &m_parts[i];

		*file << fmt::sprintf(
			"Part {" SEOL
			TAB "Name: \"%s\"" SEOL
			TAB "PieceCount: %i" SEOL
			TAB "LocatorCount: %i" SEOL,
				currentPart->m_name.c_str(),
				currentPart->m_pieceCount,
				currentPart->m_locatorCount
			);

		*file << TAB "Pieces: ";
		for (uint32_t j = 0; j < currentPart->m_pieceCount; ++j)
		{
			*file << fmt::sprintf("%i ", currentPart->m_pieceId + j);
		}
		*file << SEOL;

		*file << TAB "Locators: ";
		for (uint32_t j = 0; j < currentPart->m_locatorCount; ++j)
		{
			*file << fmt::sprintf("%i ", currentPart->m_locatorId + j);
		}
		*file << SEOL;

		*file << "}" SEOL; // part
	}

	for (uint32_t i = 0; i < m_locators.size(); ++i)
	{
		const Locator *currentLocator = &m_locators[i];

		*file << fmt::sprintf(
			"Locator {"										SEOL
			TAB "Name: \"%s\""								SEOL,
				currentLocator->m_name.c_str()
			);

		if (currentLocator->m_hookup.length() > 0)
		{
			*file << fmt::sprintf(
				TAB "Hookup: \"%s\""						SEOL,
					currentLocator->m_hookup.c_str()
				);
		}

		*file << fmt::sprintf(
			TAB "Index: %i"									SEOL
			TAB "Position: ( %s )"							SEOL
			TAB "Rotation: ( %s )"							SEOL
			TAB "Scale: ( %s )"								SEOL,
				currentLocator->m_index,
				to_string(currentLocator->m_position).c_str(),
				to_string(currentLocator->m_rotation).c_str(),
				to_string(currentLocator->m_scale).c_str()
			);

		*file << "}" SEOL; // locator
	}

	if (m_bones.size() > 0)
	{
		*file << "Bones {" SEOL;
		for (uint32_t i = 0; i < m_bones.size(); ++i)
		{
			*file << fmt::sprintf(TAB "%-5i( \"%s\" )" SEOL, i, m_bones[i].m_name.c_str());
		}
		*file << "}" SEOL;
	}

	if (m_skinVertCount > 0)
	{
		*file << "Skin {" SEOL;
		*file << TAB "StreamCount: 1"			SEOL;
		*file << TAB "SkinStream {"				SEOL;
		unsigned itemIdx = 0, weightIdx = 0;
		Array<String> skinStreams;

		for (uint32_t i = 0; i < m_pieces.size(); ++i)
		{
			if (m_pieces[i].m_bones == 0)
				continue;

			for (uint32_t j = 0; j < m_pieces[i].m_vertices.size(); ++j)
			{
				const Vertex *const vert = &m_pieces[i].m_vertices[j];

				String skinStream;
				skinStream += fmt::sprintf(
					TAB TAB "%-6i( ( %s )" SEOL,
					itemIdx, to_string(vert->m_position).c_str()
				);

				uint32_t weights = 0;
				for (uint32_t k = 0; k < m_pieces[i].m_bones; ++k)
				{
					if (vert->m_boneWeight[k] != 0)
					{
						weights++;
					}
				}
				weightIdx += weights;

				skinStream += fmt::sprintf(
					TAB TAB TAB TAB "Weights: %-6i ",
					weights
				);

				for (uint32_t k = 0; k < m_pieces[i].m_bones; ++k)
				{
					if (vert->m_boneWeight[k] != 0)
					{
						float weight = (float)vert->m_boneWeight[k] / 255.f;
						skinStream += fmt::sprintf(
							"%-4i " FLT_FT " ",
							vert->m_boneIndex[k], flh(weight)
						);
					}
				}

				skinStream += SEOL;

				skinStream += fmt::sprintf(
					TAB TAB TAB TAB "Clones: %-6i %-4i %-6i" SEOL,
					1, i, j
				);

				skinStream += TAB TAB "      )"				SEOL;
				++itemIdx;

				skinStreams.push_back(skinStream);
			}
		}

		*file << fmt::sprintf(
			TAB TAB "Format: %s"			SEOL
			TAB TAB "Tag: \"%s\""			SEOL
			TAB TAB "ItemCount: %i"			SEOL
			TAB TAB "TotalWeightCount: %i"	SEOL
			TAB TAB "TotalCloneCount: %i"	SEOL,
				"FLOAT3",
				"_POSITION",
				itemIdx,
				weightIdx,
				itemIdx
		);

		for (const auto &stream : skinStreams)
		{
			*file << stream.c_str();
		}
		*file << TAB "}" SEOL;
		*file << "}" SEOL;
	}
	return true;
}

bool Model::saveToPit(String exportPath) const
{
	const String pitFilePath = exportPath + m_filePath + ".pit";
	auto file = getSFS()->open(pitFilePath, FileSystem::write | FileSystem::binary);
	if (!file)
	{
		error_f("model", m_filePath, "Unable to save trait file [%s] (%s)!", pitFilePath, strerror(errno));
		return false;
	}

	Pix::Value root;

	Pix::Value &header = root["Header"];
	header["FormatVersion"] = 1;
	header["Source"] = STRING_VERSION;
	header["Type"] = "Trait";
	header["Name"] = m_fileName;

	Pix::Value &global = root["Global"];
	global["LookCount"] = m_looks.size();
	global["VariantCount"] = m_variants.size();
	global["PartCount"] = m_parts.size();
	global["MaterialCount"] = m_materialCount;

	for (const auto &l : m_looks)
	{
		Pix::Value &look = root["Look"];
		look["Name"] = l.m_name;
		for (const auto &mat : l.m_materials)
		{
			look["Material"] = mat.toPixDefinition();
		}
	}

	for (const auto &v : m_variants)
	{
		Pix::Value &variant = root["Variant"];
		variant["Name"] = v.m_name;
		for (uint32_t i = 0; i < m_parts.size(); ++i)
		{
			Pix::Value &part = variant["Part"];
			part["Name"] = m_parts[i].m_name;
			part["AttributeCount"] = v.m_parts[i].m_attributes.size();
			for (uint32_t k = 0; k < v.m_parts[i].m_attributes.size(); ++k)
			{
				part["Attribute"] = v.m_parts[i][k].toPixDefinition();
			}
		}
	}

	Pix::StyledFileWriter writer;
	writer.write(file.get(), root);
	file->flush();
	return true;
}

bool Model::saveToPis(String exportPath) const
{
	if(m_bones.size() == 0)
		return false;

	const String pitFilePath = exportPath + m_filePath + + ".pis";
	auto file = getSFS()->open(pitFilePath, FileSystem::write | FileSystem::binary);
	if (!file)
	{
		error_f("model", m_filePath, "Unable to save skeleton file [%s] (%s)!", pitFilePath, strerror(errno));
		return false;
	}

	*file << fmt::sprintf(
		"Header {"					SEOL
		TAB "FormatVersion: 1"		SEOL
		TAB "Source: \"%s\""		SEOL
		TAB "Type: \"Skeleton\""	SEOL
		TAB "Name: \"%s\""			SEOL
		"}"							SEOL,
			STRING_VERSION,
			m_fileName.c_str()
		);

	*file << fmt::sprintf(
		"Global {"					SEOL
		TAB "BoneCount: %i"			SEOL
		"}"							SEOL,
			(int)m_bones.size()
		);

	*file << "Bones {"				SEOL;
	{
		for (size_t i = 0; i < m_bones.size(); ++i)
		{
			prism::mat4 mat = glm_cast(m_bones[i].m_transformation);

			*file << fmt::sprintf(
				TAB "%-5i ( Name:  \"%s\""													SEOL
				TAB TAB "   Parent: \"%s\""													SEOL
				TAB TAB "   Matrix: ( " FLT_FT "  " FLT_FT "  " FLT_FT "  " FLT_FT ""		SEOL
				TAB TAB "             " FLT_FT "  " FLT_FT "  " FLT_FT "  " FLT_FT ""		SEOL
				TAB TAB "             " FLT_FT "  " FLT_FT "  " FLT_FT "  " FLT_FT ""		SEOL
				TAB TAB "             " FLT_FT "  " FLT_FT "  " FLT_FT "  " FLT_FT " )"		SEOL
				TAB "  )" SEOL,
					(int)i, m_bones[i].m_name.c_str(),
					(m_bones[i].m_parent != -1 ? m_bones[m_bones[i].m_parent].m_name.c_str() : ""),
					flh(mat[0][0]), flh(mat[1][0]), flh(mat[2][0]), flh(mat[3][0]),
					flh(mat[0][1]), flh(mat[1][1]), flh(mat[2][1]), flh(mat[3][1]),
					flh(mat[0][2]), flh(mat[1][2]), flh(mat[2][2]), flh(mat[3][2]),
					flh(mat[0][3]), flh(mat[1][3]), flh(mat[2][3]), flh(mat[3][3])
				);
		}
	}
	*file << "}"					SEOL;
	return true;
}

void Model::convertTextures(String exportPath) const
{
	for (size_t i = 0; i < m_looks.size(); ++i)
	{
		for (size_t j = 0; j < m_looks[i].m_materials.size(); ++j)
		{
			m_looks[i].m_materials[j].convertTextures(exportPath);
		}
	}
}

void Model::saveToMidFormat(String exportPath, bool convertTexture) const
{
	bool pim = saveToPim(exportPath);
	bool pit = saveToPit(exportPath);
	bool pis = saveToPis(exportPath);
	bool pic = m_collision ? m_collision->saveToPic(exportPath) : false;
	bool pip = m_prefab ? m_prefab->saveToPip(exportPath) : false;
	if (convertTexture) { convertTextures(exportPath); }

	auto state = [](bool x) -> const char * { return x ? "yes" : "no"; };

	info_f("model", m_fileName, "pim:%s pit:%s pis:%s pic:%s pip:%s vertices:%i indices:%i materials:%i",
		   state(pim), state(pit), state(pis), state(pic), state(pip), m_vertCount, m_triangleCount, m_materialCount);
}

Bone *Model::bone(size_t index)
{
	assert(index >= 0 && index < m_bones.size());
	return &m_bones[index];
}

auto Variant::Part::operator[](String attribute) const -> const Attribute &
{
	const auto it = std::find_if(m_attributes.cbegin(), m_attributes.cend(),
	[&](const Attribute &attr) {
		return attr.getName() == attribute;
	});
	assert(it != m_attributes.cend());
	return (*it);
}

auto Variant::Part::operator[](String attribute) -> Attribute &
{
	auto it = std::find_if(m_attributes.begin(), m_attributes.end(),
	[&](const Attribute &attr) {
		return attr.getName() == attribute;
	});
	assert(it != m_attributes.end());
	return (*it);
}

auto Variant::Part::operator[](size_t attribute) const -> const Attribute &
{
	assert(attribute < m_attributes.size());
	return m_attributes[attribute];
}

auto Variant::Part::operator[](size_t attribute) -> Attribute &
{
	assert(attribute < m_attributes.size());
	return m_attributes[attribute];
}

void Variant::setPartCount(size_t parts)
{
	m_parts.resize(parts);
}

auto Variant::operator[](size_t id) const -> const Part &
{
	assert(id >= 0 && id < m_parts.size());
	return m_parts[id];
}

auto Variant::operator[](size_t id) -> Part &
{
	assert(id >= 0 && id < m_parts.size());
	return m_parts[id];
}

String Variant::Attribute::toDefinition(const String &prefix) const
{
	String result;
	result += prefix + "Attribute {" SEOL;
	{
		result += prefix + fmt::sprintf(TAB "Format: %s" SEOL, m_type == INT ? "INT" : "UNKNOWN");
		result += prefix + fmt::sprintf(TAB "Tag: \"%s\"" SEOL, m_name.c_str());
		result += prefix + fmt::sprintf(TAB "Value: ( %i )" SEOL, m_intValue);
	}
	result += prefix + "}" SEOL;
	return result;
}

Pix::Value Variant::Attribute::toPixDefinition() const
{
	Pix::Value root;
	root["Format"] = Pix::Value::Enumeration(m_type == INT ? "INT" : "UNKNOWN");
	root["Tag"] = m_name;
	root["Value"] = prism::vec_t<int, 1>(m_intValue);
	return root;
}

/* eof */
