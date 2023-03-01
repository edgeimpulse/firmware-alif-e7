#!/usr/bin/env python3

#
# Copyright (c) 2021 Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the License); you may
# not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import os
import re
import subprocess
import sys
import elftools.elf.elffile as elffile

def generate_binaries(args):
    outfiles = []

    with open(args.input, 'rb') as f:
        elf = elffile.ELFFile(f)
        for segment in elf.iter_segments():
            if segment['p_type'] == 'PT_LOAD' and segment['p_filesz'] > 0:
                os.makedirs(args.output, exist_ok=True)
                out = os.path.join(
                        args.output,
                        '%s_0x%08x.bin' % (os.path.basename(args.input), segment['p_paddr']))
                with open(out, 'wb') as of:
                    of.write(segment.data())

                outfiles.append(out)

    if args.d:
        with open(args.d, 'w') as f:
            f.writelines(outfiles)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate binaries from ELF file')
    parser.add_argument('-o', '--output', default='.', required=False, help='Output directory for binaries')
    parser.add_argument('-d', help='Dependency file')
    parser.add_argument('input', metavar='inputfile', type=str, help='ELF file to extract binaries from')

    args = parser.parse_args()

    generate_binaries(args)
