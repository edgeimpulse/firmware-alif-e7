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

# Exclude this app if ThreadX target does not exist  or if NPU configuration is not H128
if (NOT TARGET threadx OR NOT TARGET ethosu_core_driver OR NOT ETHOSU_TARGET_NPU_CONFIG STREQUAL "ethos-u55-128")
    message("Skipping ThreadX application")
    return()
endif()

ethosu_add_executable_test(threadx_demo PRIVATE
    SOURCES main.cpp
    SOURCES tx_initialize_low_level.c
    LIBRARIES threadx)
target_include_directories(threadx_demo PRIVATE models/mobilenet_v2)

# Use the fast memory to support both 11.12.57 and 11.13.x release
# of the FVP
# The size is defined to force the .bss.tensor_arena to be placed in DDR
target_compile_options(threadx_demo PRIVATE
    -UETHOSU_FAST_MEMORY_SIZE -DETHOSU_FAST_MEMORY_SIZE=0xa0000
    -UETHOSU_ARENA -DETHOSU_ARENA=1
    -UETHOSU_MODEL -DETHOSU_MODEL=1)
