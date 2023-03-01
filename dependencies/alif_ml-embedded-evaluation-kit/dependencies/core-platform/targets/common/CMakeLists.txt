#
# Copyright (c) 2021 Arm Limited. All rights reserved.
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
# Core software
#############################################################################

set(ETHOSU_CORE_SOFTWARE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../core_software" CACHE PATH "Path to Arm Ethos-U Core Software")

set(CORE_SOFTWARE_RTOS "All" CACHE STRING "")
set(TFLU_BUILD_TYPE "release_with_logs" CACHE STRING "Tensorflow Lite Micro build type (release, release_with_logs, debug)")

add_subdirectory(${ETHOSU_CORE_SOFTWARE_PATH} core_software)

add_subdirectory(../../drivers drivers)

###############################################################################
# Target
#
# ethosu_target_init       # Init and drivers
#     |
#     v
# ethosu_target_startup    # CMSIS startup
#     |
#     v
# ethosu_target_link       # Target linker script
#     |
#     v
# ethosu_target_common     # Common for all targets
###############################################################################

option(CPU_CACHE_ENABLE "Enable CPU instruction- and data cache" OFF)

# Common
add_library(ethosu_target_common INTERFACE)
target_include_directories(ethosu_target_common INTERFACE include)

# Link
add_library(ethosu_target_link INTERFACE)
target_link_libraries(ethosu_target_link INTERFACE ethosu_target_common)

# Configuring caches
if (CPU_CACHE_ENABLE)
    target_compile_definitions(ethosu_target_link INTERFACE CPU_CACHE_ENABLE)
endif()

# Startup
add_library(ethosu_target_startup INTERFACE)
target_link_libraries(ethosu_target_startup INTERFACE ethosu_target_link)
target_link_libraries(ethosu_target_startup INTERFACE cmsis_system)

# Init
add_library(ethosu_target_init INTERFACE)
target_link_libraries(ethosu_target_init INTERFACE ethosu_target_startup)
target_sources(ethosu_target_init INTERFACE src/init.cpp)

