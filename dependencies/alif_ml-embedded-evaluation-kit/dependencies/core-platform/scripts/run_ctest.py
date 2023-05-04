#!/usr/bin/env python3

#
# Copyright (c) 2021-2022 Arm Limited. All rights reserved.
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
import subprocess
import sys
import os
from pathlib import Path

def __print_arguments(args):
    if isinstance(args, list):
        print("$ " + " ".join(args))
    else:
        print(args)

def Popen(args, **kwargs):
    __print_arguments(args)
    return subprocess.Popen(args, **kwargs)

def call(args, **kwargs):
    __print_arguments(args)
    return subprocess.call(args, **kwargs)

def check_call(args, **kwargs):
    __print_arguments(args)
    return subprocess.check_call(args, **kwargs)

def check_output(args, **kwargs):
    __print_arguments(args)
    return subprocess.check_output(args, **kwargs)

def run_fvp(cmd):
    # Run FVP and tee output to console while scanning for exit tag
    ret = 1
    proc = Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    while True:
        line = proc.stdout.readline().decode()
        if not line:
                break

        if 'Application exit code: 0.' in line:
            ret = 0

        sys.stdout.write(line)
        sys.stdout.flush()

    return ret

def run_corstone_300(args):
    if not args.arch or args.arch == 'ethos-u55':
        fvp = 'FVP_Corstone_SSE-300_Ethos-U55'
    elif args.arch == 'ethos-u65':
        fvp = 'FVP_Corstone_SSE-300_Ethos-U65'
    else:
        raise 'Unsupported NPU arch'

    # Verify supported FVP version
    version = subprocess.check_output([fvp, '--version']).decode()
    supported_version = ['11.13', '11.14', '11.15', '11.16']

    if not [s for s in supported_version if s in version]:
        raise Exception("Incorrect FVP version. Supported versions are '{}'.".format(supported_version))

    # FVP executable
    cmd = [fvp]

    # NPU configuration
    cmd += ['-C', 'ethosu.num_macs=' + str(args.macs)]

    # Output parameters
    cmd += ['-C', 'mps3_board.visualisation.disable-visualisation=1',
            '-C', 'mps3_board.telnetterminal0.start_telnet=0',
            '-C', 'mps3_board.uart0.out_file="-"',
            '-C', 'mps3_board.uart0.unbuffered_output=1',
            '-C', 'mps3_board.uart0.shutdown_on_eot=1']

    if args.tarmac:
        try:
            pvlib_home = Path(os.getenv('PVLIB_HOME'))
        except:
            raise Exception("Environment variable PVLIB_HOME not found. Needed to produce tarmac trace.")

        if sys.platform == 'linux':
            tarmac_trace_plugin = pvlib_home / Path('plugins/Linux64_GCC-7.3/TarmacTrace.so')
        else:
            raise Exception("tarmac trace: This feature is not currently supported on" + sys.platform)

        if tarmac_trace_plugin.exists():
            print("Tarmac trace will be created");
            basename = [ e for e in args.args if e.endswith('.elf')][0][:-4]
            cmd += ['--plugin', str(tarmac_trace_plugin)]
            cmd += ['-C', f'TRACE.TarmacTrace.trace-file={basename}.trace']
        else:
            raise Exception("tarmac trace: Can't find TarmacTrace plugin in " + pvlib_home)

    cmd += args.args

    return run_fvp(cmd)

def run_corstone_310(args):
    # Verify supported FVP version
    version = subprocess.check_output(['FVP_Corstone_SSE-310', '--version']).decode()
    supported_version = ['11.17']

    if not [s for s in supported_version if s in version]:
        raise Exception("Incorrect FVP version. Supported versions are '{}'.".format(supported_version))

    # FVP executable
    cmd = ['FVP_Corstone_SSE-310']

    # NPU configuration
    cmd += ['-C', 'ethosu.num_macs=' + str(args.macs)]

    # Output parameters
    cmd += ['-C', 'mps3_board.visualisation.disable-visualisation=1',
            '-C', 'mps3_board.telnetterminal0.start_telnet=0',
            '-C', 'mps3_board.uart0.out_file="-"',
            '-C', 'mps3_board.uart0.unbuffered_output=1',
            '-C', 'mps3_board.uart0.shutdown_on_eot=1']

    cmd += args.args

    return run_fvp(cmd)

def run_corstone_polaris(args):
    # Verify supported FVP version
    version = subprocess.check_output(['FVP_Corstone-Polaris', '--version']).decode()
    supported_version = ['11.16']

    if not [s for s in supported_version if s in version]:
        raise Exception("Incorrect FVP version. Supported versions are '{}'.".format(supported_version))

    # FVP executable
    cmd = ['FVP_Corstone-Polaris']

    # NPU configuration
    cmd += ['-C', 'ethosu.num_macs=' + str(args.macs)]

    # 32kB ITCM, 32kB DTCM, 2MB SRAM
    cmd += ['-C', 'cpu0.CFGITCMSZ=6',
            '-C', 'cpu0.CFGDTCMSZ=6',
            '-C', 'mps3_board.sse300.NUMVMBANK=1',
            '-C', 'mps3_board.sse300.VM_BANK_SIZE=2048']

    # Output parameters
    cmd += ['-C', 'mps3_board.visualisation.disable-visualisation=1',
            '-C', 'mps3_board.telnetterminal0.start_telnet=0',
            '-C', 'mps3_board.uart0.out_file="-"',
            '-C', 'mps3_board.uart0.unbuffered_output=1',
            '-C', 'mps3_board.uart0.shutdown_on_eot=1']

    cmd += args.args

    return run_fvp(cmd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Run a test with given test command and test binary.')
    parser.add_argument('-t', '--target', choices=['corstone-300', 'corstone-310', 'corstone-polaris'], required=True, help='FVP target.')
    parser.add_argument('-a', '--arch', choices=['ethos-u55', 'ethos-u65'], help='NPU architecture.')
    parser.add_argument('-m', '--macs', type=int, choices=[32, 64, 128, 256, 512], default=128, help='NPU number of MACs.')
    parser.add_argument('-c', '--tarmac', dest='tarmac', action='store_true', help='Collect tarmac traces when running FVP.')

    parser.add_argument('args', nargs='+', help='Arguments.')
    args = parser.parse_args()

    if args.target == 'corstone-300':
        sys.exit(run_corstone_300(args))
    elif args.target == 'corstone-310':
        sys.exit(run_corstone_310(args))
    elif args.target == 'corstone-polaris':
        sys.exit(run_corstone_polaris(args))
