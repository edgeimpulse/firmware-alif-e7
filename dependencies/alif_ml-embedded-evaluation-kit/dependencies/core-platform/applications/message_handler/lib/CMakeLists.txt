#
# Copyright (c) 2020-2022 Arm Limited.
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

add_library(message_handler_lib STATIC)

target_include_directories(message_handler_lib PUBLIC include
        PRIVATE ${LINUX_DRIVER_STACK_PATH}/kernel)

target_link_libraries(message_handler_lib
        PUBLIC
        ethosu_mailbox
        $<$<TARGET_EXISTS:ethosu_core_driver>:ethosu_core_driver>
        PRIVATE
        cmsis_device
        freertos_kernel
        inference_process
        tflu)

target_sources(message_handler_lib PRIVATE
        message_handler.cpp
        message_queue.cpp
        core_driver_mutex.cpp
        freertos_allocator.cpp)
