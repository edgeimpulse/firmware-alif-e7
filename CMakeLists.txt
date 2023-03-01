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

# minimum version of cmake = 3.21 for compatability with Alif CMake structure
cmake_minimum_required(VERSION 3.21.0)

# Build in release mode by default
if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_BUILD_TYPE Release CACHE INTERNAL "")
endif()

message(STATUS "Build type is set to ${CMAKE_BUILD_TYPE}")

# Set language standards. TensorFlow Lite requires
# std=c++11.
set(CMAKE_C_STANDARD   99)
set(CMAKE_CXX_STANDARD 14)

# Make the standard a requirement => prevent fallback to previous
# supported standard
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# We want to pass standard C/C++ flags, without gnu extensions
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ALIF_REPO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/alif_ml-embedded-evaluation-kit)

# Use the alif repo's cmake scripts & depnendencies
set(CMAKE_SCRIPTS_DIR   ${ALIF_REPO_DIR}/scripts/cmake)
set(CMAKE_TOOLCHAIN_DIR ${CMAKE_SCRIPTS_DIR}/toolchains)
set(HAL_PLATFORM_DIR    ${ALIF_REPO_DIR}/source/hal/source/platform)
set(DEPENDENCY_ROOT_DIR ${ALIF_REPO_DIR}/dependencies)
set(CORE_PLATFORM_DIR   ${DEPENDENCY_ROOT_DIR}/core-platform)
set(LOG_PROJECT_DIR     ${ALIF_REPO_DIR}/source/log)

# and also add some EI specific dependencies
set(EI_SCRIPTS_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake)
set(SRC_PATH            ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(EI_SDK_PATH         ${SRC_PATH}/edge-impulse-sdk)

if (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    message(FATAL_ERROR "Source and build are in the same directory")
else()
    message(STATUS "Source directory: ${CMAKE_SOURCE_DIR}")
    message(STATUS "Binary directory: ${CMAKE_BINARY_DIR}")
endif()

include(${EI_SCRIPTS_DIR}/ei_user_options.cmake)

add_platform_build_configuration(TARGET_PLATFORM ${TARGET_PLATFORM})

set_platform_global_defaults()

message(STATUS "Platform: Ensemble platform with minimal peripherals")
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_DIR}/bare-metal-gcc.cmake)

assert_defined(LOG_LEVEL)
assert_defined(TARGET_PLATFORM)
assert_defined(TARGET_SUBSYSTEM)
assert_defined(ETHOS_U_NPU_ENABLED)
assert_defined(CPU_PROFILE_ENABLED)
assert_defined(CMAKE_TOOLCHAIN_FILE)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${DEFAULT_TOOLCHAIN_FILE}
        CACHE FILEPATH "Toolchain file")
endif()
message(STATUS "Using CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

if(POLICY CMP0123)
    cmake_policy(SET CMP0123 NEW)
endif()

project(edge_impulse_alif
        VERSION     22.00 DESCRIPTION "Example program for Edge Impulse running on Alif hardware")

enforce_compiler_version()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# Add ethos, ensemble, and CMSIS library dependencies through alif repo CMakeLists
add_subdirectory(${ALIF_REPO_DIR}/source/math ${CMAKE_BINARY_DIR}/source/math EXCLUDE_FROM_ALL)
add_subdirectory(${ALIF_REPO_DIR}/source/hal ${CMAKE_BINARY_DIR}/source/hal EXCLUDE_FROM_ALL)

# Add Edge Impulse SDK dependencies

# Add application sources & executable target
set(TARGET_NAME "app")
add_executable(${TARGET_NAME})
# Include directories:
set(APPLICATION_INCLUDE_DIRS
    "${SRC_PATH}/firmware-copies"
    "${SRC_PATH}/QCBOR/inc"
    "${SRC_PATH}/mbedtls_hmac_sha256_sw" 
    "${SRC_PATH}/model-parameters"
    "${SRC_PATH}/tflite-model"
    )

add_subdirectory("${SRC_PATH}")
add_subdirectory("${SRC_PATH}/firmware-sdk-alif")

file(GLOB_RECURSE SRC_UTIL
    "${SRC_PATH}/firmware-copies/*.c"
    "${SRC_PATH}/QCBOR/*.c"
    "${SRC_PATH}/mbedtls_hmac_sha256_sw/mbedtls/src/*.c"
    )

set(SRC_MAIN "${SRC_PATH}/main.cpp")
list(APPEND SRC_MAIN "${SRC_PATH}/ei_classifier_porting.cpp")
list(APPEND SRC_MAIN "${SRC_PATH}/ei_microphone.cpp")
list(APPEND SRC_MAIN "${SRC_PATH}/ei_camera.cpp")
list(APPEND SRC_MAIN "${SRC_PATH}/ei_run_impulse.cpp")
list(APPEND SRC_MAIN "${SRC_PATH}/ei_device_alif_e7.cpp")
list(APPEND SRC_MAIN "${SRC_PATH}/ei_classifier_porting.cpp")

get_target_property(C_FILES app SOURCES)
set_target_properties(app PROPERTIES SOURCES "${C_FILES}")

target_sources(${TARGET_NAME} PUBLIC ${SRC_MAIN} ${SRC_UTIL})
target_include_directories(${TARGET_NAME} PRIVATE ${APPLICATION_INCLUDE_DIRS})

target_compile_options(${TARGET_NAME} PUBLIC -Wno-unused-parameter -Wno-missing-field-initializers)

# add EI definition for selecting model based on ethos core
if (TARGET_SUBSYSTEM STREQUAL RTSS-HP)
    target_compile_definitions(${TARGET_NAME} PUBLIC EI_CONFIG_ETHOS_U55_256) 
elseif (TARGET_SUBSYSTEM STREQUAL RTSS-HE)
    target_compile_definitions(${TARGET_NAME} PUBLIC EI_CONFIG_ETHOS_U55_128) 
endif()

set_source_files_properties(${SRC_MAIN}
    PROPERTIES COMPILE_DEFINITIONS
    "PRJ_VER_STR=\"${PROJECT_VERSION}\";PRJ_DES_STR=\"${PROJECT_DESCRIPTION}\"")

target_link_libraries(${TARGET_NAME} PUBLIC log arm_math hal)

# Postbuild steps generate map, axf, binaries
# TODO: SEUTILS config
platform_custom_post_build(TARGET_NAME ${TARGET_NAME})
print_useroptions()
