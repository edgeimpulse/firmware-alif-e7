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

#include "message_handler.hpp"

#include "cmsis_compiler.h"

#ifdef ETHOSU
#include <ethosu_driver.h>
#include <pmu_ethosu.h>
#endif

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include <cstring>
#include <vector>

using namespace EthosU;
using namespace MessageQueue;

namespace MessageHandler {

/****************************************************************************
 * IncomingMessageHandler
 ****************************************************************************/

IncomingMessageHandler::IncomingMessageHandler(
    EthosU::ethosu_core_queue &_inputMessageQueue,
    EthosU::ethosu_core_queue &_outputMessageQueue,
    Mailbox::Mailbox &_mailbox,
    std::shared_ptr<Queue<EthosU::ethosu_core_inference_req>> _inferenceInputQueue,
    QueueHandle_t _inferenceOutputQueue,
    SemaphoreHandle_t _messageNotify,
    std::shared_ptr<Networks> _networks) :
    inputMessageQueue(_inputMessageQueue),
    outputMessageQueue(_outputMessageQueue), mailbox(_mailbox), inferenceInputQueue(_inferenceInputQueue),
    inferenceOutputQueue(_inferenceOutputQueue), messageNotify(_messageNotify), networks(_networks) {
    mailbox.registerCallback(handleIrq, reinterpret_cast<void *>(this));
    readCapabilties(capabilities);
}

void IncomingMessageHandler::run() {
    while (true) {
        // Wait for event
        xSemaphoreTake(messageNotify, portMAX_DELAY);

        // Handle all inference outputs and all messages in queue
        while (handleInferenceOutput() || handleMessage()) {}
    }
}

void IncomingMessageHandler::handleIrq(void *userArg) {
    if (userArg == nullptr) {
        return;
    }
    IncomingMessageHandler *_this = reinterpret_cast<IncomingMessageHandler *>(userArg);
    xSemaphoreGiveFromISR(_this->messageNotify, nullptr);
}

void IncomingMessageHandler::sendErrorAndResetQueue(ethosu_core_msg_err_type type, const char *message) {
    ethosu_core_msg_err error;
    error.type = type;

    for (size_t i = 0; i < sizeof(error.msg) && message[i]; i++) {
        error.msg[i] = message[i];
    }
    printf("ERROR: Msg: \"%s\"\n", error.msg);

    if (!outputMessageQueue.write(ETHOSU_CORE_MSG_ERR, error)) {
        printf("ERROR: Msg: Failed to write error response. No mailbox message sent\n");
    } else {
        mailbox.sendMessage();
    }
    inputMessageQueue.reset();
}

bool IncomingMessageHandler::handleInferenceOutput() {
    struct ethosu_core_inference_rsp rsp;
    if (pdTRUE != xQueueReceive(inferenceOutputQueue, &rsp, 0)) {
        return false;
    }

    sendInferenceRsp(rsp);
    return true;
}

bool IncomingMessageHandler::handleMessage() {
    struct ethosu_core_msg msg;

    if (inputMessageQueue.available() == 0) {
        return false;
    }

    // Read msg header
    // Only process a complete message header, else send error message
    // and reset queue
    if (!inputMessageQueue.read(msg)) {
        sendErrorAndResetQueue(ETHOSU_CORE_MSG_ERR_INVALID_SIZE, "Failed to read a complete header");
        return false;
    }

    printf("Msg: header magic=%" PRIX32 ", type=%" PRIu32 ", length=%" PRIu32 "\n", msg.magic, msg.type, msg.length);

    if (msg.magic != ETHOSU_CORE_MSG_MAGIC) {
        printf("Msg: Invalid Magic\n");
        sendErrorAndResetQueue(ETHOSU_CORE_MSG_ERR_INVALID_MAGIC, "Invalid magic");
        return false;
    }

    switch (msg.type) {
    case ETHOSU_CORE_MSG_PING: {
        printf("Msg: Ping\n");
        sendPong();
        break;
    }
    case ETHOSU_CORE_MSG_ERR: {
        ethosu_core_msg_err error;
        if (!inputMessageQueue.read(error)) {
            printf("ERROR: Msg: Failed to receive error message\n");
        } else {
            printf("Msg: Received an error response, type=%" PRIu32 ", msg=\"%s\"\n", error.type, error.msg);
        }

        inputMessageQueue.reset();
        return false;
    }
    case ETHOSU_CORE_MSG_VERSION_REQ: {
        printf("Msg: Version request\n");
        sendVersionRsp();
        break;
    }
    case ETHOSU_CORE_MSG_CAPABILITIES_REQ: {
        ethosu_core_capabilities_req req;
        if (!inputMessageQueue.read(req)) {
            sendErrorAndResetQueue(ETHOSU_CORE_MSG_ERR_INVALID_PAYLOAD, "CapabilitiesReq. Failed to read payload");
            break;
        }

        printf("Msg: Capabilities request.user_arg=0x%" PRIx64 "\n", req.user_arg);
        sendCapabilitiesRsp(req.user_arg);
        break;
    }
    case ETHOSU_CORE_MSG_INFERENCE_REQ: {
        ethosu_core_inference_req req;
        if (!inputMessageQueue.read(req)) {
            sendErrorAndResetQueue(ETHOSU_CORE_MSG_ERR_INVALID_PAYLOAD, "InferenceReq. Failed to read payload");
            break;
        }

        printf("Msg: InferenceReq. user_arg=0x%" PRIx64 ", network_type=%" PRIu32 ", ", req.user_arg, req.network.type);

        if (req.network.type == ETHOSU_CORE_NETWORK_BUFFER) {
            printf("network.buffer={0x%" PRIx32 ", %" PRIu32 "},\n", req.network.buffer.ptr, req.network.buffer.size);
        } else {
            printf("network.index=%" PRIu32 ",\n", req.network.index);
        }

        printf("ifm_count=%" PRIu32 ", ifm=[", req.ifm_count);
        for (uint32_t i = 0; i < req.ifm_count; ++i) {
            if (i > 0) {
                printf(", ");
            }

            printf("{0x%" PRIx32 ", %" PRIu32 "}", req.ifm[i].ptr, req.ifm[i].size);
        }
        printf("]");

        printf(", ofm_count=%" PRIu32 ", ofm=[", req.ofm_count);
        for (uint32_t i = 0; i < req.ofm_count; ++i) {
            if (i > 0) {
                printf(", ");
            }

            printf("{0x%" PRIx32 ", %" PRIu32 "}", req.ofm[i].ptr, req.ofm[i].size);
        }
        printf("]\n");

        if (!inferenceInputQueue->push(req)) {
            printf("Msg: Inference queue full. Rejecting inference user_arg=0x%" PRIx64 "\n", req.user_arg);
            sendFailedInferenceRsp(req.user_arg, ETHOSU_CORE_STATUS_REJECTED);
        }
        break;
    }
    case ETHOSU_CORE_MSG_CANCEL_INFERENCE_REQ: {
        ethosu_core_cancel_inference_req req;
        if (!inputMessageQueue.read(req)) {
            sendErrorAndResetQueue(ETHOSU_CORE_MSG_ERR_INVALID_PAYLOAD, "CancelInferenceReq. Failed to read payload");
            break;
        }
        printf("Msg: CancelInferenceReq. user_arg=0x%" PRIx64 ", inference_handle=0x%" PRIx64 "\n",
               req.user_arg,
               req.inference_handle);

        bool found =
            inferenceInputQueue->erase([req](auto &inf_req) { return inf_req.user_arg == req.inference_handle; });

        // NOTE: send an inference response with status ABORTED if the inference has been droped from the queue
        if (found) {
            sendFailedInferenceRsp(req.inference_handle, ETHOSU_CORE_STATUS_ABORTED);
        }

        sendCancelInferenceRsp(req.user_arg, found ? ETHOSU_CORE_STATUS_OK : ETHOSU_CORE_STATUS_ERROR);
        break;
    }
    case ETHOSU_CORE_MSG_NETWORK_INFO_REQ: {
        ethosu_core_network_info_req req;
        if (!inputMessageQueue.read(req)) {
            sendErrorAndResetQueue(ETHOSU_CORE_MSG_ERR_INVALID_PAYLOAD, "NetworkInfoReq. Failed to read payload");
            break;
        }

        printf("Msg: NetworkInfoReq. user_arg=0x%" PRIx64 "\n", req.user_arg);
        sendNetworkInfoRsp(req.user_arg, req.network);
        break;
    }
    default: {
        char errMsg[128];
        snprintf(&errMsg[0],
                 sizeof(errMsg),
                 "Msg: Unknown type: %" PRIu32 " with payload length %" PRIu32 " bytes\n",
                 msg.type,
                 msg.length);

        sendErrorAndResetQueue(ETHOSU_CORE_MSG_ERR_UNSUPPORTED_TYPE, errMsg);
        return false;
    }
    }

    return true;
}

void IncomingMessageHandler::sendPong() {
    if (!outputMessageQueue.write(ETHOSU_CORE_MSG_PONG)) {
        printf("ERROR: Msg: Failed to write pong response. No mailbox message sent\n");
    } else {
        mailbox.sendMessage();
    }
}

void IncomingMessageHandler::sendVersionRsp() {
    ethosu_core_msg_version version = {
        ETHOSU_CORE_MSG_VERSION_MAJOR,
        ETHOSU_CORE_MSG_VERSION_MINOR,
        ETHOSU_CORE_MSG_VERSION_PATCH,
        0,
    };

    if (!outputMessageQueue.write(ETHOSU_CORE_MSG_VERSION_RSP, version)) {
        printf("ERROR: Failed to write version response. No mailbox message sent\n");
    } else {
        mailbox.sendMessage();
    }
}

void IncomingMessageHandler::sendCapabilitiesRsp(uint64_t userArg) {
    capabilities.user_arg = userArg;

    if (!outputMessageQueue.write(ETHOSU_CORE_MSG_CAPABILITIES_RSP, capabilities)) {
        printf("ERROR: Failed to write capabilities response. No mailbox message sent\n");
    } else {
        mailbox.sendMessage();
    }
}

void IncomingMessageHandler::sendNetworkInfoRsp(uint64_t userArg, ethosu_core_network_buffer &network) {
    ethosu_core_network_info_rsp rsp;
    rsp.user_arg  = userArg;
    rsp.ifm_count = 0;
    rsp.ofm_count = 0;

    void *buffer;
    size_t size;

    bool failed = networks->getNetwork(network, buffer, size);

    if (!failed) {
        failed = parser.parseModel(buffer,
                                   size,
                                   rsp.desc,
                                   InferenceProcess::makeArray(rsp.ifm_size, rsp.ifm_count, ETHOSU_CORE_BUFFER_MAX),
                                   InferenceProcess::makeArray(rsp.ofm_size, rsp.ofm_count, ETHOSU_CORE_BUFFER_MAX));
    }
    rsp.status = failed ? ETHOSU_CORE_STATUS_ERROR : ETHOSU_CORE_STATUS_OK;

    if (!outputMessageQueue.write(ETHOSU_CORE_MSG_NETWORK_INFO_RSP, rsp)) {
        printf("ERROR: Msg: Failed to write network info response. No mailbox message sent\n");
    } else {
        mailbox.sendMessage();
    }
}

void IncomingMessageHandler::sendInferenceRsp(ethosu_core_inference_rsp &rsp) {
    if (!outputMessageQueue.write(ETHOSU_CORE_MSG_INFERENCE_RSP, rsp)) {
        printf("ERROR: Msg: Failed to write inference response. No mailbox message sent\n");
    } else {
        mailbox.sendMessage();
    }
}

void IncomingMessageHandler::sendFailedInferenceRsp(uint64_t userArg, uint32_t status) {
    ethosu_core_inference_rsp rsp;
    rsp.user_arg = userArg;
    rsp.status   = status;
    if (!outputMessageQueue.write(ETHOSU_CORE_MSG_INFERENCE_RSP, rsp)) {
        printf("ERROR: Msg: Failed to write inference response. No mailbox message sent\n");
    } else {
        mailbox.sendMessage();
    }
}
void IncomingMessageHandler::sendCancelInferenceRsp(uint64_t userArg, uint32_t status) {
    ethosu_core_cancel_inference_rsp cancellation;
    cancellation.user_arg = userArg;
    cancellation.status   = status;
    if (!outputMessageQueue.write(ETHOSU_CORE_MSG_CANCEL_INFERENCE_RSP, cancellation)) {
        printf("ERROR: Msg: Failed to write cancel inference response. No mailbox message sent\n");
    } else {
        mailbox.sendMessage();
    }
}

void IncomingMessageHandler::readCapabilties(ethosu_core_msg_capabilities_rsp &rsp) {
    rsp = {};

#ifdef ETHOSU
    struct ethosu_driver_version version;
    ethosu_get_driver_version(&version);

    struct ethosu_hw_info info;
    struct ethosu_driver *drv = ethosu_reserve_driver();
    ethosu_get_hw_info(drv, &info);
    ethosu_release_driver(drv);

    rsp.user_arg           = 0;
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

/****************************************************************************
 * InferenceHandler
 ****************************************************************************/

InferenceHandler::InferenceHandler(uint8_t *tensorArena,
                                   size_t arenaSize,
                                   std::shared_ptr<Queue<EthosU::ethosu_core_inference_req>> _inferenceInputQueue,
                                   QueueHandle_t _inferenceOutputQueue,
                                   SemaphoreHandle_t _messageNotify,
                                   std::shared_ptr<Networks> _networks) :
    inferenceInputQueue(_inferenceInputQueue),
    inferenceOutputQueue(_inferenceOutputQueue), messageNotify(_messageNotify), inference(tensorArena, arenaSize),
    networks(_networks) {}

void InferenceHandler::run() {
    ethosu_core_inference_req req;
    ethosu_core_inference_rsp rsp;

    while (true) {
        inferenceInputQueue->pop(req);

        runInference(req, rsp);

        xQueueSend(inferenceOutputQueue, &rsp, portMAX_DELAY);
        xSemaphoreGive(messageNotify);
    }
}

void InferenceHandler::runInference(ethosu_core_inference_req &req, ethosu_core_inference_rsp &rsp) {
    currentReq = &req;
    currentRsp = &rsp;

    /*
     * Run inference
     */

    InferenceProcess::InferenceJob job;
    bool failed = getInferenceJob(req, job);

    if (!failed) {
        job.invalidate();
        failed = inference.runJob(job);
        job.clean();
    }

#if defined(ETHOSU)
    /*
     * Print PMU counters
     */

    if (!failed) {
        const int numEvents = std::min(static_cast<int>(ETHOSU_PMU_Get_NumEventCounters()), ETHOSU_CORE_PMU_MAX);

        for (int i = 0; i < numEvents; i++) {
            printf("ethosu_pmu_cntr%d : %" PRIu32 "\n", i, rsp.pmu_event_count[i]);
        }

        if (rsp.pmu_cycle_counter_enable) {
            printf("ethosu_pmu_cycle_cntr : %" PRIu64 " cycles\n", rsp.pmu_cycle_counter_count);
        }
    }
#endif

    /*
     * Send inference response
     */

    rsp.user_arg  = req.user_arg;
    rsp.ofm_count = job.output.size();
    rsp.status    = failed ? ETHOSU_CORE_STATUS_ERROR : ETHOSU_CORE_STATUS_OK;

    for (size_t i = 0; i < job.output.size(); ++i) {
        rsp.ofm_size[i] = job.output[i].size;
    }

    currentReq = nullptr;
    currentRsp = nullptr;
}

bool InferenceHandler::getInferenceJob(const ethosu_core_inference_req &req, InferenceProcess::InferenceJob &job) {
    bool failed = networks->getNetwork(req.network, job.networkModel.data, job.networkModel.size);
    if (failed) {
        return true;
    }

    for (uint32_t i = 0; i < req.ifm_count; ++i) {
        job.input.push_back(InferenceProcess::DataPtr(reinterpret_cast<void *>(req.ifm[i].ptr), req.ifm[i].size));
    }

    for (uint32_t i = 0; i < req.ofm_count; ++i) {
        job.output.push_back(InferenceProcess::DataPtr(reinterpret_cast<void *>(req.ofm[i].ptr), req.ofm[i].size));
    }

    job.externalContext = this;

    return false;
}

} // namespace MessageHandler

#if defined(ETHOSU)
extern "C" void ethosu_inference_begin(struct ethosu_driver *drv, void *userArg) {
    MessageHandler::InferenceHandler *self = static_cast<MessageHandler::InferenceHandler *>(userArg);

    // Calculate maximum number of events
    const int numEvents = std::min(static_cast<int>(ETHOSU_PMU_Get_NumEventCounters()), ETHOSU_CORE_PMU_MAX);

    // Enable PMU
    ETHOSU_PMU_Enable(drv);

    // Configure and enable events
    for (int i = 0; i < numEvents; i++) {
        ETHOSU_PMU_Set_EVTYPER(drv, i, static_cast<ethosu_pmu_event_type>(self->currentReq->pmu_event_config[i]));
        ETHOSU_PMU_CNTR_Enable(drv, 1 << i);
    }

    // Enable cycle counter
    if (self->currentReq->pmu_cycle_counter_enable) {
        ETHOSU_PMU_PMCCNTR_CFG_Set_Stop_Event(drv, ETHOSU_PMU_NPU_IDLE);
        ETHOSU_PMU_PMCCNTR_CFG_Set_Start_Event(drv, ETHOSU_PMU_NPU_ACTIVE);

        ETHOSU_PMU_CNTR_Enable(drv, ETHOSU_PMU_CCNT_Msk);
        ETHOSU_PMU_CYCCNT_Reset(drv);
    }

    // Reset all counters
    ETHOSU_PMU_EVCNTR_ALL_Reset(drv);
}

extern "C" void ethosu_inference_end(struct ethosu_driver *drv, void *userArg) {
    MessageHandler::InferenceHandler *self = static_cast<MessageHandler::InferenceHandler *>(userArg);

    // Get cycle counter
    self->currentRsp->pmu_cycle_counter_enable = self->currentReq->pmu_cycle_counter_enable;
    if (self->currentReq->pmu_cycle_counter_enable) {
        self->currentRsp->pmu_cycle_counter_count = ETHOSU_PMU_Get_CCNTR(drv);
    }

    // Calculate maximum number of events
    const int numEvents = std::min(static_cast<int>(ETHOSU_PMU_Get_NumEventCounters()), ETHOSU_CORE_PMU_MAX);

    // Get event counters
    int i;
    for (i = 0; i < numEvents; i++) {
        self->currentRsp->pmu_event_config[i] = self->currentReq->pmu_event_config[i];
        self->currentRsp->pmu_event_count[i]  = ETHOSU_PMU_Get_EVCNTR(drv, i);
    }

    for (; i < ETHOSU_CORE_PMU_MAX; i++) {
        self->currentRsp->pmu_event_config[i] = 0;
        self->currentRsp->pmu_event_count[i]  = 0;
    }

    // Disable PMU
    ETHOSU_PMU_Disable(drv);
}
#endif
