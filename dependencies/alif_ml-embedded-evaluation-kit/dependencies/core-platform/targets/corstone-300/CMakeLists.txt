#
# Copyright (c) 2020-2022 Arm Limited. All rights reserved.
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
# Default parameters
#############################################################################

set(TARGET_CPU "cortex-m55" CACHE INTERNAL "")

if (NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/toolchain/armclang.cmake")
endif()

set(ETHOSU_TARGET_NPU_CONFIG "ethos-u55-128" CACHE STRING "NPU configuration")
set(ETHOSU_TARGET_NPU_COUNT 1 CACHE INTERNAL "Number of NPUs")
set(ETHOSU_TARGET_NPU_TA_COUNT 2 CACHE INTERNAL "Number of timing adapters per NPU")

set(ETHOSU_PMU_EVENT_0 -1 CACHE STRING "PMU Event #0")
set(ETHOSU_PMU_EVENT_1 -1 CACHE STRING "PMU Event #1")
set(ETHOSU_PMU_EVENT_2 -1 CACHE STRING "PMU Event #2")
set(ETHOSU_PMU_EVENT_3 -1 CACHE STRING "PMU Event #3")

set(MEMORY_MODEL "dram" CACHE STRING "Memory config for model")
set(MEMORY_ARENA "dram" CACHE STRING "Memory config for arena")

#############################################################################
# Project
#############################################################################

cmake_minimum_required(VERSION 3.21)

project(ethos-u-corstone-300 VERSION 0.0.1)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/helpers.cmake)

#############################################################################
# Corstone-300
#############################################################################

get_filename_component(ETHOSU_TARGET ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message("Configuring target ${ETHOSU_TARGET}")

# Enable CTest
include(CTest)
set(Python3_FIND_STRATEGY LOCATION)
find_package(Python3 COMPONENTS Interpreter)
ethosu_get_architecture(${ETHOSU_TARGET_NPU_CONFIG})
set(ETHOSU_COMMAND_OPTIONS
        -t corstone-300
        -a ethos-${ETHOSU_ARCH}
        -m ${ETHOSU_NUM_MACS}
        CACHE INTERNAL "Default test command options")

if (ETHOSU_CTEST_TARMAC_TRACE)
    list(APPEND ETHOSU_COMMAND_OPTIONS "--tarmac")
endif()

set(ETHOSU_COMMAND_DEFAULT ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/run_ctest.py
        ${ETHOSU_COMMAND_OPTIONS}
    CACHE INTERNAL "Default test command")

# Enable trustzone support in core_software
set(TRUSTZONE_BUILD ON)

# Include common targets
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common target)

target_compile_definitions(ethosu_target_common INTERFACE
    # Confiugure NPU architecture and number of timing adapters
    ETHOSU_NPU_COUNT=${ETHOSU_TARGET_NPU_COUNT}
    ETHOSU_NPU_TA_COUNT=${ETHOSU_TARGET_NPU_TA_COUNT}

    # Configure PMU events
    ETHOSU_PMU_EVENT_0=${ETHOSU_PMU_EVENT_0}
    ETHOSU_PMU_EVENT_1=${ETHOSU_PMU_EVENT_1}
    ETHOSU_PMU_EVENT_2=${ETHOSU_PMU_EVENT_2}
    ETHOSU_PMU_EVENT_3=${ETHOSU_PMU_EVENT_3}

    # Placement or TLFu model and area. 0 = SRAM, 1 = DRAM
    ETHOSU_MODEL=$<STREQUAL:${MEMORY_MODEL},dram>
    ETHOSU_ARENA=$<STREQUAL:${MEMORY_ARENA},dram>)

# AXI Timing adaptors
set(registers MAXR MAXW MAXRW RLATENCY WLATENCY PULSE_ON PULSE_OFF BWCAP PERFCTRL PERFCNT MODE HISTBIN HISTCNT)

foreach(register ${registers})
    foreach(index RANGE 0 1)
        set(name ETHOSU_TA_${register}_${index})
        set(${name} -1 CACHE STRING "${name}")

        if (${name} GREATER_EQUAL 0)
            target_compile_definitions(ethosu_target_common INTERFACE ${name}=${${name}})
        endif()
    endforeach()
endforeach()

# Linker script
set(LINK_FILE platform CACHE STRING "Link file")

ethosu_target_link_options(ethosu_target_link INTERFACE
    LINK_FILE ${LINK_FILE}
    ENTRY Reset_Handler)

# Add drivers
target_sources(ethosu_target_startup INTERFACE
    retarget.c
    target.cpp)

target_link_libraries(ethosu_target_startup INTERFACE
    $<$<TARGET_EXISTS:ethosu_core_driver>:ethosu_core_driver;timing_adapter>
    mpu
    ethosu_mhu_dummy
    ethosu_uart_cmsdk_apb)

if (TARGET ethosu_core_driver)
    target_compile_definitions(ethosu_core_driver PUBLIC
        ETHOSU)
endif()

if (TARGET event_recorder)
    target_include_directories(event_recorder BEFORE INTERFACE
        event_recorder)
endif()

###############################################################################
# Applications
###############################################################################

# Add all applications
add_subdirectory(../../applications applications)
