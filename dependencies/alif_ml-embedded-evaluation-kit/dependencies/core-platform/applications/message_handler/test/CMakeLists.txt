#
# Copyright (c) 2022 Arm Limited.
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

add_subdirectory(test_message_handler)

set(TEST_MESSAGE_HANDLER_MODEL_0 "model.h" CACHE STRING "Path to built in model 0")
set(TEST_MESSAGE_HANDLER_MODEL_1 "" CACHE STRING "Path to built in model 1")
set(TEST_MESSAGE_HANDLER_MODEL_2 "" CACHE STRING "Path to built in model 2")
set(TEST_MESSAGE_HANDLER_MODEL_3 "" CACHE STRING "Path to built in model 3")

function(ethosu_add_message_handler_test testname)
    if(TARGET ethosu_core_driver)
        file(GLOB models LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/../../baremetal/models/${ETHOSU_TARGET_NPU_CONFIG}/*")
    endif()

    foreach(model ${models})
        get_filename_component(modelname ${model} NAME)
        ethosu_add_executable_test(mh_${testname}_${modelname}
            SOURCES
            ${testname}.cpp
            message_client.cpp
            LIBRARIES
            message_handler_lib
            freertos_kernel
            ethosu_mhu_dummy)

        target_include_directories(mh_${testname}_${modelname} PRIVATE
            ../indexed_networks
            ${model}
            ${LINUX_DRIVER_STACK_PATH}/kernel)

        target_compile_definitions(mh_${testname}_${modelname} PRIVATE
            TENSOR_ARENA_SIZE=${MESSAGE_HANDLER_ARENA_SIZE}
            $<$<BOOL:${TEST_MESSAGE_HANDLER_MODEL_0}>:MODEL_0=${TEST_MESSAGE_HANDLER_MODEL_0}>
            $<$<BOOL:${TEST_MESSAGE_HANDLER_MODEL_1}>:MODEL_1=${TEST_MESSAGE_HANDLER_MODEL_1}>
            $<$<BOOL:${TEST_MESSAGE_HANDLER_MODEL_2}>:MODEL_2=${TEST_MESSAGE_HANDLER_MODEL_2}>
            $<$<BOOL:${TEST_MESSAGE_HANDLER_MODEL_3}>:MODEL_3=${TEST_MESSAGE_HANDLER_MODEL_3}>)
    endforeach()
endfunction()

ethosu_add_message_handler_test(run_inference_test)
ethosu_add_message_handler_test(cancel_reject_inference_test)
