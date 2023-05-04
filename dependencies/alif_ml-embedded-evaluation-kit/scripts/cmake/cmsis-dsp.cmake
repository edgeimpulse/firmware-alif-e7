# This file was ported to work on Alif Semiconductor Ensemble family of devices.

#  Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
#  Use, distribution and modification of this code is permitted under the
#  terms stated in the Alif Semiconductor Software License Agreement
#
#  You should have received a copy of the Alif Semiconductor Software
#  License Agreement with this file. If not, please write to:
#  contact@alifsemi.com, or visit: https://alifsemi.com/license

#----------------------------------------------------------------------------
#  SPDX-FileCopyrightText: Copyright 2021 Arm Limited and/or its affiliates <open-source-office@arm.com>
#  SPDX-License-Identifier: Apache-2.0
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#----------------------------------------------------------------------------

# CMSIS-DSP library CMake helper script.

# Check if CMSIS sources have been defined
if (NOT DEFINED CMSIS_DSP_SRC_PATH)
    message(FATAL_ERROR "CMSIS-DSP path should be defined for CMSIS-DSP library to be built")
endif()
if (NOT DEFINED CMSIS_SRC_PATH)
    message(FATAL_ERROR "CMSIS-5 path should be defined to include CMSIS-CORE")
endif()

# 3. Form a list of all the sources we need in CSMS-DSP library
set(CMSIS_DSP_SRC_DIR       "${CMSIS_DSP_SRC_PATH}/Source")
set(CMSIS_DSP_INC_DIR       "${CMSIS_DSP_SRC_PATH}/Include")
set(CMSIS_DSP_PRI_INC_DIR   "${CMSIS_DSP_SRC_PATH}/PrivateInclude")
set(CMSIS_CORE_INC_DIR      "${CMSIS_SRC_PATH}/CMSIS/Core/Include")

file(GLOB_RECURSE
    CMSIS_DSP_SRC

    "${CMSIS_DSP_SRC_DIR}/BasicMathFunctions/arm_*.c"
    "${CMSIS_DSP_SRC_DIR}/FastMathFunctions/arm_*.c"
    "${CMSIS_DSP_SRC_DIR}/CommonTables/arm_*.c"
    "${CMSIS_DSP_SRC_DIR}/TransformFunctions/arm_*.c"
    "${CMSIS_DSP_SRC_DIR}/StatisticsFunctions/arm_*.c"

    # Issue with q15 and q31 functions with Arm GNU toolchain, we only
    # need f32 functions.
    "${CMSIS_DSP_SRC_DIR}/ComplexMathFunctions/arm_*f32.c")

# 4. Create static library
set(CMSIS_DSP_TARGET        cmsis-dsp)

add_library(${CMSIS_DSP_TARGET} STATIC ${CMSIS_DSP_SRC})

target_include_directories(${CMSIS_DSP_TARGET} PUBLIC
                           ${CMSIS_DSP_INC_DIR}
                           ${CMSIS_CORE_INC_DIR})
target_include_directories(${CMSIS_DSP_TARGET} PRIVATE
                           ${CMSIS_DSP_PRI_INC_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${CMSIS_DSP_TARGET} PUBLIC -flax-vector-conversions)

    # There is a known issue with -O0 optimisation option that affects
    # FFT functions from CMSIS-DSP when compiling with Arm GNU embedded
    # toolchain version 10.2.1 or 10.3-2021.07
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        message(WARNING "There are known issues with CMSIS-DSP builds using "
                        "MVE extension without optimisation. Forcing -O3 "
                        "optimisation level")
        target_compile_options(${CMSIS_DSP_TARGET} PUBLIC -O3)
    endif()
endif ()

# 5. General compile definitions
target_compile_definitions(${CMSIS_DSP_TARGET} PUBLIC
    ARM_MATH_LOOPUNROLL
    ARM_DSP_CONFIG_TABLES
    ARM_FFT_ALLOW_TABLES
    ARM_TABLE_TWIDDLECOEF_F32_256
    ARM_TABLE_TWIDDLECOEF_F32_512
    ARM_TABLE_TWIDDLECOEF_F32_1024
    ARM_TABLE_TWIDDLECOEF_RFFT_F32_512
    ARM_TABLE_TWIDDLECOEF_RFFT_F32_1024
    ARM_TABLE_BITREVIDX_FXT_256
    ARM_TABLE_BITREVIDX_FLT_256
    ARM_TABLE_BITREVIDX_FXT_512
    ARM_TABLE_BITREVIDX_FLT_512 # see https://github.com/ARM-software/CMSIS-DSP/issues/61
    ARM_TABLE_BITREVIDX_FXT_1024
    ARM_TABLE_BITREVIDX_FLT_1024
    ARM_FAST_ALLOW_TABLES
    ARM_ALL_FAST_TABLES
)

# 6. Provide the library path for the top level CMake to use:
set(CMSIS_DSP_LIB   "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${CMSIS_DSP_TARGET}.a")
message(STATUS "CMSIS_DSP_LIB set to be generated here: ${CMSIS_DSP_LIB}")

message(STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "*******************************************************")
message(STATUS "Library                                : " ${CMSIS_DSP_TARGET})
message(STATUS "Build type                             : " ${CMAKE_BUILD_TYPE})
message(STATUS "TARGET_PLATFORM                        : " ${TARGET_PLATFORM})
message(STATUS "CMAKE_SYSTEM_PROCESSOR                 : " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "*******************************************************")
