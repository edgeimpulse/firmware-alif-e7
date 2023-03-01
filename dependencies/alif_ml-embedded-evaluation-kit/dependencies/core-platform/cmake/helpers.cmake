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

# Add .elf to all executables
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

#############################################################################
# Link options
#############################################################################

function(ethosu_target_link_options target scope)
    cmake_parse_arguments(ARG "" "LINK_FILE;ENTRY" "" ${ARGN})

    # Store the link file in a property to be evaluated by the executable.
    get_filename_component(LINK_FILE_PATH ${ARG_LINK_FILE} ABSOLUTE)
    set_property(GLOBAL PROPERTY ETHOSU_TARGET_LINK_FILE ${LINK_FILE_PATH})

    if (ARG_ENTRY)
        target_link_options(${target} ${scope} --entry Reset_Handler)
    endif()
endfunction()

function(ethosu_eval_link_options target)
    # Get the link file from the cache
    get_property(LINK_FILE GLOBAL PROPERTY ETHOSU_TARGET_LINK_FILE)

    set(prop "$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>")
    set(opt "$<TARGET_PROPERTY:${target},COMPILE_OPTIONS>")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "ARMClang")
        set(LINK_FILE_EXT scatter)
        set(LINK_FILE_OPTION "--scatter")
        # Note: the -mcpu flag is added to avoid warnings caused when using the default cpu/arch.
        set(COMPILER_PREPROCESSOR_OPTIONS -mcpu=${CMAKE_SYSTEM_PROCESSOR} --target=arm-arm-none-eabi -E -x c -P)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(LINK_FILE_EXT ld)
        set(LINK_FILE_OPTION "-T")
        set(COMPILER_PREPROCESSOR_OPTIONS -E -x c -P)
    endif()

    if (EXISTS ${LINK_FILE})
        set(LINK_FILE_IN ${LINK_FILE})
    else()
        set(LINK_FILE_IN ${LINK_FILE}.${LINK_FILE_EXT})
    endif()

    get_filename_component(LINK_FILE_OUT_BASE ${LINK_FILE} NAME)
    set(LINK_FILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/${LINK_FILE_OUT_BASE}-${target}.${LINK_FILE_EXT})

    add_custom_command(
        OUTPUT ${LINK_FILE_OUT}
        DEPENDS ${LINK_FILE_IN}
        COMMAND ${CMAKE_C_COMPILER} ${COMPILER_PREPROCESSOR_OPTIONS} -o ${LINK_FILE_OUT} ${LINK_FILE_IN}
        COMMAND_EXPAND_LISTS "-D$<JOIN:${prop},;-D>"
        COMMAND_EXPAND_LISTS "$<FILTER:${opt},INCLUDE,^(-U|-D)>"
        COMMAND_EXPAND_LISTS "-I$<JOIN:$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>,;-I>"
        COMMENT "Preprocessing and generating linker script"
        VERBATIM)
    add_custom_target(${target}-linker-script
        DEPENDS ${LINK_FILE_OUT}
        VERBATIM)
    add_dependencies(${target} ${target}-linker-script)

    target_link_options(${target} PUBLIC ${LINK_FILE_OPTION} ${LINK_FILE_OUT})
    set_target_properties(${target} PROPERTIES LINK_DEPENDS ${LINK_FILE_OUT})
endfunction()

#############################################################################
# Executable
#############################################################################

function(ethosu_add_executable target)
    cmake_parse_arguments(ARGS "" "TARGET_LIBRARY" "SOURCES;LIBRARIES" ${ARGN})
    add_executable(${target})

    target_sources(${target} PRIVATE
        ${ARGS_SOURCES})

    if(NOT ARGS_TARGET_LIBRARY)
        set(ARGS_TARGET_LIBRARY ethosu_target_init)
    endif()

    target_link_libraries(${target} PRIVATE
        ${ARGS_TARGET_LIBRARY} ethosu_core ${ARGS_LIBRARIES})

    ethosu_eval_link_options(${target})

    if (CMAKE_CXX_COMPILER_ID STREQUAL "ARMClang")
        target_link_options(${target} PRIVATE --map --list=${target}.map)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(${target} PRIVATE -Xlinker -Map=${target}.map)
    endif()

    ethosu_add_binaries(${target})
endfunction()

#############################################################################
# Test
#############################################################################

set(ETHOSU_CTEST_TARMAC_TRACE OFF CACHE BOOL "Enable to generate tarmac trace files when running ctest")

function(ethosu_add_test target)
    if(NOT BUILD_TESTING)
        return()
    endif()

    cmake_parse_arguments(ARGS "" "NAME" "COMMAND" ${ARGN})

    if (NOT ARGS_COMMAND)
        set(ARGS_COMMAND ${ETHOSU_COMMAND_DEFAULT})
    endif()

    if (NOT ARGS_NAME)
        set(ARGS_NAME ${target})
    endif()

    add_test(NAME ${ARGS_NAME}
        COMMAND ${ARGS_COMMAND} $<TARGET_FILE:${target}>)
endfunction()

#############################################################################
# Executable and test
#############################################################################

function(ethosu_add_executable_test target)
    ethosu_add_executable(${target} ${ARGN})
    ethosu_add_test(${target} ${ARGN})
endfunction()

#############################################################################
# Generate binaries
#############################################################################

set(Python3_FIND_STRATEGY LOCATION)
find_package(Python3 COMPONENTS Interpreter)

function(ethosu_add_binaries target)
    set(SCRIPTS_DIR ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../scripts)

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${Python3_EXECUTABLE} ${SCRIPTS_DIR}/generate_binaries.py --output ./fw $<TARGET_FILE:${target}> -d ${target}.d
        DEPFILE ${CMAKE_CURRENT_BINARY_DIR}/${target}.d
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generate binaries for ${target}")
endfunction()

#############################################################################
# Ethos-U NPU configuration
#############################################################################

function(ethosu_get_architecture config)
    string(TOLOWER ${config} config)

    if(${config} STREQUAL "none")
        set(ETHOSU_ARCH "none" PARENT_SCOPE)
        set(ETHOSU_NUM_MACS 0 PARENT_SCOPE)
    elseif(${config} MATCHES "^ethos-(u[0-9]+|uz)-([0-9]+$)")
        set(ETHOSU_ARCH ${CMAKE_MATCH_1} PARENT_SCOPE)
        set(ETHOSU_NUM_MACS ${CMAKE_MATCH_2} PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unknown NPU config ${config}")
    endif()
endfunction()
