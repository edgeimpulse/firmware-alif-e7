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
#include "input.h"
#include "message_client.hpp"
#include "message_handler.hpp"
#include "message_queue.hpp"
#include "networks.hpp"
#include "output.h"
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

void inferenceTask(void *pvParameters) {
    printf("Starting inference task\n");
    TaskParams *params = reinterpret_cast<TaskParams *>(pvParameters);

    InferenceHandler process(tensorArena,
                             arenaSize,
                             params->inferenceInputQueue,
                             params->inferenceOutputQueue,
                             params->messageNotify,
                             params->networks);

    process.run();
}

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

void testPing(MessageClient client) {
    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_PING));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_PONG));
}

void testVersion(MessageClient client) {
    ethosu_core_msg_version ver;
    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_VERSION_REQ));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_VERSION_RSP, ver));

    TEST_ASSERT(ver.major == ETHOSU_CORE_MSG_VERSION_MAJOR);
    TEST_ASSERT(ver.minor == ETHOSU_CORE_MSG_VERSION_MINOR);
    TEST_ASSERT(ver.patch == ETHOSU_CORE_MSG_VERSION_PATCH);
}

void readCapabilities(ethosu_core_msg_capabilities_rsp &rsp) {
#ifdef ETHOSU
    struct ethosu_driver_version version;
    ethosu_get_driver_version(&version);

    struct ethosu_hw_info info;
    struct ethosu_driver *drv = ethosu_reserve_driver();
    ethosu_get_hw_info(drv, &info);
    ethosu_release_driver(drv);

    rsp.version_status     = info.version.version_status;
    rsp.version_minor      = info.version.version_minor;
    rsp.version_major      = info.version.version_major;
    rsp.product_major      = info.version.product_major;
    rsp.arch_patch_rev     = info.version.arch_patch_rev;
    rsp.arch_minor_rev     = info.version.arch_minor_rev;
    rsp.arch_major_rev     = info.version.arch_major_rev;
    rsp.driver_patch_rev   = version.patch;
    rsp.driver_minor_rev   = version.minor;
    rsp.driver_major_rev   = version.major;
    rsp.macs_per_cc        = info.cfg.macs_per_cc;
    rsp.cmd_stream_version = info.cfg.cmd_stream_version;
    rsp.custom_dma         = info.cfg.custom_dma;
#endif
}

void testCapabilities(MessageClient client) {
    const uint64_t fake_user_arg     = 42;
    ethosu_core_capabilities_req req = {fake_user_arg};
    ethosu_core_msg_capabilities_rsp expected_rsp;
    ethosu_core_msg_capabilities_rsp rsp;

    readCapabilities(expected_rsp);
    expected_rsp.user_arg = req.user_arg;

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_CAPABILITIES_REQ, req));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_CAPABILITIES_RSP, rsp));

    TEST_ASSERT(expected_rsp.version_status == rsp.version_status);
    TEST_ASSERT(expected_rsp.version_minor == rsp.version_minor);
    TEST_ASSERT(expected_rsp.version_major == rsp.version_major);
    TEST_ASSERT(expected_rsp.product_major == rsp.product_major);
    TEST_ASSERT(expected_rsp.arch_patch_rev == rsp.arch_patch_rev);
    TEST_ASSERT(expected_rsp.arch_minor_rev == rsp.arch_minor_rev);
    TEST_ASSERT(expected_rsp.arch_major_rev == rsp.arch_major_rev);
    TEST_ASSERT(expected_rsp.driver_patch_rev == rsp.driver_patch_rev);
    TEST_ASSERT(expected_rsp.driver_minor_rev == rsp.driver_minor_rev);
    TEST_ASSERT(expected_rsp.driver_major_rev == rsp.driver_major_rev);
    TEST_ASSERT(expected_rsp.macs_per_cc == rsp.macs_per_cc);
    TEST_ASSERT(expected_rsp.cmd_stream_version == rsp.cmd_stream_version);
    TEST_ASSERT(expected_rsp.custom_dma == rsp.custom_dma);

#ifdef ETHOSU
    TEST_ASSERT(rsp.version_status > 0);
    TEST_ASSERT(rsp.product_major > 0);
    TEST_ASSERT(rsp.arch_major_rev > 0 || rsp.arch_minor_rev > 0 || rsp.arch_patch_rev > 0);
    TEST_ASSERT(rsp.driver_major_rev > 0 || rsp.driver_minor_rev > 0 || rsp.driver_patch_rev > 0);
    TEST_ASSERT(rsp.macs_per_cc > 0);
#endif
}

void testNetworkInfoIndex(MessageClient client) {
    const uint64_t fake_user_arg     = 42;
    const uint32_t network_index     = 0;
    ethosu_core_network_info_req req = networkInfoIndexedRequest(fake_user_arg, network_index);
    ethosu_core_network_info_rsp rsp;
    ethosu_core_network_info_rsp expected_rsp = networkInfoResponse(fake_user_arg);

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_NETWORK_INFO_REQ, req));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_NETWORK_INFO_RSP, rsp));

    TEST_ASSERT(expected_rsp.user_arg == rsp.user_arg);
    TEST_ASSERT(std::strncmp(expected_rsp.desc, rsp.desc, sizeof(rsp.desc)) == 0);
    TEST_ASSERT(expected_rsp.ifm_count == rsp.ifm_count);
    TEST_ASSERT(expected_rsp.ofm_count == rsp.ofm_count);
    TEST_ASSERT(expected_rsp.status == rsp.status);
}

void testNetworkInfoNonExistantIndex(MessageClient client) {
    const uint64_t fake_user_arg     = 42;
    const uint32_t network_index     = 1;
    ethosu_core_network_info_req req = networkInfoIndexedRequest(fake_user_arg, network_index);
    ethosu_core_network_info_rsp rsp;

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_NETWORK_INFO_REQ, req));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_NETWORK_INFO_RSP, rsp));

    TEST_ASSERT(fake_user_arg == rsp.user_arg);
    TEST_ASSERT(ETHOSU_CORE_STATUS_ERROR == rsp.status);
}

void testNetworkInfoBuffer(MessageClient client) {
    const uint64_t fake_user_arg     = 42;
    uint32_t size                    = sizeof(Model0::networkModelData);
    unsigned char *ptr               = Model0::networkModelData;
    ethosu_core_network_info_req req = networkInfoBufferRequest(fake_user_arg, ptr, size);
    ethosu_core_network_info_rsp rsp;
    ethosu_core_network_info_rsp expected_rsp = networkInfoResponse(fake_user_arg);

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_NETWORK_INFO_REQ, req));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_NETWORK_INFO_RSP, rsp));

    TEST_ASSERT(expected_rsp.user_arg == rsp.user_arg);
    TEST_ASSERT(std::strncmp(expected_rsp.desc, rsp.desc, sizeof(rsp.desc)) == 0);
    TEST_ASSERT(expected_rsp.ifm_count == rsp.ifm_count);
    TEST_ASSERT(expected_rsp.ofm_count == rsp.ofm_count);
    TEST_ASSERT(expected_rsp.status == rsp.status);
}

void testNetworkInfoUnparsableBuffer(MessageClient client) {
    const uint64_t fake_user_arg     = 42;
    uint32_t size                    = sizeof(Model0::networkModelData) / 4;
    unsigned char *ptr               = Model0::networkModelData + size;
    ethosu_core_network_info_req req = networkInfoBufferRequest(fake_user_arg, ptr, size);
    ethosu_core_network_info_rsp rsp;

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_NETWORK_INFO_REQ, req));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_NETWORK_INFO_RSP, rsp));

    TEST_ASSERT(42 == rsp.user_arg);
    TEST_ASSERT(ETHOSU_CORE_STATUS_ERROR == rsp.status);
}

void testInferenceRunIndex(MessageClient client) {
    const uint64_t fake_user_arg = 42;
    const uint32_t network_index = 0;
    uint8_t data[sizeof(expectedOutputData)];
    ethosu_core_inference_req req =
        inferenceIndexedRequest(fake_user_arg, network_index, inputData, sizeof(inputData), data, sizeof(data));
    ethosu_core_inference_rsp rsp;

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_INFERENCE_REQ, req));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_INFERENCE_RSP, rsp));

    TEST_ASSERT(req.user_arg == rsp.user_arg);
    TEST_ASSERT(rsp.ofm_count == 1);
    TEST_ASSERT(std::memcmp(expectedOutputData, data, sizeof(expectedOutputData)) == 0);
    TEST_ASSERT(rsp.status == ETHOSU_CORE_STATUS_OK);
    TEST_ASSERT(rsp.pmu_cycle_counter_enable == req.pmu_cycle_counter_enable);
    TEST_ASSERT(std::memcmp(rsp.pmu_event_config, req.pmu_event_config, sizeof(req.pmu_event_config)) == 0);
}

void testInferenceRunNonExistingIndex(MessageClient client) {
    const uint64_t fake_user_arg = 42;
    const uint32_t network_index = 1;
    uint8_t data[sizeof(expectedOutputData)];
    ethosu_core_inference_req req =
        inferenceIndexedRequest(fake_user_arg, network_index, inputData, sizeof(inputData), data, sizeof(data));
    ethosu_core_inference_rsp rsp;

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_INFERENCE_REQ, req));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_INFERENCE_RSP, rsp));

    TEST_ASSERT(req.user_arg == rsp.user_arg);
    TEST_ASSERT(rsp.status == ETHOSU_CORE_STATUS_ERROR);
}

void testInferenceRunBuffer(MessageClient client) {
    const uint64_t fake_user_arg = 42;
    uint32_t network_size        = sizeof(Model0::networkModelData);
    unsigned char *network_ptr   = Model0::networkModelData;
    uint8_t data[sizeof(expectedOutputData)];
    ethosu_core_inference_req req = inferenceBufferRequest(
        fake_user_arg, network_ptr, network_size, inputData, sizeof(inputData), data, sizeof(data));
    ethosu_core_inference_rsp rsp;

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_INFERENCE_REQ, req));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_INFERENCE_RSP, rsp));

    TEST_ASSERT(req.user_arg == rsp.user_arg);
    TEST_ASSERT(rsp.ofm_count == 1);
    TEST_ASSERT(std::memcmp(expectedOutputData, data, sizeof(expectedOutputData)) == 0);
    TEST_ASSERT(rsp.status == ETHOSU_CORE_STATUS_OK);
    TEST_ASSERT(rsp.pmu_cycle_counter_enable == req.pmu_cycle_counter_enable);
    TEST_ASSERT(std::memcmp(rsp.pmu_event_config, req.pmu_event_config, sizeof(req.pmu_event_config)) == 0);
}

void testInferenceRunUnparsableBuffer(MessageClient client) {
    const uint64_t fake_user_arg = 42;
    uint32_t network_size        = sizeof(Model0::networkModelData) / 4;
    unsigned char *network_ptr   = Model0::networkModelData + network_size;
    uint8_t data[sizeof(expectedOutputData)];
    ethosu_core_inference_req req = inferenceBufferRequest(
        fake_user_arg, network_ptr, network_size, inputData, sizeof(inputData), data, sizeof(data));
    ethosu_core_inference_rsp rsp;

    TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_INFERENCE_REQ, req));
    TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_INFERENCE_RSP, rsp));

    TEST_ASSERT(req.user_arg == rsp.user_arg);
    TEST_ASSERT(rsp.status == ETHOSU_CORE_STATUS_ERROR);
}

void testSequentiallyQueuedInferenceRuns(MessageClient client) {
    int runs = 5;
    uint8_t data[runs][sizeof(expectedOutputData)];
    const uint64_t fake_user_arg = 42;
    const uint32_t network_index = 0;
    ethosu_core_inference_req req;
    ethosu_core_inference_rsp rsp[runs];

    for (int i = 0; i < runs; i++) {
        vTaskDelay(150);

        req = inferenceIndexedRequest(
            fake_user_arg + i, network_index, inputData, sizeof(inputData), data[i], sizeof(data[i]));
        TEST_ASSERT(client.sendInputMessage(ETHOSU_CORE_MSG_INFERENCE_REQ, req));
    }

    for (int i = 0; i < runs; i++) {
        TEST_ASSERT(client.waitAndReadOutputMessage(ETHOSU_CORE_MSG_INFERENCE_RSP, rsp[i]));
        TEST_ASSERT(uint64_t(fake_user_arg + i) == rsp[i].user_arg);
        TEST_ASSERT(rsp[i].ofm_count == 1);
        TEST_ASSERT(std::memcmp(expectedOutputData, data[i], sizeof(expectedOutputData)) == 0);
        TEST_ASSERT(rsp[i].status == ETHOSU_CORE_STATUS_OK);
        TEST_ASSERT(rsp[i].pmu_cycle_counter_enable == req.pmu_cycle_counter_enable);
        TEST_ASSERT(std::memcmp(rsp[i].pmu_event_config, req.pmu_event_config, sizeof(req.pmu_event_config)) == 0);
    }
}

void clientTask(void *) {
    printf("Starting client task\n");

    MessageClient client(*inputMessageQueue.toQueue(), *outputMessageQueue.toQueue(), mailbox);

    vTaskDelay(50);

    testPing(client);
    testVersion(client);
    testCapabilities(client);
    testNetworkInfoIndex(client);
    testNetworkInfoNonExistantIndex(client);
    testNetworkInfoBuffer(client);
    testNetworkInfoUnparsableBuffer(client);
    testInferenceRunIndex(client);
    testInferenceRunNonExistingIndex(client);
    testInferenceRunBuffer(client);
    testInferenceRunUnparsableBuffer(client);
    testSequentiallyQueuedInferenceRuns(client);

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

    ret = xTaskCreate(inferenceTask, "inferenceTask", 8 * 1024, &taskParams, 3, nullptr);
    if (ret != pdPASS) {
        printf("Failed to create 'inferenceTask'\n");
        return ret;
    }

    // Task for handling incoming /outgoing messages from the remote host
    ret = xTaskCreate(clientTask, "clientTask", 1024, nullptr, 2, nullptr);
    if (ret != pdPASS) {
        printf("Failed to create 'messageTask'\n");
        return ret;
    }

    // Start Scheduler
    vTaskStartScheduler();

    return 1;
}
