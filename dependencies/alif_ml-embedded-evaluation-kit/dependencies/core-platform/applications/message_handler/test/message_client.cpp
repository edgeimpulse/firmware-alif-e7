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

#include "FreeRTOS.h"
#include "task.h"

#include "ethosu_core_interface.h"
#include "message_client.hpp"

using namespace EthosU;

namespace MessageHandler {

MessageClient::MessageClient(EthosU::ethosu_core_queue &_inputMessageQueue,
                             EthosU::ethosu_core_queue &_outputMessageQueue,
                             Mailbox::Mailbox &_mailbox) :
    input(_inputMessageQueue),
    output(_outputMessageQueue), mailbox(_mailbox) {}

bool MessageClient::sendInputMessage(const uint32_t type, const void *src, uint32_t length) {
    if (!input.write(type, src, length)) {
        printf("ERROR: Msg: Failed to write message request. No mailbox message sent\n");
        return false;
    }

    mailbox.sendMessage();
    mailbox.handleMessage();
    return true;
}

bool MessageClient::waitAndReadOutputMessage(const uint32_t expected_type, uint8_t *dst, uint32_t length) {
    constexpr TickType_t delay    = pdMS_TO_TICKS(5);
    constexpr TickType_t deadline = pdMS_TO_TICKS(/* 1 minute */ 60 * 1000 * 1000);
    struct ethosu_core_msg msg;

    TickType_t totalDelay = 0;
    while (output.available() == 0) {
        vTaskDelay(delay);
        totalDelay += delay;
        if (totalDelay >= deadline) {
            return false;
        }
    }

    if (!output.read(msg)) {
        printf("ERROR: Failed to read msg header\n");
        return false;
    }

    if (msg.magic != ETHOSU_CORE_MSG_MAGIC) {
        printf("ERROR: Invalid Magic\n");
        return false;
    }

    if (msg.type != expected_type) {
        printf("ERROR: Wrong message type. Got %" PRIu32 " expected %" PRIu32 "\n", msg.type, expected_type);
        return false;
    }

    if (msg.length != length) {
        printf("ERROR: Wrong message size\n");
        return false;
    }

    if (length == 0) {
        return true;
    }

    if (!output.read(dst, length)) {
        printf("ERROR: Failed to read msg payload\n");
        return false;
    }

    return true;
}
} // namespace MessageHandler
