##############################################################################
#		  _____                          _            _____ _______   __
#		 / ____|                        | |          |  __ \_   _\ \ / /
#		| |     ___  _ ____   _____ _ __| |_ ___ _ __| |__) || |  \ V /
#		| |    / _ \| '_ \ \ / / _ \ '__| __/ _ \ '__|  ___/ | |   > <
#		| |___| (_) | | | \ V /  __/ |  | ||  __/ |  | |    _| |_ / . \
#		 \_____\___/|_| |_|\_/ \___|_|   \__\___|_|  |_|   |_____/_/ \_\
#
#  Copyright (C) 2024 Michal Wojtowicz.
#  All rights reserved.
#
#   This software is ditributed WITHOUT ANY WARRANTY; without even
#   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#   PURPOSE. See the copyright file for more information.
#
##############################################################################

import subprocess
import hashlib
import os
import shutil
import logging

scs_packer = 'scs_packer'
converter_pix = os.path.abspath('../../bin/win_x86/converter_pix_d')

logger = logging.getLogger(__name__)
logging.basicConfig(filename='test.log.txt', encoding='utf-8', level=logging.DEBUG, filemode='w')

def md5sum(filename, blocksize=65536):
    hash = hashlib.md5()
    with open(filename, "rb") as f:
        for block in iter(lambda: f.read(blocksize), b""):
            hash.update(block)
    return hash.hexdigest()


def run_command(args):
    logger.debug(f'{" ".join(args)}')
    subprocess.run(args, check=True)

archive_number = 0
tests_count = 0
tests_count_passed = 0

def run_test(files):
    global tests_count
    tests_count += 1

    global archive_number
    data_archive_dir = "data_archive_" + str(archive_number)
    archive_number += 1

    if os.path.exists(data_archive_dir):
        shutil.rmtree(data_archive_dir)
    os.makedirs(data_archive_dir, 0o777, True)
    
    for file in files:
        shutil.copy2('../data/' + file, data_archive_dir)

    data_archive_scs = data_archive_dir + '.scs'

    run_command(['scs_packer', 'create', os.path.abspath(data_archive_scs), '-root', os.path.abspath(data_archive_dir)])

    data_archive_extracted_dir = data_archive_dir + "_extracted"

    if os.path.exists(data_archive_extracted_dir):
        shutil.rmtree(data_archive_extracted_dir)
    os.makedirs(data_archive_extracted_dir, 0o777, True)

    run_command([converter_pix, '-b', os.path.abspath(data_archive_scs), '-extract_d', '/', '-e', os.path.abspath(data_archive_extracted_dir)])

    data_archive_extracted_scs = data_archive_dir + '_extracted' + '.scs'

    run_command(['scs_packer', 'create', os.path.abspath(data_archive_extracted_scs), '-root', os.path.abspath(data_archive_extracted_dir)])

    if md5sum(data_archive_scs) != md5sum(data_archive_extracted_scs):
        print(f'ERROR: archives are different {data_archive_scs} != {data_archive_extracted_scs} {files}');
        return

    global tests_count_passed
    tests_count_passed += 1


os.makedirs("temp", 0o777, True)
os.chdir("temp")

run_test(['b8g8r8_nomips_3_3_2_2_2_0.tobj', 'b8g8r8_nomips.dds'])
run_test(['b8g8r8_mips_3_3_2_2_2_0.tobj', 'b8g8r8_mips.dds'])
run_test(['cubemap_dxt1_nomips.tobj', 'cubemap_dxt1_nomips_b.dds', 'cubemap_dxt1_nomips_s.dds'])
run_test(['dxt5_mips.tobj', 'dxt5_mips.dds'])

print(f'tests passed: {tests_count_passed} / {tests_count}')

#eof#