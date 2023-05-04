#
# Copyright (c) 2021-2022 Arm Limited. All rights reserved.
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

if (NOT ${ETHOSU_TARGET} STREQUAL "corstone-300" OR NOT TARGET ethosu_core_driver OR NOT ETHOSU_TARGET_NPU_CONFIG STREQUAL "ethos-u55-128")
    return()
endif()

#############################################################################
# Secure world
#############################################################################

set(TRUSTZONE_BUILD ON)
set(TRUSTZONE_PARTITION_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/secure" CACHE FILEPATH "")

set(SECURE_GATEWAY_LIB "${CMAKE_CURRENT_BINARY_DIR}/secure_gw.o")

add_subdirectory(secure)

add_subdirectory(nonsecure)

add_dependencies(trustzone_nonsecure trustzone_secure)

add_custom_target(trustzone)
add_dependencies(trustzone trustzone_secure nonsecure_binary)

ethosu_add_test(trustzone_secure
    NAME trustzone
    COMMAND ${ETHOSU_COMMAND_DEFAULT} -- --data ${CMAKE_CURRENT_BINARY_DIR}/nonsecure/ns_bin@0x7c000)
