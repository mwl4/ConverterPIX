# ConverterPIX

Tool for conversion from game formats to middle formats (Euro Truck Simulator 2 and American Truck Simulator)

 Parameters:

  -h                   - prints help text  
  -m <model_path>      - turns into single model mode and specifies model path (relative to base)  
  -t <tobj_path>       - turns into single tobj mode and specifies tobj path (relative to base)  
  -d <dds_path>        - turns into single dds mode and prints debug info (absolute path)  
  -b <base_path>       - specify base path  
  -e <export_path>     - specify export path  


 Usage:  
  converter_pix -b C:\\ets2_base -m /vehicle/truck/man_tgx/interior/anim s_wheel  
    ^ will export into C:\\ets2_base_exp single model with s_wheel animation.  
    ^ instead of exact animation name you can use * to convert every anim file from model directory.  
    ^ when anim name is not started by /, then converter is looking for it in model directory.

  converter_pix C:\\ets2_base  
    ^ will convert whole base, it will export it into: <base_path>_exp (C:\\ets2_base_exp in this example).  
    ^ you can also specify export path using the -e parameter.

  converter_pix -b C:\\ets2_base -t /material/environment/vehicle_reflection.tobj  
    ^ will convert tobj file and copy texture to export path.

  converter_pix -b C:\\ets2_base -m /model/mover/characters/models/generic/m_afam_01 /model/mover/characters/animations/man/walk/walk_01  
    ^ animations are located in another directory than the models. These animations can be used for multiple models.

  converter_pix -b C:\\ets2_base -b C:\\mod_base -m /vehicle/truck/man_tgx/interior/model  
    ^ multiple base paths (you can specify more paths in the same way)

 Note that animations will not be converted when converting the whole base.

 This is caused by lack of information, so you have to convert each model individually to edit animations.

 Supported formats: pmg(0x13, 0x14), pmd(0x04), pma(0x03), ppd(0x15), pmc, tobj, mat

 This is Open-Source software under the GNU LGPL License.

 --- Copyright (c) 2017 mwl4 ---  
 ---   All rights reserved   ---

 Enjoy