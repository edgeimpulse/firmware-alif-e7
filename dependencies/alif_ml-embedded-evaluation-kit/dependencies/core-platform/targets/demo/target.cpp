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

#include "target.hpp"

#ifdef ETHOSU
#include <ethosu_driver.h>
#endif

#include "mpu.hpp"
#include "uart_stdout.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace EthosU;

/****************************************************************************
 * Defines
 ****************************************************************************/

#ifdef ETHOSU
#define ETHOSU_BASE_ADDRESS <TODO>
#define ETHOSU_IRQ          <TODO>
#endif

/****************************************************************************
 * Variables
 ****************************************************************************/

#ifdef ETHOSU
#if defined(ETHOSU_FAST_MEMORY_SIZE) && ETHOSU_FAST_MEMORY_SIZE > 0
__attribute__((aligned(16), section(".bss.ethosu_scratch"))) uint8_t ethosu_scratch[ETHOSU_FAST_MEMORY_SIZE];
#else
#define ethosu_scratch          0
#define ETHOSU_FAST_MEMORY_SIZE 0
#endif

struct ethosu_driver ethosu0_driver;
#endif

/****************************************************************************
 * Cache maintenance
 ****************************************************************************/

#if defined(CPU_CACHE_ENABLE) && defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
extern "C" {
void ethosu_flush_dcache(uint32_t *p, size_t bytes) {
    if (p)
        SCB_CleanDCache_by_Addr(p, bytes);
    else
        SCB_CleanDCache();
}

void ethosu_invalidate_dcache(uint32_t *p, size_t bytes) {
    if (p)
        SCB_InvalidateDCache_by_Addr(p, bytes);
    else
        SCB_InvalidateDCache();
}
}
#endif

/****************************************************************************
 * Init
 ****************************************************************************/

namespace {

#ifdef ETHOSU
void ethosuIrqHandler() {
    ethosu_irq_handler(&ethosu0_driver);
}
#endif

} // namespace

namespace EthosU {

void targetSetup() {
    // Initialize UART driver
    UartStdOutInit();

#ifdef ETHOSU
    // Initialize Ethos-U NPU driver
    if (ethosu_init(&ethosu0_driver,
                    reinterpret_cast<void *>(ETHOSU_BASE_ADDRESS),
                    ethosu_scratch,
                    ETHOSU_FAST_MEMORY_SIZE,
                    1,
                    1)) {
        printf("Failed to initialize NPU.\n");
        return;
    }

    // Assumes SCB->VTOR point to RW memory
    NVIC_SetVector(static_cast<IRQn_Type>(ETHOSU_IRQ), (uint32_t)&ethosuIrqHandler);
    NVIC_EnableIRQ(static_cast<IRQn_Type>(ETHOSU_IRQ));
#endif

    // MPU setup
    // TODO Add memory protection unit configuration
    const std::vector<ARM_MPU_Region_t> mpuConfig = {};

    // Setup MPU configuration
    Mpu::loadAndEnableConfig(&mpuConfig[0], mpuConfig.size());

#if defined(CPU_CACHE_ENABLE) && defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_EnableICache();
    SCB_EnableDCache();
#endif
}

} // namespace EthosU
