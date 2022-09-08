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
# Modified by Edge Impulse

# minimum version of cmake = 3.15 for legit reason:
# armclang support doesn't work work in previous releases
cmake_minimum_required(VERSION 3.15.0)

if (CMAKE_BUILD_TYPE STREQUAL Release)
    message(STATUS "Building with -O3 has not been stable. 
    -O2 is generally recommended, and will lead to a much smaller image as well.
    See https://wiki.gentoo.org/wiki/GCC_optimization#-O for explanation.
    Foring -02")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE INTERNAL "")
endif()

if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE INTERNAL "")
endif()

message(STATUS "Build type is set to ${CMAKE_BUILD_TYPE}")

# Set language standards. TensorFlow Lite requires
# std=c++11.
set(CMAKE_C_STANDARD   99)
set(CMAKE_CXX_STANDARD 11)

# Make the standard a requirement => prevent fallback to previous
# supported standard
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# We want to pass standard C/C++ flags, without gnu extensions
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_SCRIPTS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake)
set(CMAKE_TOOLCHAIN_DIR ${CMAKE_SCRIPTS_DIR}/toolchains)

set(EI_FOLDER source)
set(EI_SDK_FOLDER ${EI_FOLDER}/edge-impulse-sdk)
set(ALIF_SDK_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/alif-cmsis-pack)

add_compile_options(-fdata-sections)
add_compile_definitions(EI_CLASSIFIER_ALLOCATION_STATIC=1)

include(${CMAKE_SCRIPTS_DIR}/util_functions.cmake)

if (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    message(FATAL_ERROR "Source and build are in the same directory")
else()
    message(STATUS "Source directory: ${CMAKE_SOURCE_DIR}")
    message(STATUS "Binary directory: ${CMAKE_BINARY_DIR}")
endif()

# Possible log levels
#define LOG_LEVEL_TRACE       0
#define LOG_LEVEL_DEBUG       1
#define LOG_LEVEL_INFO        2
#define LOG_LEVEL_WARN        3
#define LOG_LEVEL_ERROR       4


USER_OPTION(LOG_LEVEL "Log level for the application"
    LOG_LEVEL_ERROR
    STRING)

set(TARGET_PLATFORM ensemble)

USER_OPTION(ETHOS_U_NPU_ENABLED "Select if Ethos-U55 is available for the platform and subsystem"
    ON
    BOOL)

USER_OPTION(TARGET_SUBSYSTEM 
    "Choose target, 'HP' (for M55 Core 0, High Performance), or 'HE' (for M55 Core 1, High Effiency)"
    "HP"
    STRING)

USER_OPTION(CPU_PROFILE_ENABLED "Output CPU performance profiling information. Should be used only for MPS3 board."
    OFF
    BOOL)

message(STATUS "Platform: Ensemble platform with minimal peripherals")
set(DEFAULT_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_DIR}/bare-metal-armclang.cmake)

if (TARGET_SUBSYSTEM STREQUAL "HP")
    add_compile_definitions(EI_CONFIG_ALIF_HP=1)
	add_compile_options(-DM55_HP)
    set(DEVICE_HEADERS
        ${ALIF_SDK_FOLDER}/Device/M55_HP
        ${ALIF_SDK_FOLDER}/Device/M55_HP/Config
        ${ALIF_SDK_FOLDER}/Device/M55_HP/Include)
    set(DEVICE_STARTUP
        ${ALIF_SDK_FOLDER}/Device/M55_HP/Source)
elseif(TARGET_SUBSYSTEM STREQUAL "HE")
    add_compile_definitions(EI_CONFIG_ALIF_HE=1)
	add_compile_options(-DM55_HE)
    set(DEVICE_HEADERS
        ${ALIF_SDK_FOLDER}/Device/M55_HE
        ${ALIF_SDK_FOLDER}/Device/M55_HE/Config
        ${ALIF_SDK_FOLDER}/Device/M55_HE/Include)
    set(DEVICE_STARTUP
        ${ALIF_SDK_FOLDER}/Device/M55_HE/Source)
else()
    message(FATAL_ERROR "Please choose either HP or HE target subsystem (ex: cmake -DTARGET_SUBSYSTEM=HP)")
endif()

#        ${ALIF_SDK_FOLDER}/CMSIS/Driver/Src

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${DEFAULT_TOOLCHAIN_FILE}
        CACHE FILEPATH "Toolchain file")
endif()
message(STATUS "Using CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

project(edge_impulse_alif
        VERSION     21.06
        DESCRIPTION "Example program for Edge Impulse running on Alif hardware")

enforce_compiler_version()

if (ETHOS_U_NPU_ENABLED)
    add_compile_definitions(EI_ETHOS=1 EI_ALIF_ADDR_TRANSLATION=1)
    if (TARGET_SUBSYSTEM STREQUAL "HP")
        add_compile_definitions(EI_CONFIG_ETHOS_U55_256=1)
    else()
        add_compile_definitions(EI_CONFIG_ETHOS_U55_128=1)
    endif()
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/source)

if (CPU_PROFILE_ENABLED)
    set(PROFILING_OPT "${PROFILING_OPT} -DCPU_PROFILE_ENABLED")
endif()

set(PLATFORM_SOURCES_CMAKE_FILE ${CMAKE_SCRIPTS_DIR}/bare-metal-sources.cmake)

USER_OPTION(CMSIS_SRC_PATH
    "Path to CMSIS-5 sources"
    "${EI_SDK_FOLDER}"
    PATH
    )

USER_OPTION(ALIF_CMSIS_SRC_PATH
    "Path to Alif CMSIS pack sources"
    "${ALIF_SDK_FOLDER}"
    PATH
    )

if (CMAKE_BUILD_TYPE STREQUAL Debug AND CMAKE_CXX_COMPILER_ID STREQUAL ARMClang)
    USER_OPTION(ARMCLANG_DEBUG_DWARF_LEVEL
        "Dwarf conformance level for armclang toolchain"
        "4" # Default = 4 (Arm-DS etc). For model debugger specify "3"
        STRING
        )
elseif (DEFINED ARMCLANG_DEBUG_DWARF_LEVEL)
    message(WARNING "ARMCLANG_DEBUG_DWARF_LEVEL definition is unsupported"
                    "within current configuration. Removing definition...")
    unset(ARMCLANG_DEBUG_DWARF_LEVEL CACHE)
endif()

message(STATUS "Including ${PLATFORM_SOURCES_CMAKE_FILE}")
include(${PLATFORM_SOURCES_CMAKE_FILE})

if (${CMAKE_CROSSCOMPILING})
    enable_language(ASM)

    # For non-native builds, we build with CMSIS-DSP support.
    include(${CMAKE_SCRIPTS_DIR}/cmsis-dsp.cmake)

    # All CMSIS headers to be used:
    set(CMSIS_HEADERS
        ${CMSIS_CORE_INC_DIR}
        ${DEVICE_HEADERS}
        ${ALIF_SDK_FOLDER}/CMSIS/Driver/Inc
        ${ALIF_SDK_FOLDER}/CMSIS/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cmsis/Device/ARM/ARMCM55/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cmsis/Device/ARM/ARMCM55/Include/Template)
endif ()

# Very important this line stays here!  Must come AFTER all includes, but BEFORE the next step (which needs target reference)
# Executable application:
set(TARGET_NAME "app")
add_executable(${TARGET_NAME})

# If we need NPU libraries:
if (ETHOS_U_NPU_ENABLED)

    message(STATUS "Using ARM Ethos-U55 - adding core-driver includes and libraries")

    USER_OPTION(ETHOS_U_NPU_DRIVER_SRC_PATH
        "Path to Ethos-U55 core driver sources"
        "${EI_FOLDER}/edge-impulse-sdk/porting/ethos-core-driver"
        PATH
        )

    include_directories("${EI_FOLDER}/edge-impulse-sdk/CMSIS/Core/Include")

    add_subdirectory("${ETHOS_U_NPU_DRIVER_SRC_PATH}" ${CMAKE_BINARY_DIR}/ethos-driver)
    
    set(ETHOSU_INCLUDES ${ETHOS_U_NPU_DRIVER_SRC_PATH}/include)
endif ()

set(DEP_INCLUDE_DIRS
    ${ETHOSU_INCLUDES}
    ${CMSIS_HEADERS}
    )

## All TPIP includes
set(DEP_RUNTIME_INCLUDE_DIRS
    ${DEP_INCLUDE_DIRS}
    ${DEP_FLATBUF_INCLUDE}
    )

# Edge Impulse
set(HAL_DIR ${SRC_PATH}/hal)

# HAL API sources
file(GLOB_RECURSE SRC_HAL
    "${DEVICE_STARTUP}/*.c"
    "${ALIF_SDK_FOLDER}/CMSIS/Driver/Src/*.c"
    "${HAL_DIR}/*.c"
    "${EI_FOLDER}/firmware-copies/*.cpp"
    "${EI_FOLDER}/QCBOR/*.c"
    "${EI_FOLDER}/mbedtls_hmac_sha256_sw/mbedtls/src/*.c"
    "${HAL_DIR}/image_processing/*.c"
    "${HAL_DIR}/image_capture/*.c"
    )

# Include directories:
set(APPLICATION_INCLUDE_DIRS
    ${HAL_DIR}
    ${HAL_DIR}/include
    ${HAL_DIR}/image_capture
    ${HAL_DIR}/image_processing
    ${PLAT_INCLUDE_DIRS}
    "${EI_FOLDER}/firmware-copies"
    "${EI_FOLDER}/QCBOR/inc"
    "${EI_FOLDER}/mbedtls_hmac_sha256_sw" 
    "${EI_FOLDER}/model-parameters"
    "${EI_FOLDER}/tflite-model"
    )

add_subdirectory("${EI_FOLDER}")
add_subdirectory("${EI_FOLDER}/firmware-sdk-alif")
set(SRC_MAIN "${SRC_PATH}/main.cpp")
list(APPEND SRC_MAIN "${SRC_PATH}/ei_classifier_porting.cpp")
list(APPEND SRC_MAIN "${SRC_PATH}/ei_microphone.cpp")
list(APPEND SRC_MAIN "${SRC_PATH}/ei_camera.cpp")
list(APPEND SRC_MAIN "${SRC_PATH}/ei_run_impulse.cpp")
list(APPEND SRC_MAIN "${SRC_PATH}/ei_device_alif_e7.cpp")

# Exclude DSP b/c we build it in a lib above, b/c it needs the O3 flag per ARM
get_target_property(C_FILES app SOURCES)
list(FILTER C_FILES EXCLUDE REGEX "CMSIS/DSP")
set_target_properties(app PROPERTIES SOURCES "${C_FILES}")


target_sources(${TARGET_NAME} PUBLIC 
    ${SRC_MAIN} 
    ${PLAT_RETARGET_SOURCE}
    ${SRC_APPLICATION}
    ${SRC_HAL}
    ${UC_SRC}
    ${SRC_GEN}
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${APPLICATION_INCLUDE_DIRS}
    ${DEP_RUNTIME_INCLUDE_DIRS}
    ${UC_INCLUDE}
    ${INC_GEN_DIR}
)

target_compile_options(${TARGET_NAME} PUBLIC -Wno-unused-parameter -Wno-missing-field-initializers)

set_source_files_properties(${SRC_MAIN}
    PROPERTIES COMPILE_DEFINITIONS
    "PRJ_VER_STR=\"${PROJECT_VERSION}\";PRJ_DES_STR=\"${PROJECT_DESCRIPTION}\"")


if (${CMAKE_CROSSCOMPILING})
    # If we are building timing adapter, set the dependency:
    if (ETHOS_U_NPU_ENABLED)
        target_link_libraries(${TARGET_NAME} PUBLIC ethosu_core_driver)
    endif()

    # If building with CMSIS-DSP support:
    if (DEFINED CMSIS_DSP_TARGET)
        message(STATUS "Adding ${CMSIS_DSP_TARGET} as a dependency to ${TARGET_NAME}")
        add_dependencies(${TARGET_NAME} ${CMSIS_DSP_TARGET})
    endif()
endif()

target_link_libraries(${TARGET_NAME} PUBLIC
    $<$<BOOL:${ETHOS_U_NPU_ENABLED}>:${ETHOS_U_NPU_LIBS}>
    $<$<BOOL:${CMSIS_DSP_LIB}>:${CMSIS_DSP_LIB}>)

if (${CMAKE_CROSSCOMPILING})
    set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".axf")
    add_target_map_file(${TARGET_NAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}.map)
endif()

    set(SECTORS_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sectors)
    set(SECTORS_BIN_DIR ${SECTORS_DIR}/${use_case})

    file(REMOVE_RECURSE ${SECTORS_BIN_DIR})
    file(MAKE_DIRECTORY ${SECTORS_BIN_DIR})

    add_bin_generation_command(
        TARGET_NAME ${TARGET_NAME}
        OUTPUT_DIR  ${SECTORS_BIN_DIR}
        AXF_PATH    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}.axf
        SECTION_PATTERNS    "${LINKER_SECTION_TAGS}"
        OUTPUT_BIN_NAMES    "${LINKER_OUTPUT_BIN_TAGS}")

print_useroptions()