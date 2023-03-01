
/*
 * Copyright (c) 2022 Arm Limited.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <inttypes.h>
#include <stdio.h>

#include "ethosu_core_interface.h"

namespace MessageHandler {

ethosu_core_network_info_req networkInfoIndexedRequest(uint64_t user_arg, uint32_t index) {
    ethosu_core_network_info_req req = {user_arg,                   // user_arg
                                        {                           // network
                                         ETHOSU_CORE_NETWORK_INDEX, // type
                                         {{
                                             index, // index
                                             0      // ignored padding of union
                                         }}}};
    return req;
}

ethosu_core_network_info_req networkInfoBufferRequest(uint64_t user_arg, unsigned char *ptr, uint32_t ptr_size) {
    ethosu_core_network_info_req req = {user_arg,                    // user_arg
                                        {                            // network
                                         ETHOSU_CORE_NETWORK_BUFFER, // type
                                         {{
                                             reinterpret_cast<uint32_t>(ptr), // ptr
                                             ptr_size                         // size
                                         }}}};
    return req;
}

ethosu_core_network_info_rsp networkInfoResponse(uint64_t user_arg) {
    ethosu_core_network_info_rsp rsp = {
        user_arg,               // user_arg
        "Vela Optimised",       // description
        1,                      // ifm_count
        {/* not comparable */}, // ifm_sizes
        1,                      // ofm_count
        {/* not comparable */}, // ofm_sizes
        ETHOSU_CORE_STATUS_OK   // status
    };
    return rsp;
}

ethosu_core_inference_req inferenceIndexedRequest(uint64_t user_arg,
                                                  uint32_t index,
                                                  unsigned char *input_data,
                                                  uint32_t input_data_size,
                                                  uint8_t *output_data,
                                                  uint32_t output_data_size) {
    ethosu_core_inference_req req = {
        user_arg, // user_arg
        1,        // ifm_count
        {         // ifm
         {
             reinterpret_cast<uint32_t>(input_data), // ptr
             input_data_size                         // size
         }},
        1, // ofm_count
        {  // ofm
         {
             reinterpret_cast<uint32_t>(output_data), // ptr
             output_data_size                         // size
         }},
        {                           // network
         ETHOSU_CORE_NETWORK_INDEX, // type
         {{
             index, // index
             0      // ignored padding of union
         }}},
        {0, 0, 0, 0, 0, 0, 0, 0}, // pmu_event_config
        0                         // pmu_cycle_counter_enable
    };
    return req;
}

ethosu_core_inference_req inferenceBufferRequest(uint64_t user_arg,
                                                 unsigned char *ptr,
                                                 uint32_t ptr_size,
                                                 unsigned char *input_data,
                                                 uint32_t input_data_size,
                                                 uint8_t *output_data,
                                                 uint32_t output_data_size) {
    ethosu_core_inference_req req = {
        user_arg, // user_arg
        1,        // ifm_count
        {         // ifm
         {
             reinterpret_cast<uint32_t>(input_data), // ptr
             input_data_size                         // size
         }},
        1, // ofm_count
        {  // ofm
         {
             reinterpret_cast<uint32_t>(output_data), // ptr
             output_data_size                         // size
         }},
        {                            // network
         ETHOSU_CORE_NETWORK_BUFFER, // type
         {{
             reinterpret_cast<uint32_t>(ptr), // ptr
             ptr_size                         // size
         }}},
        {0, 0, 0, 0, 0, 0, 0, 0}, // pmu_event_config
        0                         // pmu_cycle_counter_enable
    };
    return req;
}
} // namespace MessageHandler

#endif
