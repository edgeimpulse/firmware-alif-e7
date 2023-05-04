/*
 * Copyright (c) 2020-2021 Arm Limited. All rights reserved.
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
#include <stdio.h>

/****************************************************************************
 * Defines
 ****************************************************************************/
#define SYSTEM_CLOCK   6000000
#define SYSTICK_CYCLES ((SYSTEM_CLOCK / 100) - 1)

/****************************************************************************
 * Externs
 ****************************************************************************/
void _tx_timer_interrupt(void);

/****************************************************************************
 * Functions
 ****************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_initialize_low_level                          Cortex-M4/GNU     */
/*                                                           6.1          */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is responsible for any low-level processor            */
/*    initialization, including setting up interrupt vectors, setting     */
/*    up a periodic timer interrupt source, saving the system stack       */
/*    pointer for use in ISR processing later, and finding the first      */
/*    available RAM memory address for tx_application_define.             */
/*    this function has an empty implementation in this example.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_initialize_kernel_enter           ThreadX entry function        */
/*                                                                        */
/**************************************************************************/
VOID _tx_initialize_low_level(VOID) {
    __asm__ volatile(
        /* Disable interrupts during ThreadX initialization.  */
        "    CPSID   i\n"
        /* Enable the cycle count register.  */
        "    LDR     r0, =0xE0001000\n" // Build address of DWT register
        "    LDR     r1, [r0]\n"        // Pickup the current value
        "    ORR     r1, r1, #1\n"      // Set the CYCCNTENA bit
        "    STR     r1, [r0]\n"        // Enable the cycle count register
        /* Configure SysTick for 100Hz clock, or 16384 cycles if no reference.  */
        "    MOV     r0, #0xE000E000\n" // Build address of NVIC registers
        "    LDR     r1, =59999\n"      // 59,999 = SYSTICK_CYCLES = ((6,000,000 / 100) -1)
        "    STR     r1, [r0, #0x14]\n" // Setup SysTick Reload Value
        "    MOV     r1, #0x7\n"        // Build SysTick Control Enable Value
        "    STR     r1, [r0, #0x10]\n" // Setup SysTick Control
        /* Configure handler priorities.  */
        "    LDR     r1, =0x00000000\n"  // Rsrv, UsgF, BusF, MemM
        "    STR     r1, [r0, #0xD18]\n" // Setup System Handlers 4-7 Priority Registers
        "    LDR     r1, =0xFF000000\n"  // SVCl, Rsrv, Rsrv, Rsrv
        "    STR     r1, [r0, #0xD1C]\n" // Setup System Handlers 8-11 Priority Registers
                                         // Note: SVC must be lowest priority, which is 0xFF
        "    LDR     r1, =0x40FF0000\n"  // SysT, PnSV, Rsrv, DbgM
        "    STR     r1, [r0, #0xD20]\n" // Setup System Handlers 12-15 Priority Registers
                                         // Note: PnSV must be lowest priority, which is 0xFF
        /* Return to caller.  */
        "    BX      lr\n");
}

/* SysTick_Handler has a generic interrupt handler template. */
/* to implement a new handler, replace the call to           */
/* _tx_timer_interrupt to some other c function              */
VOID SysTick_Handler(VOID) {
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    _tx_execution_isr_enter(); // Call the ISR enter function
#endif

    _tx_timer_interrupt();

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    _tx_execution_isr_exit(); // Call the ISR exit function
#endif

    return;
}
