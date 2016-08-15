/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : model.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include "model.h"

#include <file.h>
#include <resource_lib.h>
#include <texture/texture.h>
#include <prefab/prefab.h>

using namespace prism;

void Variant::setPartCount(size_t parts)
{
	m_parts.resize(parts);
}

auto Variant::operator[](u32 id) const -> const std::vector<Attribute> &
{
	assert(id >= 0 && id < m_parts.size());
	return m_parts[id];
}

auto Variant::operator[](u32 id) -> std::vector<Attribute> &
{
	assert(id >= 0 && id < m_parts.size());
	return m_parts[id];
}

std::string Variant::Attribute::toDefinition(const std::string &prefix) const
{
	std::string result;
	result += prefix + "Attribute {" SEOL;
	{
		result += prefix + fmt::sprintf(TAB "Format: %s" SEOL, m_type == INT ? "INT" : "UNKNOWN");
		result += prefix + fmt::sprintf(TAB "Tag: \"%s\"" SEOL, m_name.c_str());
		result += prefix + fmt::sprintf(TAB "Value: ( %i )" SEOL, m_intValue);
	}
	result += prefix + "}" SEOL;
	return result;
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
	m_basePath = "";
	m_fileName = "";
}

bool Model::load(std::string basePath, std::string filePath)
{
	if (m_loaded)
		destroy();

	m_basePath = basePath;
	m_filePath = filePath;
	m_directory = directory(filePath);
	m_fileName = filePath.substr(m_directory.length() + 1);

	if (!loadDescriptor()) return false;
	if (!loadModel()) return false;

	if (fileExists(m_basePath + m_filePath + ".ppd"))
	{
		m_prefab = std::make_shared<Prefab>();
		if (!m_prefab->load(basePath, filePath))
		{
			m_prefab.reset();
		}
	}
	
	m_loaded = true;
	return true;
}

bool Model::loadModel()
{
	std::string pmgPath = m_basePath + m_filePath + ".pmg";

	File file;
	if (!file.open(pmgPath.c_str(), "rb"))
	{
		printf("Cannot open geometry file: \"%s\"! %s" SEOL, pmgPath.c_str(), strerror(errno));
		return false;
	}

	size_t fileSize = file.getSize();

	std::unique_ptr<uint8_t[]> buffer(new uint8_t[fileSize]);
	file.read((char *)buffer.get(), sizeof(char), fileSize);

	file.close();

	pmg_header *header = (pmg_header *)(buffer.get());
	if (header->m_version != pmg_header::SUPPORTED_VERSION || !(header->m_signature[2] == 'P' && header->m_signature[1] == 'm' && header->m_signature[0] == 'g'))
	{
		printf("Invalid version of geometry file! (have: %i signature: %c%c%c, expected: %i)" SEOL, 
			   header->m_version, header->m_signature[2], header->m_signature[1], header->m_signature[0], pmg_header::SUPPORTED_VERSION);
		return false;
	}

	m_pieces.resize(header->m_piece_count);
	m_bones.resize(header->m_bone_count);
	m_locators.resize(header->m_locator_count);
	m_parts.resize(header->m_part_count);

	pmg_bone *bone = (pmg_bone *)(buffer.get() + header->m_bone_offset);
	for (int32_t i = 0; i < header->m_bone_count; ++i, ++bone)
	{
		Bone *currentBone = &m_bones[i];
		currentBone->m_index = i;
		currentBone->m_name = token_to_string(bone->m_name);
		currentBone->m_transReversed = bone->m_transformation_reversed;
		currentBone->m_transformation = bone->m_transformation;
		currentBone->m_stretch = bone->m_stretch;
		currentBone->m_rotation = bone->m_rotation;
		currentBone->m_translation = bone->m_translation;
		currentBone->m_scale = bone->m_scale;
		currentBone->m_signOfDeterminantOfMatrix = bone->m_sign_of_determinant_of_matrix;
		currentBone->m_parent = bone->m_parent;
	}

	pmg_part *part = (pmg_part *)(buffer.get() + header->m_part_offset);
	for (int32_t i = 0; i < header->m_part_count; ++i, ++part)
	{
		Part *currentPart = &m_parts[i];
		currentPart->m_name = token_to_string(part->m_name);
		currentPart->m_locatorCount = part->m_locator_count;
		currentPart->m_locatorId = part->m_locators_idx;
		currentPart->m_pieceCount = part->m_piece_count;
		currentPart->m_pieceId = part->m_pieces_idx;
	}

	pmg_locator *locator = (pmg_locator *)(buffer.get() + header->m_locator_offset);
	for (int32_t i = 0; i < header->m_locator_count; ++i, ++locator)
	{
		Locator *currentLocator = &m_locators[i];
		currentLocator->m_index = i;
		currentLocator->m_position = locator->m_position;
		currentLocator->m_rotation = locator->m_rotation;
		currentLocator->m_scale = locator->m_scale;
		currentLocator->m_name = token_to_string(locator->m_name);

		if (locator->m_name_block_offset != -1) {
			currentLocator->m_hookup = (const char *)(buffer.get() + header->m_locator_name_offset + locator->m_name_block_offset);
		} else {
			currentLocator->m_hookup = "";
		}
	}

	pmg_piece *piece = (pmg_piece *)(buffer.get() + header->m_piece_offset);
	for (int32_t i = 0; i < header->m_piece_count; ++i, ++piece)
	{
		Piece *currentPiece = &m_pieces[i];
		currentPiece->m_index = i;
		currentPiece->m_maskUV = piece->m_uv_mask;
		currentPiece->m_UVs = piece->m_uv_channels;
		currentPiece->m_bones = piece->m_bone_count;
		currentPiece->m_material = piece->m_material;

		currentPiece->m_vertices.resize(piece->m_verts);
		m_vertCount += piece->m_verts;

		currentPiece->m_triangles.resize(piece->m_triangles / 3);
		m_triangleCount += (piece->m_triangles / 3);

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
			poolSizeStatic += sizeof(pmg_vert_tangent);
		}
		if (piece->m_vert_uv_offset != -1)
		{
			currentPiece->m_uv = true;
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
			Vertex *vert = &currentPiece->m_vertices[j];

			if (currentPiece->m_position)
			{
				vert->m_position = *(float3 *)(buffer.get() + piece->m_vert_position_offset + poolSizeStatic*j);
			}
			if (currentPiece->m_normal)
			{
				vert->m_normal = *(float3 *)(buffer.get() + piece->m_vert_normal_offset + poolSizeStatic*j);
			}
			if (currentPiece->m_tangent)
			{
				pmg_vert_tangent *vertTangent = (pmg_vert_tangent *)(buffer.get() + piece->m_vert_tangent_offset + poolSizeStatic*j);
				vert->m_tangent[0] = vertTangent->w;
				vert->m_tangent[1] = vertTangent->x;
				vert->m_tangent[2] = vertTangent->y;
				vert->m_tangent[3] = vertTangent->z;
			}
			if (currentPiece->m_uv)
			{
				for (int32_t k = 0; k < piece->m_uv_channels; ++k)
				{
					vert->m_uv[k] = *(float2 *)(buffer.get() + piece->m_vert_uv_offset + poolSizeDynamic*j + sizeof(float2)*k);
				}
			}
			if (currentPiece->m_color)
			{
				pmg_vert_color *vertRgba = (pmg_vert_color *)(buffer.get() + piece->m_vert_rgba_offset + poolSizeDynamic*j);
				vert->m_color[0] = 2.f * vertRgba->m_r / 255.f;
				vert->m_color[1] = 2.f * vertRgba->m_g / 255.f;
				vert->m_color[2] = 2.f * vertRgba->m_b / 255.f;
				vert->m_color[3] = 2.f * vertRgba->m_a / 255.f;
			}
			if (currentPiece->m_color2)
			{
				pmg_vert_color *vertRgba = (pmg_vert_color *)(buffer.get() + piece->m_vert_rgba2_offset + poolSizeDynamic*j);
				vert->m_color2[0] = 2.f * vertRgba->m_r / 255.f;
				vert->m_color2[1] = 2.f * vertRgba->m_g / 255.f;
				vert->m_color2[2] = 2.f * vertRgba->m_b / 255.f;
				vert->m_color2[3] = 2.f * vertRgba->m_a / 255.f;
			}
			if (piece->m_anim_bind_offset != -1)
			{
				vert->m_animBind = *(uint16_t *)(buffer.get() + piece->m_anim_bind_offset + j * sizeof(uint16_t));
			}
		}

		pmg_triangle *triangle = (pmg_triangle *)(buffer.get() + piece->m_triangle_offset);
		for (int32_t j = 0; j < (piece->m_triangles / 3); ++j)
		{
			currentPiece->m_triangles[j].m_a[0] = triangle->a[0];
			currentPiece->m_triangles[j].m_a[1] = triangle->a[1];
			currentPiece->m_triangles[j].m_a[2] = triangle->a[2];

			++triangle;
		}

		if (piece->m_bone_count > 0)
		{
			currentPiece->m_animBinds.resize((piece->m_anim_bind_bones_weight_offset - piece->m_anim_bind_bones_offset) / piece->m_bone_count);
			for (uint32_t j = 0; j < currentPiece->m_animBinds.size(); ++j)
			{
				currentPiece->m_animBinds[j].setBoneCount(piece->m_bone_count);
				for (int32_t k = 0; k < piece->m_bone_count; ++k)
				{
					currentPiece->m_animBinds[j].bone(k) = *(int8_t *)(buffer.get() + piece->m_anim_bind_bones_offset + j*piece->m_bone_count + k);
					currentPiece->m_animBinds[j].weight(k) = *(uint8_t *)(buffer.get() + piece->m_anim_bind_bones_weight_offset + j*piece->m_bone_count + k);
				}
			}
		}
	}
	return true;
}

bool Model::loadDescriptor()
{
	std::string pmdPath = m_basePath + m_filePath + ".pmd";
	
	File file;
	if (!file.open(pmdPath.c_str(), "rb"))
	{
		printf("Cannot open descriptor file! \"%s\" errno = %X" SEOL, pmdPath.c_str(), errno);
		return false;
	}

	size_t fileSize = file.getSize();
	std::unique_ptr<uint8_t[]> buffer(new uint8_t[fileSize]);
	file.read((char *)buffer.get(), sizeof(uint8_t), fileSize);
	file.close();

	pmd_header *header = (pmd_header *)(buffer.get());
	if (header->m_version != pmd_header::SUPPORTED_VERSION)
	{
		printf("Invalid version of descriptor file! (have: %i, expected: %i)" SEOL, header->m_version, pmd_header::SUPPORTED_VERSION);
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
			currentLook->m_materials[j].load(m_basePath, materialPath[0] == '/' ? materialPath : (m_directory + "/" + materialPath));
			if (i == 0)
			{
				if (currentLook->m_materials[j].m_textures.size() > 0)
				{
					std::string textureName = std::string(currentLook->m_materials[j].m_textures[0].texture().c_str());
					textureName = textureName.substr(0, textureName.size() - 5);
					size_t lastSlash = textureName.rfind('/');
					if (lastSlash != std::string::npos)
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

	m_variants.resize(header->m_variant_count);

	for (uint32_t i = 0; i < m_variants.size(); ++i)
	{
		Variant *variant = &m_variants[i];
		token_t variantName = *(token_t *)(buffer.get() + header->m_variant_offset + i*sizeof(token_t));

		variant->m_name = token_to_string(variantName);
		variant->setPartCount(header->m_part_count);
	
		for (uint32_t j = 0; j < header->m_part_count; ++j)
		{
			pmd_attrib_link *attribLink = (pmd_attrib_link *)(buffer.get() + header->m_part_attribs_offset + j*sizeof(pmd_attrib_link));
			for (int32_t k = attribLink->m_from; k < attribLink->m_to; ++k)
			{
				pmd_attrib_def *attribDef = (pmd_attrib_def *)(buffer.get() + header->m_attribs_offset + k*sizeof(pmd_attrib_def));
				pmd_attrib_value *attribValue = (pmd_attrib_value *)(buffer.get() + header->m_attribs_value_offset + attribDef->m_offset + i*header->m_attribs_values_size);
				Variant::Attribute attrib(token_to_string(attribDef->m_name));
				switch (attribDef->m_type)
				{
					case 0:
					{
						attrib.m_type = Variant::Attribute::INT;
						attrib.m_intValue = attribValue->m_int_value;
					} break;
					// TODO: More attributes
					default: printf("Invalid attribute value <%i>!", attribDef->m_type);
				}
				(*variant)[j].push_back(attrib);
			}
		}
	}
	return true;
}

bool Model::loadCollision()
{
	// TODO: collisions import
	return true;
}

bool Model::saveToPim(std::string exportPath) const
{
	std::string pimFilePath = exportPath + m_filePath + ".pim";

	File file;
	if (!file.open(pimFilePath, "wb"))
	{
		printf("Cannot open file: \"%s\"! %s" SEOL, pimFilePath.c_str(), strerror(errno));
		return false;
	}

	file << fmt::sprintf(
		"Header {"							SEOL
		TAB "FormatVersion: 5"				SEOL
		TAB "Source: \"%s\""				SEOL
		TAB "Type: \"Model\""				SEOL
		TAB "Name: \"%s\""					SEOL
		"}"									SEOL, 
			STRING_VERSION,
			m_fileName.c_str()
		);

	file << fmt::sprintf(
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
			file << m_looks[0].m_materials[i].toDeclaration();
		}
	}

	for (uint32_t i = 0; i < m_pieces.size(); ++i)
	{
		const Piece *currentPiece = &m_pieces[i];

		file << fmt::sprintf(
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
			file << fmt::sprintf(
				TAB "Stream {"				SEOL
				TAB TAB "Format: %s"		SEOL
				TAB TAB "Tag: \"%s\""		SEOL,
					"FLOAT3", 
					"_POSITION"
				);

			for (uint32_t j = 0; j < currentPiece->m_vertices.size(); ++j)
			{
				file << fmt::sprintf(
					TAB TAB "%-5i( %s )" SEOL,
						j, to_string(currentPiece->m_vertices[j].m_position).c_str()
					);
			}

			file << TAB "}" SEOL;
		}
		if (currentPiece->m_normal)
		{
			file << fmt::sprintf(
				TAB "Stream {"				SEOL
				TAB TAB "Format: %s"		SEOL
				TAB TAB "Tag: \"%s\""		SEOL,
					"FLOAT3",
					"_NORMAL"
				);

			for (uint32_t j = 0; j < currentPiece->m_vertices.size(); ++j)
			{
				file << fmt::sprintf(
					TAB TAB "%-5i( %s )" SEOL,
						j, to_string(currentPiece->m_vertices[j].m_normal).c_str()
					);
			}

			file << TAB "}" SEOL;
		}
		if (currentPiece->m_tangent)
		{
			file << fmt::sprintf(
				TAB "Stream {"				SEOL
				TAB TAB "Format: %s"		SEOL
				TAB TAB "Tag: \"%s\""		SEOL,
					"FLOAT4",
					"_TANGENT"
				);
			
			for (uint32_t j = 0; j < currentPiece->m_vertices.size(); ++j)
			{
				file << fmt::sprintf(
					TAB TAB "%-5i( %s )" SEOL,
						j, to_string(currentPiece->m_vertices[j].m_tangent).c_str()
					);
			}

			file << TAB "}" SEOL;
		}
		if (currentPiece->m_uv)
		{
			for (uint32_t j = 0; j < currentPiece->m_UVs; ++j)
			{
				std::vector<uint32_t> texCoords = currentPiece->texCoords(j);

				file << fmt::sprintf(
					TAB "Stream {"				SEOL
					TAB TAB "Format: FLOAT2"	SEOL
					TAB TAB "Tag: \"_UV%i\""	SEOL
					TAB TAB "AliasCount: %i"	SEOL
					TAB TAB "Aliases: " ,
						j, texCoords.size()
					);

				for (const uint32_t& texCoord : texCoords)
				{
					file << fmt::sprintf("\"_TEXCOORD%i\" ", texCoord);
				}
				file << SEOL;

				for (uint32_t k = 0; k < currentPiece->m_vertices.size(); ++k)
				{
					file << fmt::sprintf(
						TAB TAB "%-5i( %s )" SEOL,
							k, to_string(currentPiece->m_vertices[k].m_uv[j]).c_str()
						);
				}

				file << TAB "}" SEOL;
			}

		}
		if (currentPiece->m_color)
		{
			file << fmt::sprintf(
				TAB "Stream {" SEOL
				TAB TAB "Format: %s" SEOL
				TAB TAB "Tag: \"%s\"" SEOL, 
					"FLOAT4", 
					"_RGBA"
				);

			for (uint32_t j = 0; j < currentPiece->m_vertices.size(); ++j)
			{
				file << fmt::sprintf(
					TAB TAB "%-5i( %s )" SEOL,
						j, to_string(currentPiece->m_vertices[j].m_color).c_str()
					);
			}

			file << TAB "}" SEOL;
		}

		{ // triangles
			file << fmt::sprintf(
				TAB "%s {" SEOL,
					"Triangles"
				);

			for (uint32_t j = 0; j < currentPiece->m_triangles.size(); ++j)
			{
				file << fmt::sprintf(
					TAB TAB "%-5i( %-5i %-5i %-5i )" SEOL,
						j, currentPiece->m_triangles[j].m_a[0], currentPiece->m_triangles[j].m_a[1], currentPiece->m_triangles[j].m_a[2]
					);
			}

			file << TAB "}" SEOL;
		}

		file << "}" SEOL; // piece
	}

	for (uint32_t i = 0; i < m_parts.size(); ++i)
	{
		const Part *currentPart = &m_parts[i];

		file << fmt::sprintf(
			"Part {" SEOL
			TAB "Name: \"%s\"" SEOL
			TAB "PieceCount: %i" SEOL
			TAB "LocatorCount: %i" SEOL, 
				currentPart->m_name.c_str(), 
				currentPart->m_pieceCount, 
				currentPart->m_locatorCount
			);

		file << TAB "Pieces: ";
		for (uint32_t j = 0; j < currentPart->m_pieceCount; ++j)
		{
			file << fmt::sprintf("%i ", currentPart->m_pieceId + j);
		}
		file << SEOL;

		file << TAB "Locators: ";
		for (uint32_t j = 0; j < currentPart->m_locatorCount; ++j)
		{
			file << fmt::sprintf("%i ", currentPart->m_locatorId + j);
		}
		file << SEOL;

		file << "}" SEOL; // part
	}

	for (uint32_t i = 0; i < m_locators.size(); ++i)
	{
		const Locator *currentLocator = &m_locators[i];

		file << fmt::sprintf(
			"Locator {"										SEOL
			TAB "Name: \"%s\""								SEOL,
				currentLocator->m_name.c_str()
			);

		if (currentLocator->m_hookup.length() > 0)
		{
			file << fmt::sprintf(
				TAB "Hookup: \"%s\""						SEOL,
					currentLocator->m_hookup.c_str()
				);
		}

		file << fmt::sprintf(
			TAB "Index: %i"									SEOL
			TAB "Position: ( %s )"							SEOL
			TAB "Rotation: ( %s )"							SEOL
			TAB "Scale: ( %s )"								SEOL,
				currentLocator->m_index,
				to_string(currentLocator->m_position).c_str(),
				to_string(currentLocator->m_rotation).c_str(),
				to_string(currentLocator->m_scale).c_str()
			);

		file << "}" SEOL; // locator
	}

	if (m_bones.size() > 0)
	{
		file << "Bones {" SEOL;
		for (uint32_t i = 0; i < m_bones.size(); ++i)
		{
			file << fmt::sprintf(TAB "%-5i( \"%s\" )" SEOL, i, m_bones[i].m_name.c_str());
		}
		file << "}" SEOL;
	}

	if (m_skinVertCount > 0)
	{
		file << "Skin {" SEOL;
		file << TAB "StreamCount: 1"			SEOL;
		file << TAB "SkinStream {"				SEOL;
		unsigned itemIdx = 0, weightIdx = 0;
		std::vector<std::string> skinStreams;

		for (uint32_t i = 0; i < m_pieces.size(); ++i)
		{
			if (m_pieces[i].m_bones == 0)
				continue;

			for (uint32_t j = 0; j < m_pieces[i].m_vertices.size(); ++j)
			{
				const Vertex *vert = &m_pieces[i].m_vertices[j];
				const AnimBind *bind = &m_pieces[i].m_animBinds[vert->m_animBind];

				std::string skinStream;
				skinStream += fmt::sprintf(
					TAB TAB "%-6i( ( %s )" SEOL,
					itemIdx, to_string(vert->m_position).c_str()
				);

				uint32_t weights = 0;
				for (uint32_t k = 0; k < m_pieces[i].m_bones; ++k)
				{
					if (bind->bone(k) != -1)
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
					if (bind->bone(k) != -1)
					{
						float weight = (float)bind->weight(k) / 255.f;
						skinStream += fmt::sprintf(
							"%-4i " FLT_FT " ",
							bind->bone(k), flh(weight)
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

		file << fmt::sprintf(
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

		for (auto& stream : skinStreams)
		{
			file << stream.c_str();
		}
		file << TAB "}" SEOL;
		file << "}" SEOL;
	}

	file.close();
	return true;
}

bool Model::saveToPit(std::string exportPath) const
{
	std::string pitFilePath = exportPath + m_filePath + ".pit";

	File file;
	if (!file.open(pitFilePath, "wb"))
	{
		printf("Cannot open file! \"%s\" errno = %X" SEOL, pitFilePath.c_str(), errno);
		return false;
	}

	file << fmt::sprintf(
		"Header {"					SEOL
		TAB "FormatVersion: 1"		SEOL
		TAB "Source: \"%s\""		SEOL
		TAB "Type: \"Trait\""		SEOL
		TAB "Name: \"%s\""			SEOL
		"}"							SEOL,
			STRING_VERSION,
			m_fileName.c_str()
		);

	file << fmt::sprintf(
		"Global {"					SEOL
		TAB "LookCount: %i"			SEOL
		TAB "VariantCount: %i"		SEOL
		TAB "PartCount: %i"			SEOL
		TAB "MaterialCount: %i"		SEOL
		"}"							SEOL,
			(int)m_looks.size(),
			(int)m_variants.size(),
			(int)m_parts.size(),
			m_materialCount
		);

	for (uint32_t i = 0; i < m_looks.size(); ++i)
	{
		file << fmt::sprintf(
			"Look {"				SEOL
			TAB "Name: \"%s\""		SEOL,
				m_looks[i].m_name.c_str()
			);

		for (uint32_t j = 0; j < (uint32_t)m_looks[i].m_materials.size(); ++j)
		{
			file << m_looks[i].m_materials[j].toDefinition(TAB);
		}

		file << "}"					SEOL;
	}

	for (uint32_t i = 0; i < m_variants.size(); ++i)
	{
		file << fmt::sprintf(
			"Variant {"				SEOL
			TAB "Name: \"%s\""		SEOL,
				m_variants[i].m_name.c_str()
			);

		for (uint32_t j = 0; j < m_parts.size(); ++j)
		{
			file << fmt::sprintf(
				TAB "Part {"		SEOL
				TAB TAB "Name: \"%s\""			SEOL
				TAB TAB "AttributeCount: %i"	SEOL,
					m_parts[j].m_name.c_str(),
					m_variants[i].m_parts[j].size()
				);

			for (uint32_t k = 0; k < m_variants[i].m_parts[j].size(); ++k)
			{
				file << m_variants[i].m_parts[j][k].toDefinition(TAB TAB);
			}

			file << TAB "}"			SEOL;
		}

		file << "}"				SEOL;
	}

	file.close();
	return true;
}

bool Model::saveToPis(std::string exportPath) const
{
	if(m_bones.size() == 0)
		return false;

	std::string pitFilePath = exportPath + m_filePath + + ".pis";

	File file;
	if (!file.open(pitFilePath, "wb"))
	{
		printf("Cannot open file! \"%s\" errno = %X" SEOL, pitFilePath.c_str(), errno);
		return false;
	}

	file << fmt::sprintf(
		"Header {"					SEOL
		TAB "FormatVersion: 1"		SEOL
		TAB "Source: \"%s\""		SEOL
		TAB "Type: \"Skeleton\""	SEOL
		TAB "Name: \"%s\""			SEOL
		"}"							SEOL,
			STRING_VERSION,
			m_fileName.c_str()
		);

	file << fmt::sprintf(
		"Global {"					SEOL
		TAB "BoneCount: %i"			SEOL
		"}"							SEOL,
			(int)m_bones.size()
		);

	file << "Bones {"				SEOL;
	{
		for (size_t i = 0; i < m_bones.size(); ++i)
		{
			prism::mat4 mat = m_bones[i].m_transformation;
			file << fmt::sprintf(
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
	file << "}"						SEOL;
	return true;
}

void Model::convertTextures(std::string exportPath) const
{
	for (size_t i = 0; i < m_looks.size(); ++i)
	{
		for (size_t j = 0; j < m_looks[i].m_materials.size(); ++j)
		{
			m_looks[i].m_materials[j].convertTextures(exportPath);
		}
	}
}

void Model::saveToMidFormat(std::string exportPath, bool convertTexture) const
{
	bool pim = saveToPim(exportPath);
	bool pit = saveToPit(exportPath);
	bool pis = saveToPis(exportPath);
	bool pip = m_prefab.get() ? m_prefab->saveToPip(exportPath) : false;
	if (convertTexture)
	{
		convertTextures(exportPath);
	}
	auto state = [](bool x) -> const char * { return x ? "yes" : "no"; };
	printf("%s: pim:%s pit:%s pis:%s pip:%s. vertices: %i materials: %i" SEOL, 
		   m_fileName.c_str(), state(pim), state(pit), state(pis), state(pip), m_vertCount, m_materialCount);
}

Bone *Model::bone(size_t index)
{
	assert(index >= 0 && index < m_bones.size());
	return &m_bones[index];
}

/* eof */
