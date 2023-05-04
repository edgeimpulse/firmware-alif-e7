#
# Copyright (c) 2020-2021 Arm Limited. All rights reserved.
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
# Mailbox driver
#############################################################################
add_subdirectory(mailbox)

#############################################################################
# MHU drivers
#############################################################################
# NOTE: These are all built and linked from a CMake perspective. However the
#       application code can instantiate one or more of the drivers. The
#       one(s) not used will later be removed by the linker.
add_subdirectory(mhu_v2)
add_subdirectory(mhu_juno)
add_subdirectory(mhu_dummy)

#############################################################################
# UART drivers
#############################################################################
# NOTE: All UART drivers are built, however a platform application should
#       link the appropriate driver target (see drivers/uart/CMakeLists.txt).
add_subdirectory(uart)

#############################################################################
# Timing adapter driver
#############################################################################
add_subdirectory(timing_adapter)

#############################################################################
# MPU driver
#############################################################################
add_subdirectory(mpu)
