#----------------------------------------------------------------------------
#  Copyright (c) 2021 Arm Limited. All rights reserved.
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
# specify the cross compiler
set(CMAKE_C_COMPILER                armclang)
set(CMAKE_CXX_COMPILER              armclang)
set(CMAKE_C_LINKER_PREFERENCE       armlink)
set(CMAKE_ASM_LINKER_PREFERENCE     armlink)
set(CMAKE_ASM_COMPILER              armasm)
set(CMAKE_ASM_COMPILER_AR           armar)

set(CMAKE_CROSSCOMPILING            true)
set(CMAKE_SYSTEM_NAME               Generic)

set(MIN_ARM_CLANG_VERSION           6.15)

# Skip compiler test execution
set(CMAKE_C_COMPILER_WORKS          1)
set(CMAKE_CXX_COMPILER_WORKS        1)
set(PLATFORM_HAL                    1)

if (NOT DEFINED CMAKE_SYSTEM_PROCESSOR)
    set(CMAKE_SYSTEM_PROCESSOR      cortex-m55)
endif()

if (CMAKE_SYSTEM_PROCESSOR STREQUAL cortex-m55)
    # Flags for cortex-m55
    set(CPU_COMPILE_DEF             CPU_CORTEX_M55)
    set(CPU_NAME                    ${CMAKE_SYSTEM_PROCESSOR})
    set(FLOAT_ABI                   hard)
    set(ARM_MATH_DSP                1)
    set(ARM_MATH_LOOPUNROLL         1)
    set(CPU_LINK_OPT                "--cpu=8.1-M.Main.dsp")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL cortex-m33)
    # Flags for cortex-m33 to go here
endif()

set(${CPU_COMPILE_DEF}              1)

# Warning options
add_compile_options(
    -Wall
    -Wextra
    -Wvla)

# General purpose compile options:
add_compile_options(
    -funsigned-char
    "$<$<COMPILE_LANGUAGE:CXX>:-fno-unwind-tables;-fno-rtti;-fno-exceptions>")

# Arch compile options:
add_compile_options(
    -mthumb
    -mcpu=${CPU_NAME}
    -mfloat-abi=${FLOAT_ABI}
    --target=arm-arm-non-eabi
    -mlittle-endian
    -MD)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_compile_options(-fstack-protector-strong)
endif()

# Compile definitions:
add_compile_definitions(
    PLATFORM_HAL=${PLATFORM_HAL}
    ${CPU_COMPILE_DEF}=1
    $<$<BOOL:${ARM_MATH_DSP}>:ARM_MATH_DSP>
    $<$<BOOL:${ARM_MATH_LOOPUNROLL}>:ARM_MATH_LOOPUNROLL>)

# Link options:
add_link_options(${CPU_LINK_OPT})
set(CMAKE_ASM_FLAGS "${CPU_LINK_OPT}")

# Warnings to be ignored:
# L6314W = No section matches pattern
# L6439W = Multiply defined Global Symbol
add_link_options(
    --diag_suppress=L6439W,L6314W
    --info sizes,totals,unused,veneers
    --strict
    --callgraph
    --load_addr_map_info
    --xref
    --datacompressor off
    "$<$<CONFIG:RELEASE>:--no_debug>")

# Function to add a map file output for the linker to dump diagnostic information to.
function(add_target_map_file TARGET_NAME MAP_FILE_PATH)
    target_link_options(${TARGET_NAME} PUBLIC
        --map --symbols --list=${MAP_FILE_PATH})
endfunction()

# Function to add linker option to use the chosen linker script (scatter file).
function(add_linker_script SCRIPT_DIR SCRIPT_NAME)
    set(LINKER_SCRIPT_PATH ${SCRIPT_DIR}/${SCRIPT_NAME}.sct
        CACHE STRING "Linker script path")
    if (NOT EXISTS ${LINKER_SCRIPT_PATH})
        message(FATAL_ERROR "Scatter file not found: ${LINKER_SCRIPT_PATH}")
    endif()
    message(STATUS "Using linker script: ${LINKER_SCRIPT_PATH}")
    add_link_options(--scatter=${LINKER_SCRIPT_PATH})
endfunction()

# Function to set the command to copy/extract contents from an elf
# into a binary file.
function(add_bin_generation_command)

    set(multiValueArgs SECTION_PATTERNS OUTPUT_BIN_NAMES)
    set(oneValueArgs TARGET_NAME OUTPUT_DIR AXF_PATH)
    cmake_parse_arguments(PARSED "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_custom_command(TARGET ${PARSED_TARGET_NAME}
        POST_BUILD
        COMMAND fromelf --bin --output=${PARSED_OUTPUT_DIR}/
        ${PARSED_AXF_PATH})

endfunction()

# Function to assert the compiler version
function(enforce_compiler_version)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${MIN_ARM_CLANG_VERSION})
        message( FATAL_ERROR "Arm compiler version must be ${MIN_ARM_CLANG_VERSION} or greater to support ${CMAKE_SYSTEM_PROCESSOR} architecture." )
    endif()
endfunction()
