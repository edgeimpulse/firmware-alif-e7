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

/****************************************************************************
 * Includes
 ****************************************************************************/

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include <inttypes.h>
#include <stdio.h>

#include "ethosu_core_interface.h"
#include "indexed_networks.hpp"
#include "message_client.hpp"
#include "message_handler.hpp"
#include "message_queue.hpp"
#include "networks.hpp"
#include "test_assertions.hpp"
#include "test_helpers.hpp"

#include <mailbox.hpp>
#include <mhu_dummy.hpp>

/* Disable semihosting */
__asm(".global __use_no_semihosting\n\t");

using namespace EthosU;
using namespace MessageHandler;

/****************************************************************************
 * Defines
 ****************************************************************************/

// TensorArena static initialisation
constexpr size_t arenaSize = TENSOR_ARENA_SIZE;

__attribute__((section(".bss.tensor_arena"), aligned(16))) uint8_t tensorArena[arenaSize];

// Message queue from remote host
__attribute__((section("ethosu_core_in_queue"))) MessageQueue::Queue<1000> inputMessageQueue;

// Message queue to remote host
__attribute__((section("ethosu_core_out_queue"))) MessageQueue::Queue<1000> outputMessageQueue;

namespace {
Mailbox::MHUDummy mailbox;
} // namespace

/****************************************************************************
 * Application
 ****************************************************************************/
namespace {

struct TaskParams {
    TaskParams() :
        messageNotify(xSemaphoreCreateBinary()),
        inferenceInputQueue(std::make_shared<Queue<ethosu_core_inference_req>>()),
        inferenceOutputQueue(xQueueCreate(5, sizeof(ethosu_core_inference_rsp))),
        networks(std::make_shared<WithIndexedNetworks>()) {}

    SemaphoreHandle_t messageNotify;
    // Used to pass inference requests to the inference runner task
    std::shared_ptr<Queue<ethosu_core_inference_req>> inferenceInputQueue;
    // Queue for message responses to the remote host
    QueueHandle_t inferenceOutputQueue;
    // Networks provider
    std::shared_ptr<Networks> networks;
};

void messageTask(void *pvParameters) {
    printf("Starting message task\n");
    TaskParams *params = reinterpret_cast<TaskParams *>(pvParameters);

    IncomingMessageHandler process(*inputMessageQueue.toQueue(),
                                   *outputMessageQueue.toQueue(),
                                   mailbox,
                                   params->inferenceInputQueue,
                                   params->inferenceOutputQueue,
                                   params->messageNotify,
                                   params->networks);
    process.run();
}

void testCancelInference(MessageClient client) {
    const uint64_t fake_inference_user_arg = 42;
    const uint32_t network_index           = 0;
    ethosu_core_inference_req inference_req =
        inferenceIndexedRequest(fake_inference_user_arg, network_index, nullptr, 0, nullptr, 0);

    const uint64_t fake_cancel_inference_user_arg = 55;
    ethosu_core_cancel_inference_req cancel_req   = {fake_cancel_inference_user_arg, fake_inference_user_arg};

    ethosu_core_inference_rsp inference_rsp;
    ethosu_core_cancel_inference_rsp cancel_rsp;

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_INFERENCE_REQ, inference_req));
    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_CANCEL_INFERENCE_REQ, cancel_req));

    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_INFERENCE_RSP, inference_rsp));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_CANCEL_INFERENCE_RSP, cancel_rsp));

    TEST_ASSERT(inference_req.user_arg == inference_rsp.user_arg);
    TEST_ASSERT(inference_rsp.status == ETHOSU_CORE_STATUS_ABORTED);

    TEST_ASSERT(cancel_req.user_arg == cancel_rsp.user_arg);
    TEST_ASSERT(cancel_rsp.status == ETHOSU_CORE_STATUS_OK);
}

void testCancelNonExistentInference(MessageClient client) {
    const uint64_t fake_inference_user_arg        = 42;
    const uint64_t fake_cancel_inference_user_arg = 55;
    ethosu_core_cancel_inference_req cancel_req   = {fake_cancel_inference_user_arg, fake_inference_user_arg};
    ethosu_core_cancel_inference_rsp cancel_rsp;

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_CANCEL_INFERENCE_REQ, cancel_req));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_CANCEL_INFERENCE_RSP, cancel_rsp));

    TEST_ASSERT(cancel_req.user_arg == cancel_rsp.user_arg);
    TEST_ASSERT(cancel_rsp.status == ETHOSU_CORE_STATUS_ERROR);
}

void testCannotCancelRunningInference(MessageClient client,
                                      std::shared_ptr<Queue<ethosu_core_inference_req>> inferenceInputQueue) {
    const uint64_t fake_inference_user_arg = 42;
    const uint32_t network_index           = 0;
    ethosu_core_inference_req inference_req =
        inferenceIndexedRequest(fake_inference_user_arg, network_index, nullptr, 0, nullptr, 0);

    const uint64_t fake_cancel_inference_user_arg = 55;
    ethosu_core_cancel_inference_req cancel_req   = {fake_cancel_inference_user_arg, fake_inference_user_arg};
    ethosu_core_cancel_inference_rsp cancel_rsp;

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_INFERENCE_REQ, inference_req));

    // fake start of the inference by removing the inference from the queue
    ethosu_core_inference_req start_req;
    inferenceInputQueue->pop(start_req);
    TEST_ASSERT(inference_req.user_arg == start_req.user_arg);

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_CANCEL_INFERENCE_REQ, cancel_req));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_CANCEL_INFERENCE_RSP, cancel_rsp));

    TEST_ASSERT(cancel_req.user_arg == cancel_rsp.user_arg);
    TEST_ASSERT(cancel_rsp.status == ETHOSU_CORE_STATUS_ERROR);
}

void testRejectInference(MessageClient client) {
    int runs                                      = 6;
    const uint64_t fake_inference_user_arg        = 42;
    const uint32_t network_index                  = 0;
    const uint64_t fake_cancel_inference_user_arg = 55;
    ethosu_core_inference_req req;
    ethosu_core_inference_rsp rsp;

    for (int i = 0; i < runs; i++) {

        req = inferenceIndexedRequest(fake_inference_user_arg + i, network_index, nullptr, 0, nullptr, 0);
        TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_INFERENCE_REQ, req));
        vTaskDelay(150);
    }

    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_INFERENCE_RSP, rsp));
    TEST_ASSERT(uint64_t(fake_inference_user_arg + runs - 1) == rsp.user_arg);
    TEST_ASSERT(rsp.status == ETHOSU_CORE_STATUS_REJECTED);

    // let's cleanup the queue
    ethosu_core_cancel_inference_req cancel_req = {0, 0};
    ethosu_core_cancel_inference_rsp cancel_rsp;
    ethosu_core_inference_rsp inference_rsp;

    for (int i = 0; i < runs - 1; i++) {
        cancel_req.user_arg         = fake_cancel_inference_user_arg + i;
        cancel_req.inference_handle = fake_inference_user_arg + i;
        TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_CANCEL_INFERENCE_REQ, cancel_req));

        TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_INFERENCE_RSP, inference_rsp));
        TEST_ASSERT(inference_rsp.user_arg = cancel_req.inference_handle);

        TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_CANCEL_INFERENCE_RSP, cancel_rsp));
        TEST_ASSERT(cancel_req.user_arg == cancel_rsp.user_arg);
        TEST_ASSERT(cancel_rsp.status == ETHOSU_CORE_STATUS_OK);
    }
}

void clientTask(void *pvParameters) {
    printf("Starting client task\n");
    TaskParams *params = reinterpret_cast<TaskParams *>(pvParameters);

    MessageClient client(*inputMessageQueue.toQueue(), *outputMessageQueue.toQueue(), mailbox);

    vTaskDelay(50);

    testCancelInference(client);
    testCancelNonExistentInference(client);
    testCannotCancelRunningInference(client, params->inferenceInputQueue);
    testRejectInference(client);

    exit(0);
}

/*
 * Keep task parameters as global data as FreeRTOS resets the stack when the
 * scheduler is started.
 */
TaskParams taskParams;

} // namespace

// FreeRTOS application. NOTE: Additional tasks may require increased heap size.
int main() {
    BaseType_t ret;

    if (!mailbox.verifyHardware()) {
        printf("Failed to verify mailbox hardware\n");
        return 1;
    }

    // Task for handling incoming /outgoing messages from the remote host
    ret = xTaskCreate(messageTask, "messageTask", 1024, &taskParams, 2, nullptr);
    if (ret != pdPASS) {
        printf("Failed to create 'messageTask'\n");
        return ret;
    }

    // Task for handling incoming /outgoing messages from the remote host
    ret = xTaskCreate(clientTask, "clientTask", 1024, &taskParams, 2, nullptr);
    if (ret != pdPASS) {
        printf("Failed to create 'messageTask'\n");
        return ret;
    }

    // Start Scheduler
    vTaskStartScheduler();

    return 1;
}
