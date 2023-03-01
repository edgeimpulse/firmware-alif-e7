#
# Copyright (c) 2022 Arm Limited.
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

#############################################################################
# Paths
#############################################################################

set(CORE_PLATFORM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/<TODO>/core_platform" CACHE PATH "Path to Core Platform")

#############################################################################
# Default toolchain
#############################################################################

set(TARGET_CPU "<TODO e.g. cortex-m55>" CACHE STRING "Target CPU")

if (NOT CMAKE_TOOLCHAIN_FILE)
    # TODO Select default toolchain
    #   - Arm Clang: ${CORE_PLATFORM_PATH}/cmake/toolchain/armclang.cmake
    #   - GCC      : ${CORE_PLATFORM_PATH}/cmake/toolchain/arm-none-eabi-gcc.cmake
    #   - Else     : Implement your own toolchain file
    set(CMAKE_TOOLCHAIN_FILE "${CORE_PLATFORM_PATH}/cmake/toolchain/armclang.cmake")
endif()

#############################################################################
# Default configuration
#############################################################################

get_filename_component(ETHOSU_TARGET ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message("Configuring target ${ETHOSU_TARGET}")

set(ETHOSU_TARGET_NPU_CONFIG "<TODO> e.g. ethos-u65-256" CACHE STRING "NPU configuration")
set(ETHOSU_TARGET_NPU_COUNT 1 CACHE INTERNAL "Number of NPUs")
set(ETHOSU_TARGET_NPU_TA_COUNT 0 CACHE INTERNAL "Number of timing adapters per NPU")

# Fast memory size
# If the TFLM model and arena are placed in Flash/DRAM, and if the NPU is Ethos-U65,
# then a smaller fast memory buffer can be placed in SRAM. This is called 'spilling'.
set(FAST_MEMORY_SIZE 0 CACHE STRING "Size of relocated fast memory scratch tensor")
set(MEMORY_MODEL "sram" CACHE STRING "Memory config for model")
set(MEMORY_ARENA "sram" CACHE STRING "Memory config for arena")

# UART settings
set(UART0_BASE "<TODO>" CACHE INTERNAL "UART base address")
set(UART0_BAUDRATE "<TODO e.g. 115200>" CACHE INTERNAL "UART baudrate, N/A for model and juno")
set(SYSTEM_CORE_CLOCK "<TODO e.g. 25000000>" CACHE INTERNAL "System core clock (Hz)")

#############################################################################
# Project
#############################################################################

cmake_minimum_required(VERSION 3.21)

project(ethos-u-demo VERSION 0.0.1)

include(${CORE_PLATFORM_PATH}/cmake/helpers.cmake)

#############################################################################
# Target
#############################################################################

# Include common target
add_subdirectory(${CORE_PLATFORM_PATH}/targets/common core_platform/target/common)

# Include drivers
add_subdirectory(../../drivers drivers)

# Common defines
target_compile_definitions(ethosu_target_common INTERFACE
    # Configure NPU architecture and number of timing adapters
    ETHOSU_NPU_COUNT=${ETHOSU_TARGET_NPU_COUNT}
    ETHOSU_NPU_TA_COUNT=${ETHOSU_TARGET_NPU_TA_COUNT}

    # Placement or TLFu model and area. 0 = SRAM, 1 = DRAM
    # The scatter file and linker script must be designed to switch on these defines
    ETHOSU_FAST_MEMORY_SIZE=${FAST_MEMORY_SIZE}
    ETHOSU_MODEL=$<STREQUAL:${MEMORY_MODEL},dram>
    ETHOSU_ARENA=$<STREQUAL:${MEMORY_ARENA},dram>)

# Linker script
set(LINK_FILE platform CACHE STRING "Link file")

ethosu_target_link_options(ethosu_target_link INTERFACE
    LINK_FILE ${LINK_FILE}
    ENTRY Reset_Handler)

target_sources(ethosu_target_startup INTERFACE
    retarget.c
    target.cpp)

target_link_libraries(ethosu_target_startup INTERFACE
    $<$<TARGET_EXISTS:ethosu_core_driver>:ethosu_core_driver>
    # TODO customize which libraries to include
    mpu
    ethosu_mhu_dummy
    ethosu_uart_cmsdk_apb)

if (TARGET ethosu_core_driver)
    target_compile_definitions(ethosu_core_driver PUBLIC
        ETHOSU

        # The TFLM arena is accessed over base address 1. The region config
        # controls if the memory transactions are routed over AXI 0 (region config
        # 0 or 1) or AXI 1 (region config 2 or 3).
        NPU_REGIONCFG_1=$<if:$<STREQUAL:${MEMORY_ARENA},dram>,0,2>)
endif()

###############################################################################
# CTest
###############################################################################

# TODO Uncomment to enable ctest
# include(CTest)

# Uncomment if Python interpreter is needed
#set(Python3_FIND_STRATEGY LOCATION)
#find_package(Python3 COMPONENTS Interpreter)

# TODO Uncomment if ETHOSU_ARCH and ETHOSU_NUM_MACS are needed
# ethosu_get_architecture(${ETHOSU_TARGET_NPU_CONFIG})

set(ETHOSU_COMMAND_DEFAULT <TODO test command> CACHE INTERNAL "Default test command")

###############################################################################
# Applications
###############################################################################

add_subdirectory(${CORE_PLATFORM_PATH}/applications core_platform/applications)
add_subdirectory(../../applications applications)
