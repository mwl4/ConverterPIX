/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/model/collision.h
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

#pragma once

#include <structs/pmc.h>

class Collision
{
public:
	class Piece;
	class Locator;
	class ConvexLocator;
	class CylinderLocator;
	class BoxLocator;
	class SphereLocator;
	class Variant;

public:
	bool load(Model *const model, String filePath);
	void destroy();

	bool saveToPic(String exportPath) const;

	void assignLocatorsToParts();

private:
	Model *m_model = nullptr;
	String m_filePath;		// @example /vehicle/truck/man_tgx/truck
	Array<Piece> m_pieces;
	Array<Variant> m_variants;
	Array<SharedPtr<Locator>> m_locators;

	unsigned int m_vertCount = 0;
	unsigned int m_triangleCount = 0;
};

class Collision::Piece
{
public:
	Array<prism::float3> m_verts;
	Array<prism::pmc_triangle_t> m_triangles;
};

class Collision::Locator
{
public:
	virtual String type() const = 0;
	virtual String toDefinition() const;

public:
	int m_type = 0; // 1 - box, 2 - ..., 4 - ..., 8 - convex
	String m_name;
	size_t m_index;
	Float3 m_position;
	Quaternion m_rotation;
	float m_weight;
	const Part *m_owner = nullptr;
};

class Collision::ConvexLocator : public Locator
{
public:
	virtual String type() const override { return "Convex"; }
	virtual String toDefinition() const override;

public:
	unsigned int m_convexPiece = 0;
};

class Collision::CylinderLocator : public Locator
{
public:
	virtual String type() const override { return "Cylinder"; }
	virtual String toDefinition() const override;

public:
	float m_radius;
	float m_depth;
};

class Collision::BoxLocator : public Locator
{
public:
	virtual String type() const override { return "Box"; }
	virtual String toDefinition() const override;

public:
	Float3 m_scale;
};

class Collision::SphereLocator : public Locator
{
public:
	virtual String type() const override { return "Sphere"; }
	virtual String toDefinition() const override;

public:
	float m_radius;
};

class Collision::Variant
{
public:
	String m_name;
	Array<SharedPtr<Locator>> m_locators;
	const ::Variant *m_modelVariant = nullptr;
};

/* eof */
