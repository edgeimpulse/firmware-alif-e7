/*
 * Copyright (c) 2019-2022 Arm Limited.
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
#include "message_handler.hpp"
#include "message_queue.hpp"
#include "networks.hpp"

#include <mailbox.hpp>
#if defined(MHU_V2)
#include <mhu_v2.hpp>
#elif defined(MHU_JUNO)
#include <mhu_juno.hpp>
#else
#include <mhu_dummy.hpp>
#endif

/* Disable semihosting */
__asm(".global __use_no_semihosting\n\t");

using namespace EthosU;
using namespace MessageHandler;

/****************************************************************************
 * Defines
 ****************************************************************************/

// Message queue from remote host
__attribute__((section("ethosu_core_in_queue"))) MessageQueue::Queue<1000> inputMessageQueue;

// Message queue to remote host
__attribute__((section("ethosu_core_out_queue"))) MessageQueue::Queue<1000> outputMessageQueue;

namespace {

// Mailbox driver
#ifdef MHU_V2
Mailbox::MHUv2 mailbox(MHU_TX_BASE_ADDRESS, MHU_RX_BASE_ADDRESS); // txBase, rxBase
#elif defined(MHU_JUNO)
Mailbox::MHUJuno mailbox(MHU_BASE_ADDRESS);
#else
Mailbox::MHUDummy mailbox;
#endif

} // namespace

/****************************************************************************
 * Application
 ****************************************************************************/
namespace {

struct TaskParams {
    TaskParams() :
        messageNotify(xSemaphoreCreateBinary()),
        inferenceInputQueue(std::make_shared<Queue<ethosu_core_inference_req>>()),
        inferenceOutputQueue(xQueueCreate(10, sizeof(ethosu_core_inference_rsp))),
        networks(std::make_shared<WithIndexedNetworks>()) {}

    SemaphoreHandle_t messageNotify;
    // Used to pass inference requests to the inference runner task
    std::shared_ptr<Queue<ethosu_core_inference_req>> inferenceInputQueue;
    // Queue for message responses to the remote host
    QueueHandle_t inferenceOutputQueue;
    // Networks provider
    std::shared_ptr<Networks> networks;
};

#ifdef MHU_IRQ
void mailboxIrqHandler() {
    mailbox.handleMessage();
}
#endif

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

#ifdef MHU_IRQ
    // Register mailbox interrupt handler
    NVIC_SetVector((IRQn_Type)MHU_IRQ, (uint32_t)&mailboxIrqHandler);
    NVIC_EnableIRQ((IRQn_Type)MHU_IRQ);
#endif

    process.run();
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

    // Start Scheduler
    vTaskStartScheduler();

    return 1;
}
