#
# Copyright (c) 2019-2021 Arm Limited. All rights reserved.
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

add_library(ethosu_uart_common INTERFACE)

target_include_directories(ethosu_uart_common INTERFACE
                           include
                           ${CMAKE_CURRENT_BINARY_DIR})

# UART configuration (Can be overriden from user project, default value is for  target "Corestone-300")
set(UART0_BASE        "0x49303000" CACHE STRING "UART base address")
set(UART0_BAUDRATE    "115200"     CACHE STRING "UART baudrate")
set(SYSTEM_CORE_CLOCK "25000000"   CACHE STRING "System core clock (Hz)")

# Generate UART configuration file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/uart_config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/uart_config.h")

# Drivers
add_library(ethosu_uart_cmsdk_apb STATIC src/uart_cmsdk_apb.c)
target_link_libraries(ethosu_uart_cmsdk_apb PUBLIC ethosu_uart_common)

add_library(ethosu_uart_pl011 STATIC src/uart_pl011.c)
target_link_libraries(ethosu_uart_pl011 PUBLIC ethosu_uart_common)

add_library(ethosu_uart_dummy STATIC src/uart_dummy.c)
target_link_libraries(ethosu_uart_dummy PUBLIC ethosu_uart_common)
