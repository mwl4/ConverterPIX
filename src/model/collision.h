/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : collision.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <structs/pmc.h>

class Collision
{
public:
	class Piece
	{
	public:
		Array<prism::float3> m_verts;
		Array<prism::pmc_triangle_t> m_triangles;
	};
	class Locator
	{
	public:
		int m_type = 0; // 1 - box, 2 - ..., 4 - ..., 8 - convex
		String m_name;
		size_t m_index;
		Float3 m_position;
		Quaternion m_rotation;
		float m_weight;
		const Part *m_owner = nullptr;
	public:
		virtual String type() const = 0;
		virtual String toDefinition() const;
	};
	class ConvexLocator : public Locator
	{
	public:
		unsigned int m_convexPiece = 0;
	public:
		virtual String type() const override { return "Convex"; }
		virtual String toDefinition() const override;
	};
	class CylinderLocator : public Locator
	{
	public:
		float m_radius;
		float m_depth;
	public:
		virtual String type() const override { return "Cylinder"; }
		virtual String toDefinition() const override;
	};
	class BoxLocator : public Locator
	{
	public:
		Float3 m_scale;
	public:
		virtual String type() const override { return "Box"; }
		virtual String toDefinition() const override;
	};
	class SphereLocator : public Locator
	{
	public:
		float m_radius;
	public:
		virtual String type() const override { return "Sphere"; }
		virtual String toDefinition() const override;
	};
	class Variant
	{
	public:
		String m_name;
		Array<SharedPtr<Locator>> m_locators;
		const ::Variant *m_modelVariant = nullptr;
	};
private:
	Model *m_model = nullptr;
	String m_filePath;		// @example /vehicle/truck/man_tgx/truck
	Array<Piece> m_pieces;
	Array<Variant> m_variants;
	Array<SharedPtr<Locator>> m_locators;

	unsigned int m_vertCount = 0;
	unsigned int m_triangleCount = 0;
public:
	bool load(Model *const model, String filePath);
	void destroy();

	bool saveToPic(String exportPath) const;

	void assignLocatorsToParts();
};

/* eof */
