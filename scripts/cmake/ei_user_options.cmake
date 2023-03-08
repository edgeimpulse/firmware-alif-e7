# This file was ported to work on Alif Semiconductor Ensemble family of devices.

#  Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
#  Use, distribution and modification of this code is permitted under the
#  terms stated in the Alif Semiconductor Software License Agreement
#
#  You should have received a copy of the Alif Semiconductor Software
#  License Agreement with this file. If not, please write to:
#  contact@alifsemi.com, or visit: https://alifsemi.com/license

#----------------------------------------------------------------------------
#  SPDX-FileCopyrightText: Copyright 2021-2022 Arm Limited and/or its affiliates <open-source-office@arm.com>
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

#----------------------------------------------------------------------------
# This file should contain all the common user options for the application
# For use case specific options, see individual usecase.cmake files under
# each example use case.
#----------------------------------------------------------------------------

if (NOT DEFINED USER_OPTIONS_INCLUDED)
    set(USER_OPTIONS_INCLUDED ON)
else()
    return()
endif()

message(STATUS "Assessing ei variant of alif common options...")

include(${CMAKE_SCRIPTS_DIR}/util_functions.cmake)

USER_OPTION(LOG_LEVEL "Log level for the application"
    LOG_LEVEL_INFO
    STRING)

USER_OPTION(TARGET_PLATFORM "Target platform to execute evaluation application: mps3, simple_platform, native or ensemble"
    ensemble 
    STRING)

USER_OPTION(TARGET_SUBSYSTEM "Specify platform target subsystem: RTSS-HP, RTSS-HE or none"
    RTSS-HP
    STRING)

USER_OPTION(TARGET_BOARD "Specify board target : AppKit_Alpha1, AppKit_Alpha2, or DevKit"
    AppKit_Alpha1
    STRING)

USER_OPTION(ETHOS_U_NPU_ENABLED "If Arm Ethos-U NPU is enabled in the target system."
    ON
    BOOL)

USER_OPTION(CPU_PROFILE_ENABLED "Output CPU performance profiling information. Should be used only for MPS3 board."
    OFF
    BOOL)

USER_OPTION(GLCD_UI "Use ARM GLCD UI?"
    OFF
    BOOL)

if (NOT TARGET_PLATFORM STREQUAL native)
    
    USER_OPTION(CMSIS_SRC_PATH
        "Path to CMSIS-5 sources"
        "${DEPENDENCY_ROOT_DIR}/cmsis"
        PATH)

    USER_OPTION(CMSIS_DSP_SRC_PATH
        "Path to CMSIS-5 DSP sources"
        "${DEPENDENCY_ROOT_DIR}/cmsis-dsp"
        PATH)

    USER_OPTION(CMSIS_NN_SRC_PATH
        "Path to CMSIS-5 NN sources"
        "${DEPENDENCY_ROOT_DIR}/cmsis-nn"
        PATH)

    USER_OPTION(LVGL_SRC_PATH
        "Path to LVGL sources"
        "${DEPENDENCY_ROOT_DIR}/lvgl"
        PATH)

    USER_OPTION(ARM_2D_SRC_PATH
        "Path to Arm-2D sources"
        "${DEPENDENCY_ROOT_DIR}/Arm-2D"
        PATH)

    USER_OPTION(ETHOS_U_NPU_TIMING_ADAPTER_SRC_PATH
        "Path to Ethos-U NPU timing adapter sources"
        "${DEPENDENCY_ROOT_DIR}/core-platform/drivers/timing_adapter"
        PATH
        )

    # FIXME(@dasch0 2023/02/26): Currently an old copy of the ETHOS driver is included in the EI SDK, should either be version bumped or removed
    USER_OPTION(ETHOS_U_NPU_DRIVER_SRC_PATH
        "Path to Ethos-U NPU core driver sources"
        "${DEPENDENCY_ROOT_DIR}/core-driver"
        PATH
        )

    USER_OPTION(ETHOS_U_NPU_ID "Arm Ethos-U NPU IP (U55 or U65)"
        "U55"
        STRING)

    # add EI definition for selecting model based on core
    if (TARGET_SUBSYSTEM STREQUAL RTSS-HP)
        set(EI_CONFIG_ETHOS_U55_256 ON BOOL) 
    elseif (TARGET_SUBSYSTEM STREQUAL RTSS-HE)
        set(EI_CONFIG_ETHOS_U55_128 ON BOOL)
    endif()

    if ((ETHOS_U_NPU_ID STREQUAL U55) OR (ETHOS_U_NPU_ID STREQUAL U65))
        if (ETHOS_U_NPU_ID STREQUAL U55)
            set(DEFAULT_NPU_MEM_MODE    "Shared_Sram")
            set(DEFAULT_NPU_CONFIG_ID   "H128")
        elseif(ETHOS_U_NPU_ID STREQUAL U65)
            set(DEFAULT_NPU_MEM_MODE    "Dedicated_Sram")
            set(DEFAULT_NPU_CONFIG_ID   "Y256")
            set(DEFAULT_NPU_CACHE_SIZE  "393216")

            USER_OPTION(ETHOS_U_NPU_CACHE_SIZE "Arm Ethos-U65 NPU Cache Size"
                "${DEFAULT_NPU_CACHE_SIZE}"
                STRING)
        endif()
    else ()
        message(FATAL_ERROR "Non compatible Ethos-U NPU processor ${ETHOS_U_NPU_ID}")
    endif ()

    USER_OPTION(ETHOS_U_NPU_MEMORY_MODE "Specifies the memory mode used in the Vela command."
        "${DEFAULT_NPU_MEM_MODE}"
        STRING)

    USER_OPTION(ETHOS_U_NPU_CONFIG_ID "Specifies the configuration ID for the NPU."
        "${DEFAULT_NPU_CONFIG_ID}"
        STRING)

    if (ETHOS_U_NPU_ID STREQUAL U55)
        set(DEFAULT_TA_CONFIG_FILE_PATH "${CMAKE_CURRENT_LIST_DIR}/timing_adapter/ta_config_u55_high_end.cmake")
    else ()
        set(DEFAULT_TA_CONFIG_FILE_PATH "${CMAKE_CURRENT_LIST_DIR}/timing_adapter/ta_config_u65_high_end.cmake")
    endif ()

    USER_OPTION(ETHOS_U_NPU_TIMING_ADAPTER_ENABLED "Specifies if the Ethos-U timing adapter is enabled"
        OFF 
        BOOL)

    if (ETHOS_U_NPU_TIMING_ADAPTER_ENABLED)
        USER_OPTION(TA_CONFIG_FILE "Path to the timing adapter configuration file"
                ${DEFAULT_TA_CONFIG_FILE_PATH}
                FILEPATH)
    endif()
endif()
