/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : collision.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>

#include <structs/pmc.h>

class Collision
{
public:
	class Piece
	{
	public:
		std::vector<prism::float3> m_verts;
		std::vector<prism::pmc_triangle> m_triangles;
	};
	class Locator
	{
	public:
		int m_type = 0; // 1 - box, 2 - ..., 4 - ..., 8 - convex
		std::string m_name;
		size_t m_index;
		Float3 m_position;
		Quaternion m_rotation;
		float m_weight;
		const Part *m_owner = nullptr;
	public:
		virtual std::string type() const = 0;
		virtual std::string toDefinition() const;
	};
	class ConvexLocator : public Locator
	{
	public:
		unsigned int m_convexPiece = 0;
	public:
		virtual std::string type() const override { return "Convex"; }
		virtual std::string toDefinition() const override;
	};
	class CylinderLocator : public Locator
	{
	public:
		float m_radius;
		float m_depth;
	public:
		virtual std::string type() const override { return "Cylinder"; }
		virtual std::string toDefinition() const override;
	};
	class BoxLocator : public Locator
	{
	public:
		Float3 m_scale;
	public:
		virtual std::string type() const override { return "Box"; }
		virtual std::string toDefinition() const override;
	};
	class SphereLocator : public Locator
	{
	public:
		float m_radius;
	public:
		virtual std::string type() const override { return "Sphere"; }
		virtual std::string toDefinition() const override;
	};
	class Variant
	{
	public:
		std::string m_name;
		std::vector<std::shared_ptr<Locator>> m_locators;
		const ::Variant *m_modelVariant = nullptr;
	};
private:
	Model *m_model = nullptr;
	std::string m_basePath;		// @example C:/ets2/base
	std::string m_filePath;		// @example /vehicle/truck/man_tgx/truck
	std::vector<Piece> m_pieces;
	std::vector<Variant> m_variants;
	std::vector<std::shared_ptr<Locator>> m_locators;

	unsigned int m_vertCount = 0;
	unsigned int m_triangleCount = 0;
public:
	bool load(Model *const model, std::string basePath, std::string filePath);
	void destroy();

	bool saveToPic(std::string exportPath) const;

	void assignLocatorsToParts();
};

/* eof */
