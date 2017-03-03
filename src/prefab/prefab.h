/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : prefab.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>
#include <math/vector.h>

#include "node.h"
#include "curve.h"
#include "sign.h"
#include "semaphore.h"
#include "spawn_point.h"
#include "map_point.h"
#include "terrain_point_variant.h"
#include "trigger_point.h"
#include "intersection.h"

class TerrainPoint
{
	friend Prefab;
private:
	Float3 m_position;
	Float3 m_normal;
};

class Prefab
{
private:
	std::vector<Node> m_nodes;
	std::vector<Curve> m_curves;
	std::vector<Sign> m_signs;
	std::vector<Semaphore> m_semaphores;
	std::vector<SpawnPoint> m_spawnPoints;
	std::vector<TerrainPoint> m_terrainPoints;
	std::vector<MapPoint> m_mapPoints;
	std::vector<TerrainPointVariant> m_terrainPointVariants;
	std::vector<TriggerPoint> m_triggerPoints;
	std::vector<Intersection> m_intersections;

	bool m_loaded = false;

	std::string m_filePath;		// @example /vehicle/truck/man_tgx/interior/anim
	std::string m_fileName;		// @example anim
	std::string m_directory;	// @example /vehicle/truck/man_tgx/interior
public:
	bool load(std::string filePath);
	void destroy();

	bool saveToPip(std::string exportPath) const;
};

/* eof */
