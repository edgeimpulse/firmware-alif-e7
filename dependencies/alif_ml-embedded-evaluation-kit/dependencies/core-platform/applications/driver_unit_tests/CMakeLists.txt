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

if (NOT TARGET ethosu_core_driver)
  return()
endif()

add_library(driver_unit INTERFACE)

target_sources(driver_unit INTERFACE
    command_stream.cpp)

ethosu_add_executable_test(driver_unit_irq PRIVATE
    SOURCES irq.cpp
    LIBRARIES driver_unit)

# Exclude driver_unit_conv if NPU configuration is not H128
if (NOT ETHOSU_TARGET_NPU_CONFIG STREQUAL "ethos-u55-128")
    message("Skipping driver_unit_conv application")
    return()
endif()

ethosu_add_executable_test(driver_unit_conv PRIVATE
    SOURCES conv.cpp
    LIBRARIES driver_unit)
