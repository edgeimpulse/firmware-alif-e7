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
#include "tx_api.h"

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
// Nr. of threads to process inferences with. Thread reserves driver & runs inference (Normally 1 per NPU, but not a
// must)
#define NUM_INFERENCE_THREADS 1
// Nr. of threads to create jobs and recieve responses
#define NUM_JOB_THREADS 2
// Nr. of jobs to create per job thread
#define NUM_JOBS_PER_THREAD 1

#define MAX_THREAD_NAME_SIZE 128

#define PROCESS_THREAD_STACK_SIZE   (32 * 1024)
#define SENDER_THREAD_STACK_SIZE    (2 * 1024)
#define PROCESS_THREAD_CONTEXT_SIZE (sizeof(TX_THREAD))
#define SENDER_THREAD_CONTEXT_SIZE  (sizeof(TX_THREAD))

// Tensor arena size
#ifdef TENSOR_ARENA_SIZE // If defined in model.h
#define TENSOR_ARENA_SIZE_PER_INFERENCE TENSOR_ARENA_SIZE
#else // If not defined, use maximum available - 2M
#define TENSOR_ARENA_SIZE               2000000
#define TENSOR_ARENA_SIZE_PER_INFERENCE (TENSOR_ARENA_SIZE / NUM_INFERENCE_THREADS)
#endif

#define PROCESS_QUEUE_SIZE (NUM_JOBS_PER_THREAD * NUM_JOB_THREADS * sizeof(xInferenceJob *))
#define SENDER_QUEUE_SIZE  (NUM_JOBS_PER_THREAD * sizeof(xInferenceJob *))

/* BYTE_POOL_SIZE_OVERHEAD is used to increase the memory byte pool size, as the number of
   allocatable bytes in a memory byte pool is slightly less than what was specified during creation */
#define BYTE_POOL_SIZE_OVERHEAD (512)
#define BYTE_POOL_SIZE                                                                               \
    (((PROCESS_THREAD_CONTEXT_SIZE + PROCESS_THREAD_STACK_SIZE) * NUM_INFERENCE_THREADS) +           \
     (SENDER_THREAD_CONTEXT_SIZE + SENDER_THREAD_STACK_SIZE + SENDER_QUEUE_SIZE) * NUM_JOB_THREADS + \
     PROCESS_QUEUE_SIZE + BYTE_POOL_SIZE_OVERHEAD)

/****************************************************************************
 * Structures
 ****************************************************************************/
struct ProcessThreadParams {
    ProcessThreadParams() : queueHandle(nullptr), tensorArena(nullptr), arenaSize(0) {}
    ProcessThreadParams(TX_QUEUE *_queue, uint8_t *_tensorArena, size_t _arenaSize) :
        queueHandle(_queue), tensorArena(_tensorArena), arenaSize(_arenaSize) {}

    TX_QUEUE *queueHandle;
    uint8_t *tensorArena;
    size_t arenaSize;
};

// Wrapper around InferenceProcess::InferenceJob. Adds responseQueue and status for ThreadX multi-threaded purposes.
struct xInferenceJob : public InferenceJob {
    TX_QUEUE *responseQueue;
    bool status;

    xInferenceJob() : InferenceJob(), responseQueue(nullptr), status(false) {}
    xInferenceJob(const string &_name,
                  const DataPtr &_networkModel,
                  const vector<DataPtr> &_input,
                  const vector<DataPtr> &_output,
                  const vector<DataPtr> &_expectedOutput,
                  const size_t _numBytesToPrint,
                  void *_userArg,
                  TX_QUEUE *_queue) :
        InferenceJob(_name, _networkModel, _input, _output, _expectedOutput, _numBytesToPrint, _userArg),
        responseQueue(_queue), status(false) {}
};

/****************************************************************************
 * Global and static variables
 ****************************************************************************/
namespace {
// Number of total completed jobs, needed to exit application correctly if NUM_JOB_THREADS > 1
int totalCompletedJobs = 0;

// TensorArena static initialisation
const size_t arenaSize = TENSOR_ARENA_SIZE_PER_INFERENCE;

TX_QUEUE inferenceProcessQueue;
char inferenceProcessQueueName[] = "inferenceProcessQueue";

ProcessThreadParams threadParams[NUM_INFERENCE_THREADS];

TX_BYTE_POOL bytePool;
char bytePoolName[] = "byte pool";

ULONG memoryArea[BYTE_POOL_SIZE / sizeof(ULONG)];
} // namespace

__attribute__((section(".bss.tensor_arena"), aligned(16)))
uint8_t inferenceProcessTensorArena[NUM_INFERENCE_THREADS][arenaSize];

/****************************************************************************
 * Mutex & Semaphore
 * Overrides weak-linked symbols in ethosu_driver.c to implement thread handling
 ****************************************************************************/
extern "C" {
void *ethosu_mutex_create(void) {
    UINT status;
    TX_MUTEX *mutex;

    mutex  = new TX_MUTEX;
    status = tx_mutex_create(mutex, nullptr, TX_NO_INHERIT);
    if (status != TX_SUCCESS) {
        printf("mutex create failed, error - %d\n", status);
    }
    return (void *)mutex;
}

int ethosu_mutex_lock(void *mutex) {
    UINT status;
    status = tx_mutex_get(reinterpret_cast<TX_MUTEX *>(mutex), TX_WAIT_FOREVER);
    if (status != TX_SUCCESS) {
        printf("mutex get failed, error - %d\n", status);
        return -1;
    }
    return 0;
}

int ethosu_mutex_unlock(void *mutex) {
    UINT status;
    status = tx_mutex_put(reinterpret_cast<TX_MUTEX *>(mutex));
    if (status != TX_SUCCESS) {
        printf("mutex put failed, error - %d\n", status);
        return -1;
    }
    return 0;
}

void *ethosu_semaphore_create(void) {
    UINT status;
    TX_SEMAPHORE *semaphore;

    semaphore = new TX_SEMAPHORE;
    status    = tx_semaphore_create(semaphore, nullptr, 0);

    if (status != TX_SUCCESS) {
        printf("Semaphore create failed, error - %d\n", status);
    }

    return (void *)semaphore;
}

int ethosu_semaphore_take(void *sem) {
    UINT status;

    status = tx_semaphore_get(reinterpret_cast<TX_SEMAPHORE *>(sem), TX_WAIT_FOREVER);

    if (status != TX_SUCCESS) {
        printf("Semaphore get/take, error - %d\n", status);
        return -1;
    }

    return 0;
}

int ethosu_semaphore_give(void *sem) {
    UINT status;

    status = tx_semaphore_put(reinterpret_cast<TX_SEMAPHORE *>(sem));

    if (status != TX_SUCCESS) {
        printf("Semaphore put/give, error - %d\n", status);
        return -1;
    }

    return 0;
}
}

/****************************************************************************
 * Functions
 ****************************************************************************/
//  inferenceProcessThread - Run jobs from queue with available driver
void inferenceProcessThread(ULONG pvParameters) {
    ProcessThreadParams params = *reinterpret_cast<ProcessThreadParams *>(pvParameters);
    UINT tx_status             = TX_QUEUE_ERROR;

    class InferenceProcess inferenceProcess(params.tensorArena, params.arenaSize);

    for (;;) {
        xInferenceJob *xJob;

        // Get the job details from the process queue
        tx_status = tx_queue_receive(params.queueHandle, &xJob, TX_WAIT_FOREVER);
        if (tx_status != TX_SUCCESS) {
            printf("process failed to receive from Queue, error - %d\n", tx_status);
            exit(1);
        }

        // run the job
        bool status  = inferenceProcess.runJob(*xJob);
        xJob->status = status;

        // Send response for the job in the response queue
        tx_status = tx_queue_send(xJob->responseQueue, &xJob, TX_WAIT_FOREVER);
        if (tx_status != TX_SUCCESS) {
            printf("process inferenceProcessThread failed to send to Queue, error - %d\n", tx_status);
            exit(1);
        }
    }

    tx_status = tx_thread_terminate(nullptr);
    if (tx_status != TX_SUCCESS) {
        printf("process inferenceProcessThread failed to terminate thread, error - %d\n", tx_status);
        exit(1);
    }
}

//  inferenceSenderThread - Creates NUM_INFERNECE_JOBS jobs, queues them, and then listens for completion status
void inferenceSenderThread(ULONG pvParameters) {
    int ret = 0;
    TX_QUEUE senderQueue;
    UINT status                          = TX_QUEUE_ERROR;
    TX_QUEUE *inferenceProcessQueueLocal = reinterpret_cast<TX_QUEUE *>(pvParameters);
    xInferenceJob jobs[NUM_JOBS_PER_THREAD];
    CHAR *senderQueuePtr   = nullptr;
    char senderQueueName[] = "senderQueue";

    /* Allocate memory for this inference sender thread responses queue  */
    status = tx_byte_allocate(&bytePool, reinterpret_cast<VOID **>(&senderQueuePtr), SENDER_QUEUE_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS) {
        printf("Sender thread failed to allocate bytes for Queue, error - %d\n", status);
        exit(1);
    }

    /* Create responses queue for this inference sender thread */
    status = tx_queue_create(
        &senderQueue, senderQueueName, sizeof(xInferenceJob *) / sizeof(uint32_t), senderQueuePtr, SENDER_QUEUE_SIZE);

    if (status != TX_SUCCESS) {
        printf("Sender thread failed to create Queue, error - %d\n", status);
        exit(1);
    }

    /* Create the jobs and queue them in the inference process queue */
    for (int n = 0; n < NUM_JOBS_PER_THREAD; n++) {

        // Create job
        xInferenceJob *job = &jobs[n];
        job->name          = string(modelName);
        job->networkModel  = DataPtr(networkModelData, sizeof(networkModelData));
        job->input.push_back(DataPtr(inputData, sizeof(inputData)));
        job->expectedOutput.push_back(DataPtr(expectedOutputData, sizeof(expectedOutputData)));
        job->responseQueue = &senderQueue;

        // queue job
        status = tx_queue_send(inferenceProcessQueueLocal, &job, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS) {
            printf("Sender thread failed to send to Queue, error - %d\n", status);
            exit(1);
        }
    }

    /* Listen for completion status on the response queue */
    do {
        xInferenceJob *pSendJob;

        status = tx_queue_receive(&senderQueue, &pSendJob, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS) {
            printf("Sender thread failed to receive from Queue, error - %d\n", status);
            exit(1);
        }

        totalCompletedJobs++;
        ret = (pSendJob->status);
        if (pSendJob->status != 0) {
            break;
        }
    } while (totalCompletedJobs < NUM_JOBS_PER_THREAD * NUM_JOB_THREADS);

    /* delete the response queue */
    status = tx_queue_delete(&senderQueue);
    if (status != TX_SUCCESS) {
        printf("Sender thread failed to delete Queue, error - %d\n", status);
        exit(1);
    }

    exit(ret);
}

/****************************************************************************
 * Application
 ****************************************************************************/
int main() {
    /* Enter the ThreadX kernel. */
    tx_kernel_enter();
    return 0;
}

void tx_application_define(void *first_unused_memory) {
    UINT status;
    CHAR *senderThreadStackPtr[NUM_JOB_THREADS]        = {nullptr};
    CHAR *processThreadStackPtr[NUM_INFERENCE_THREADS] = {nullptr};
    CHAR *processQueuePtr                              = nullptr;
    CHAR *senderThreadPtr[NUM_JOB_THREADS]             = {nullptr};
    CHAR senderThreadNames[NUM_JOB_THREADS][MAX_THREAD_NAME_SIZE];
    CHAR *processThreadPtr[NUM_INFERENCE_THREADS] = {nullptr};
    CHAR processThreadNames[NUM_JOB_THREADS][MAX_THREAD_NAME_SIZE];

    /* Create a byte memory pool from which to allocate the threads stacks and queues.  */
    status = tx_byte_pool_create(&bytePool, bytePoolName, memoryArea, BYTE_POOL_SIZE);
    if (status != TX_SUCCESS) {
        printf("Main failed to allocate pool of bytes, error - %d\n", status);
        exit(1);
    }

    /* Allocate memory for the inference process queue  */
    status = tx_byte_allocate(&bytePool, reinterpret_cast<VOID **>(&processQueuePtr), PROCESS_QUEUE_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS) {
        printf("Main failed to allocate bytes for process queue, error - %d\n", status);
        exit(1);
    }

    status = tx_queue_create(&inferenceProcessQueue,
                             inferenceProcessQueueName,
                             sizeof(xInferenceJob *) / sizeof(uint32_t),
                             processQueuePtr,
                             PROCESS_QUEUE_SIZE);
    if (status != TX_SUCCESS) {
        printf("Main failed to create Queue, error - %d\n", status);
        exit(1);
    }

    /* inferenceSender threads to create and queue the jobs */
    for (int n = 0; n < NUM_JOB_THREADS; n++) {

        /* Allocate the thread context for the inference sender thread.  */
        status =
            tx_byte_allocate(&bytePool, reinterpret_cast<VOID **>(&senderThreadPtr[n]), sizeof(TX_THREAD), TX_NO_WAIT);
        if (status != TX_SUCCESS) {
            printf("Main failed to allocate bytes for sender tread, error - %d\n", status);
            exit(1);
        }

        /* Allocate the stack for the inference sender thread.  */
        status = tx_byte_allocate(
            &bytePool, reinterpret_cast<VOID **>(&senderThreadStackPtr[n]), SENDER_THREAD_STACK_SIZE, TX_NO_WAIT);
        if (status != TX_SUCCESS) {
            printf("Main failed to allocate bytes for sender tread stack, error - %d\n", status);
            exit(1);
        }

        snprintf(senderThreadNames[n], MAX_THREAD_NAME_SIZE, "senderThread-%d", n);

        /* Create the inference sender thread.  */
        status = tx_thread_create(reinterpret_cast<TX_THREAD *>(senderThreadPtr[n]),
                                  senderThreadNames[n],
                                  inferenceSenderThread,
                                  reinterpret_cast<ULONG>(&inferenceProcessQueue),
                                  senderThreadStackPtr[n],
                                  SENDER_THREAD_STACK_SIZE,
                                  1,
                                  1,
                                  TX_NO_TIME_SLICE,
                                  TX_AUTO_START);
        if (status != TX_SUCCESS) {
            printf("Main failed to create Thread, error - %d\n", status);
            exit(1);
        }
    }

    /* Create inferenceProcess threads to process the queued jobs */
    for (int n = 0; n < NUM_INFERENCE_THREADS; n++) {

        /* Allocate the thread context for the inference process thread.  */
        status =
            tx_byte_allocate(&bytePool, reinterpret_cast<VOID **>(&processThreadPtr[n]), sizeof(TX_THREAD), TX_NO_WAIT);
        if (status != TX_SUCCESS) {
            printf("Main failed to allocate bytes for process tread, error - %d\n", status);
            exit(1);
        }

        /* Allocate the stack for the inference process thread.  */
        status = tx_byte_allocate(
            &bytePool, reinterpret_cast<VOID **>(&processThreadStackPtr[n]), PROCESS_THREAD_STACK_SIZE, TX_NO_WAIT);
        if (status != TX_SUCCESS) {
            printf("Main failed to allocate bytes for process stack, error - %d\n", status);
            exit(1);
        }

        threadParams[n] = ProcessThreadParams(
            &inferenceProcessQueue, inferenceProcessTensorArena[n], reinterpret_cast<size_t>(arenaSize));
        snprintf(processThreadNames[n], MAX_THREAD_NAME_SIZE, "processThread-%d", n);

        /* Create the inference process thread.  */
        status = tx_thread_create(reinterpret_cast<TX_THREAD *>(processThreadPtr[n]),
                                  processThreadNames[n],
                                  inferenceProcessThread,
                                  reinterpret_cast<ULONG>(&threadParams[n]),
                                  processThreadStackPtr[n],
                                  PROCESS_THREAD_STACK_SIZE,
                                  1,
                                  1,
                                  TX_NO_TIME_SLICE,
                                  TX_AUTO_START);
        if (status != TX_SUCCESS) {
            printf("Main failed to create thread, error - %d\n", status);
            exit(1);
        }
    }

    printf("ThreadX application initialisation - Done \n");
    return;
}
