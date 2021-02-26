**ConverterPIX**
------------

Conversion tool that is able to convert from the game binary formats (.pmx) to the middle formats (.pix) in *Euro Truck Simulator 2* & *American Truck Simulator*.


----------

This software is used by blender add-on [ConverterPIXWrapper](https://github.com/simon50keda/ConverterPIXWrapper) which allows you easy use in Blender.

----------


***Parameters:***

    -h - prints help text
    
    -m <model_path> - turns into single model mode and specifies model path (relative to base)
    
    -t <tobj_path> - turns into single tobj mode and specifies tobj path (relative to base)
    
    -d <dds_path> - turns into single dds mode and prints debug info (absolute path)

    -b <base_path> - specify base path

    -e <export_path> - specify export path

    -extract_f <file_name> - extract file (relative to base)

    -extract_d <dir_name> - extract directory (relative to base)

    -listdir <dir_name> - list of directories (relative to base)

    -listdir_r <dir_name> - recursive list of directories (relative to base)

    -show_f <file_name> - show file content (relative to base)

    -c - show clean result of "listdir", "listdir_r" and "show_f" without any comments


----------
***Usage:***

    converter_pix -b C:\ets2_base -m /vehicle/truck/man_tgx/interior/anim s_wheel
    
    # export into C:\ets2_base_exp single model with s_wheel animation.
    # instead of exact animation name you can use * to convert every anim file from model directory.
    # when anim name is not started by /, then converter is looking for it in model directory.

    converter_pix C:\ets2_base
    
    # will convert whole base, it will export it into: <base_path>_exp (C:\ets2_base_exp in this example).
    # you can also specify export path using the -e parameter.
    
    converter_pix -b C:\ets2_base -t /material/environment/vehicle_reflection.tobj
    # will convert tobj file and copy texture to export path.

    converter_pix -b C:\ets2_base -m /model/mover/characters/models/generic/m_afam_01 /model/mover/characters/animations/man/walk/walk_01
    # animations are located in another directory than the models. These animations can be used for multiple models.

    converter_pix -b C:\ets2_base -b C:\mod_base -m /vehicle/truck/man_tgx/interior/model
    # multiple base paths (you can specify more paths in the same way)
    
Note that animations will not be converted when converting the whole base.

This is caused by lack of information, so you have to convert each model individually to edit animations.

----------

**Supported formats: pmg(0x13, 0x14, 0x15), pmd(0x04), pma(0x03, 0x04), ppd(0x15, 0x16, 0x17), pmc, tobj, mat, scs(SCS#, CITYHASH), zip(uncompressed & compressed)**

----------

***Usage of extracting, list of directories and show file content:***

    converter_pix -b C:\ets2_base\def.scs -extract_f /def/city/berlin.sui

    # will extract file /def/city/berlin.sui into C:\ets2_base_exp.
    # you can also specify export path using the -e parameter.

    converter_pix -b C:\ets2_base\def.scs -extract_d /def/city

    # will extract directory /def/city recursive with all files into C:\ets2_base_exp.
    # you can also specify export path using the -e parameter.

    converter_pix -b C:\ets2_base\def.scs -listdir /def/vehicle
    # will show listing of all files and subdirectories of /def/vehicle.\n"

    converter_pix -b C:\ets2_base\def.scs -listdir_r /def/vehicle\n"
    # will show recursive listing of all files and subdirectories of /def/vehicle.\n"

    converter_pix -b C:\ets2_base\def.scs -show_f /def/city/berlin.sui\n"
    # will show content of /def/city/berlin.sui file.\n"

    converter_pix -c -b C:\ets2_base\def.scs -show_f /def/city/berlin.sui\n"
    # will show clean content of /def/city/berlin.sui file.\n"

----------

**This is Open-Source software under GNU LGPL License.**

--- Copyright © 2021 Michał Wójtowicz a.k.a. mwl4 ---

--- All rights reserved ---

**Enjoy**
