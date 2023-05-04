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

set(BAREMETAL_PATH "" CACHE PATH "Path to input, output and network for baremetal")

if (IS_DIRECTORY ${BAREMETAL_PATH})
  ethosu_add_executable_test(baremetal_custom PRIVATE
    SOURCES main.cpp
    LIBRARIES $<$<TARGET_EXISTS:ethosu_monitor>:ethosu_monitor>)
  target_include_directories(baremetal_custom PRIVATE ${BAREMETAL_PATH})
endif()

if (TARGET ethosu_core_driver)
  file(GLOB models LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/models/${ETHOSU_TARGET_NPU_CONFIG}/*")
endif()

foreach(model ${models})
  get_filename_component(name ${model} NAME)
  ethosu_add_executable_test(baremetal_${name} PRIVATE
    SOURCES main.cpp
    LIBRARIES $<$<TARGET_EXISTS:ethosu_monitor>:ethosu_monitor>)
  target_include_directories(baremetal_${name} PRIVATE ${model})
endforeach()