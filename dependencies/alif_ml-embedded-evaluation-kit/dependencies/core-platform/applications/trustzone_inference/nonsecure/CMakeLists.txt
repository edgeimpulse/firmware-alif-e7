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
# Configuration
#############################################################################

set(TRUSTZONE_BUILD TRUE)
set(TRUSTZONE_SIDE nonsecure)

#############################################################################
# TrustZone Nonsecure world
#############################################################################

add_library(nonsecure_target_startup INTERFACE)
target_link_libraries(nonsecure_target_startup INTERFACE ethosu_target_link)
target_link_libraries(nonsecure_target_startup INTERFACE cmsis_startup)

ethosu_add_executable(trustzone_nonsecure PRIVATE
    TARGET_LIBRARY nonsecure_target_startup
    SOURCES main_nonsecure.cpp)

target_compile_definitions(trustzone_nonsecure PUBLIC TRUSTZONE_BUILD)
target_compile_definitions(trustzone_nonsecure PUBLIC TRUSTZONE_NONSECURE)

# Silence warning on cmse attributes
target_compile_options(trustzone_nonsecure PRIVATE -Wno-ignored-attributes)

# Add include directory for target application specifics
target_include_directories(trustzone_nonsecure PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../${ETHOSU_TARGET})

#############################################################################
# linker configuration
#############################################################################

# Pick up the gateway object file from secure build
target_link_options(trustzone_nonsecure PRIVATE ${SECURE_GATEWAY_LIB})

target_link_options(trustzone_nonsecure
    PRIVATE "--predefine=\"-I${CMAKE_CURRENT_SOURCE_DIR}/../${ETHOSU_TARGET}\"")

#############################################################################
# Helper target to extract non-secure binary
#############################################################################

add_custom_target(nonsecure_binary ALL COMMAND fromelf --bin
    ${CMAKE_CURRENT_BINARY_DIR}/trustzone_nonsecure.elf
     -o ${CMAKE_CURRENT_BINARY_DIR}/ns_bin
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/trustzone_nonsecure)
