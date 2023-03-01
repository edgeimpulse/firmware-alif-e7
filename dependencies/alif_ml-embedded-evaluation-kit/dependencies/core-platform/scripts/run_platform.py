#!/usr/bin/env python3

#
# Copyright (c) 2021-2022 Arm Limited.
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
import multiprocessing
import numpy
import os
import pathlib
import re
import shutil
import subprocess
import sys

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
from tensorflow.lite.python.interpreter import Interpreter, OpResolverType

CORE_PLATFORM_PATH = pathlib.Path(__file__).resolve().parents[1]

def run_cmd(cmd, **kwargs):
    # str() is called to handle pathlib.Path objects
    cmd_str = " ".join([str(arg) for arg in cmd])
    print(f"Running command: {cmd_str}")
    return subprocess.run(cmd, check=True, **kwargs)

def ta_parse_raw(ta_raw):
    ta_parsed = [-1, -1]
    if ta_raw:
        for v in ta_raw:
            index = v[0]
            value = v[1]
            if (index > 1):
                raise Exception("Illegal index value - Should be '0' or '1'")
            (ta_parsed)[index] = value

    return ta_parsed

def build_core_platform(output_folder, target, toolchain, memory_model, memory_arena, pmu,
                        ta_maxr, ta_maxw, ta_maxrw, ta_rlatency, ta_wlatency,
                        ta_pulse_on, ta_pulse_off, ta_bwcap, ta_perfctrl, ta_perfcnt,
                        ta_mode, ta_histbin, ta_histcnt):
    build_folder = output_folder/"model"/"build"
    maxr = ta_parse_raw(ta_maxr)
    maxw = ta_parse_raw(ta_maxw)
    maxrw = ta_parse_raw(ta_maxrw)
    rlatency = ta_parse_raw(ta_rlatency)
    wlatency = ta_parse_raw(ta_wlatency)
    pulse_on = ta_parse_raw(ta_pulse_on)
    pulse_off = ta_parse_raw(ta_pulse_off)
    bwcap = ta_parse_raw(ta_bwcap)
    perfctrl = ta_parse_raw(ta_perfctrl)
    perfcnt = ta_parse_raw(ta_perfcnt)
    mode = ta_parse_raw(ta_mode)
    histbin = ta_parse_raw(ta_histbin)
    histcnt = ta_parse_raw(ta_histcnt)
    cmake_cmd = ["cmake",
                 CORE_PLATFORM_PATH/"targets"/target,
                 f"-B{build_folder}",
                 f"-DCMAKE_TOOLCHAIN_FILE={CORE_PLATFORM_PATH/'cmake'/'toolchain'/(toolchain + '.cmake')}",
                 f"-DBAREMETAL_PATH={output_folder}",
                 f"-DMEMORY_MODEL={memory_model}",
                 f"-DMEMORY_ARENA={memory_arena}",
                 f"-DETHOSU_TA_MAXR_0={maxr[0]}",
                 f"-DETHOSU_TA_MAXR_1={maxr[1]}",
                 f"-DETHOSU_TA_MAXW_0={maxw[0]}",
                 f"-DETHOSU_TA_MAXW_1={maxw[1]}",
                 f"-DETHOSU_TA_MAXRW_0={maxrw[0]}",
                 f"-DETHOSU_TA_MAXRW_1={maxrw[1]}",
                 f"-DETHOSU_TA_RLATENCY_0={rlatency[0]}",
                 f"-DETHOSU_TA_RLATENCY_1={rlatency[1]}",
                 f"-DETHOSU_TA_WLATENCY_0={wlatency[0]}",
                 f"-DETHOSU_TA_WLATENCY_1={wlatency[1]}",
                 f"-DETHOSU_TA_PULSE_ON_0={pulse_on[0]}",
                 f"-DETHOSU_TA_PULSE_ON_1={pulse_on[1]}",
                 f"-DETHOSU_TA_PULSE_OFF_0={pulse_off[0]}",
                 f"-DETHOSU_TA_PULSE_OFF_1={pulse_off[1]}",
                 f"-DETHOSU_TA_BWCAP_0={bwcap[0]}",
                 f"-DETHOSU_TA_BWCAP_1={bwcap[1]}",
                 f"-DETHOSU_TA_PERFCTRL_0={perfctrl[0]}",
                 f"-DETHOSU_TA_PERFCTRL_1={perfctrl[1]}",
                 f"-DETHOSU_TA_PERFCNT_0={perfcnt[0]}",
                 f"-DETHOSU_TA_PERFCNT_1={perfcnt[1]}",
                 f"-DETHOSU_TA_MODE_0={mode[0]}",
                 f"-DETHOSU_TA_MODE_1={mode[1]}",
                 f"-DETHOSU_TA_HISTBIN_0={histbin[0]}",
                 f"-DETHOSU_TA_HISTBIN_1={histbin[1]}",
                 f"-DETHOSU_TA_HISTCNT_0={histcnt[0]}",
                 f"-DETHOSU_TA_HISTCNT_1={histcnt[1]}"]

    if pmu:
        for i in range(len(pmu)):
            cmake_cmd += [f"-DETHOSU_PMU_EVENT_{i}={pmu[i]}"]
    run_cmd(cmake_cmd)

    make_cmd = ["make", "-C", build_folder, f"-j{multiprocessing.cpu_count()}", "baremetal_custom"]
    run_cmd(make_cmd)

def generate_reference_data(output_folder, non_optimized_model_path, input_path, expected_output_path):
    interpreter = Interpreter(model_path=str(non_optimized_model_path.resolve()), experimental_op_resolver_type=OpResolverType.BUILTIN_REF)

    interpreter.allocate_tensors()
    input_detail  = interpreter.get_input_details()[0]
    output_detail = interpreter.get_output_details()[0]

    input_data = None
    if input_path is None:
        # Randomly generate input data
        dtype = input_detail["dtype"]
        if dtype is numpy.float32:
            rand = numpy.random.default_rng()
            input_data = rand.random(size=input_detail["shape"], dtype=numpy.float32)
        else:
            input_data = numpy.random.randint(low=numpy.iinfo(dtype).min, high=numpy.iinfo(dtype).max, size=input_detail["shape"], dtype=dtype)
    else:
        # Load user provided input data
        input_data = numpy.load(input_path)

    output_data = None
    if expected_output_path is None:
        # Run the network with input_data to get reference output
        interpreter.set_tensor(input_detail["index"], input_data)
        interpreter.invoke()
        output_data = interpreter.get_tensor(output_detail["index"])
    else:
        # Load user provided output data
        output_data = numpy.load(expected_output_path)

    network_input_path  = output_folder/"ref_input.bin"
    network_output_path = output_folder/"ref_output.bin"

    with network_input_path.open("wb") as fp:
        fp.write(input_data.tobytes())
    with network_output_path.open("wb") as fp:
        fp.write(output_data.tobytes())

    output_folder = pathlib.Path(output_folder)
    dump_c_header(network_input_path, output_folder/"input.h", "inputData", "input_data_sec", 4)
    dump_c_header(network_output_path, output_folder/"output.h", "expectedOutputData", "expected_output_data_sec", 4)

def dump_c_header(input_path, output_path, array_name, section, alignment, extra_data=""):
    byte_array = []
    with open(input_path, "rb") as fp:
        byte_string = fp.read()
        byte_array = [f"0x{format(byte, '02x')}" for byte in byte_string]

        last = byte_array[-1]
        byte_array = [byte + "," for byte in byte_array[:-1]] + [last]

        byte_array = ["  " + byte if idx % 12 == 0 else byte
                      for idx, byte in enumerate(byte_array)]

        byte_array = [byte + "\n" if (idx + 1) % 12 == 0 else byte + " "
                      for idx, byte in enumerate(byte_array)]

    with open(output_path, "w") as carray:
        header = f"uint8_t {array_name}[] __attribute__((section(\"{section}\"), aligned({alignment}))) = {{\n"
        carray.write(extra_data)
        carray.write(header)
        carray.write("".join(byte_array))
        carray.write("\n};\n")

def optimize_network(output_folder, network_path, accelerator_conf):
    vela_cmd  = ["vela",
                 network_path,
                 "--output-dir", output_folder,
                 "--accelerator-config", accelerator_conf]
    res = run_cmd(vela_cmd)
    optimized_model_path = output_folder/(network_path.stem + "_vela.tflite")
    model_name = network_path.stem
    dump_c_header(optimized_model_path, output_folder/"model.h", "networkModelData", "network_model_sec", 16, extra_data=f"""
#include <stddef.h>

const size_t tensorArenaSize = 2000000;
const char* modelName = \"{model_name}\";
""")

def run_model(output_folder):
    build_folder = output_folder/"model"/"build"
    model_cmd = ["ctest", "-V", "-R", "^baremetal_custom$" ]
    res = run_cmd(model_cmd, cwd=build_folder)

def main():
    target_mapping = {
        "corstone-300": "ethos-u55-128"
    }
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output-folder", type=pathlib.Path, default="output", help="Output folder for build and generated files")
    parser.add_argument("--network-path", type=pathlib.Path, required=True, help="Path to .tflite file")
    parser.add_argument("--target", choices=target_mapping, default="corstone-300", help=f"Configure target")
    parser.add_argument("--toolchain", choices=["armclang", "arm-none-eabi-gcc"], default="armclang", help=f"Configure toolchain")
    parser.add_argument("--memory_model", choices=["sram", "dram"], default="dram", help=f"Configure memory_model")
    parser.add_argument("--memory_arena", choices=["sram", "dram"], default="sram", help=f"Configure memory_arena")
    parser.add_argument("--pmu", type=int, action='append', help="PMU Event Counters")
    parser.add_argument("--custom-input", type=pathlib.Path, help="Custom input to network")
    parser.add_argument("--custom-output", type=pathlib.Path, help="Custom expected output data for network")
    parser.add_argument("--ta-maxr", type=int, nargs=2, action='append', help="Max no. of pending reads")
    parser.add_argument("--ta-maxw", type=int, nargs=2, action='append', help="Max no. of pending writes")
    parser.add_argument("--ta-maxrw", type=int, nargs=2, action='append', help="Max no. of pending reads+writes")
    parser.add_argument("--ta-rlatency", type=int, nargs=2, action='append', help="Minimum latency (clock cycles) from AVALID to RVALID")
    parser.add_argument("--ta-wlatency", type=int, nargs=2, action='append', help="Minimum latency (clock cycles) from WVALID&WLAST to BVALID")
    parser.add_argument("--ta-pulse_on", type=int, nargs=2, action='append', help="No. of cycles addresses let through (0-65535)")
    parser.add_argument("--ta-pulse_off", type=int, nargs=2, action='append', help="No. of cycles addresses blocked (0-65535)")
    parser.add_argument("--ta-bwcap", type=int, nargs=2, action='append', help="Max no. of 64-bit words transfered per pulse cycle 0=infinite")
    parser.add_argument("--ta-perfctrl", type=int, nargs=2, action='append', help="selecting an event for event counter 0=default")
    parser.add_argument("--ta-perfcnt", type=int, nargs=2, action='append', help="event counter")
    parser.add_argument("--ta-mode", type=int, nargs=2, action='append', help="Max no. of pending reads")
    parser.add_argument("--ta-histbin", type=int, nargs=2, action='append', help="Controlls which histogram bin (0-15) that should be accessed by HISTCNT")
    parser.add_argument("--ta-histcnt", type=int, nargs=2, action='append', help="Read/write the selected histogram bin")

    args = parser.parse_args()
    args.output_folder.mkdir(exist_ok=True)

    try:
        optimize_network(args.output_folder, args.network_path, target_mapping[args.target])
        generate_reference_data(args.output_folder, args.network_path, args.custom_input, args.custom_output)
        build_core_platform(args.output_folder, args.target, args.toolchain, args.memory_model, args.memory_arena, args.pmu,
                            args.ta_maxr, args.ta_maxw, args.ta_maxrw, args.ta_rlatency, args.ta_wlatency,
                            args.ta_pulse_on, args.ta_pulse_off, args.ta_bwcap, args.ta_perfctrl, args.ta_perfcnt,
                            args.ta_mode, args.ta_histbin, args.ta_histcnt)
        run_model(args.output_folder)
    except subprocess.CalledProcessError as err:
        print(f"Command: '{err.cmd}' failed", file=sys.stderr)
        return 1
    return 0

if __name__ == "__main__":
    sys.exit(main())
