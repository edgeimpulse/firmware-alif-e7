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

#ifndef NETWORKS_H
#define NETWORKS_H

#include <ethosu_core_interface.h>

#include <cstdio>
#include <inttypes.h>

using namespace EthosU;

namespace MessageHandler {

class Networks {
public:
    virtual ~Networks() {}
    virtual bool getNetwork(const ethosu_core_network_buffer &buffer, void *&data, size_t &size) = 0;
};

template <typename T>
class BaseNetworks : public Networks {
public:
    bool getNetwork(const ethosu_core_network_buffer &buffer, void *&data, size_t &size) override {
        switch (buffer.type) {
        case ETHOSU_CORE_NETWORK_BUFFER:
            data = reinterpret_cast<void *>(buffer.buffer.ptr);
            size = buffer.buffer.size;
            return false;
        case ETHOSU_CORE_NETWORK_INDEX:
            return T::getIndexedNetwork(buffer.index, data, size);
        default:
            printf("Error: Unsupported network model type. type=%" PRIu32 "\n", buffer.type);
            return true;
        }
    }
};

class NoIndexedNetworks : public BaseNetworks<NoIndexedNetworks> {
    static bool getIndexedNetwork(const uint32_t index, void *&data, size_t &size) {
        printf("Error: Network model index out of range. index=%" PRIu32 "\n", index);
        return true;
    }
};

} // namespace MessageHandler

#endif
