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

# CMake configuration file for peripheral memory map for simple platform. This is a stripped down
# version of Arm Corstone-300 platform with minimal peripherals to be able to use Ethos-U55. However,
# for ease of integration with Arm FastModel Tools, it uses PL011 as the UART component instead of
# the CMSDK UART block used by the MPS3 FPGA and FVP implementations.

###################################################################################################
#                              Application specific config                                        #
###################################################################################################

# This parameter is based on the linker/scatter script for simple platform. Do not change this
# parameter in isolation.
set(ACTIVATION_BUF_SRAM_SZ "0x00200000"      CACHE STRING "Maximum SRAM size for activation buffers")
set(DESIGN_NAME            "ENSEMBLE"        CACHE STRING "Design name")

###################################################################################################
#                                         Base addresses                                          #
###################################################################################################
set(PL011_UART0_BASE            "0x49303000" CACHE STRING "PL011 UART 0 Base Address")

if (ETHOS_U_NPU_ENABLED)
    set(ETHOS_U_NPU_BASE          "0x400E1000" CACHE STRING "Ethos-U NPU base address")
endif ()

###################################################################################################
#                                           IRQ numbers                                           #
###################################################################################################
if (ETHOS_U_NPU_ENABLED)
    set(EthosU_IRQn             "55"         CACHE STRING "Ethos-U NPU Interrupt")
endif ()
