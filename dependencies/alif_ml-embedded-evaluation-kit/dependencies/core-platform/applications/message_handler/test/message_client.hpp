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

#ifndef MESSAGE_CLIENT_H
#define MESSAGE_CLIENT_H

#include <inttypes.h>
#include <stdio.h>

#include "message_queue.hpp"
#include <mailbox.hpp>

namespace MessageHandler {

class MessageClient {
public:
    MessageClient(EthosU::ethosu_core_queue &inputMessageQueue,
                  EthosU::ethosu_core_queue &outputMessageQueue,
                  Mailbox::Mailbox &mailbox);

    template <typename T>
    bool sendInputMessage(const uint32_t type, const T &src) {
        return sendInputMessage(type, reinterpret_cast<const uint8_t *>(&src), sizeof(src));
    }
    bool sendInputMessage(const uint32_t type, const void *src = nullptr, uint32_t length = 0);
    template <typename T>
    bool waitAndReadOutputMessage(const uint32_t expected_type, T &dst) {
        return waitAndReadOutputMessage(expected_type, reinterpret_cast<uint8_t *>(&dst), sizeof(dst));
    }
    bool waitAndReadOutputMessage(const uint32_t expected_type, uint8_t *dst = nullptr, uint32_t length = 0);

private:
    MessageQueue::QueueImpl input;
    MessageQueue::QueueImpl output;
    Mailbox::Mailbox &mailbox;
};
} // namespace MessageHandler

#endif
