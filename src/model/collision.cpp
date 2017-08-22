/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : collision.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "collision.h"

#include <model/model.h>
#include <fs/file.h>
#include <fs/uberfilesystem.h>
#include <fs/sysfilesystem.h>

bool Collision::load(Model *const model, String filePath)
{
	m_filePath = filePath;
	m_model = model;

	const String pmcPath = m_filePath + ".pmc";
	auto file = getUFS()->open(pmcPath, FileSystem::read | FileSystem::binary);
	if (!file)
	{
		error("collision", m_filePath, "Unable to open collision file!");
		return false;
	}

	const size_t fileSize = file->size();
	UniquePtr<uint8_t[]> buffer(new uint8_t[fileSize]);
	file->read((char *)buffer.get(), sizeof(char), fileSize);
	file.reset();

	const auto header = reinterpret_cast<prism::pmc_header_t *>(buffer.get());
	if (header->m_version != prism::pmc_header_t::SUPPORTED_VERSION)
	{
		error_f("collision", m_filePath, "Invalid version of collision file! (have: %i expected: %i)", header->m_version, prism::pmc_header_t::SUPPORTED_VERSION);
		return false;
	}

	for (size_t i = 0; i < header->m_piece_count; ++i)
	{
		const auto piecef = reinterpret_cast<prism::pmc_piece_t *>(buffer.get() + header->m_piece_offset) + i;

		Piece piece;
		piece.m_verts.resize(piecef->m_verts);
		for (size_t v = 0; v < piece.m_verts.size(); ++v)
		{
			piece.m_verts[v] = *(reinterpret_cast<prism::float3 *>(buffer.get() + piecef->m_vert_offset) + v);
		}
		piece.m_triangles.resize(piecef->m_edges / 3);
		for (size_t t = 0; t < piece.m_triangles.size(); ++t)
		{
			piece.m_triangles[t] = *(reinterpret_cast<prism::pmc_triangle_t *>(buffer.get() + piecef->m_face_offset) + t);
		}
		m_vertCount += piece.m_verts.size();
		m_triangleCount += piece.m_triangles.size();
		m_pieces.push_back(piece);
	}

	for (size_t i = 0; i < header->m_variant_count; ++i)
	{
		const auto variantName = reinterpret_cast<prism::pmc_variant_t *>(buffer.get() + header->m_variant_offset) + i;
		const auto variantDef = reinterpret_cast<prism::pmc_variant_def_t *>(buffer.get() + header->m_variant_def_offset) + i;

		Variant variant;
		variant.m_name = variantName->m_name.to_string();
		variant.m_modelVariant = &m_model->getVariants()[i];
		for (size_t j = 0, currentOffset = 0; ; ++j)
		{
			assert((variantDef->m_offset + currentOffset) < fileSize);

			const auto locatorf = reinterpret_cast<prism::pmc_locator_t *>(buffer.get() + variantDef->m_offset + currentOffset);
			if (locatorf->m_data_size == -1)
				break;

			currentOffset += locatorf->m_data_size;

			const auto it = std::find_if(m_locators.begin(), m_locators.end(), [&](SharedPtr<Locator> &loc) {
				/* I have not found better method to recognize locators */
				return loc->m_name == locatorf->m_name.to_string()
					&& loc->m_type == locatorf->m_type
					&& fl_eq(loc->m_position[0], locatorf->m_position[0])
					&& fl_eq(loc->m_position[1], locatorf->m_position[1])
					&& fl_eq(loc->m_position[2], locatorf->m_position[2]);
			});
			if (it != m_locators.end())
			{
				variant.m_locators.push_back(*it);
				continue;
			}

			SharedPtr<Locator> locator;
			switch (locatorf->m_data_size)
			{
				case sizeof(prism::pmc_locator_convex_t):
				{
					auto loc = std::make_shared<ConvexLocator>();
					const auto locf = static_cast<prism::pmc_locator_convex_t *>(locatorf);
					loc->m_rotation = locf->m_rotation;
					loc->m_convexPiece = locf->m_convex_piece;
					locator = std::move(loc);
				} break;
				case sizeof(prism::pmc_locator_cylinder_t):
				{
					auto loc = std::make_shared<CylinderLocator>();
					const auto locf = static_cast<prism::pmc_locator_cylinder_t *>(locatorf);
					loc->m_rotation = locf->m_rotation;
					loc->m_radius = locf->m_radius;
					loc->m_depth = locf->m_depth;
					locator = std::move(loc);
				} break;
				case sizeof(prism::pmc_locator_box_t):
				{
					auto loc = std::make_shared<BoxLocator>();
					const auto locf = static_cast<prism::pmc_locator_box_t *>(locatorf);
					loc->m_rotation = locf->m_rotation;
					loc->m_scale = locf->m_scale;
					locator = std::move(loc);
				} break;
				case sizeof(prism::pmc_locator_sphere_t):
				{
					auto loc = std::make_shared<SphereLocator>();
					const auto locf = static_cast<prism::pmc_locator_sphere_t *>(locatorf);
					loc->m_radius = locf->m_radius;
					locator = std::move(loc);
				} break;
				default: {
					warning_f("collision", m_filePath, "Not known locator type! size = %i name = %s", locatorf->m_data_size, locatorf->m_name.to_string());
					return false;
				}
			}
			if (locator)
			{
				locator->m_type = locatorf->m_type;
				locator->m_index = m_locators.size();
				locator->m_weight = locatorf->m_weight;
				locator->m_name = locatorf->m_name.to_string();
				locator->m_position = locatorf->m_position;

				variant.m_locators.push_back(locator);
				m_locators.push_back(locator);
			}
		}
		m_variants.push_back(variant);
	}
	assignLocatorsToParts();
	return true;
}

void Collision::destroy()
{
	m_model = nullptr;
	m_filePath.clear();
	m_pieces.clear();
	m_variants.clear();
	m_locators.clear();
	m_vertCount = 0;
	m_triangleCount = 0;
}

void Collision::assignLocatorsToParts()
{
	for (auto &loc : m_locators)
	{
		Array<const Variant *> belongsToVariants;
		std::for_each(m_variants.begin(), m_variants.end(),
			[&belongsToVariants, &loc](const Variant &v) {
			if (std::find(v.m_locators.begin(), v.m_locators.end(), loc) != v.m_locators.end())
			{
				belongsToVariants.push_back(&v);
			}
		});

		for (size_t i = 0; i < m_model->getParts().size(); ++i)
		{
			const auto &part = m_model->getParts()[i];
			Array<const Variant *> partBelongsToVariants;
			std::for_each(m_variants.begin(), m_variants.end(),
				[&partBelongsToVariants, &part, i](const Variant &v) {
				if ((*v.m_modelVariant)[i]["visible"].getInt() == 1)
				{
					partBelongsToVariants.push_back(&v);
				}
			});

			if (belongsToVariants == partBelongsToVariants)
			{
				loc->m_owner = &part;
			}
		}

		if (!loc->m_owner)
		{
			warning_f("collision", m_filePath, "Could not find part for locator: %s(%s)", loc->m_name, loc->type());
		}
	}
}

bool Collision::saveToPic(String exportPath) const
{
	const String picFilePath = exportPath + m_filePath + ".pic";
	auto file = getSFS()->open(picFilePath, FileSystem::write | FileSystem::binary);
	if (!file)
	{
		error_f("collision", picFilePath, "Unable to save file! (%s)", getSFS()->getError());
		return false;
	}

	*file << fmt::sprintf(
		"Header {"							SEOL
		TAB "FormatVersion: 2"				SEOL
		TAB "Source: \"%s\""				SEOL
		TAB "Type: \"Collision\""			SEOL
		TAB "Name: \"%s\""					SEOL
		"}"									SEOL,
			STRING_VERSION,
			m_model->fileName().c_str()
	);

	*file << fmt::sprintf(
		"Global {"							SEOL
		TAB "VertexCount: %u"				SEOL
		TAB "TriangleCount: %u"				SEOL
		TAB "MaterialCount: %u"				SEOL
		TAB "PieceCount: %i"				SEOL
		TAB "PartCount: %i"					SEOL
		TAB "LocatorCount: %i"				SEOL
		"}"									SEOL,
			m_vertCount,
			m_triangleCount,
			(m_pieces.empty() ? 0 : 1),
			(int)m_pieces.size(),
			(int)m_model->getParts().size(),
			(int)m_locators.size()
	);

	if (!m_pieces.empty())
	{
		*file << fmt::sprintf(
			"Material {"						SEOL
			TAB "Alias: \"%s\""					SEOL
			TAB "Effect: \"%s\""				SEOL
			"}"									SEOL,
			"convex",
			"dry.void"
		);
	}

	for (size_t i = 0; i < m_pieces.size(); ++i)
	{
		const auto &piece = m_pieces[i];
		*file << fmt::sprintf(
			"Piece {"						SEOL
			TAB "Index: %i"					SEOL
			TAB "Material: 0"				SEOL
			TAB "VertexCount: %i"			SEOL
			TAB "TriangleCount: %i"			SEOL
			TAB "StreamCount: 1"			SEOL,
			(int)i, (int)piece.m_verts.size(), (int)piece.m_triangles.size()
		);
		*file << fmt::sprintf(
			TAB "Stream {"					SEOL
			TAB TAB "Format: FLOAT3"		SEOL
			TAB TAB "Tag: \"_POSITION\""	SEOL
		);
		for (size_t j = 0; j < piece.m_verts.size(); ++j)
		{
			*file << fmt::sprintf(
				TAB TAB "%-5i( %s )"		SEOL,
				j, prism::to_string(piece.m_verts[j]).c_str()
			);
		}
		*file << TAB "}"					SEOL; // Stream {

		*file << TAB "Triangles {"			SEOL;
		for (size_t j = 0; j < piece.m_triangles.size(); ++j)
		{
			*file << fmt::sprintf(
				TAB TAB "%-5i( %-5i %-5i %-5i )" SEOL,
				(int)j, piece.m_triangles[j].a[2], piece.m_triangles[j].a[1], piece.m_triangles[j].a[0]
			);
		}
		*file << TAB "}"					SEOL; // Triangles {

		*file << "}"						SEOL; // Piece {
	}

	for (size_t i = 0; i < m_model->getParts().size(); ++i)
	{
		const auto &part = m_model->getParts()[i];

		Array<int> locators;
		for (size_t j = 0; j < m_locators.size(); ++j)
		{
			if (m_locators[j]->m_owner == &part)
			{
				locators.push_back(j);
			}
		}

		Array<int> pieces;
		/* No idea what are pieces here */

		*file << fmt::sprintf(
			"Part {"						SEOL
			TAB "Name: \"%s\""				SEOL
			TAB "PieceCount: %i"			SEOL
			TAB "LocatorCount: %i"			SEOL,
				part.m_name.c_str(),
				(int)pieces.size(),
				(int)locators.size()
		);

		*file << TAB "Pieces: ";
		for (const auto piece : pieces)
		{
			*file << fmt::sprintf("%i ", piece);
		}
		*file <<							SEOL;

		*file << TAB "Locators: ";
		for (const auto loc : locators)
		{
			*file << fmt::sprintf("%i ", loc);
		}
		*file <<							SEOL;

		*file << "}"						SEOL; // Part {
	}

	for (const auto &locator : m_locators)
	{
		*file << locator->toDefinition() << SEOL;
	}

	return true;
}

String Collision::Locator::toDefinition() const
{
	return
		"Locator {" SEOL
		TAB + fmt::sprintf("Name: \"%s\"" SEOL, m_name.c_str()) +
		TAB + fmt::sprintf("Index: %i" SEOL, m_index) +
		TAB + fmt::sprintf("Position: ( %s )" SEOL, prism::to_string(m_position)) +
		TAB + fmt::sprintf("Rotation: ( %s )" SEOL, prism::to_string(m_rotation)) +
		TAB + fmt::sprintf("Alias: \"%s\"" SEOL,  /* TODO: */ "") +
		TAB + fmt::sprintf("Weight: " FLT_FT SEOL, flh(m_weight)) +
		TAB + fmt::sprintf("Type: \"%s\"" SEOL, type().c_str())
	;
}

String Collision::ConvexLocator::toDefinition() const
{
	return Locator::toDefinition() +
		TAB + fmt::sprintf("ConvexPiece: %i" SEOL, m_convexPiece) +
	"}";
}

String Collision::CylinderLocator::toDefinition() const
{
	return Locator::toDefinition() +
		TAB + fmt::sprintf("Parameters: ( " FLT_FT "  " FLT_FT "  " FLT_FT "  " FLT_FT " )" SEOL, flh(m_radius), flh(m_depth), 0, 0) +
	"}";
}

String Collision::BoxLocator::toDefinition() const
{
	return Locator::toDefinition() +
		TAB + fmt::sprintf("Parameters: ( %s  " FLT_FT " )" SEOL, prism::to_string(m_scale), 0) +
	"}";
}

String Collision::SphereLocator::toDefinition() const
{
	return Locator::toDefinition() +
		TAB + fmt::sprintf("Parameters: ( " FLT_FT "  " FLT_FT "  " FLT_FT "  " FLT_FT " )" SEOL, flh(m_radius), 0, 0, 0) +
		"}";
}

/* eof */
