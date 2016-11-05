/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : collision.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include "collision.h"

#include <model/model.h>
#include <file.h>

bool Collision::load(Model *const model, std::string basePath, std::string filePath)
{
	m_basePath = basePath;
	m_filePath = filePath;
	m_model = model;

	std::string pmcPath = m_basePath + m_filePath + ".pmc";

	File file;
	if (!file.open(pmcPath.c_str(), "rb"))
	{
		printf("Cannot open collision file: \"%s\"! %s" SEOL, pmcPath.c_str(), strerror(errno));
		return false;
	}

	const size_t fileSize = file.getSize();
	std::unique_ptr<uint8_t[]> buffer(new uint8_t[fileSize]);
	file.read((char *)buffer.get(), sizeof(char), fileSize);
	file.close();

	const auto header = reinterpret_cast<prism::pmc_header *>(buffer.get());
	if (header->m_version != prism::pmc_header::SUPPORTED_VERSION)
	{
		printf("Invalid version of collision file: \"%s\"! (have: %i expected: %i)" SEOL, m_filePath.c_str(), header->m_version, prism::pmc_header::SUPPORTED_VERSION);
		return false;
	}

	for (size_t i = 0; i < header->m_piece_count; ++i)
	{
		const auto piecef = reinterpret_cast<prism::pmc_piece *>(buffer.get() + header->m_piece_offset) + i;

		Piece piece;
		piece.m_verts.resize(piecef->m_verts);
		for (size_t v = 0; v < piece.m_verts.size(); ++v)
		{
			piece.m_verts[v] = *(reinterpret_cast<prism::float3 *>(buffer.get() + piecef->m_vert_offset) + v);
		}
		piece.m_triangles.resize(piecef->m_edges / 3);
		for (size_t t = 0; t < piece.m_triangles.size(); ++t)
		{
			piece.m_triangles[t] = *(reinterpret_cast<prism::pmc_triangle *>(buffer.get() + piecef->m_face_offset) + t);
		}
		m_vertCount += piece.m_verts.size();
		m_triangleCount += piece.m_triangles.size();
		m_pieces.push_back(piece);
	}

	for (size_t i = 0; i < header->m_variant_count; ++i)
	{
		const auto variantName = reinterpret_cast<prism::pmc_variant *>(buffer.get() + header->m_variant_offset) + i;
		const auto variantDef = reinterpret_cast<prism::pmc_variant_def *>(buffer.get() + header->m_variant_def_offset) + i;

		Variant variant;
		variant.m_name = variantName->m_name.to_string();
		variant.m_modelVariant = &m_model->getVariants()[i];
		for (size_t j = 0, currentOffset = 0; ; ++j)
		{
			assert((variantDef->m_offset + currentOffset) < fileSize);

			const auto locatorf = reinterpret_cast<prism::pmc_locator *>(buffer.get() + variantDef->m_offset + currentOffset);
			if (locatorf->m_data_size == -1)
				break;

			currentOffset += locatorf->m_data_size;

			const auto it = std::find_if(m_locators.begin(), m_locators.end(), [&](std::shared_ptr<Locator> &loc) {
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

			std::shared_ptr<Locator> locator;
			switch (locatorf->m_data_size)
			{
				case sizeof(prism::pmc_locator_convex):
				{
					auto loc = std::make_shared<ConvexLocator>();
					const auto locf = static_cast<prism::pmc_locator_convex *>(locatorf);
					loc->m_rotation = locf->m_rotation;
					loc->m_convexPiece = locf->m_convex_piece;
					locator = std::move(loc);
				} break;
				case sizeof(prism::pmc_locator_cylinder):
				{
					auto loc = std::make_shared<CylinderLocator>();
					const auto locf = static_cast<prism::pmc_locator_cylinder *>(locatorf);
					loc->m_rotation = locf->m_rotation;
					loc->m_radius = locf->m_radius;
					loc->m_depth = locf->m_depth;
					locator = std::move(loc);
				} break;
				case sizeof(prism::pmc_locator_box):
				{
					auto loc = std::make_shared<BoxLocator>();
					const auto locf = static_cast<prism::pmc_locator_box *>(locatorf);
					loc->m_rotation = locf->m_rotation;
					loc->m_scale = locf->m_scale;
					locator = std::move(loc);
				} break;
				case sizeof(prism::pmc_locator_sphere):
				{
					auto loc = std::make_shared<SphereLocator>();
					const auto locf = static_cast<prism::pmc_locator_sphere *>(locatorf);
					loc->m_radius = locf->m_radius;
					locator = std::move(loc);
				} break;
				default: {
					printf("Not known locator type (\"%s\")! size = %i name = %s\n", m_filePath.c_str(), locatorf->m_data_size, locatorf->m_name.to_string().c_str());
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
	m_basePath.clear();
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
		std::vector<const Variant *> belongsToVariants;
		std::for_each(m_variants.begin(), m_variants.end(), [&belongsToVariants, &loc](const Variant &v) {
			if (std::find(v.m_locators.begin(), v.m_locators.end(), loc) != v.m_locators.end())
			{
				belongsToVariants.push_back(&v);
			}
		});

		for (size_t i = 0; i < m_model->getParts().size(); ++i)
		{
			const auto &part = m_model->getParts()[i];
			std::vector<const Variant *> partBelongsToVariants;
			std::for_each(m_variants.begin(), m_variants.end(), [&partBelongsToVariants, &part, i](const Variant &v) {
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
			printf("[coll] Could not find part for locator: %s(%s)\n", loc->m_name.c_str(), loc->type().c_str());
		}
	}
}

bool Collision::saveToPic(std::string exportPath) const
{
	std::string picFilePath = exportPath + m_filePath + ".pic";

	File file;
	if (!file.open(picFilePath, "wb"))
	{
		printf("Cannot open file: \"%s\"! %s" SEOL, picFilePath.c_str(), strerror(errno));
		return false;
	}

	file << fmt::sprintf(
		"Header {"							SEOL
		TAB "FormatVersion: 2"				SEOL
		TAB "Source: \"%s\""				SEOL
		TAB "Type: \"Collision\""			SEOL
		TAB "Name: \"%s\""					SEOL
		"}"									SEOL,
			STRING_VERSION,
			m_model->fileName().c_str()
	);

	file << fmt::sprintf(
		"Global {"							SEOL
		TAB "VertexCount: %u"				SEOL
		TAB "TriangleCount: %u"				SEOL
		TAB "MaterialCount: 1"				SEOL
		TAB "PieceCount: %i"				SEOL
		TAB "PartCount: %i"					SEOL
		TAB "LocatorCount: %i"				SEOL
		"}"									SEOL,
			m_vertCount,
			m_triangleCount,
			(int)m_pieces.size(),
			(int)m_model->getParts().size(),
			(int)m_locators.size()
	);

	file << fmt::sprintf(
		"Material {"						SEOL
		TAB "Alias: \"%s\""					SEOL
		TAB "Effect: \"%s\""				SEOL
		"}"									SEOL,
			"convex",
			"dry.void"
	);

	for (size_t i = 0; i < m_pieces.size(); ++i)
	{
		const auto &piece = m_pieces[i];
		file << fmt::sprintf(
			"Piece {"						SEOL
			TAB "Index: %i"					SEOL
			TAB "Material: 0"				SEOL
			TAB "VertexCount: %i"			SEOL
			TAB "TriangleCount: %i"		SEOL
			TAB "StreamCount: 1"			SEOL,
			(int)i, (int)piece.m_verts.size(), (int)piece.m_triangles.size()
		);
		file << fmt::sprintf(
			TAB "Stream {"					SEOL
			TAB TAB "Format: FLOAT3"		SEOL
			TAB TAB "Tag: \"_POSITION\""	SEOL
		);
		for (size_t j = 0; j < piece.m_verts.size(); ++j)
		{
			file << fmt::sprintf(
				TAB TAB "%-5i( %s )"		SEOL,
				j, prism::to_string(piece.m_verts[j]).c_str()
			);
		}
		file << TAB "}"						SEOL; // Stream {
		
		file << TAB "Triangles {"			SEOL;
		for (size_t j = 0; j < piece.m_triangles.size(); ++j)
		{
			file << fmt::sprintf(
				TAB TAB "%-5i( %-5i %-5i %-5i )" SEOL,
				(int)j, piece.m_triangles[j].a[2], piece.m_triangles[j].a[1], piece.m_triangles[j].a[0]
			);
		}
		file << TAB "}"						SEOL; // Triangles {

		file << "}"							SEOL; // Piece {
	}

	for (size_t i = 0; i < m_model->getParts().size(); ++i)
	{
		const auto &part = m_model->getParts()[i];

		std::vector<int> locators;
		for (size_t j = 0; j < m_locators.size(); ++j)
		{
			if (m_locators[j]->m_owner == &part)
			{
				locators.push_back(j);
			}
		}

		std::vector<int> pieces;
		/* No idea what are pieces here */

		file << fmt::sprintf(
			"Part {"						SEOL
			TAB "Name: \"%s\""				SEOL
			TAB "PieceCount: %i"			SEOL
			TAB "LocatorCount: %i"			SEOL,
				part.m_name.c_str(),
				(int)pieces.size(),
				(int)locators.size()
		);

		file << TAB "Pieces: ";
		for (const auto piece : pieces)
		{
			file << fmt::sprintf("%i ", piece);
		}
		file <<								SEOL;

		file << TAB "Locators: ";
		for (const auto loc : locators)
		{
			file << fmt::sprintf("%i ", loc);
		}
		file <<								SEOL;

		file << "}"							SEOL; // Part {
	}
	
	for (const auto &locator : m_locators)
	{
		file << locator->toDefinition() << SEOL;
	}

	return true;
}

std::string Collision::Locator::toDefinition() const
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

std::string Collision::ConvexLocator::toDefinition() const
{
	return Locator::toDefinition() +
		TAB + fmt::sprintf("ConvexPiece: %i" SEOL, m_convexPiece) +
	"}";
}

std::string Collision::CylinderLocator::toDefinition() const
{
	return Locator::toDefinition() + 
		TAB + fmt::sprintf("Parameters: ( " FLT_FT "  " FLT_FT "  " FLT_FT "  " FLT_FT " )" SEOL, flh(m_radius), flh(m_depth), 0, 0) +
	"}";
}

std::string Collision::BoxLocator::toDefinition() const
{
	return Locator::toDefinition() +
		TAB + fmt::sprintf("Parameters: ( %s  " FLT_FT " )" SEOL, prism::to_string(m_scale), 0) +
	"}";
}

std::string Collision::SphereLocator::toDefinition() const
{
	return Locator::toDefinition() +
		TAB + fmt::sprintf("Parameters: ( " FLT_FT "  " FLT_FT "  " FLT_FT "  " FLT_FT " )" SEOL, flh(m_radius), 0, 0, 0) +
		"}";
}

/* eof */
