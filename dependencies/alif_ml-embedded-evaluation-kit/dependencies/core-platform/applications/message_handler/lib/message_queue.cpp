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

#include "message_queue.hpp"

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <inttypes.h>

namespace MessageQueue {

QueueImpl::QueueImpl(EthosU::ethosu_core_queue &_queue) : queue(_queue) {
    cleanHeaderData();
}

bool QueueImpl::empty() const {
    invalidateHeaderData();

    return queue.header.read == queue.header.write;
}

size_t QueueImpl::available() const {
    invalidateHeaderData();

    size_t avail = queue.header.write - queue.header.read;

    if (queue.header.read > queue.header.write) {
        avail += queue.header.size;
    }

    return avail;
}

size_t QueueImpl::capacity() const {
    return queue.header.size - available() - 1;
}

bool QueueImpl::read(uint8_t *dst, uint32_t length) {
    const uint8_t *end = dst + length;

    // Available will invalidate the cache
    if (length > available()) {
        return false;
    }

    uint32_t rpos = queue.header.read;

    while (dst < end) {
        *dst++ = queue.data[rpos];
        rpos   = (rpos + 1) % queue.header.size;
    }

    queue.header.read = rpos;

    cleanHeader();

    return true;
}

bool QueueImpl::write(const Vec *vec, size_t length) {
    size_t total = 0;

    for (size_t i = 0; i < length; i++) {
        total += vec[i].length;
    }

    invalidateHeader();

    if (total > capacity()) {
        return false;
    }

    uint32_t wpos = queue.header.write;

    for (size_t i = 0; i < length; i++) {
        const uint8_t *src = reinterpret_cast<const uint8_t *>(vec[i].base);
        const uint8_t *end = src + vec[i].length;

        while (src < end) {
            queue.data[wpos] = *src++;
            wpos             = (wpos + 1) % queue.header.size;
        }
    }

    // Update the write position last
    queue.header.write = wpos;

    cleanHeaderData();

    return true;
}

bool QueueImpl::write(const uint32_t type, const void *src, uint32_t length) {
    EthosU::ethosu_core_msg msg = {ETHOSU_CORE_MSG_MAGIC, type, length};
    Vec vec[2]                  = {{&msg, sizeof(msg)}, {src, length}};

    return write(vec, 2);
}

// Skip to magic or end of queue
void QueueImpl::reset() {
    invalidateHeader();
    queue.header.read = queue.header.write;
    cleanHeader();
}

void QueueImpl::cleanHeader() const {
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanDCache_by_Addr(reinterpret_cast<uint32_t *>(&queue.header), sizeof(queue.header));
#endif
}

void QueueImpl::cleanHeaderData() const {
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanDCache_by_Addr(reinterpret_cast<uint32_t *>(&queue.header), sizeof(queue.header));
    uintptr_t queueDataPtr = reinterpret_cast<uintptr_t>(&queue.data[0]);
    SCB_CleanDCache_by_Addr(reinterpret_cast<uint32_t *>(queueDataPtr & ~3), queue.header.size + (queueDataPtr & 3));
#endif
}

void QueueImpl::invalidateHeader() const {
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_InvalidateDCache_by_Addr(reinterpret_cast<uint32_t *>(&queue.header), sizeof(queue.header));
#endif
}

void QueueImpl::invalidateHeaderData() const {
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_InvalidateDCache_by_Addr(reinterpret_cast<uint32_t *>(&queue.header), sizeof(queue.header));
    uintptr_t queueDataPtr = reinterpret_cast<uintptr_t>(&queue.data[0]);
    SCB_InvalidateDCache_by_Addr(reinterpret_cast<uint32_t *>(queueDataPtr & ~3),
                                 queue.header.size + (queueDataPtr & 3));
#endif
}
} // namespace MessageQueue
