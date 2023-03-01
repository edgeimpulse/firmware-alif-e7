/****************************************************************************
 * @file     partition_ARMCM55.h
 * @brief    CMSIS-CORE Initial Setup for Secure / Non-Secure Zones for Armv8.1-M Mainline
 * @version  V1.0.0
 * @date     20. March 2020
 ******************************************************************************/
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

#ifndef PARTITION_ARMCM55_H
#define PARTITION_ARMCM55_H

#include "trustzone.h"

#define SAU_INIT_CTRL        0
#define SAU_INIT_CTRL_ENABLE 1
#define SAU_INIT_CTRL_ALLNS  0

#define SAU_REGIONS_MAX 8 /* Max. number of SAU regions */

#define SAU_INIT_REGION0 1
#define SAU_INIT_START0  TZ_S_ITCM_START                        /* start address of SAU region 0 */
#define SAU_INIT_END0    (TZ_S_ITCM_START + TZ_S_ITCM_SIZE - 1) /* end address of SAU region 0 */
#define SAU_INIT_NSC0    1

#define SAU_INIT_REGION1 1
#define SAU_INIT_START1  (TZ_NS_ITCM_START)
#define SAU_INIT_END1    (TZ_NS_ITCM_START + TZ_NS_ITCM_SIZE - 1)
#define SAU_INIT_NSC1    0

#define SAU_INIT_REGION2 1
#define SAU_INIT_START2  (TZ_S_SRAM_START)
#define SAU_INIT_END2    (TZ_S_SRAM_START + TZ_S_SRAM_SIZE - 1)
#define SAU_INIT_NSC2    1

#define SAU_INIT_REGION3 1
#define SAU_INIT_START3  (TZ_NS_SRAM_START)
#define SAU_INIT_END3    (TZ_NS_SRAM_START + TZ_NS_SRAM_SIZE - 1)
#define SAU_INIT_NSC3    0

#define SAU_INIT_REGION4 1
#define SAU_INIT_START4  TZ_S_DTCM_START                        /* start address of SAU region 4 */
#define SAU_INIT_END4    (TZ_S_DTCM_START + TZ_S_DTCM_SIZE - 1) /* end address of SAU region 4 */
#define SAU_INIT_NSC4    1

#define SAU_INIT_REGION5 1
#define SAU_INIT_START5  (TZ_NS_DTCM_START)
#define SAU_INIT_END5    (TZ_NS_DTCM_START + TZ_NS_DTCM_SIZE - 1)
#define SAU_INIT_NSC5    0

#define SAU_INIT_REGION6 1
#define SAU_INIT_START6  (TZ_NSC_START)
#define SAU_INIT_END6    (TZ_NSC_START + TZ_NSC_SIZE - 1)
#define SAU_INIT_NSC6    1

/*
// <e>Setup behaviour of Sleep and Exception Handling
*/
#define SCB_CSR_AIRCR_INIT 1

/*
//   <o> Deep Sleep can be enabled by
//     <0=>Secure and Non-Secure state
//     <1=>Secure state only
//   <i> Value for SCB->CSR register bit DEEPSLEEPS
*/
#define SCB_CSR_DEEPSLEEPS_VAL 1

/*
//   <o>System reset request accessible from
//     <0=> Secure and Non-Secure state
//     <1=> Secure state only
//   <i> Value for SCB->AIRCR register bit SYSRESETREQS
*/
#define SCB_AIRCR_SYSRESETREQS_VAL 0

/*
//   <o>Priority of Non-Secure exceptions is
//     <0=> Not altered
//     <1=> Lowered to 0x80-0xFF
//   <i> Value for SCB->AIRCR register bit PRIS
*/
#define SCB_AIRCR_PRIS_VAL 1

/*
//   <o>BusFault, HardFault, and NMI target
//     <0=> Secure state
//     <1=> Non-Secure state
//   <i> Value for SCB->AIRCR register bit BFHFNMINS
*/
#define SCB_AIRCR_BFHFNMINS_VAL 0

/*
// </e>
*/

/*
// <e>Setup behaviour of Floating Point and Vector Unit (FPU/MVE)
*/
#define TZ_FPU_NS_USAGE 1

/*
// <o>Floating Point and Vector Unit usage
//     <0=> Secure state only
//     <3=> Secure and Non-Secure state
//   <i> Value for SCB->NSACR register bits CP10, CP11
*/
#define SCB_NSACR_CP10_11_VAL 3

/*
// <o>Treat floating-point registers as Secure
//     <0=> Disabled
//     <1=> Enabled
//   <i> Value for FPU->FPCCR register bit TS
*/
#define FPU_FPCCR_TS_VAL 0

/*
// <o>Clear on return (CLRONRET) accessibility
//     <0=> Secure and Non-Secure state
//     <1=> Secure state only
//   <i> Value for FPU->FPCCR register bit CLRONRETS
*/
#define FPU_FPCCR_CLRONRETS_VAL 0

/*
// <o>Clear floating-point caller saved registers on exception return
//     <0=> Disabled
//     <1=> Enabled
//   <i> Value for FPU->FPCCR register bit CLRONRET
*/
#define FPU_FPCCR_CLRONRET_VAL 1

/*
// </e>
*/

/*
// <h>Setup Interrupt Target
*/

/*
//   <e>Initialize ITNS 0 (Interrupts 0..31)
*/
#define NVIC_INIT_ITNS0     1
#define NVIC_INIT_ITNS0_VAL 0x00000000

/*
//   <e>Initialize ITNS 1 (Interrupts 32..63)
*/
#define NVIC_INIT_ITNS1     1
#define NVIC_INIT_ITNS1_VAL 0x00000000 // 0x01000000

/*
//   <e>Initialize ITNS 2 (Interrupts 64..95)
*/
#define NVIC_INIT_ITNS2     1
#define NVIC_INIT_ITNS2_VAL 0x00000000
/*
//   <e>Initialize ITNS 3 (Interrupts 96..127)
*/
#define NVIC_INIT_ITNS3     0
#define NVIC_INIT_ITNS3_VAL 0x00000000
/*
//   <e>Initialize ITNS 4 (Interrupts 128..159)
*/
#define NVIC_INIT_ITNS4     0
#define NVIC_INIT_ITNS4_VAL 0x00000000
/*
//   <e>Initialize ITNS 5 (Interrupts 160..191)
*/
#define NVIC_INIT_ITNS5     0
#define NVIC_INIT_ITNS5_VAL 0x00000000
/*
//   <e>Initialize ITNS 6 (Interrupts 192..223)
*/
#define NVIC_INIT_ITNS6     0
#define NVIC_INIT_ITNS6_VAL 0x00000000

/*
//   <e>Initialize ITNS 7 (Interrupts 224..255)
*/
#define NVIC_INIT_ITNS7     0
#define NVIC_INIT_ITNS7_VAL 0x00000000
/*
//   <e>Initialize ITNS 8 (Interrupts 256..287)
*/
#define NVIC_INIT_ITNS8     0
#define NVIC_INIT_ITNS8_VAL 0x00000000
/*
//   <e>Initialize ITNS 9 (Interrupts 288..319)
*/
#define NVIC_INIT_ITNS9     0
#define NVIC_INIT_ITNS9_VAL 0x00000000
/*
//   <e>Initialize ITNS 10 (Interrupts 320..351)
*/
#define NVIC_INIT_ITNS10     0
#define NVIC_INIT_ITNS10_VAL 0x00000000
/*
//   <e>Initialize ITNS 11 (Interrupts 352..383)
*/
#define NVIC_INIT_ITNS11     0
#define NVIC_INIT_ITNS11_VAL 0x00000000
/*
//   <e>Initialize ITNS 12 (Interrupts 384..415)
*/
#define NVIC_INIT_ITNS12     0
#define NVIC_INIT_ITNS12_VAL 0x00000000
/*
//   <e>Initialize ITNS 13 (Interrupts 416..447)
*/
#define NVIC_INIT_ITNS13     0
#define NVIC_INIT_ITNS13_VAL 0x00000000
/*
//   <e>Initialize ITNS 14 (Interrupts 448..479)
*/
#define NVIC_INIT_ITNS14     0
#define NVIC_INIT_ITNS14_VAL 0x00000000
/*
//   <e>Initialize ITNS 15 (Interrupts 480..511)
*/
#define NVIC_INIT_ITNS15     0
#define NVIC_INIT_ITNS15_VAL 0x00000000

/*
    max 128 SAU regions.
    SAU regions are defined in partition.h
 */

#define SAU_INIT_REGION(n)                                \
    SAU->RNR  = (n & SAU_RNR_REGION_Msk);                 \
    SAU->RBAR = (SAU_INIT_START##n & SAU_RBAR_BADDR_Msk); \
    SAU->RLAR = (SAU_INIT_END##n & SAU_RLAR_LADDR_Msk) | ((SAU_INIT_NSC##n << SAU_RLAR_NSC_Pos) & SAU_RLAR_NSC_Msk) | 1U

/**
  \brief   Setup a SAU Region
  \details Writes the region information contained in SAU_Region to the
           registers SAU_RNR, SAU_RBAR, and SAU_RLAR
 */
__STATIC_INLINE void TZ_SAU_Setup(void) {

#if defined(__SAUREGION_PRESENT) && (__SAUREGION_PRESENT == 1U)

#if defined(SAU_INIT_REGION0) && (SAU_INIT_REGION0 == 1U)
    SAU_INIT_REGION(0);
#endif

#if defined(SAU_INIT_REGION1) && (SAU_INIT_REGION1 == 1U)
    SAU_INIT_REGION(1);
#endif

#if defined(SAU_INIT_REGION2) && (SAU_INIT_REGION2 == 1U)
    SAU_INIT_REGION(2);
#endif

#if defined(SAU_INIT_REGION3) && (SAU_INIT_REGION3 == 1U)
    SAU_INIT_REGION(3);
#endif

#if defined(SAU_INIT_REGION4) && (SAU_INIT_REGION4 == 1U)
    SAU_INIT_REGION(4);
#endif

#if defined(SAU_INIT_REGION5) && (SAU_INIT_REGION5 == 1U)
    SAU_INIT_REGION(5);
#endif

#if defined(SAU_INIT_REGION6) && (SAU_INIT_REGION6 == 1U)
    SAU_INIT_REGION(6);
#endif

#if defined(SAU_INIT_REGION7) && (SAU_INIT_REGION7 == 1U)
    SAU_INIT_REGION(7);
#endif

    /* repeat this for all possible SAU regions */

#endif /* defined (__SAUREGION_PRESENT) && (__SAUREGION_PRESENT == 1U) */

#if defined(SAU_INIT_CTRL) && (SAU_INIT_CTRL == 1U)
    SAU->CTRL = ((SAU_INIT_CTRL_ENABLE << SAU_CTRL_ENABLE_Pos) & SAU_CTRL_ENABLE_Msk) |
                ((SAU_INIT_CTRL_ALLNS << SAU_CTRL_ALLNS_Pos) & SAU_CTRL_ALLNS_Msk);
#endif

#if defined(SCB_CSR_AIRCR_INIT) && (SCB_CSR_AIRCR_INIT == 1U)
    SCB->SCR = (SCB->SCR & ~(SCB_SCR_SLEEPDEEPS_Msk)) |
               ((SCB_CSR_DEEPSLEEPS_VAL << SCB_SCR_SLEEPDEEPS_Pos) & SCB_SCR_SLEEPDEEPS_Msk);

    SCB->AIRCR = (SCB->AIRCR & ~(SCB_AIRCR_VECTKEY_Msk | SCB_AIRCR_SYSRESETREQS_Msk | SCB_AIRCR_BFHFNMINS_Msk |
                                 SCB_AIRCR_PRIS_Msk)) |
                 ((0x05FAU << SCB_AIRCR_VECTKEY_Pos) & SCB_AIRCR_VECTKEY_Msk) |
                 ((SCB_AIRCR_SYSRESETREQS_VAL << SCB_AIRCR_SYSRESETREQS_Pos) & SCB_AIRCR_SYSRESETREQS_Msk) |
                 ((SCB_AIRCR_PRIS_VAL << SCB_AIRCR_PRIS_Pos) & SCB_AIRCR_PRIS_Msk) |
                 ((SCB_AIRCR_BFHFNMINS_VAL << SCB_AIRCR_BFHFNMINS_Pos) & SCB_AIRCR_BFHFNMINS_Msk);
#endif /* defined (SCB_CSR_AIRCR_INIT) && (SCB_CSR_AIRCR_INIT == 1U) */

#if (((defined(__FPU_USED) && (__FPU_USED == 1U)) || (defined(__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0))) && \
     (defined(TZ_FPU_NS_USAGE) && (TZ_FPU_NS_USAGE == 1U)))

    SCB->NSACR = (SCB->NSACR & ~(SCB_NSACR_CP10_Msk | SCB_NSACR_CP11_Msk)) |
                 ((SCB_NSACR_CP10_11_VAL << SCB_NSACR_CP10_Pos) & (SCB_NSACR_CP10_Msk | SCB_NSACR_CP11_Msk));

    FPU->FPCCR = (FPU->FPCCR & ~(FPU_FPCCR_TS_Msk | FPU_FPCCR_CLRONRETS_Msk | FPU_FPCCR_CLRONRET_Msk)) |
                 ((FPU_FPCCR_TS_VAL << FPU_FPCCR_TS_Pos) & FPU_FPCCR_TS_Msk) |
                 ((FPU_FPCCR_CLRONRETS_VAL << FPU_FPCCR_CLRONRETS_Pos) & FPU_FPCCR_CLRONRETS_Msk) |
                 ((FPU_FPCCR_CLRONRET_VAL << FPU_FPCCR_CLRONRET_Pos) & FPU_FPCCR_CLRONRET_Msk);
#endif
#if defined(NVIC_INIT_ITNS0) && (NVIC_INIT_ITNS0 == 1U)
    NVIC->ITNS[0] = NVIC_INIT_ITNS0_VAL;
#endif

#if defined(NVIC_INIT_ITNS1) && (NVIC_INIT_ITNS1 == 1U)
    NVIC->ITNS[1] = NVIC_INIT_ITNS1_VAL;
#endif

#if defined(NVIC_INIT_ITNS2) && (NVIC_INIT_ITNS2 == 1U)
    NVIC->ITNS[2] = NVIC_INIT_ITNS2_VAL;
#endif

#if defined(NVIC_INIT_ITNS3) && (NVIC_INIT_ITNS3 == 1U)
    NVIC->ITNS[3] = NVIC_INIT_ITNS3_VAL;
#endif

#if defined(NVIC_INIT_ITNS4) && (NVIC_INIT_ITNS4 == 1U)
    NVIC->ITNS[4] = NVIC_INIT_ITNS4_VAL;
#endif

#if defined(NVIC_INIT_ITNS5) && (NVIC_INIT_ITNS5 == 1U)
    NVIC->ITNS[5] = NVIC_INIT_ITNS5_VAL;
#endif

#if defined(NVIC_INIT_ITNS6) && (NVIC_INIT_ITNS6 == 1U)
    NVIC->ITNS[6] = NVIC_INIT_ITNS6_VAL;
#endif

#if defined(NVIC_INIT_ITNS7) && (NVIC_INIT_ITNS7 == 1U)
    NVIC->ITNS[7] = NVIC_INIT_ITNS7_VAL;
#endif

#if defined(NVIC_INIT_ITNS8) && (NVIC_INIT_ITNS8 == 1U)
    NVIC->ITNS[8] = NVIC_INIT_ITNS8_VAL;
#endif

#if defined(NVIC_INIT_ITNS9) && (NVIC_INIT_ITNS9 == 1U)
    NVIC->ITNS[9] = NVIC_INIT_ITNS9_VAL;
#endif

#if defined(NVIC_INIT_ITNS10) && (NVIC_INIT_ITNS10 == 1U)
    NVIC->ITNS[10] = NVIC_INIT_ITNS10_VAL;
#endif

#if defined(NVIC_INIT_ITNS11) && (NVIC_INIT_ITNS11 == 1U)
    NVIC->ITNS[11] = NVIC_INIT_ITNS11_VAL;
#endif

#if defined(NVIC_INIT_ITNS12) && (NVIC_INIT_ITNS12 == 1U)
    NVIC->ITNS[12] = NVIC_INIT_ITNS12_VAL;
#endif

#if defined(NVIC_INIT_ITNS13) && (NVIC_INIT_ITNS13 == 1U)
    NVIC->ITNS[13] = NVIC_INIT_ITNS13_VAL;
#endif

#if defined(NVIC_INIT_ITNS14) && (NVIC_INIT_ITNS14 == 1U)
    NVIC->ITNS[14] = NVIC_INIT_ITNS14_VAL;
#endif

#if defined(NVIC_INIT_ITNS15) && (NVIC_INIT_ITNS15 == 1U)
    NVIC->ITNS[15] = NVIC_INIT_ITNS15_VAL;
#endif
    /* repeat this for all possible ITNS elements */
}

#endif /* PARTITION_ARMCM55_H */
