/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/prefab/prefab.h
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

/* forward declarations */
class TerrainPoint;

class Prefab
{
public:
	bool load(String filePath);
	void destroy();

	bool saveToPip(String exportPath) const;

private:
	bool loadVersion0x15(const uint8_t *const buffer, const size_t size);
	bool loadVersion0x16(const uint8_t *const buffer, const size_t size);
	bool loadVersion0x17(const uint8_t *const buffer, const size_t size);
	bool loadVersion0x18(const uint8_t *const buffer, const size_t size);

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
};

class TerrainPoint
{
private:
	Float3 m_position;
	Float3 m_normal;

	friend Prefab;
};

/* eof */
