/*
 * Copyright (c) 2021 Arm Limited. All rights reserved.
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

#ifndef TRUSTZONE_H_
#define TRUSTZONE_H_

/* For the I-/DTCM memory the S/NS gating is handled by the I-/DTGU block.
 * The block defines a block size depending on the size of the memory.
 * The slice between secure and nonsecure has to be on a block boundary
 * to be able to map whole respective region to its security state.
 * For MPS3 with SSE300 + U55 the block size is 0x2000.
 */

#define TZ_S_ITCM_START 0x10000000
#define TZ_S_ITCM_SIZE  0x0007c000
#define TZ_S_BRAM_START 0x11000000
#define TZ_S_BRAM_SIZE  0x000fc000
#define TZ_S_DTCM_START 0x30000000
#define TZ_S_DTCM_SIZE  0x00040000
#define TZ_S_SRAM_START 0x31000000
#define TZ_S_SRAM_SIZE  0x00100000
#define TZ_S_STACK_HEAP 0x30040000

#define TZ_NSC_START 0x110fb000
#define TZ_NSC_SIZE  0x00001000

#define TZ_NS_ITCM_START 0x0007c000
#define TZ_NS_ITCM_SIZE  0x00004000
#define TZ_NS_BRAM_START 0x010fc000
#define TZ_NS_BRAM_SIZE  0x00004000
#define TZ_NS_DTCM_START 0x20040000
#define TZ_NS_DTCM_SIZE  0x00040000
#define TZ_NS_SRAM_START 0x21100000
#define TZ_NS_SRAM_SIZE  0x00100000
#define TZ_NS_STACK_HEAP 0x20080000

/* Add separate definition for secure world to use for booting
 * non secure world.
 */
#define TZ_NS_START_VECTOR TZ_NS_ITCM_START

#ifdef TRUSTZONE_SECURE
#define ITCM_START TZ_S_ITCM_START
#define ITCM_SIZE  TZ_S_ITCM_SIZE
#define BRAM_START TZ_S_BRAM_START
#define BRAM_SIZE  TZ_S_BRAM_SIZE
#define DTCM_START TZ_S_DTCM_START
#define DTCM_SIZE  TZ_S_DTCM_SIZE
#define SRAM_START TZ_S_SRAM_START
#define SRAM_SIZE  TZ_S_SRAM_SIZE
#define STACK_HEAP TZ_S_STACK_HEAP
#else
#define ITCM_START TZ_NS_ITCM_START
#define ITCM_SIZE  TZ_NS_ITCM_SIZE
#define BRAM_START TZ_NS_BRAM_START
#define BRAM_SIZE  TZ_NS_BRAM_SIZE
#define DTCM_START TZ_NS_DTCM_START
#define DTCM_SIZE  TZ_NS_DTCM_SIZE
#define SRAM_START TZ_NS_SRAM_START
#define SRAM_SIZE  TZ_NS_SRAM_SIZE
#define STACK_HEAP TZ_NS_STACK_HEAP
#endif

#define LR_START ITCM_START
#define LR_SIZE  ITCM_SIZE

#define ITCM_TOTAL_SIZE 0x00080000
#define ITCM_ITGU       0xE001E500

#define DTCM_TOTAL_SIZE 0x00080000
#define DTCM_DTGU       0xE001E600

#define BRAM_BASE_S     0x11000000
#define BRAM_BASE_NS    0x01000000
#define BRAM_TOTAL_SIZE 0x00100000
#define BRAM_MPC        0x57000000

#define SRAM0_BASE_S  0x31000000
#define SRAM0_BASE_NS 0x21000000
#define SRAM0_SIZE    0x00200000
#define SRAM0_MPC     0x50083000

#define DDR0_BASE_S  0x70000000
#define DDR0_BASE_NS 0x60000000
#define DDR0_SIZE    0x02000000

#ifdef TRUSTZONE_BUILD
#define DDR_START DDR0_BASE_S
#else
#define DDR_START DDR0_BASE_NS
#endif
/* Separate DDRs for secure and nonsecure */
#define DDR_SIZE DDR0_SIZE

#if (S_TZ_ITCM_SIZE + TZ_NS_ITCM_SIZE) > ITCM_TOTAL_SIZE
#error Missconfigured ITCM memory
#endif

#if (S_TZ_DTCM_SIZE + TZ_NS_DTCM_SIZE) > DTCM_TOTAL_SIZE
#error Missconfigured ITCM memory
#endif

#endif // TRUSTZONE_H_
