/******************************************************************************
 *
 *  Project:	ConverterPIX @ Core
 *  File:		/material/material_converter_147.h
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

class MaterialConverter147
{
	class AttributeConvert
	{
	public:
		AttributeConvert(String name, int valueCount, int startIndex);
	private:
		String m_name;
		uint32_t m_valueCount;
		/* some new attributes map to a single old attribute
		* example:
		* detail_fadeout_from -> aux[5][0]
		* detail_fadeout_range -> aux[5][1]
		* detail_blend_bias -> aux[5][2]
		* detail_uv_scale -> aux[5][3]
		* that's why we need the startIndex to insert the value at specified offset
		*/
		uint32_t m_startIndex;

		friend MaterialConverter147;
	};

	typedef Map<String, AttributeConvert> AttributeConvertMapPre147; //since 1.47 - map of new attribute names to old attribute names
	typedef Map<String, Array<AttributeConvert> > AttributeConvertMapPost147; //since 1.47 - map of new attribute names to old attribute names

	static AttributeConvertMapPre147 m_convertMapToPre147;

	static AttributeConvertMapPost147 m_convertMapToPost147;
	static AttributeConvertMapPost147 m_convertMapToPost147_TruckpaintRaw;
	static AttributeConvertMapPost147 m_convertMapToPost147_Flipflake;
	static AttributeConvertMapPost147 m_convertMapToPost147_Weight;
	static AttributeConvertMapPost147 m_convertMapToPost147_Leaves;
	static AttributeConvertMapPost147 m_convertMapToPost147_Tg0Tg1;
	static AttributeConvertMapPost147 m_convertMapToPost147_Lightmap;
	static AttributeConvertMapPost147 m_convertMapToPost147_Light;
	static AttributeConvertMapPost147 m_convertMapToPost147_Water;
	static AttributeConvertMapPost147 m_convertMapToPost147_Animsheet;
	static AttributeConvertMapPost147 m_convertMapToPost147_Detail;
	static AttributeConvertMapPost147 m_convertMapToPost147_Anim;
	static AttributeConvertMapPost147 m_convertMapToPost147_Showroom;
	static AttributeConvertMapPost147 m_convertMapToPost147_Mult;
	static AttributeConvertMapPost147 m_convertMapToPost147_Mult2;
	static AttributeConvertMapPost147 m_convertMapToPost147_LampAnim;

public:
	static bool convertAttributesToPost147Format(const String &effectName, const String &attrName, const Array<String> &attrValues, Material::AttributesMap &outAttributes);
	static void convertAttributesToPre147Format(const String &effect, const Material::AttributesMap &inAttributes, Material::AttributesMap &outAttributes);
};