/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : prefab.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "prefab.h"

#include <fs/file.h>
#include <fs/uberfilesystem.h>
#include <fs/sysfilesystem.h>

#include <structs/ppd.h>

#include <prefab/node.h>
#include <prefab/curve.h>
#include <prefab/sign.h>
#include <prefab/semaphore.h>
#include <prefab/spawn_point.h>
#include <prefab/terrain_point_variant.h>
#include <prefab/map_point.h>
#include <prefab/trigger_point.h>
#include <prefab/intersection.h>

using namespace prism;

bool Prefab::load(String filePath)
{
	if (m_loaded)
		destroy();

	m_filePath = filePath;
	m_directory = directory(filePath);
	m_fileName = filePath.substr(m_directory.length() + 1);

	String ppdPath = m_filePath + ".ppd";
	auto file = getUFS()->open(ppdPath, FileSystem::read | FileSystem::binary);
	if (!file)
	{
		error("prefab", m_filePath, "Unable to open file!");
		return false;
	}

	size_t fileSize = file->size();
	UniquePtr<uint8_t[]> buffer(new uint8_t[fileSize]);
	file->read((char *)buffer.get(), sizeof(uint8_t), fileSize);
	file.reset();

	ppd_header_t *header = (ppd_header_t *)(buffer.get());
	if (header->m_version != ppd_header_t::SUPPORTED_VERSION)
	{
		error_f("prefab", m_filePath, "Invalid version of prefab file! (have: %i, expected: %i)", header->m_version, ppd_header_t::SUPPORTED_VERSION);
		return false;
	}

	if (header->m_node_offset > fileSize
	 || header->m_nav_curve_offset > fileSize
	 || header->m_sign_offset > fileSize
	 || header->m_semaphore_offset > fileSize
	 || header->m_spawn_point_offset > fileSize
	 || header->m_terrain_point_pos_offset > fileSize
	 || header->m_terrain_point_normal_offset > fileSize
	 || header->m_terrain_point_variant_offset > fileSize
	 || header->m_map_point_offset > fileSize
	 || header->m_trigger_point_offset > fileSize
	 || header->m_intersection_offset > fileSize)
	{
		error_f("prefab", m_filePath, "Offset inside file exceeds file size!");
		return false;
	}

	Node node; Curve curve; Sign sign; Semaphore semaphore; SpawnPoint spawnPoint; MapPoint mapPoint;
	TerrainPointVariant terrainPointVariant; TriggerPoint triggerPoint; Intersection intersection;
	TerrainPoint terrainPoint;

	for (u32 i = 0; i < header->m_node_count; ++i)
	{
		ppd_node_t *fnode = (ppd_node_t *)(buffer.get() + header->m_node_offset) + i;
		node.m_terrainPointIdx = fnode->m_terrain_point_idx;
		node.m_terrainPointCount = fnode->m_terrain_point_count;
		node.m_variantIdx = fnode->m_variant_idx;
		node.m_variantCount = fnode->m_variant_count;
		node.m_position = fnode->m_pos;
		node.m_direction = fnode->m_dir;
		for (u32 j = 0; j < 8; ++j)
		{
			node.m_inputLines[j] = fnode->m_input_lines[j];
			node.m_outputLines[j] = fnode->m_output_lines[j];
		}
		m_nodes.push_back(node);
	}

	for (u32 i = 0; i < header->m_nav_curve_count; ++i)
	{
		ppd_curve_t *fcurve = (ppd_curve_t *)(buffer.get() + header->m_nav_curve_offset) + i;
		curve.m_name = token_to_string(fcurve->m_name);
		curve.m_flags = fcurve->m_flags;
		curve.m_leadsToNodes = fcurve->m_leads_to_nodes;
		curve.m_startPosition = fcurve->m_start_pos;
		curve.m_startRotation = fcurve->m_start_rot;
		curve.m_endPosition = fcurve->m_end_pos;
		curve.m_endRotation = fcurve->m_end_rot;
		curve.m_length = fcurve->m_length;
		for (u32 j = 0; j < 4; ++j)
		{
			curve.m_nextLines[j] = fcurve->m_next_lines[j];
			curve.m_prevLines[j] = fcurve->m_prev_lines[j];
		}
		curve.m_nextLinesCount = fcurve->m_count_next;
		curve.m_prevLinesCount = fcurve->m_count_prev;
		curve.m_semaphoreId = fcurve->m_semaphore_id;
		curve.m_trafficRule = token_to_string(fcurve->m_traffic_rule);
		m_curves.push_back(curve);
	}

	for (u32 i = 0; i < header->m_sign_count; ++i)
	{
		ppd_sign_t *fsign = (ppd_sign_t *)(buffer.get() + header->m_sign_offset) + i;
		sign.m_name = token_to_string(fsign->m_name);
		sign.m_position = fsign->m_position;
		sign.m_rotation = fsign->m_rotation;
		sign.m_model = token_to_string(fsign->m_model);
		sign.m_part = token_to_string(fsign->m_part);
		m_signs.push_back(sign);
	}

	for (u32 i = 0; i < header->m_semaphore_count; ++i)
	{
		ppd_semaphore_t *fsemaphore = (ppd_semaphore_t *)(buffer.get() + header->m_semaphore_offset) + i;
		semaphore.m_position = fsemaphore->m_position;
		semaphore.m_rotation = fsemaphore->m_rotation;
		semaphore.m_type = fsemaphore->m_type;
		semaphore.m_semaphoreId = fsemaphore->m_semaphore_id;
		semaphore.m_intervals = fsemaphore->m_intervals;
		semaphore.m_cycle = fsemaphore->m_cycle;
		semaphore.m_profile = token_to_string(fsemaphore->m_profile);
		m_semaphores.push_back(semaphore);
	}

	for (u32 i = 0; i < header->m_spawn_point_count; ++i)
	{
		ppd_spawn_point_t *fspawnpt = (ppd_spawn_point_t *)(buffer.get() + header->m_spawn_point_offset) + i;
		spawnPoint.m_position = fspawnpt->m_position;
		spawnPoint.m_rotation = fspawnpt->m_rotation;
		spawnPoint.m_type = fspawnpt->m_type;
		m_spawnPoints.push_back(spawnPoint);
	}

	for (u32 i = 0; i < header->m_terrain_point_count; ++i)
	{
		terrainPoint.m_position = *((float3 *)(buffer.get() + header->m_terrain_point_pos_offset) + i);
		terrainPoint.m_normal = *((float3 *)(buffer.get() + header->m_terrain_point_normal_offset) + i);
		m_terrainPoints.push_back(terrainPoint);
	}

	for (u32 i = 0; i < header->m_terrain_point_variant_count; ++i)
	{
		ppd_terrain_point_variant_t *ftpv = (ppd_terrain_point_variant_t *)(buffer.get() + header->m_terrain_point_variant_offset) + i;
		terrainPointVariant.m_attach0 = ftpv->m_attach0;
		terrainPointVariant.m_attach1 = ftpv->m_attach1;
		m_terrainPointVariants.push_back(terrainPointVariant);
	}

	for (u32 i = 0; i < header->m_map_point_count; ++i)
	{
		ppd_map_point_t *fmappt = (ppd_map_point_t *)(buffer.get() + header->m_map_point_offset) + i;
		mapPoint.m_mapVisualFlags = fmappt->m_map_visual_flags;
		mapPoint.m_mapNavFlags = fmappt->m_map_nav_flags;
		mapPoint.m_position = fmappt->m_position;
		for (u32 j = 0; j < 6; ++j)
		{
			mapPoint.m_neighbour[j] = fmappt->m_neighbours[j];
		}
		mapPoint.m_neighbourCount = fmappt->m_neighbour_count;
		m_mapPoints.push_back(mapPoint);
	}

	for (u32 i = 0; i < header->m_trigger_point_count; ++i)
	{
		ppd_trigger_point_t *triggerpt = (ppd_trigger_point_t *)(buffer.get() + header->m_trigger_point_offset) + i;
		triggerPoint.m_id = triggerpt->m_trigger_id;
		triggerPoint.m_action = token_to_string(triggerpt->m_trigger_action);
		triggerPoint.m_range = triggerpt->m_trigger_range;
		triggerPoint.m_reset_delay = triggerpt->m_trigger_reset_delay;
		triggerPoint.m_reset_dist = triggerpt->m_trigger_reset_dist;
		triggerPoint.m_flags = triggerpt->m_flags;
		triggerPoint.m_position = triggerpt->m_position;
		triggerPoint.m_neighbours[0] = triggerpt->m_neighbours[0];
		triggerPoint.m_neighbours[1] = triggerpt->m_neighbours[1];
		m_triggerPoints.push_back(triggerPoint);
	}

	for (u32 i = 0; i < header->m_intersection_count; ++i)
	{
		ppd_intersection_t *is = (ppd_intersection_t *)(buffer.get() + header->m_intersection_offset) + i;
		intersection.m_curveId = is->m_inter_curve_id;
		intersection.m_position = is->m_inter_position;
		intersection.m_radius = is->m_inter_radius;
		intersection.m_flags = is->m_flags;
		m_intersections.push_back(intersection);
	}

	m_loaded = true;
	return m_loaded;
}

void Prefab::destroy()
{
	m_nodes.clear();
	m_curves.clear();
	m_signs.clear();
	m_semaphores.clear();
	m_spawnPoints.clear();
	m_mapPoints.clear();
	m_terrainPointVariants.clear();
	m_triggerPoints.clear();
	m_intersections.clear();
	m_loaded = false;
}

bool Prefab::saveToPip(String exportPath) const
{
	String pipFilePath = exportPath + m_filePath + ".pip";
	auto file = getSFS()->open(pipFilePath, FileSystem::write | FileSystem::binary);
	if (!file)
	{
		error_f("prefab", pipFilePath, "Unable to save file (%s)", getSFS()->getError());
		return false;
	}

	*file << fmt::sprintf(
		"Header {"							SEOL
		TAB "FormatVersion: 2"				SEOL
		TAB "Source: \"%s\""				SEOL
		TAB "Type: \"Prefab\""				SEOL
		TAB "Name: \"%s\""					SEOL
		"}"									SEOL,
			STRING_VERSION,
			m_fileName.c_str()
		);

	*file << fmt::sprintf(
		"Global {"							SEOL
		TAB "NodeCount: %i"					SEOL
		TAB "TerrainPointCount : %i"		SEOL
		TAB "TerrainPointVariantCount : %i"	SEOL
		TAB "NavCurveCount : %i"			SEOL
		TAB "SignCount : %i"				SEOL
		TAB "SpawnPointCount : %i"			SEOL
		TAB "SemaphoreCount : %i"			SEOL
		TAB "MapPointCount : %i"			SEOL
		TAB "TriggerPointCount : %i"		SEOL
		TAB "IntersectionCount : %i"		SEOL
		"}"									SEOL,
			(int)m_nodes.size(),
			(int)m_terrainPoints.size(),
			(int)m_terrainPointVariants.size(),
			(int)m_curves.size(),
			(int)m_signs.size(),
			(int)m_spawnPoints.size(),
			(int)m_semaphores.size(),
			(int)m_mapPoints.size(),
			(int)m_triggerPoints.size(),
			(int)m_intersections.size()
		);

	for (size_t i = 0; i < m_nodes.size(); ++i)
	{
		const Node *node = &m_nodes[i];
		*file << "Node {" SEOL;
		*file << fmt::sprintf(
			TAB "Index: %i" SEOL
			TAB "Position : ( %s )" SEOL
			TAB "Direction : ( %s )" SEOL,
				i,
				to_string(node->m_position).c_str(),
				to_string(node->m_direction).c_str()
			);

		*file << TAB "InputLanes: ("; for (u32 j = 0; j < 8; ++j) { *file << fmt::sprintf(" %i", node->m_inputLines[j]); } *file << " )" SEOL;
		*file << TAB "OutputLanes: ("; for (u32 j = 0; j < 8; ++j) { *file << fmt::sprintf(" %i", node->m_outputLines[j]); } *file << " )" SEOL;
		*file << SEOL;

		*file << fmt::sprintf(
			TAB "TerrainPointCount: %i" SEOL
			TAB "TerrainPointVariantCount : %i" SEOL
			TAB "StreamCount : %i" SEOL,
				node->m_terrainPointCount,
				node->m_variantCount,
				(node->m_variantCount > 0 ? 1 : 0) + (node->m_terrainPointCount > 0 ? 2 : 0)
			);

		if (node->m_terrainPointCount > 0)
		{
			*file << fmt::sprintf(
				TAB "Stream {" SEOL
				TAB TAB "Format: %s" SEOL
				TAB TAB "Tag: \"%s\"" SEOL,
					"FLOAT3",
					"_POSITION"
				);

			for (u32 j = 0; j < node->m_terrainPointCount; ++j)
			{
				if (node->m_terrainPointIdx + j >= m_terrainPoints.size())
				{
					warning_f("prefab", m_filePath, "Terrain point index exceeds terrain points count!");
					break;
				}

				*file << fmt::sprintf(
					TAB TAB "%-5i( %s )" SEOL,
						j, to_string(m_terrainPoints[node->m_terrainPointIdx + j].m_position).c_str()
					);
			}

			*file << TAB "}" SEOL;

			*file << fmt::sprintf(
				TAB "Stream {" SEOL
				TAB TAB "Format: %s" SEOL
				TAB TAB "Tag: \"%s\"" SEOL,
					"FLOAT3",
					"_NORMAL"
				);

			for (u32 j = 0; j < node->m_terrainPointCount; ++j)
			{
				if (node->m_terrainPointIdx + j >= m_terrainPoints.size())
				{
					// Error has been printed already (see above)
					break;
				}

				*file << fmt::sprintf(
					TAB TAB "%-5i( %s )" SEOL,
						j, to_string(m_terrainPoints[node->m_terrainPointIdx + j].m_normal).c_str()
					);
			}

			*file << TAB "}" SEOL;
		}

		if (node->m_variantCount > 0)
		{
			*file << fmt::sprintf(
				TAB "Stream {" SEOL
				TAB TAB "Format: %s" SEOL
				TAB TAB "Tag: \"%s\"" SEOL,
					"INT2",
					"_VARIANT_BLOCK"
				);

			for (u32 j = 0; j < node->m_variantCount; ++j)
			{
				TerrainPointVariant data = m_terrainPointVariants[node->m_variantIdx + j];
				*file << fmt::sprintf(
					TAB TAB "%-5i( %i %i )" SEOL,
						j, data.m_attach0, data.m_attach1
					);
			}

			*file << TAB "}" SEOL;
		}
		*file << "}" SEOL;
	}

	for (size_t i = 0; i < m_curves.size(); ++i)
	{
		const Curve *curve = &m_curves[i];
		*file << "Curve {" SEOL;
		*file << fmt::sprintf(
			TAB "Index: %i" SEOL
			TAB "Name: \"%s\"" SEOL
			TAB "Flags: %u" SEOL
			TAB "LeadsToNodes: %i" SEOL,
				i,
				curve->m_name.c_str(),
				curve->m_flags,
				curve->m_leadsToNodes
			);

		if (curve->m_trafficRule.length() > 0)
		{
			*file << fmt::sprintf(
				TAB "TrafficRule: \"%s\"" SEOL,
				curve->m_trafficRule.c_str()
			);
		}

		if (curve->m_semaphoreId != -1)
		{
			*file << fmt::sprintf(
				TAB "SemaphoreID: %i" SEOL,
				curve->m_semaphoreId
			);
		}

		*file << TAB "NextCurves: ("; for (u32 j = 0; j < 4; ++j) { *file << fmt::sprintf(" %i", curve->m_nextLines[j]); } *file << " )" SEOL;
		*file << TAB "PrevCurves: ("; for (u32 j = 0; j < 4; ++j) { *file << fmt::sprintf(" %i", curve->m_prevLines[j]); } *file << " )" SEOL;

		*file << fmt::sprintf(
			TAB "Length: " FLT_FT SEOL,
				flh(curve->m_length)
			);

		*file << TAB "Bezier {" SEOL;
		*file << fmt::sprintf(
			TAB TAB "Start {" SEOL
			TAB TAB TAB "Position: ( %s )" SEOL
			TAB TAB TAB "Rotation: ( %s )" SEOL
			TAB TAB "}" SEOL,
				to_string(curve->m_startPosition).c_str(),
				to_string(curve->m_startRotation).c_str()
			);

		*file << fmt::sprintf(
			TAB TAB "End {" SEOL
			TAB TAB TAB "Position: ( %s )" SEOL
			TAB TAB TAB "Rotation: ( %s )" SEOL
			TAB TAB "}" SEOL,
				to_string(curve->m_endPosition).c_str(),
				to_string(curve->m_endRotation).c_str()
			);

		*file << TAB "}" SEOL;
		*file << "}" SEOL;
	}

	for (size_t i = 0; i < m_signs.size(); ++i)
	{
		const Sign *sign = &m_signs[i];
		*file << "Sign {" SEOL;
		*file << fmt::sprintf(
			TAB "Name: \"%s\"" SEOL
			TAB "Position: ( %s )" SEOL
			TAB "Rotation: ( %s )" SEOL
			TAB "Model: \"%s\"" SEOL
			TAB "Part: \"%s\"" SEOL,
				sign->m_name.c_str(),
				to_string(sign->m_position).c_str(),
				to_string(sign->m_rotation).c_str(),
				sign->m_model.c_str(),
				sign->m_part.c_str()
			);
		*file << "}" SEOL;
	}

	for (size_t i = 0; i < m_spawnPoints.size(); ++i)
	{
		const SpawnPoint *sp = &m_spawnPoints[i];
		*file << "SpawnPoint {" SEOL;
		*file << fmt::sprintf(
			TAB "Name: \"%s\"" SEOL
			TAB "Position: ( %s )" SEOL
			TAB "Rotation: ( %s )" SEOL
			TAB "Type: %i" SEOL,
				fmt::sprintf("sp_%i", (int)i).c_str(),
				to_string(sp->m_position).c_str(),
				to_string(sp->m_rotation).c_str(),
				sp->m_type
			);
		*file << "}" SEOL;
	}

	for (size_t i = 0; i < m_semaphores.size(); ++i)
	{
		const Semaphore *semaphore = &m_semaphores[i];
		*file << "Semaphore {" SEOL;
		*file << fmt::sprintf(
			TAB "Position: ( %s )" SEOL
			TAB "Rotation: ( %s )" SEOL
			TAB "Type: %i" SEOL
			TAB "SemaphoreID: %i" SEOL
			TAB "Intervals: ( %s )" SEOL
			TAB "Cycle: " FLT_FT SEOL
			TAB "Profile: \"%s\"" SEOL,
				to_string(semaphore->m_position).c_str(),
				to_string(semaphore->m_rotation).c_str(),
				semaphore->m_type,
				semaphore->m_semaphoreId,
				to_string(semaphore->m_intervals).c_str(),
				flh(semaphore->m_cycle),
				semaphore->m_profile.c_str()
			);
		*file << "}" SEOL;
	}

	for (size_t i = 0; i < m_mapPoints.size(); ++i)
	{
		const MapPoint *mp = &m_mapPoints[i];
		*file << "MapPoint {" SEOL;
		*file << fmt::sprintf(
			TAB "Index: %i" SEOL
			TAB "MapVisualFlags: %u" SEOL
			TAB "MapNavFlags: %u" SEOL
			TAB "Position: ( %s )" SEOL
			TAB "Neighbours: ( %s )" SEOL,
				i,
				mp->m_mapVisualFlags,
				mp->m_mapNavFlags,
				to_string(mp->m_position).c_str(),
				to_string(mp->m_neighbour).c_str()
			);
		*file << "}" SEOL;
	}

	for (size_t i = 0; i < m_triggerPoints.size(); ++i)
	{
		const TriggerPoint *tp = &m_triggerPoints[i];
		*file << "TriggerPoint {" SEOL;
		*file << fmt::sprintf(
			TAB "Index: %i" SEOL
			TAB "TriggerID: %i" SEOL
			TAB "TriggerAction: \"%s\"" SEOL
			TAB "TriggerRange: %f" SEOL
			TAB "TriggerResetDelay: %f" SEOL
			TAB "TriggerResetDist: %f" SEOL
			TAB "Flags: %u" SEOL
			TAB "Position: ( %s )" SEOL
			TAB "Neighbours: ( %s )" SEOL,
				(int)i,
				tp->m_id,
				tp->m_action.c_str(),
				tp->m_range,
				tp->m_reset_delay,
				tp->m_reset_dist,
				tp->m_flags,
				to_string(tp->m_position).c_str(),
				to_string(tp->m_neighbours).c_str()
			);
		*file << "}" SEOL;
	}

	for (size_t i = 0; i < m_intersections.size(); ++i)
	{
		const Intersection *is = &m_intersections[i];
		*file << "Intersection {" SEOL;
		*file << fmt::sprintf(
			TAB "InterCurveID: %i" SEOL
			TAB "InterPosition: %f" SEOL
			TAB "InterRadius: %f" SEOL
			TAB "Flags: %u" SEOL,
				is->m_curveId,
				is->m_position,
				is->m_radius,
				is->m_flags
			);
		*file << "}" SEOL;
	}
	file.reset();
	return true;
}

/* eof */
