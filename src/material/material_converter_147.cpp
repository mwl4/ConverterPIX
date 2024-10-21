/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/material/material_converter_147.cpp
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

#include <prerequisites.h>

#include "material.h"
#include "material_converter_147.h"
#include "math/vector.h"

MaterialConverter147::AttributeConvert::AttributeConvert(String name, int valueCount, int startIndex)
	: m_name(name)
	, m_valueCount(valueCount)
	, m_startIndex(startIndex)
{
}

MaterialConverter147::AttributeConvertMapPre147 MaterialConverter147::m_convertMapToPre147 = {
	{ "additional_ambient", MaterialConverter147::AttributeConvert("add_ambient", 1, 0) },
	{ "glass_tint_color", MaterialConverter147::AttributeConvert("tint", 3, 0) },
	{ "glass_tint_opacity", MaterialConverter147::AttributeConvert("tint_opacity", 1, 0) },
	{ "shadowmap_bias", MaterialConverter147::AttributeConvert("shadow_bias", 1, 0) },
	{ "paintjob_base_color", MaterialConverter147::AttributeConvert("aux[8]", 3, 0) },
	{ "specular_secondary", MaterialConverter147::AttributeConvert("aux[3]", 4, 0) },
	{ "shininess_secondary", MaterialConverter147::AttributeConvert("aux[3]", 4, 3) },
	{ "reflection_secondary", MaterialConverter147::AttributeConvert("reflection2", 1, 0) },
	{ "lod_selector", MaterialConverter147::AttributeConvert("aux[1]", 1, 0) },
	{ "shadow_offset", MaterialConverter147::AttributeConvert("aux[0]", 1, 0) },
	{ "amod_decal_blending_factors", MaterialConverter147::AttributeConvert("aux[0]", 2, 0) }, //not present in materials until 1.47
	{ "texgen_0_gen", MaterialConverter147::AttributeConvert("aux[0]", 4, 0) }, //was float2, now float4
	{ "texgen_0_rot", MaterialConverter147::AttributeConvert("aux[0]", 4, 2) }, //not present in materials until 1.47
	{ "texgen_1_gen", MaterialConverter147::AttributeConvert("aux[1]", 4, 0) }, //was float2, now float4
	{ "texgen_1_rot", MaterialConverter147::AttributeConvert("aux[1]", 4, 2) }, //not present in materials until 1.47
	{ "far_color", MaterialConverter147::AttributeConvert("aux[2]", 4, 0) }, //not present in materials until 1.47
	{ "far_specular_power", MaterialConverter147::AttributeConvert("aux[2]", 4, 3) }, //not present in materials until 1.47
	{ "depth_bias", MaterialConverter147::AttributeConvert("aux[0]", 1, 0) },
	{ "luminance_output", MaterialConverter147::AttributeConvert("aux[5]", 2, 0) },
	{ "luminance_night", MaterialConverter147::AttributeConvert("aux[5]", 2, 1) },
	{ "interior_atlas_dimensions", MaterialConverter147::AttributeConvert("aux[1]", 2, 0) }, //not present in materials until 1.51
	{ "interior_glass_color", MaterialConverter147::AttributeConvert("aux[2]", 4, 0) }, //not present in materials until 1.51
	{ "interior_unit_room_dimensions", MaterialConverter147::AttributeConvert("aux[0]", 2, 0) }, //not present in materials until 1.51
	{ "water_distances", MaterialConverter147::AttributeConvert("aux[0]", 3, 0) },
	{ "water_near_color", MaterialConverter147::AttributeConvert("aux[1]", 3, 0) },
	{ "water_horizon_color", MaterialConverter147::AttributeConvert("aux[2]", 3, 0) },
	{ "water_layer_0_yaw", MaterialConverter147::AttributeConvert("aux[3]", 4, 0) },
	{ "water_layer_0_speed", MaterialConverter147::AttributeConvert("aux[3]", 4, 1) },
	{ "water_layer_0_scale", MaterialConverter147::AttributeConvert("aux[3]", 4, 2) },
	{ "water_layer_1_yaw", MaterialConverter147::AttributeConvert("aux[4]", 4, 0) },
	{ "water_layer_1_speed", MaterialConverter147::AttributeConvert("aux[4]", 4, 1) },
	{ "water_layer_1_scale", MaterialConverter147::AttributeConvert("aux[4]", 4, 2) },
	{ "water_mirror", MaterialConverter147::AttributeConvert("aux[5]", 1, 0) },
	{ "animsheet_cfg_fps", MaterialConverter147::AttributeConvert("aux[0]", 3, 0) },
	{ "animsheet_cfg_frames_row", MaterialConverter147::AttributeConvert("aux[0]", 3, 1) },
	{ "animsheet_cfg_frames_total", MaterialConverter147::AttributeConvert("aux[0]", 3, 2) },
	{ "animsheet_frame_width", MaterialConverter147::AttributeConvert("aux[1]", 2, 0) },
	{ "animsheet_frame_height", MaterialConverter147::AttributeConvert("aux[1]", 2, 1) },
	{ "detail_fadeout_from", MaterialConverter147::AttributeConvert("aux[5]", 4, 0) },
	{ "detail_fadeout_range", MaterialConverter147::AttributeConvert("aux[5]", 4, 1) },
	{ "detail_blend_bias", MaterialConverter147::AttributeConvert("aux[5]", 4, 2) },
	{ "detail_uv_scale", MaterialConverter147::AttributeConvert("aux[5]", 4, 3) },
	{ "animation_speed", MaterialConverter147::AttributeConvert("aux[0]", 1, 0) },
	{ "showroom_r_color", MaterialConverter147::AttributeConvert("aux[0]", 1, 0) }, //was single float, since 1.47 is 3d vector
	{ "showroom_speed", MaterialConverter147::AttributeConvert("aux[4]", 3, 0) },
	{ "flake_uvscale", MaterialConverter147::AttributeConvert("aux[5]", 4, 0) },
	{ "flake_shininess", MaterialConverter147::AttributeConvert("aux[5]", 4, 1) },
	{ "flake_clearcoat_rolloff", MaterialConverter147::AttributeConvert("aux[5]", 4, 2) },
	{ "flake_vratio", MaterialConverter147::AttributeConvert("aux[5]", 4, 3) },
	{ "flake_color", MaterialConverter147::AttributeConvert("aux[6]", 4, 0) },
	{ "flake_density", MaterialConverter147::AttributeConvert("aux[6]", 4, 3) },
	{ "flip_color", MaterialConverter147::AttributeConvert("aux[7]", 4, 0) },
	{ "flip_strength", MaterialConverter147::AttributeConvert("aux[7]", 4, 3) },
	{ "mix00_diffuse_secondary", MaterialConverter147::AttributeConvert("aux[0]", 3, 0) }, //not present in materials until 1.47
	{ "mult_uvscale", MaterialConverter147::AttributeConvert("aux[5]", 4, 0) },
	{ "mult_uvscale_secondary", MaterialConverter147::AttributeConvert("aux[5]", 4, 2) },
	{ "sheet_frame_size_r", MaterialConverter147::AttributeConvert("aux[0]", 4, 0) },
	{ "sheet_frame_size_g", MaterialConverter147::AttributeConvert("aux[0]", 4, 2) },
	{ "sheet_frame_size_b", MaterialConverter147::AttributeConvert("aux[1]", 4, 0) },
	{ "sheet_frame_size_a", MaterialConverter147::AttributeConvert("aux[1]", 4, 2) },
	{ "paintjob_r_color", MaterialConverter147::AttributeConvert("aux[5]", 3, 0) },
	{ "paintjob_g_color", MaterialConverter147::AttributeConvert("aux[6]", 3, 0) },
	{ "paintjob_b_color", MaterialConverter147::AttributeConvert("aux[7]", 3, 0) }
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147 = {
	{ "add_ambient", { MaterialConverter147::AttributeConvert("additional_ambient", 1, 0) } },
	{ "shadow_bias", { MaterialConverter147::AttributeConvert("shadowmap_bias", 1, 0) } },
	{ "tint", { MaterialConverter147::AttributeConvert("glass_tint_color", 3, 0) } },
	{ "tint_opacity", { MaterialConverter147::AttributeConvert("glass_tint_opacity", 1, 0) } },
	{ "reflection_secondary", { MaterialConverter147::AttributeConvert("reflection2", 1, 0) } },
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_TruckpaintRaw = {
	{ "aux[5]", { MaterialConverter147::AttributeConvert("paintjob_r_color", 3, 0) } },
	{ "aux[6]", { MaterialConverter147::AttributeConvert("paintjob_g_color", 3, 0) } },
	{ "aux[7]", { MaterialConverter147::AttributeConvert("paintjob_b_color", 3, 0) } },
	{ "aux[8]", { MaterialConverter147::AttributeConvert("paintjob_base_color", 3, 0) } }
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Flipflake = {
	{ "aux[5]", {
		MaterialConverter147::AttributeConvert("flake_uvscale", 1, 0),
		MaterialConverter147::AttributeConvert("flake_shininess", 1, 1),
		MaterialConverter147::AttributeConvert("flake_clearcoat_rolloff", 1, 2),
		MaterialConverter147::AttributeConvert("flake_vratio", 1, 3)
		}
	},
	{ "aux[6]", {
		MaterialConverter147::AttributeConvert("flake_color", 3, 0),
		MaterialConverter147::AttributeConvert("flake_density", 1, 3)
		}
	},
	{ "aux[7]", {
		MaterialConverter147::AttributeConvert("flip_color", 3, 0),
		MaterialConverter147::AttributeConvert("flip_strength", 1, 3)
		}
	},
	{ "aux[8]", { MaterialConverter147::AttributeConvert("paintjob_base_color", 3, 0) } }
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Weight = {
	{ "aux[3]", {
		MaterialConverter147::AttributeConvert("specular_secondary", 3, 0),
		MaterialConverter147::AttributeConvert("shininess_secondary", 1, 3)
		}
	},
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Leaves = {
	{ "aux[0]", { MaterialConverter147::AttributeConvert("shadow_offset", 1, 0) } },
	{ "aux[1]", { MaterialConverter147::AttributeConvert("lod_selector", 1, 0) } }
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Tg0Tg1 = {
	{ "aux[0]", { MaterialConverter147::AttributeConvert("texgen_0_gen", 2, 0) } },
	{ "aux[0]", { MaterialConverter147::AttributeConvert("texgen_0_rot", 1, 2) } },
	{ "aux[1]", { MaterialConverter147::AttributeConvert("texgen_1_gen", 2, 0) } },
	{ "aux[1]", { MaterialConverter147::AttributeConvert("texgen_1_rot", 1, 2) } }
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Lightmap = {
	{ "aux[0]", { MaterialConverter147::AttributeConvert("depth_bias", 1, 0) } }
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Light = {
	{ "aux[5]", {
		MaterialConverter147::AttributeConvert("luminance_output", 1, 0),
		MaterialConverter147::AttributeConvert("luminance_night", 1, 1) }
	}
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Water = {
	{ "aux[0]", { MaterialConverter147::AttributeConvert("water_distances", 3, 0) } },
	{ "aux[1]", { MaterialConverter147::AttributeConvert("water_near_color", 3, 0) } },
	{ "aux[2]", { MaterialConverter147::AttributeConvert("water_horizon_color", 3, 0) } },
	{ "aux[3]", {
		MaterialConverter147::AttributeConvert("water_layer_0_yaw", 1, 0),
		MaterialConverter147::AttributeConvert("water_layer_0_speed", 1, 1),
		MaterialConverter147::AttributeConvert("water_layer_0_scale", 2, 2) }
	},
	{ "aux[4]", {
		MaterialConverter147::AttributeConvert("water_layer_1_yaw", 1, 0),
		MaterialConverter147::AttributeConvert("water_layer_1_speed", 1, 1),
		MaterialConverter147::AttributeConvert("water_layer_1_scale", 2, 2) }
	},
	{ "aux[5]", { MaterialConverter147::AttributeConvert("water_mirror", 1, 0) } }
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Animsheet = {
	{ "aux[0]", {
		MaterialConverter147::AttributeConvert("animsheet_cfg_fps", 1, 0),
		MaterialConverter147::AttributeConvert("animsheet_cfg_frames_row", 1, 1),
		MaterialConverter147::AttributeConvert("animsheet_cfg_frames_total", 1, 2) }
	},
	{ "aux[1]", {
		MaterialConverter147::AttributeConvert("animsheet_frame_width", 1, 0),
		MaterialConverter147::AttributeConvert("animsheet_frame_height", 1, 1) }
	}
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Detail = {
	{ "aux[5]", {
		MaterialConverter147::AttributeConvert("detail_fadeout_from", 1, 0),
		MaterialConverter147::AttributeConvert("detail_fadeout_range", 1, 1),
		MaterialConverter147::AttributeConvert("detail_blend_bias", 1, 2),
		MaterialConverter147::AttributeConvert("detail_uv_scale", 1, 3) }
	}
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Anim = {
	{ "aux[0]", { MaterialConverter147::AttributeConvert("animation_speed", 1, 0) } }
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Showroom = {
	{ "aux[0]", { MaterialConverter147::AttributeConvert("showroom_r_color", 3, 0) } }, //was single float, since 1.47 is 3d vector
	{ "aux[4]", { MaterialConverter147::AttributeConvert("showroom_speed", 3, 0) } }
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Mult = {
	{ "aux[5]", {
		MaterialConverter147::AttributeConvert("mult_uvscale", 2, 0) }
	}
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_Mult2 = {
	{ "aux[5]", {
		MaterialConverter147::AttributeConvert("mult_uvscale_secondary", 2, 2) }
	}
};

MaterialConverter147::AttributeConvertMapPost147 MaterialConverter147::m_convertMapToPost147_LampAnim = {
	{ "aux[0]", {
		MaterialConverter147::AttributeConvert("sheet_frame_size_r", 2, 0),
		MaterialConverter147::AttributeConvert("sheet_frame_size_g", 2, 2) }
	},
	{ "aux[1]", {
		MaterialConverter147::AttributeConvert("sheet_frame_size_b", 4, 0),
		MaterialConverter147::AttributeConvert("sheet_frame_size_a", 4, 2) }
	}
};

bool MaterialConverter147::convertAttributesToPost147Format(const String &effectName, const String &attrName, const Array<String> &attrValues, Material::AttributesMap &outAttributes)
{
	auto checkAndWriteAttributes = [&] (auto &convertMapToPost147) {
		auto convertRule = convertMapToPost147.find(attrName);
		if (convertRule != convertMapToPost147.end())
		{
			for (auto it = convertRule->second.begin(); it != convertRule->second.end(); ++it)
			{
				Material::Attribute& newAttr = outAttributes[(*it).m_name];
				newAttr.m_name = (*it).m_name;
				newAttr.m_valueType = Material::Attribute::FLOAT;
				newAttr.m_valueCount = (*it).m_valueCount;

				Material::setValues(newAttr, attrValues, (*it).m_startIndex);
				//info_f("material", effectName, "Converted attribute to post 1.47 format: %s -> %s(%s)", attrName.c_str(), newAttr.m_name.c_str(), prism::to_string(newAttr.m_value).c_str());
			}
			return true;
		}
		return false;
	};

	if (checkAndWriteAttributes(m_convertMapToPost147))
		return true;

	if (effectName.find("dif.spec.weight") != String::npos) //aux[3]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Weight))
			return true;
	}

	if (effectName.find("dif.spec.weight.mult2.weight2") != String::npos) //aux[5]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Mult2))
			return true;
	}

	if (effectName.find("dif.spec.weight.mult2") != String::npos) //aux[5]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Mult))
			return true;
	}

	if (effectName.find("leaves") != String::npos) //aux[0], aux[1]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Leaves))
			return true;
	}

	if (effectName.find("tg0") != String::npos || effectName.find("tg1") != String::npos) //aux[0], aux[1]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Tg0Tg1))
			return true;
	}

	if (effectName.find("light") != String::npos) //aux[0]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Lightmap))
			return true;
	}

	if (effectName.find("night") != String::npos || effectName.find("flare") != String::npos
		|| effectName.find("unlit") != String::npos || effectName.find(".lum") != String::npos
		|| effectName.find("light.tex") != String::npos) //aux[5]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Light))
			return true;
	}

	if (effectName.find("water") != String::npos) //aux[0], aux[1], aux[2], aux[3], aux[4], aux[5]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Water))
			return true;
	}

	if (effectName.find(".flipsheet") != String::npos || effectName.find(".fadesheet") != String::npos
		|| effectName.find(".bounce") != String::npos) //aux[0], aux[1]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Animsheet))
			return true;
	}

	if (effectName.find(".fade.") != String::npos) //aux[5]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Detail))
			return true;
	}

	if (effectName.find("lamp.anim") != String::npos)
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_LampAnim)) //aux[0], aux[1]
			return true;
	}
	else if (effectName.find(".anim.") != String::npos) //aux[0]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Anim))
			return true;
	}

	if (effectName.find("showroom_v2") != String::npos) //aux[0], aux[4]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Showroom))
			return true;
	}

	if (effectName.find("flipflake") != String::npos) //aux[5], aux[6], aux[7], aux[8]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_Flipflake))
			return true;
	}
	else if (effectName.find("truckpaint") != String::npos) //aux[5], aux[6], aux[7], aux[8]
	{
		if (checkAndWriteAttributes(m_convertMapToPost147_TruckpaintRaw))
			return true;
	}


	return false;
}

void MaterialConverter147::convertAttributesToPre147Format(const String &effect, const Material::AttributesMap& inAttributes, Material::AttributesMap& outAttributes)
{
	for (auto attribute : inAttributes)
	{
		AttributeConvertMapPre147::iterator convertRule = m_convertMapToPre147.find(attribute.second.m_name);
		if (convertRule != m_convertMapToPre147.end())
		{
			Material::Attribute &convertedAttribute = outAttributes[convertRule->second.m_name];
			convertedAttribute.m_name = convertRule->second.m_name;
			convertedAttribute.m_valueCount = convertRule->second.m_valueCount;
			convertedAttribute.m_valueType = attribute.second.m_valueType;
			for (uint32_t i=0; i<attribute.second.m_valueCount; i++)
			{
				convertedAttribute.m_value[convertRule->second.m_startIndex+i] = attribute.second.m_value[i];
			}
			//info_f("material", effect, "Converted attribute to pre 1.47 format: %s(%s) -> %s(%s)", attribute.second.m_name.c_str(), prism::to_string(attribute.second.m_value).c_str(), convertedAttribute.m_name.c_str(), prism::to_string(convertedAttribute.m_value).c_str());
		}
		else
			outAttributes[attribute.first] = attribute.second;
	}
}