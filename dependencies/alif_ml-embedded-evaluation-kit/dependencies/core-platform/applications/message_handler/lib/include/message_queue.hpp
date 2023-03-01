/*
 * Copyright (c) 2020-2022 Arm Limited.
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

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <cstddef>
#include <ethosu_core_interface.h>

namespace MessageQueue {

template <uint32_t SIZE>
struct Queue {
    EthosU::ethosu_core_queue_header header;
    uint8_t data[SIZE];

    constexpr Queue() : header({SIZE, 0, {0}, 0}) {}

    constexpr EthosU::ethosu_core_queue *toQueue() {
        return reinterpret_cast<EthosU::ethosu_core_queue *>(&header);
    }
};

class QueueImpl {
public:
    struct Vec {
        const void *base;
        size_t length;
    };

    QueueImpl(EthosU::ethosu_core_queue &queue);

    bool empty() const;
    size_t available() const;
    size_t capacity() const;
    void reset();
    bool read(uint8_t *dst, uint32_t length);
    template <typename T>
    bool read(T &dst) {
        return read(reinterpret_cast<uint8_t *>(&dst), sizeof(dst));
    }
    bool write(const Vec *vec, size_t length);
    bool write(const uint32_t type, const void *src = nullptr, uint32_t length = 0);
    template <typename T>
    bool write(const uint32_t type, const T &src) {
        return write(type, reinterpret_cast<const void *>(&src), sizeof(src));
    }

private:
    void cleanHeader() const;
    void cleanHeaderData() const;
    void invalidateHeader() const;
    void invalidateHeaderData() const;

    EthosU::ethosu_core_queue &queue;
};
} // namespace MessageQueue

#endif
