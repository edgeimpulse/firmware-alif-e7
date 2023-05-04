/*
 * Copyright (c) 2019-2022 Arm Limited. All rights reserved.
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
#include "portmacro.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include <inttypes.h>
#include <stdio.h>
#include <vector>

#include "inference_process.hpp"

// Model data (Defined & changable by modifiying compile definition in CMakeLists.txt)
#include "input.h"
#include "model.h"
#include "output.h"

using namespace std;
using namespace InferenceProcess;

/****************************************************************************
 * Defines
 ****************************************************************************/

// Nr. of tasks to process inferences with. Task reserves driver & runs inference (Normally 1 per NPU, but not a must)
#define NUM_INFERENCE_TASKS 1
// Nr. of tasks to create jobs and recieve responses
#define NUM_JOB_TASKS 2
// Nr. of jobs to create per job task
#define NUM_JOBS_PER_TASK 1

// Tensor arena size
#ifdef TENSOR_ARENA_SIZE // If defined in model.h
#define TENSOR_ARENA_SIZE_PER_INFERENCE TENSOR_ARENA_SIZE
#else // If not defined, use maximum available
#define TENSOR_ARENA_SIZE_PER_INFERENCE 2000000 / NUM_INFERENCE_TASKS
#endif

/****************************************************************************
 * Override new operators to call in FreeRTOS allocator
 ****************************************************************************/

void *operator new(size_t size) {
    return pvPortMalloc(size);
}

void *operator new[](size_t size) {
    return pvPortMalloc(size);
}

void operator delete(void *ptr) {
    vPortFree(ptr);
}

void operator delete(void *ptr, std::size_t) {
    vPortFree(ptr);
}

void operator delete[](void *ptr) {
    vPortFree(ptr);
}

void operator delete[](void *ptr, std::size_t) {
    vPortFree(ptr);
}

/****************************************************************************
 * InferenceJob
 ****************************************************************************/

struct ProcessTaskParams {
    ProcessTaskParams() : queueHandle(nullptr), tensorArena(nullptr), arenaSize(0) {}
    ProcessTaskParams(QueueHandle_t _queue, uint8_t *_tensorArena, size_t _arenaSize) :
        queueHandle(_queue), tensorArena(_tensorArena), arenaSize(_arenaSize) {}

    QueueHandle_t queueHandle;
    uint8_t *tensorArena;
    size_t arenaSize;
};

namespace {
// Number of total completed jobs, needed to exit application correctly if NUM_JOB_TASKS > 1
int totalCompletedJobs = 0;

// TensorArena static initialisation
const size_t arenaSize = TENSOR_ARENA_SIZE_PER_INFERENCE;

// Declare below variables in global scope to avoid stack since FreeRTOS resets stack when the scheduler is started
QueueHandle_t inferenceProcessQueue;
ProcessTaskParams taskParams[NUM_INFERENCE_TASKS];
} // namespace

__attribute__((section(".bss.tensor_arena"), aligned(16)))
uint8_t inferenceProcessTensorArena[NUM_INFERENCE_TASKS][arenaSize];

// Wrapper around InferenceProcess::InferenceJob. Adds responseQueue and status for FreeRTOS multi-tasking purposes.
struct xInferenceJob : public InferenceJob {
    QueueHandle_t responseQueue;
    bool status;

    xInferenceJob() : InferenceJob(), responseQueue(nullptr), status(false) {}
    xInferenceJob(const string &_name,
                  const DataPtr &_networkModel,
                  const vector<DataPtr> &_input,
                  const vector<DataPtr> &_output,
                  const vector<DataPtr> &_expectedOutput,
                  const size_t _numBytesToPrint,
                  void *_userArg,
                  QueueHandle_t _queue) :
        InferenceJob(_name, _networkModel, _input, _output, _expectedOutput, _numBytesToPrint, _userArg),
        responseQueue(_queue), status(false) {}
};

/****************************************************************************
 * Mutex & Semaphore
 * Overrides weak-linked symbols in ethosu_driver.c to implement thread handling
 ****************************************************************************/

extern "C" {

void *ethosu_mutex_create(void) {
    SemaphoreHandle_t sem = xSemaphoreCreateMutex();
    if (sem == NULL) {
        printf("Error: Failed to create mutex.\n");
    }
    return (void *)sem;
}

int ethosu_mutex_lock(void *mutex) {
    SemaphoreHandle_t handle = reinterpret_cast<SemaphoreHandle_t>(mutex);
    if (xSemaphoreTake(handle, portMAX_DELAY) != pdTRUE) {
        printf("Error: Failed to lock mutex.\n");
        return -1;
    }
    return 0;
}

int ethosu_mutex_unlock(void *mutex) {
    SemaphoreHandle_t handle = reinterpret_cast<SemaphoreHandle_t>(mutex);
    if (xSemaphoreGive(handle) != pdTRUE) {
        printf("Error: Failed to unlock mutex.\n");
        return -1;
    }
    return 0;
}

void *ethosu_semaphore_create(void) {
    SemaphoreHandle_t sem = xSemaphoreCreateBinary();
    if (sem == NULL) {
        printf("Error: Failed to create semaphore.\n");
    }
    return (void *)sem;
}

int ethosu_semaphore_take(void *sem) {
    SemaphoreHandle_t handle = reinterpret_cast<SemaphoreHandle_t>(sem);
    if (xSemaphoreTake(handle, portMAX_DELAY) != pdTRUE) {
        printf("Error: Failed to take semaphore.\n");
        return -1;
    }
    return 0;
}

int ethosu_semaphore_give(void *sem) {
    SemaphoreHandle_t handle = reinterpret_cast<SemaphoreHandle_t>(sem);

    if (xPortIsInsideInterrupt()) {
        if (xSemaphoreGiveFromISR(handle, NULL) != pdTRUE) {
            printf("Error: Failed to give semaphore from ISR.\n");
            return -1;
        }
    } else {
        /* A FreeRTOS binary semaphore is fundamentally a queue that can only hold one item. If the queue is full,
         * xSemaphoreGive will return a pdFALSE value. Ignoring the return value in here, as a semaphore give failure
         * does not affect the application correctness. */
        if (xSemaphoreGive(handle) != pdTRUE) {
            // do nothing
        }
    }
    return 0;
}
}

/****************************************************************************
 * Functions
 ****************************************************************************/

//  inferenceProcessTask - Run jobs from queue with available driver
void inferenceProcessTask(void *pvParameters) {
    ProcessTaskParams params = *reinterpret_cast<ProcessTaskParams *>(pvParameters);

    class InferenceProcess inferenceProcess(params.tensorArena, params.arenaSize);

    for (;;) {
        xInferenceJob *xJob;

        if (xQueueReceive(params.queueHandle, &xJob, portMAX_DELAY) != pdPASS) {
            printf("Error: inferenceProcessTask failed in receive from Q.\n");
            exit(1);
        }

        bool status  = inferenceProcess.runJob(*xJob);
        xJob->status = status;
        if (xQueueSend(xJob->responseQueue, &xJob, portMAX_DELAY) != pdPASS) {
            printf("Error: inferenceProcessTask failed in send to Q.\n");
            exit(1);
        }
    }
    vTaskDelete(nullptr);
}

//  inferenceSenderTask - Creates NUM_INFERNECE_JOBS jobs, queues them, and then listens for completion status
void inferenceSenderTask(void *pvParameters) {
    int ret = 0;

    QueueHandle_t _inferenceProcessQueue = reinterpret_cast<QueueHandle_t>(pvParameters);
    xInferenceJob jobs[NUM_JOBS_PER_TASK];

    // Create queue for response messages
    QueueHandle_t senderQueue = xQueueCreate(NUM_JOBS_PER_TASK, sizeof(xInferenceJob *));

    // Create and queue the jobs
    for (int n = 0; n < NUM_JOBS_PER_TASK; n++) {
        // Create job
        xInferenceJob *job = &jobs[n];
        job->name          = string(modelName);
        job->networkModel  = DataPtr(networkModelData, sizeof(networkModelData));
        job->input.push_back(DataPtr(inputData, sizeof(inputData)));
        job->expectedOutput.push_back(DataPtr(expectedOutputData, sizeof(expectedOutputData)));
        job->responseQueue = senderQueue;
        // Send job
        printf("inferenceSenderTask: Sending inference job: job=%p, name=%s\n", job, job->name.c_str());
        if (xQueueSend(_inferenceProcessQueue, &job, portMAX_DELAY) != pdPASS) {
            printf("Error: inferenceSenderTask failed in send to Q.\n");
            exit(1);
        }
    }

    // Listen for completion status
    do {
        xInferenceJob *pSendJob;
        if (xQueueReceive(senderQueue, &pSendJob, portMAX_DELAY) != pdPASS) {
            printf("Error: inferenceSenderTask failed in receive from Q.\n");
            exit(1);
        }
        printf("inferenceSenderTask: received response for job: %s, status = %u\n",
               pSendJob->name.c_str(),
               pSendJob->status);

        totalCompletedJobs++;
        ret = (pSendJob->status);
        if (pSendJob->status != 0) {
            break;
        }
    } while (totalCompletedJobs < NUM_JOBS_PER_TASK * NUM_JOB_TASKS);

    vQueueDelete(senderQueue);

    printf("FreeRTOS application returning %d.\n", ret);
    exit(ret);
}

/****************************************************************************
 * Application
 ****************************************************************************/
// FreeRTOS application. NOTE: Additional tasks may require increased heap size.
int main() {
    BaseType_t ret;
    inferenceProcessQueue = xQueueCreate(NUM_JOBS_PER_TASK, sizeof(xInferenceJob *));

    // inferenceSender tasks to create and queue the jobs
    for (int n = 0; n < NUM_JOB_TASKS; n++) {
        ret = xTaskCreate(inferenceSenderTask, "inferenceSenderTask", 2 * 1024, inferenceProcessQueue, 2, nullptr);
        if (ret != pdPASS) {
            printf("Error: Failed to create 'inferenceSenderTask%i'\n", n);
            exit(1);
        }
    }

    // Create inferenceProcess tasks to process the queued jobs
    for (int n = 0; n < NUM_INFERENCE_TASKS; n++) {
        taskParams[n] = ProcessTaskParams(inferenceProcessQueue, inferenceProcessTensorArena[n], arenaSize);
        ret           = xTaskCreate(inferenceProcessTask, "inferenceProcessTask", 8 * 1024, &taskParams[n], 3, nullptr);
        if (ret != pdPASS) {
            printf("Error: Failed to create 'inferenceProcessTask%i'\n", n);
            exit(1);
        }
    }

    // Start Scheduler
    vTaskStartScheduler();

    printf("Error: FreeRTOS application failed to initialise.\n");
    exit(1);

    return 0;
}
