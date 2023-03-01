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

// Ethos-U
#include "ethosu_driver.h"

// Trustzone defines and MPC driver
#include "../common/secure_entries.hpp"
#include "mpc_sie_drv.h"
#include "trustzone.h"

#include "inference_process.hpp"

// System includes
#include <arm_cmse.h>
#include <inttypes.h>
#include <stdio.h>

using namespace std;

funcptr_ns nonsecure_result_checker = 0;

/****************************************************************************
 * InferenceJob
 ****************************************************************************/

#define TENSOR_ARENA_SIZE 0xa0000
__attribute__((section(".bss.tensor_arena"), aligned(16))) uint8_t TFLuTensorArena[TENSOR_ARENA_SIZE];

InferenceProcess::InferenceProcess inferenceProcess(TFLuTensorArena, TENSOR_ARENA_SIZE);

/****************************************************************************
 * Functions
 ****************************************************************************/

namespace {

#include "input.h"
#include "model.h"
#include "output.h"

} // namespace

#define N_MEM_RANGES (2)
static int setup_sram0_mpc(const uint32_t baseaddr_s,  /* Secure base address */
                           const uint32_t len_s,       /* Length (in bytes) of secure region */
                           const uint32_t baseaddr_ns, /* Non-secure base address */
                           const uint32_t len_ns)      /* Length (in bytes) of non-secure region */
{
    const char *mem_name = "SRAM0";

    /* Secure range */
    const struct mpc_sie_memory_range_t mpc_range_s = {/* base */ SRAM0_BASE_S,
                                                       /* limit */ SRAM0_BASE_S + SRAM0_SIZE - 1,
                                                       /* range_offset */ 0,
                                                       /* attr */ MPC_SIE_SEC_ATTR_SECURE};

    /* Non secure range */
    const struct mpc_sie_memory_range_t mpc_range_ns = {/* base */ SRAM0_BASE_NS,
                                                        /* limit */ SRAM0_BASE_NS + SRAM0_SIZE - 1,
                                                        /* range_offset */ 0,
                                                        /* attr */ MPC_SIE_SEC_ATTR_NONSECURE};

    /* Consolidated ranges */
    const struct mpc_sie_memory_range_t *mpc_range_list[N_MEM_RANGES] = {&mpc_range_s, &mpc_range_ns};

    /* MPC device configuration controller */
    const struct mpc_sie_dev_cfg_t mpc_dev_cfg = {SRAM0_MPC};

    /* MPC device data */
    struct mpc_sie_dev_data_t mpc_dev_data = {};

    /* MPC device itself */
    struct mpc_sie_dev_t mpc_dev = {&mpc_dev_cfg, &mpc_dev_data};

    enum mpc_sie_error_t ret = MPC_SIE_ERR_NONE;

    printf("Configuring MPC for %s\n", mem_name);

    /* Initialise this MPC device */
    ret = mpc_sie_init(&mpc_dev, mpc_range_list, N_MEM_RANGES);
    if (MPC_SIE_ERR_NONE != ret) {
        printf("Error initialising MPC for %s\n", mem_name);
        return 1;
    }

    /* Configure the non secure region */
    ret = mpc_sie_config_region(&mpc_dev, baseaddr_ns, baseaddr_ns + len_ns - 1, MPC_SIE_SEC_ATTR_NONSECURE);
    if (MPC_SIE_ERR_NONE != ret) {
        printf("Error configuring non-secure region for %s (%d)\n", mem_name, ret);
        return 1;
    }

    /* Configure the secure region */
    ret = mpc_sie_config_region(&mpc_dev, baseaddr_s, baseaddr_s + len_s - 1, MPC_SIE_SEC_ATTR_SECURE);
    if (MPC_SIE_ERR_NONE != ret) {
        printf("Error configuring secure region for %s\n", mem_name);
        return 1;
    }

    /* Lock down the configuration */
    ret = mpc_sie_lock_down(&mpc_dev);
    if (MPC_SIE_ERR_NONE != ret) {
        printf("Error locking down MPC for %s\n", mem_name);
        return 1;
    }

    return 0;
}

#define N_MEM_RANGES (2)
static int setup_bram_mpc(const uint32_t baseaddr_s,  /* Secure base address */
                          const uint32_t len_s,       /* Length (in bytes) of secure region */
                          const uint32_t baseaddr_ns, /* Non-secure base address */
                          const uint32_t len_ns)      /* Length (in bytes) of non-secure region */
{
    const char *mem_name = "BRAM";

    /* Secure range */
    const struct mpc_sie_memory_range_t mpc_range_s = {/* base */ BRAM_BASE_S,
                                                       /* limit */ BRAM_BASE_S + BRAM_TOTAL_SIZE - 1,
                                                       /* range_offset */ 0,
                                                       /* attr */ MPC_SIE_SEC_ATTR_SECURE};

    /* Non secure range */
    const struct mpc_sie_memory_range_t mpc_range_ns = {/* base */ BRAM_BASE_NS,
                                                        /* limit */ BRAM_BASE_NS + BRAM_TOTAL_SIZE - 1,
                                                        /* range_offset */ 0,
                                                        /* attr */ MPC_SIE_SEC_ATTR_NONSECURE};

    /* Consolidated ranges */
    const struct mpc_sie_memory_range_t *mpc_range_list[N_MEM_RANGES] = {&mpc_range_s, &mpc_range_ns};

    /* MPC device configuration controller */
    const struct mpc_sie_dev_cfg_t mpc_dev_cfg = {BRAM_MPC};

    /* MPC device data */
    struct mpc_sie_dev_data_t mpc_dev_data = {};

    /* MPC device itself */
    struct mpc_sie_dev_t mpc_dev = {&mpc_dev_cfg, &mpc_dev_data};

    enum mpc_sie_error_t ret = MPC_SIE_ERR_NONE;

    printf("Configuring MPC for %s\n", mem_name);

    /* Initialise this MPC device */
    ret = mpc_sie_init(&mpc_dev, mpc_range_list, N_MEM_RANGES);
    if (MPC_SIE_ERR_NONE != ret) {
        printf("Error initialising MPC for %s\n", mem_name);
        return 1;
    }

    /* Configure the non secure region */
    ret = mpc_sie_config_region(&mpc_dev, baseaddr_ns, baseaddr_ns + len_ns - 1, MPC_SIE_SEC_ATTR_NONSECURE);
    if (MPC_SIE_ERR_NONE != ret) {
        printf("Error configuring non-secure region for %s (%d)\n", mem_name, ret);
        return 1;
    }

    /* Configure the secure region */
    ret = mpc_sie_config_region(&mpc_dev, baseaddr_s, baseaddr_s + len_s - 1, MPC_SIE_SEC_ATTR_SECURE);
    if (MPC_SIE_ERR_NONE != ret) {
        printf("Error configuring secure region for %s (%d)\n", mem_name, ret);
        return 1;
    }

    /* Lock down the configuration */
    ret = mpc_sie_lock_down(&mpc_dev);
    if (MPC_SIE_ERR_NONE != ret) {
        printf("Error locking down MPC for %s (%d)\n", mem_name, ret);
        return 1;
    }

    return 0;
}

/* Depending on the Cortex-M configuration the LUT for the xTGU has
 * different size, set a maximum value to target all cases
 */
#define MAX_BLK_NBR (32)
void setup_xtgu_ns(uint32_t xtgu_base, uint32_t xtcm_start, uint32_t xtcm_size) {
    struct xtgu {
        uint32_t ctrl;
        uint32_t cfg;
        uint32_t reserved[2];
        uint32_t lut[];
    } *xtgu                            = (struct xtgu *)xtgu_base;
    uint32_t lut_bit_mask[MAX_BLK_NBR] = {0};

    /* Mask of the base offset of the I-/DTCM memory */
    xtcm_start &= 0x00ffffff;

    /* Read out xTGU configuration */
    uint32_t BLKSZ = 1 << (((xtgu->cfg) & 0xf) + 5);

    if (xtcm_start % BLKSZ != 0)
        printf("XTCM: start address %08x not on block size boundary\n", xtcm_start);

    if ((xtcm_start + xtcm_size) % BLKSZ != 0)
        printf("XTCM: limit address %08x not on block size boundary\n", xtcm_start + xtcm_size);

    printf("setting up xTGU LUT for mem@%08x(%08x)\n", xtcm_start, xtcm_size);

    uint32_t xtcm_end        = xtcm_start + xtcm_size - 1;
    uint32_t xtcm_address    = xtcm_start;
    uint32_t block_idx_start = (xtcm_address / BLKSZ) / 32;
    uint32_t block_idx_end;

    while (xtcm_address < xtcm_end) {
        uint32_t block_nbr = xtcm_address / BLKSZ;
        uint32_t block_idx = block_nbr / 32;
        uint32_t block_bit = 1 << (block_nbr % 32);

        if (block_idx >= MAX_BLK_NBR) {
            printf("lut bit mask too small, aborting!\n");
            exit(1);
        }
        lut_bit_mask[block_idx] |= block_bit;

        xtcm_address += BLKSZ;
        if (block_idx != block_idx_end)
            block_idx_end = block_idx;
    }

    /* Commit the LUT to the xTGU */
    for (uint32_t i = block_idx_start; i <= block_idx_end; i++) {
        xtgu->lut[i] = lut_bit_mask[i];
    }
}

int setup_secure_attributes(void) {
    int res;
    /* Setup ITGU and DTGU to give non-secure state access to ITCM and DTCM memory. */
    /* NS Code */
    setup_xtgu_ns(ITCM_ITGU, TZ_NS_ITCM_START, TZ_NS_ITCM_SIZE);

    /* NS stack location in DTCM */
    setup_xtgu_ns(DTCM_DTGU, TZ_NS_DTCM_START, TZ_NS_DTCM_SIZE);

    res = setup_bram_mpc(TZ_S_BRAM_START, TZ_S_BRAM_SIZE, TZ_NS_BRAM_START, TZ_NS_BRAM_SIZE);
    printf("BRAM MPC %s\n", res ? "Failed" : "OK");

    /* The SRAM has an MPC for each SRAM bank. Configure the first one for non-secure accesses here. */
    res = setup_sram0_mpc(TZ_S_SRAM_START, TZ_S_SRAM_SIZE, TZ_NS_SRAM_START, TZ_NS_SRAM_SIZE);
    printf("SRAM MPC %s\n", res ? "Failed" : "OK");

    /*
     * The IDAU is default not non-secure callable. In order for the code ram and data ram to be
     * non-secure callable the NSCCFG (at 0x50080014) has to be configured.
     * The register description can be found here:
     *    https://developer.arm.com/documentation/101773/0000
     * bit 0 : Sets NSC for 0x10000000-0x1fffffff
     * bit 1 : Sets NSC for 0x30000000-0x3fffffff
     */
    uint32_t *NSCCFG = (uint32_t *)(0x50080014);
    *NSCCFG          = 0x1;
    printf("NSCCFG set NSC for CODE SRAM 0x10000000-0x1fffffff\n");

    return 0;
}

void boot_non_secure() {
    /* Boot the non-secure world */
    typedef void (*ns_func)(void) __attribute__((cmse_nonsecure_call));
    ns_func NS_ResetHandler;

    printf("Setting NS MSP : 0x%x\n", *(uint32_t *)TZ_NS_START_VECTOR);
    /* Setup non-secure stack */
    __TZ_set_MSP_NS(*(uint32_t *)TZ_NS_START_VECTOR);

    /* Setup non-secure reset vector */
    printf("Setting NS VTOR : 0x%x\n", TZ_NS_START_VECTOR);
    SCB_NS->VTOR = TZ_NS_START_VECTOR;

    /* Enable features */
#if (defined(__FPU_USED) && (__FPU_USED == 1U)) || (defined(__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))
    SCB_NS->CPACR |= ((3U << 10U * 2U) | /* enable CP10 Full Access */
                      (3U << 11U * 2U)); /* enable CP11 Full Access */
#endif

#ifdef UNALIGNED_SUPPORT_DISABLE
    SCB_NS->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif

    // Enable Loop and branch info cache
    SCB_NS->CCR |= SCB_CCR_LOB_Msk;
    __ISB();

    /* Call cmse_ function to mark function as transition to non-secure state. */
    NS_ResetHandler = (ns_func)cmse_nsfptr_create(*(ns_func *)(TZ_NS_START_VECTOR + 4U));
    printf("Setting NS_ResetHandler to: %p\n", NS_ResetHandler);

    /* Now we've read the NS memory, setup our secure world compartment. */
    setup_secure_attributes();

    /* Enable SAU, we are ready to jump to non-secure. */
    SAU->CTRL = 1;

    printf("Leaving secure world.\n");
    /* Leave secure state. */
    NS_ResetHandler();
}

int main() {
    int ret = -1;
    printf("Secure main starting up.\n");
    SCB->CCR |= SCB_CCR_BFHFNMIGN_Msk;

    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk |
                  SCB_SHCSR_SECUREFAULTENA_Msk; // enable Usage-/Bus-/MPU-/Secure Fault

    boot_non_secure();

    printf("We're back in secure world!\n");

    if (nonsecure_result_checker != 0) {
        ret = nonsecure_result_checker();
    }

    return ret;
}

/****************************************************************************
 * Secure gateway functions to be callable from non-secure world
 ****************************************************************************/

uint8_t outputData[1001] __attribute__((aligned(4), section("output_data_sec")));

int run_inference(void) {
    vector<InferenceProcess::DataPtr> input;
    input.push_back(InferenceProcess::DataPtr(inputData, sizeof(inputData)));

    vector<InferenceProcess::DataPtr> output;
    output.push_back(InferenceProcess::DataPtr(outputData, sizeof(outputData)));

    vector<InferenceProcess::DataPtr> expected;
    expected.push_back(InferenceProcess::DataPtr(expectedData, sizeof(expectedData)));

    InferenceProcess::InferenceJob job(
        "secure", InferenceProcess::DataPtr(networkModelData, sizeof(networkModelData)), input, output, expected, 512);

    bool failed = inferenceProcess.runJob(job);
    printf("Status of executing the job: ");
    printf(failed ? "failed\n" : "success\n");

    return failed;
}

extern "C" int __attribute__((cmse_nonsecure_entry)) run_secure_inference(void) {
    return run_inference();
}

extern "C" void __attribute__((cmse_nonsecure_entry)) nonsecure_print(const char *p) {
    // Printing from non-secure, create RED ouput
    printf("\033[31;1m");
    printf("NS: %s\n", p);
    printf("\033[0m");
}

extern "C" void __attribute__((cmse_nonsecure_entry)) set_result_function(funcptr_ns callback_fn) {
    nonsecure_result_checker = callback_fn;
}
