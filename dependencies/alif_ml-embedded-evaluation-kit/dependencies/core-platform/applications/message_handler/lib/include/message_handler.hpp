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

#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "message_queue.hpp"
#include "networks.hpp"
#include <ethosu_core_interface.h>
#if defined(ETHOSU)
#include <ethosu_driver.h>
#endif
#include <inference_parser.hpp>
#include <inference_process.hpp>
#include <mailbox.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <inttypes.h>
#include <list>
#include <vector>

namespace MessageHandler {

template <typename T, size_t capacity = 5>
class Queue {
public:
    using Predicate = std::function<bool(const T &data)>;

    Queue() {
        mutex = xSemaphoreCreateMutex();
        size  = xSemaphoreCreateCounting(capacity, 0u);

        if (mutex == nullptr || size == nullptr) {
            printf("Error: failed to allocate memory for inference queue\n");
        }
    }

    ~Queue() {
        vSemaphoreDelete(mutex);
        vSemaphoreDelete(size);
    }

    bool push(const T &data) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        if (list.size() >= capacity) {
            xSemaphoreGive(mutex);
            return false;
        }

        list.push_back(data);
        xSemaphoreGive(mutex);

        // increase number of available inferences to pop
        xSemaphoreGive(size);
        return true;
    }

    void pop(T &data) {
        // decrease the number of available inferences to pop
        xSemaphoreTake(size, portMAX_DELAY);

        xSemaphoreTake(mutex, portMAX_DELAY);
        data = list.front();
        list.pop_front();
        xSemaphoreGive(mutex);
    }

    bool erase(Predicate pred) {
        // let's optimistically assume we are removing an inference, so decrease pop
        if (pdFALSE == xSemaphoreTake(size, 0)) {
            // if there are no inferences return immediately
            return false;
        }

        xSemaphoreTake(mutex, portMAX_DELAY);
        auto found  = std::find_if(list.begin(), list.end(), pred);
        bool erased = found != list.end();
        if (erased) {
            list.erase(found);
        }
        xSemaphoreGive(mutex);

        if (!erased) {
            // no inference erased, so let's put the size count back
            xSemaphoreGive(size);
        }

        return erased;
    }

private:
    std::list<T> list;

    SemaphoreHandle_t mutex;
    SemaphoreHandle_t size;
};

class IncomingMessageHandler {
public:
    IncomingMessageHandler(EthosU::ethosu_core_queue &inputMessageQueue,
                           EthosU::ethosu_core_queue &outputMessageQueue,
                           Mailbox::Mailbox &mailbox,
                           std::shared_ptr<Queue<EthosU::ethosu_core_inference_req>> inferenceInputQueue,
                           QueueHandle_t inferenceOutputQueue,
                           SemaphoreHandle_t messageNotify,
                           std::shared_ptr<Networks> networks);

    void run();

private:
    bool handleMessage();
    bool handleInferenceOutput();
    static void handleIrq(void *userArg);

    void sendPong();
    void sendErrorAndResetQueue(EthosU::ethosu_core_msg_err_type type, const char *message);
    void sendVersionRsp();
    void sendCapabilitiesRsp(uint64_t userArg);
    void sendNetworkInfoRsp(uint64_t userArg, EthosU::ethosu_core_network_buffer &network);
    void sendInferenceRsp(EthosU::ethosu_core_inference_rsp &inference);
    void sendFailedInferenceRsp(uint64_t userArg, uint32_t status);
    void sendCancelInferenceRsp(uint64_t userArg, uint32_t status);
    void readCapabilties(EthosU::ethosu_core_msg_capabilities_rsp &rsp);

    MessageQueue::QueueImpl inputMessageQueue;
    MessageQueue::QueueImpl outputMessageQueue;
    Mailbox::Mailbox &mailbox;
    InferenceProcess::InferenceParser parser;
    std::shared_ptr<Queue<EthosU::ethosu_core_inference_req>> inferenceInputQueue;
    QueueHandle_t inferenceOutputQueue;
    SemaphoreHandle_t messageNotify;
    EthosU::ethosu_core_msg_capabilities_rsp capabilities;
    std::shared_ptr<Networks> networks;
};

class InferenceHandler {
public:
    InferenceHandler(uint8_t *tensorArena,
                     size_t arenaSize,
                     std::shared_ptr<Queue<EthosU::ethosu_core_inference_req>> inferenceInputQueue,
                     QueueHandle_t inferenceOutputQueue,
                     SemaphoreHandle_t messageNotify,
                     std::shared_ptr<Networks> networks);

    void run();

private:
    void runInference(EthosU::ethosu_core_inference_req &req, EthosU::ethosu_core_inference_rsp &rsp);
    bool getInferenceJob(const EthosU::ethosu_core_inference_req &req, InferenceProcess::InferenceJob &job);

#if defined(ETHOSU)
    friend void ::ethosu_inference_begin(struct ethosu_driver *drv, void *userArg);
    friend void ::ethosu_inference_end(struct ethosu_driver *drv, void *userArg);
#endif
    std::shared_ptr<Queue<EthosU::ethosu_core_inference_req>> inferenceInputQueue;
    QueueHandle_t inferenceOutputQueue;
    SemaphoreHandle_t messageNotify;
    InferenceProcess::InferenceProcess inference;
    EthosU::ethosu_core_inference_req *currentReq;
    EthosU::ethosu_core_inference_rsp *currentRsp;
    std::shared_ptr<Networks> networks;
};

} // namespace MessageHandler

#endif
