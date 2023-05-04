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

#ifndef INDEXED_NETWORKS_H
#define INDEXED_NETWORKS_H

#include "networks.hpp"

#include <cstdio>
#include <inttypes.h>

#define XSTRINGIFY(src) #src
#define STRINGIFY(src)  XSTRINGIFY(src)

namespace {
#if defined(__has_include)

#if defined(MODEL_0)
namespace Model0 {
#include STRINGIFY(MODEL_0)
}
#endif

#if defined(MODEL_1)
namespace Model1 {
#include STRINGIFY(MODEL_1)
}
#endif

#if defined(MODEL_2)
namespace Model2 {
#include STRINGIFY(MODEL_2)
}
#endif

#if defined(MODEL_3)
namespace Model3 {
#include STRINGIFY(MODEL_3)
}
#endif

#endif
} // namespace

namespace MessageHandler {

class WithIndexedNetworks : public BaseNetworks<WithIndexedNetworks> {
public:
    static bool getIndexedNetwork(const uint32_t index, void *&data, size_t &size) {
        switch (index) {
#if defined(MODEL_0)
        case 0:
            data = reinterpret_cast<void *>(Model0::networkModelData);
            size = sizeof(Model0::networkModelData);
            break;
#endif

#if defined(MODEL_1)
        case 1:
            data = reinterpret_cast<void *>(Model1::networkModelData);
            size = sizeof(Model1::networkModelData);
            break;
#endif

#if defined(MODEL_2)
        case 2:
            data = reinterpret_cast<void *>(Model2::networkModelData);
            size = sizeof(Model2::networkModelData);
            break;
#endif

#if defined(MODEL_3)
        case 3:
            data = reinterpret_cast<void *>(Model3::networkModelData);
            size = sizeof(Model3::networkModelData);
            break;
#endif

        default:
            printf("Error: Network model index out of range. index=%" PRIu32 "\n", index);
            return true;
        }

        return false;
    }
};

} // namespace MessageHandler

#endif
