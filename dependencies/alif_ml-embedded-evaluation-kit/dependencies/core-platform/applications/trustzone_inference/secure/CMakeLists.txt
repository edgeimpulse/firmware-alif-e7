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

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/helpers.cmake)

#############################################################################
# Configuration for secure world
#############################################################################

set(TRUSTZONE_BUILD TRUE)
set(TRUSTZONE_SIDE secure)

#############################################################################
# TrustZone secure world
#############################################################################

ethosu_add_executable(trustzone_secure PRIVATE
    SOURCES main_secure.cpp mpc_sie_drv.c
    LIBRARIES ethosu_target_init)

target_compile_definitions(trustzone_secure PUBLIC TRUSTZONE_BUILD)
target_compile_definitions(trustzone_secure PUBLIC TRUSTZONE_SECURE)

# Use the fast memory to support both 11.12.57 and 11.13.x release
# of the FVP
target_compile_options(trustzone_secure PUBLIC
    -UETHOSU_FAST_MEMORY_SIZE -DETHOSU_FAST_MEMORY_SIZE=0xa0000
    -UETHOSU_ARENA -DETHOSU_ARENA=1
    -UETHOSU_MODEL -DETHOSU_MODEL=1)

target_compile_options(trustzone_secure PUBLIC -mcmse)

# Add include directory for target application specifics
target_include_directories(trustzone_secure PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../${ETHOSU_TARGET})

#############################################################################
# Additional linker configuration for trustzone
#############################################################################

# Output the secure gateway veneer library to be consumed by the build
# process for the non-secure world
target_link_options(trustzone_secure
    PRIVATE --import_cmse_lib_out=${SECURE_GATEWAY_LIB})

target_link_options(trustzone_secure
    PRIVATE "--predefine=\"-I${CMAKE_CURRENT_SOURCE_DIR}/../${ETHOSU_TARGET}\"")

#############################################################################
# Helper target to extract secure binaries
#############################################################################

add_custom_target(secure_binary ALL COMMAND fromelf --bin
    ${CMAKE_CURRENT_BINARY_DIR}/trustzone_secure.elf
     -o ${CMAKE_CURRENT_BINARY_DIR}/s_bin
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/trustzone_secure)

