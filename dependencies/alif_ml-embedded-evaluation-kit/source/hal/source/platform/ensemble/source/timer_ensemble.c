/* This file was ported to work on Alif Semiconductor Ensemble family of devices. */

/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "timer_ensemble.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "log_macros.h"
#include "RTE_Device.h"
#include "RTE_Components.h"
#include "Driver_PINMUX_AND_PINPAD.h"

#include CMSIS_device_header

static uint64_t cpu_cycle_count = 0;

#define UI
/**
 * @brief Adds one PMU counter to the counters' array
 * @param value Value of the counter
 * @param name  Name for the given counter
 * @param unit  Unit for the "value"
 * @param counters Pointer to the counter struct - the one to be populated.
 * @return true if successfully added, false otherwise
 */
static bool add_pmu_counter(
        uint64_t value,
        const char* name,
        const char* unit,
        pmu_counters* counters);

/**
 * SysTick initialisation
 */
int Init_SysTick(void)
{
    const uint32_t ticks_1ms = (GetSystemCoreClock() + 500)/1000;
    int err = 0;

    /* Reset CPU cycle count value. */
    cpu_cycle_count = 0;

    /* Changing configuration for sys tick => guard from being
     * interrupted. */
    NVIC_DisableIRQ(SysTick_IRQn);

    /* SysTick init - this will enable interrupt too. */
    err = SysTick_Config(ticks_1ms);

    /* SysTick_Config sets minimum priority - mid priority suits us better (see lv_port.c) */
    NVIC_SetPriority(SysTick_IRQn, 0x80 >> (8-__NVIC_PRIO_BITS));

    /* Enable interrupt again. */
    NVIC_EnableIRQ(SysTick_IRQn);

    /* Wait for SysTick to kick off */
    while (!err && !SysTick->VAL) {
        __NOP();
    }

    return err;
}

/**
 * Gets the current SysTick derived counter value
 */
uint64_t Get_SysTick_Cycle_Count(void)
{
    uint32_t systick_val;

    NVIC_DisableIRQ(SysTick_IRQn);
    systick_val = SysTick->VAL & SysTick_VAL_CURRENT_Msk;
    NVIC_EnableIRQ(SysTick_IRQn);

    return cpu_cycle_count + (SysTick->LOAD - systick_val);
}


void platform_reset_counters(void)
{
    if (0 != Init_SysTick()) {
        printf("Failed to initialise system tick config\n");
    }
#if defined (ARM_NPU)
    ethosu_pmu_init();
#endif /* defined (ARM_NPU) */
}

void platform_get_counters(pmu_counters* counters)
{
    counters->num_counters = 0;
    counters->initialised = true;
    uint32_t i = 0;

#if defined (ARM_NPU)
    ethosu_pmu_counters npu_counters = ethosu_get_pmu_counters();
    for (i = 0; i < ETHOSU_PMU_NCOUNTERS; ++i) {
        add_pmu_counter(
                npu_counters.npu_evt_counters[i].counter_value,
                npu_counters.npu_evt_counters[i].name,
                npu_counters.npu_evt_counters[i].unit,
                counters);
    }
    for (i = 0; i < ETHOSU_DERIVED_NCOUNTERS; ++i) {
        add_pmu_counter(
                npu_counters.npu_derived_counters[i].counter_value,
                npu_counters.npu_derived_counters[i].name,
                npu_counters.npu_derived_counters[i].unit,
                counters);
    }
    add_pmu_counter(
            npu_counters.npu_total_ccnt,
            "NPU TOTAL",
            "cycles",
            counters);
#else  /* defined (ARM_NPU) */
    UNUSED(i);
#endif /* defined (ARM_NPU) */

#if defined(CPU_PROFILE_ENABLED)
    add_pmu_counter(
            Get_SysTick_Cycle_Count(),
            "CPU TOTAL",
            "cycles",
            counters);
#endif /* defined(CPU_PROFILE_ENABLED) */

#if !defined(CPU_PROFILE_ENABLED)
    UNUSED(Get_SysTick_Cycle_Count);
#if !defined(ARM_NPU)
    UNUSED(add_pmu_counter);
#endif /* !defined(ARM_NPU) */
#endif /* !defined(CPU_PROFILE_ENABLED) */

}

__WEAK void lv_tick_handler(int ticks)
{
    UNUSED(ticks);
}

void SysTick_Handler(void)
{
    /* Increment the cycle counter based on load value. */
    cpu_cycle_count += SysTick->LOAD + 1;

    lv_tick_handler(1);
}

static bool add_pmu_counter(uint64_t value,
                            const char* name,
                            const char* unit,
                            pmu_counters* counters)
{
    const uint32_t idx = counters->num_counters;
    if (idx < NUM_PMU_COUNTERS) {
        counters->counters[idx].value = value;
        counters->counters[idx].name = name;
        counters->counters[idx].unit = unit;
        ++counters->num_counters;
        return true;
    }
    printf_err("Failed to add PMU counter!\n");
    return false;
}
