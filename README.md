**ConverterPIX**
------------

Conversion tool that is able to convert from the game binary formats (.pmx) to the middle formats (.pix) in *Euro Truck Simulator 2* & *American Truck Simulator*.


----------

This software is used by blender add-on [ConverterPIXWrapper](https://github.com/simon50keda/ConverterPIXWrapper) which allows you easy use in Blender.

----------


***Parameters:***

    -h - prints help text

    <base> - mode: converts entire base(directory or archive) to mid-formats

    -b <base_path> - specify base path and mount it (can be used multiple times)

    -e <export_path> - specify export path
    
    -m <model_path> - mode: convert single model to mid-formats (model_path is relative to base)
    
    -t <tobj_path> - mode: convert single tobj to mid-formats (tobj_path is relative to base)
    
    -d <dds_path> - mode: print debug info about given DDS file (dds_path is absolute path)

    -extract_f <file_path> - mode: extracts given file and saves it in export path (file_path is relative to base)

    -show_f <file_path> - mode: extracts given file and prints it to stdout (file_path is relative to base)

    -extract_d <dir_path> - mode: extracts given directory and saves it in export path (dir_path is relative to base)

    -listdir <dir_path> - mode: lists entries of given directory (dir_path is relative to base)

    -listdir_r <dir_path> - mode: lists entries of given directory recursively (dir_path is relative to base)

    -matFormat147 - switch: output materials to 1.47 mid-format

    -ddsDxt10 - switch: output DDS files in DXT10 format


----------
***Usage:***

    converter_pix -b C:\ets2_base -m /vehicle/truck/man_tgx/interior/anim s_wheel
    
    # export into C:\ets2_base_exp single model with s_wheel animation.
    # instead of exact animation name you can use * to convert every anim file from model directory.
    # when anim name is not started by /, then converter is looking for it in model directory.

    converter_pix C:\ets2_base
    
    # will convert whole base, it will export it into: <base_path>_exp (C:\ets2_base_exp in this example).
    # you can also specify export path using the -e parameter.

    converter_pix C:\base.scs
    
    # will convert whole base, it will export it into: <base_path>_exp (C:\base.scs_exp in this example).

    converter_pix -b C:\base.scs -b C:\base_share.scs C:\base_vehicle.scs

    # will convert whole base(base_vehicle.scs). It will export it into <base_path>_exp (C:\base_vehicle.scs_exp in this example).
    # mounting other archives might be important as files inside base_vehicle.scs may depend on files in other archives.

    converter_pix -b C:\ets2_base -t /material/environment/vehicle_reflection.tobj
    # will convert tobj file and copy texture to export path.

    converter_pix -b C:\ets2_base -m /model/mover/characters/models/generic/m_afam_01 /model/mover/characters/animations/man/walk/walk_01
    # animations are located in another directory than the models. These animations can be used for multiple models.

    converter_pix -b C:\ets2_base -b C:\mod_base -m /vehicle/truck/man_tgx/interior/model
    # multiple base paths (you can specify more paths in the same way)

    converter_pix -b C:\base.scs -extract_f /material/environment/vehicle_reflection.tobj
    # will extract tobj file into <base_path>_exp (if path is not set by -e parameter). C:\base.scs_exp/material/environment/vehicle_reflection.tobj in this example.

    converter_pix -b C:\base.scs -extract_d /material
    # will extract entire directory into <base_path>_exp (if path is not set by -e parameter). C:\base.scs_exp/material in this example.

    converter_pix -b C:\base.scs -listdir /material
    # will show entries (files and subdirectories) of /material directory

    converter_pix -b C:\def.scs -show_f /def/ui.sii
    # will show content of /def/ui.sii file stored in def.scs archive

    
Note that animations will not be converted when converting the whole base.

This is caused by lack of information, so you have to convert each model individually to edit animations.


----------

**Supported formats: pmg(0x13, 0x14, 0x15), pmd(0x04), pma(0x03, 0x04, 0x05), ppd(0x15, 0x16, 0x17), pmc, tobj, mat, scs(hashfs, v1, v2, hash: CITY), zip(uncompressed & compressed)**

----------

**This is Open-Source software under GNU LGPL License.**

--- Copyright © 2024 Michał Wójtowicz a.k.a. mwl4 ---

--- All rights reserved ---

**Enjoy**
