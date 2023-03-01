/*
 * Copyright (c) 2021-2022 Arm Limited. All rights reserved.
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

// Inference process
#include "inference_process.hpp"

// System includes
#include <stdio.h>
#include <vector>

// Model data
#include "input.h"
#include "model.h"
#include "output.h"

#ifdef ETHOSU
#include <ethosu_driver.h>
#include <ethosu_monitor.hpp>
#include <pmu_ethosu.h>
#endif

using namespace std;
using namespace InferenceProcess;

/****************************************************************************
 * InferenceJob
 ****************************************************************************/

__attribute__((section(".bss.tensor_arena"), aligned(16))) uint8_t TFLuTensorArena[tensorArenaSize];

class InferenceProcess inferenceProcess(TFLuTensorArena, tensorArenaSize);

uint8_t outputData[sizeof(expectedOutputData)] __attribute__((aligned(16), section("output_data_sec")));

#if !defined(ETHOSU_PMU_EVENT_0) || ETHOSU_PMU_EVENT_0 < 0 || ETHOSU_PMU_EVENT_0 >= ETHOSU_PMU_SENTINEL
#undef ETHOSU_PMU_EVENT_0
#define ETHOSU_PMU_EVENT_0 ETHOSU_PMU_CYCLE
#endif

#if !defined(ETHOSU_PMU_EVENT_1) || ETHOSU_PMU_EVENT_1 < 0 || ETHOSU_PMU_EVENT_1 >= ETHOSU_PMU_SENTINEL
#undef ETHOSU_PMU_EVENT_1
#define ETHOSU_PMU_EVENT_1 ETHOSU_PMU_NPU_ACTIVE
#endif

#if !defined(ETHOSU_PMU_EVENT_2) || ETHOSU_PMU_EVENT_2 < 0 || ETHOSU_PMU_EVENT_2 >= ETHOSU_PMU_SENTINEL
#undef ETHOSU_PMU_EVENT_2
#define ETHOSU_PMU_EVENT_2 ETHOSU_PMU_NO_EVENT
#endif

#if !defined(ETHOSU_PMU_EVENT_3) || ETHOSU_PMU_EVENT_3 < 0 || ETHOSU_PMU_EVENT_3 >= ETHOSU_PMU_SENTINEL
#undef ETHOSU_PMU_EVENT_3
#define ETHOSU_PMU_EVENT_3 ETHOSU_PMU_NO_EVENT
#endif

#ifdef ETHOSU
namespace {
std::vector<ethosu_pmu_event_type> pmuEventConfig{ethosu_pmu_event_type(ETHOSU_PMU_EVENT_0),
                                                  ethosu_pmu_event_type(ETHOSU_PMU_EVENT_1),
                                                  ethosu_pmu_event_type(ETHOSU_PMU_EVENT_2),
                                                  ethosu_pmu_event_type(ETHOSU_PMU_EVENT_3)};

const uint32_t delayMs = SystemCoreClock / 1000ul;
struct ethosu_driver *ethosuDrv;
EthosUMonitor ethosuMonitor(EthosUMonitor::Backend::EVENT_RECORDER);
} // namespace

extern "C" {

void SysTick_Handler(void) {
    ethosuMonitor.monitorSample(ethosuDrv);
}

void ethosu_inference_begin(struct ethosu_driver *drv, void *) {
    ethosuDrv = drv;
    ethosuMonitor.configure(drv, pmuEventConfig);

    // Enable polling
    SysTick_Config(delayMs);
}

void ethosu_inference_end(struct ethosu_driver *drv, void *) {
    // Disable polling
    SysTick->CTRL = 0;

    ethosuDrv = 0;
    ethosuMonitor.monitorSample(drv);
    ethosuMonitor.release(drv);
}
}
#endif

int runInference() {
    // Create job
    InferenceJob job(string(modelName),
                     DataPtr(networkModelData, sizeof(networkModelData)),
                     {DataPtr(inputData, sizeof(inputData))},
                     {DataPtr(outputData, sizeof(outputData))},
                     {DataPtr(expectedOutputData, sizeof(expectedOutputData))},
                     512);

    // Run job
    bool failed = inferenceProcess.runJob(job);
    printf("Status of executed job: ");
    printf(failed ? "Failed\n" : "Success\n");

    return failed;
}

int main() {
#ifdef ETHOSU
    EventRecorderInitialize(EventRecordAll, 1);
#endif
    int ret = runInference();
    return ret;
}
