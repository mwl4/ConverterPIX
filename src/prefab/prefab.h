/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : prefab.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include "node.h"
#include "curve.h"
#include "sign.h"
#include "semaphore.h"
#include "spawn_point.h"
#include "map_point.h"
#include "terrain_point_variant.h"
#include "trigger_point.h"
#include "intersection.h"

#include <math/vector.h>

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
	Array<Node> m_nodes;
	Array<Curve> m_curves;
	Array<Sign> m_signs;
	Array<Semaphore> m_semaphores;
	Array<SpawnPoint> m_spawnPoints;
	Array<TerrainPoint> m_terrainPoints;
	Array<MapPoint> m_mapPoints;
	Array<TerrainPointVariant> m_terrainPointVariants;
	Array<TriggerPoint> m_triggerPoints;
	Array<Intersection> m_intersections;

	bool m_loaded = false;

	String m_filePath;		// @example /vehicle/truck/man_tgx/interior/anim
	String m_fileName;		// @example anim
	String m_directory;	// @example /vehicle/truck/man_tgx/interior
public:
	bool load(String filePath);
	void destroy();

	bool saveToPip(String exportPath) const;
};

/* eof */
