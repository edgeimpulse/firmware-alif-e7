/* Copyright (c) 2021 ALIF SEMICONDUCTOR

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ALIF SEMICONDUCTOR nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/

/**************************************************************************//**
 * @file     Driver_UTIMER.c
 * @author   Girish BN
 * @email    girish.bn@alifsemi.com
 * @version  V1.0.0
 * @date     21-Aug-2020
 * @brief    CMSIS-Driver for UTIMER.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#include "Driver_UTIMER.h"
#include "UTIMER_ll_drv.h"
#include "RTE_Device.h"

#if !(RTE_UTIMER)
#error "UTIMER is not enabled in RTE_Device.h"
#endif

#if (defined(RTE_Drivers_UTIMER) && !RTE_UTIMER)
#error "UTIMER not configured in RTE_Device.h!"
#endif

UTIMER_TIMER_INFO Utimer_info[ARM_UTIMER_MAX_SUPPORTING_CHANNEL] = {
    {//CHANNEL0
        .timer_cycle            = RTE_UTIMER_CHANNEL0_SINGLE_CYCLE_ENABLE,
        .buf_trough_n_crest     = RTE_UTIMER_CHANNEL0_BUF_TROUGH_N_CREST,
        .driver_A               = RTE_UTIMER_CHANNEL0_DRIVER_A,
        .driver_B               = RTE_UTIMER_CHANNEL0_DRIVER_B,
        .cmp_drive_start_state  = RTE_UTIMER_CHANNEL0_CMP_START_STATE,
        .cmp_drive_stop_state   = RTE_UTIMER_CHANNEL0_CMP_STOP_STATE,
        .buffering_event_at_crest = RTE_UTIMER_CHANNEL0_EVENT_AT_CREST,
        .buffering_event_at_trough = RTE_UTIMER_CHANNEL0_EVENT_AT_TROUGH,
        .buffering_type         = RTE_UTIMER_CHANNEL0_BUFFERING_TYPE,
        .buffering_type_A       = RTE_UTIMER_CHANNEL0_BUFFERING_TYPE_A,
        .buffering_type_B       = RTE_UTIMER_CHANNEL0_BUFFERING_TYPE_B,
        .flag                   = UTIMER_CHANNEL_UNINITIALIZED,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL0_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL0_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL0_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL0_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL0_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL0_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL0_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority= RTE_UTIMER_CHANNEL0_UNDER_FLOW_IRQ_PRIORITY,
    },
    {//CHANNEL1
        .timer_cycle            = RTE_UTIMER_CHANNEL1_SINGLE_CYCLE_ENABLE,
        .buf_trough_n_crest     = RTE_UTIMER_CHANNEL1_BUF_TROUGH_N_CREST,
        .driver_A               = RTE_UTIMER_CHANNEL1_DRIVER_A,
        .driver_B               = RTE_UTIMER_CHANNEL1_DRIVER_B,
        .cmp_drive_start_state  = RTE_UTIMER_CHANNEL1_CMP_START_STATE,
        .cmp_drive_stop_state   = RTE_UTIMER_CHANNEL1_CMP_STOP_STATE,
        .buffering_event_at_crest = RTE_UTIMER_CHANNEL1_EVENT_AT_CREST,
        .buffering_event_at_trough = RTE_UTIMER_CHANNEL1_EVENT_AT_TROUGH,
        .buffering_type         = RTE_UTIMER_CHANNEL1_BUFFERING_TYPE,
        .buffering_type_A       = RTE_UTIMER_CHANNEL1_BUFFERING_TYPE_A,
        .buffering_type_B       = RTE_UTIMER_CHANNEL1_BUFFERING_TYPE_B,
        .flag                   = UTIMER_CHANNEL_UNINITIALIZED,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL1_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL1_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL1_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL1_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL1_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL1_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL1_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority= RTE_UTIMER_CHANNEL1_UNDER_FLOW_IRQ_PRIORITY
    },
    {//CHANNEL2
        .timer_cycle            = RTE_UTIMER_CHANNEL2_SINGLE_CYCLE_ENABLE,
        .buf_trough_n_crest     = RTE_UTIMER_CHANNEL2_BUF_TROUGH_N_CREST,
        .driver_A               = RTE_UTIMER_CHANNEL2_DRIVER_A,
        .driver_B               = RTE_UTIMER_CHANNEL2_DRIVER_B,
        .cmp_drive_start_state  = RTE_UTIMER_CHANNEL2_CMP_START_STATE,
        .cmp_drive_stop_state   = RTE_UTIMER_CHANNEL2_CMP_STOP_STATE,
        .buffering_event_at_crest = RTE_UTIMER_CHANNEL2_EVENT_AT_CREST,
        .buffering_event_at_trough = RTE_UTIMER_CHANNEL2_EVENT_AT_TROUGH,
        .buffering_type         = RTE_UTIMER_CHANNEL2_BUFFERING_TYPE,
        .buffering_type_A       = RTE_UTIMER_CHANNEL2_BUFFERING_TYPE_A,
        .buffering_type_B       = RTE_UTIMER_CHANNEL2_BUFFERING_TYPE_B,
        .flag                   = UTIMER_CHANNEL_UNINITIALIZED,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL2_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL2_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL2_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL2_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL2_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL2_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL2_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority= RTE_UTIMER_CHANNEL2_UNDER_FLOW_IRQ_PRIORITY
    },
    {//CHANNEL3
        .timer_cycle            = RTE_UTIMER_CHANNEL3_SINGLE_CYCLE_ENABLE,
        .buf_trough_n_crest     = RTE_UTIMER_CHANNEL3_BUF_TROUGH_N_CREST,
        .driver_A               = RTE_UTIMER_CHANNEL3_DRIVER_A,
        .driver_B               = RTE_UTIMER_CHANNEL3_DRIVER_B,
        .cmp_drive_start_state  = RTE_UTIMER_CHANNEL3_CMP_START_STATE,
        .cmp_drive_stop_state   = RTE_UTIMER_CHANNEL3_CMP_STOP_STATE,
        .buffering_event_at_crest = RTE_UTIMER_CHANNEL3_EVENT_AT_CREST,
        .buffering_event_at_trough = RTE_UTIMER_CHANNEL3_EVENT_AT_TROUGH,
        .buffering_type         = RTE_UTIMER_CHANNEL3_BUFFERING_TYPE,
        .buffering_type_A       = RTE_UTIMER_CHANNEL3_BUFFERING_TYPE_A,
        .buffering_type_B       = RTE_UTIMER_CHANNEL3_BUFFERING_TYPE_B,
        .flag                   = UTIMER_CHANNEL_UNINITIALIZED,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL3_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL3_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL3_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL3_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL3_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL3_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL3_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority= RTE_UTIMER_CHANNEL3_UNDER_FLOW_IRQ_PRIORITY
    },
    {//CHANNEL4
        .timer_cycle            = RTE_UTIMER_CHANNEL4_SINGLE_CYCLE_ENABLE,
        .buf_trough_n_crest     = RTE_UTIMER_CHANNEL4_BUF_TROUGH_N_CREST,
        .driver_A               = RTE_UTIMER_CHANNEL4_DRIVER_A,
        .driver_B               = RTE_UTIMER_CHANNEL4_DRIVER_B,
        .cmp_drive_start_state  = RTE_UTIMER_CHANNEL4_CMP_START_STATE,
        .cmp_drive_stop_state   = RTE_UTIMER_CHANNEL4_CMP_STOP_STATE,
        .buffering_event_at_crest = RTE_UTIMER_CHANNEL4_EVENT_AT_CREST,
        .buffering_event_at_trough = RTE_UTIMER_CHANNEL4_EVENT_AT_TROUGH,
        .buffering_type         = RTE_UTIMER_CHANNEL4_BUFFERING_TYPE,
        .buffering_type_A       = RTE_UTIMER_CHANNEL4_BUFFERING_TYPE_A,
        .buffering_type_B       = RTE_UTIMER_CHANNEL4_BUFFERING_TYPE_B,
        .flag                   = UTIMER_CHANNEL_UNINITIALIZED,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL4_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL4_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL4_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL4_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL4_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL4_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL4_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority= RTE_UTIMER_CHANNEL4_UNDER_FLOW_IRQ_PRIORITY
    },
    {//CHANNEL5
        .timer_cycle            = RTE_UTIMER_CHANNEL5_SINGLE_CYCLE_ENABLE,
        .buf_trough_n_crest     = RTE_UTIMER_CHANNEL5_BUF_TROUGH_N_CREST,
        .driver_A               = RTE_UTIMER_CHANNEL5_DRIVER_A,
        .driver_B               = RTE_UTIMER_CHANNEL5_DRIVER_B,
        .cmp_drive_start_state  = RTE_UTIMER_CHANNEL5_CMP_START_STATE,
        .cmp_drive_stop_state   = RTE_UTIMER_CHANNEL5_CMP_STOP_STATE,
        .buffering_event_at_crest = RTE_UTIMER_CHANNEL5_EVENT_AT_CREST,
        .buffering_event_at_trough = RTE_UTIMER_CHANNEL5_EVENT_AT_TROUGH,
        .buffering_type         = RTE_UTIMER_CHANNEL5_BUFFERING_TYPE,
        .buffering_type_A       = RTE_UTIMER_CHANNEL5_BUFFERING_TYPE_A,
        .buffering_type_B       = RTE_UTIMER_CHANNEL5_BUFFERING_TYPE_B,
        .flag                   = UTIMER_CHANNEL_UNINITIALIZED,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL5_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL5_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL5_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL5_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL5_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL5_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL5_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority= RTE_UTIMER_CHANNEL5_UNDER_FLOW_IRQ_PRIORITY
    },
    {//CHANNEL6
        .timer_cycle            = RTE_UTIMER_CHANNEL6_SINGLE_CYCLE_ENABLE,
        .buf_trough_n_crest     = RTE_UTIMER_CHANNEL6_BUF_TROUGH_N_CREST,
        .driver_A               = RTE_UTIMER_CHANNEL6_DRIVER_A,
        .driver_B               = RTE_UTIMER_CHANNEL6_DRIVER_B,
        .cmp_drive_start_state  = RTE_UTIMER_CHANNEL6_CMP_START_STATE,
        .cmp_drive_stop_state   = RTE_UTIMER_CHANNEL6_CMP_STOP_STATE,
        .buffering_event_at_crest = RTE_UTIMER_CHANNEL6_EVENT_AT_CREST,
        .buffering_event_at_trough = RTE_UTIMER_CHANNEL6_EVENT_AT_TROUGH,
        .buffering_type         = RTE_UTIMER_CHANNEL6_BUFFERING_TYPE,
        .buffering_type_A       = RTE_UTIMER_CHANNEL6_BUFFERING_TYPE_A,
        .buffering_type_B       = RTE_UTIMER_CHANNEL6_BUFFERING_TYPE_B,
        .flag                   = UTIMER_CHANNEL_UNINITIALIZED,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL6_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL6_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL6_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL6_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL6_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL6_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL6_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority= RTE_UTIMER_CHANNEL6_UNDER_FLOW_IRQ_PRIORITY
    },
    {//CHANNEL7
        .timer_cycle            = RTE_UTIMER_CHANNEL7_SINGLE_CYCLE_ENABLE,
        .buf_trough_n_crest     = RTE_UTIMER_CHANNEL7_BUF_TROUGH_N_CREST,
        .driver_A               = RTE_UTIMER_CHANNEL7_DRIVER_A,
        .driver_B               = RTE_UTIMER_CHANNEL7_DRIVER_B,
        .cmp_drive_start_state  = RTE_UTIMER_CHANNEL7_CMP_START_STATE,
        .cmp_drive_stop_state   = RTE_UTIMER_CHANNEL7_CMP_STOP_STATE,
        .buffering_event_at_crest = RTE_UTIMER_CHANNEL7_EVENT_AT_CREST,
        .buffering_event_at_trough = RTE_UTIMER_CHANNEL7_EVENT_AT_TROUGH,
        .buffering_type         = RTE_UTIMER_CHANNEL7_BUFFERING_TYPE,
        .buffering_type_A       = RTE_UTIMER_CHANNEL7_BUFFERING_TYPE_A,
        .buffering_type_B       = RTE_UTIMER_CHANNEL7_BUFFERING_TYPE_B,
        .flag                   = UTIMER_CHANNEL_UNINITIALIZED,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL7_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL7_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL7_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL7_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL7_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL7_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL7_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority= RTE_UTIMER_CHANNEL7_UNDER_FLOW_IRQ_PRIORITY
    },
    {//CHANNEL8
        .timer_cycle            = RTE_UTIMER_CHANNEL8_SINGLE_CYCLE_ENABLE,
        .buf_trough_n_crest     = RTE_UTIMER_CHANNEL8_BUF_TROUGH_N_CREST,
        .driver_A               = RTE_UTIMER_CHANNEL8_DRIVER_A,
        .driver_B               = RTE_UTIMER_CHANNEL8_DRIVER_B,
        .cmp_drive_start_state  = RTE_UTIMER_CHANNEL8_CMP_START_STATE,
        .cmp_drive_stop_state   = RTE_UTIMER_CHANNEL8_CMP_STOP_STATE,
        .buffering_event_at_crest = RTE_UTIMER_CHANNEL8_EVENT_AT_CREST,
        .buffering_event_at_trough = RTE_UTIMER_CHANNEL8_EVENT_AT_TROUGH,
        .buffering_type         = RTE_UTIMER_CHANNEL8_BUFFERING_TYPE,
        .buffering_type_A       = RTE_UTIMER_CHANNEL8_BUFFERING_TYPE_A,
        .buffering_type_B       = RTE_UTIMER_CHANNEL8_BUFFERING_TYPE_B,
        .flag                   = UTIMER_CHANNEL_UNINITIALIZED,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL8_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL8_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL8_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL8_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL8_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL8_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL8_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority= RTE_UTIMER_CHANNEL8_UNDER_FLOW_IRQ_PRIORITY
    },
    {//CHANNEL9
        .timer_cycle            = RTE_UTIMER_CHANNEL9_SINGLE_CYCLE_ENABLE,
        .buf_trough_n_crest     = RTE_UTIMER_CHANNEL9_BUF_TROUGH_N_CREST,
        .driver_A               = RTE_UTIMER_CHANNEL9_DRIVER_A,
        .driver_B               = RTE_UTIMER_CHANNEL9_DRIVER_B,
        .cmp_drive_start_state  = RTE_UTIMER_CHANNEL9_CMP_START_STATE,
        .cmp_drive_stop_state   = RTE_UTIMER_CHANNEL9_CMP_STOP_STATE,
        .buffering_event_at_crest = RTE_UTIMER_CHANNEL9_EVENT_AT_CREST,
        .buffering_event_at_trough = RTE_UTIMER_CHANNEL9_EVENT_AT_TROUGH,
        .buffering_type         = RTE_UTIMER_CHANNEL9_BUFFERING_TYPE,
        .buffering_type_A       = RTE_UTIMER_CHANNEL9_BUFFERING_TYPE_A,
        .buffering_type_B       = RTE_UTIMER_CHANNEL9_BUFFERING_TYPE_B,
        .flag                   = UTIMER_CHANNEL_UNINITIALIZED,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL9_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL9_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL9_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL9_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL9_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL9_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL9_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority= RTE_UTIMER_CHANNEL9_UNDER_FLOW_IRQ_PRIORITY
    },
    {//CHANNEL10
        .timer_cycle            = RTE_UTIMER_CHANNEL10_SINGLE_CYCLE_ENABLE,
        .buf_trough_n_crest     = RTE_UTIMER_CHANNEL10_BUF_TROUGH_N_CREST,
        .driver_A               = RTE_UTIMER_CHANNEL10_DRIVER_A,
        .driver_B               = RTE_UTIMER_CHANNEL10_DRIVER_B,
        .cmp_drive_start_state  = RTE_UTIMER_CHANNEL10_CMP_START_STATE,
        .cmp_drive_stop_state   = RTE_UTIMER_CHANNEL10_CMP_STOP_STATE,
        .buffering_event_at_crest = RTE_UTIMER_CHANNEL10_EVENT_AT_CREST,
        .buffering_event_at_trough = RTE_UTIMER_CHANNEL10_EVENT_AT_TROUGH,
        .buffering_type         = RTE_UTIMER_CHANNEL10_BUFFERING_TYPE,
        .buffering_type_A       = RTE_UTIMER_CHANNEL10_BUFFERING_TYPE_A,
        .buffering_type_B       = RTE_UTIMER_CHANNEL10_BUFFERING_TYPE_B,
        .flag                   = UTIMER_CHANNEL_UNINITIALIZED,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL10_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL10_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL10_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL10_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL10_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL10_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL10_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority= RTE_UTIMER_CHANNEL10_UNDER_FLOW_IRQ_PRIORITY
    },
    {//CHANNEL11
        .timer_cycle            = RTE_UTIMER_CHANNEL11_SINGLE_CYCLE_ENABLE,
        .buf_trough_n_crest     = RTE_UTIMER_CHANNEL11_BUF_TROUGH_N_CREST,
        .driver_A               = RTE_UTIMER_CHANNEL11_DRIVER_A,
        .driver_B               = RTE_UTIMER_CHANNEL11_DRIVER_B,
        .cmp_drive_start_state  = RTE_UTIMER_CHANNEL11_CMP_START_STATE,
        .cmp_drive_stop_state   = RTE_UTIMER_CHANNEL11_CMP_STOP_STATE,
        .buffering_event_at_crest = RTE_UTIMER_CHANNEL11_EVENT_AT_CREST,
        .buffering_event_at_trough = RTE_UTIMER_CHANNEL11_EVENT_AT_TROUGH,
        .buffering_type         = RTE_UTIMER_CHANNEL11_BUFFERING_TYPE,
        .buffering_type_A       = RTE_UTIMER_CHANNEL11_BUFFERING_TYPE_A,
        .buffering_type_B       = RTE_UTIMER_CHANNEL11_BUFFERING_TYPE_B,
        .flag                   = UTIMER_CHANNEL_UNINITIALIZED,
        .capture_A_irq_priority = RTE_UTIMER_CHANNEL11_CAPTURE_A_IRQ_PRIORITY,
        .capture_B_irq_priority = RTE_UTIMER_CHANNEL11_CAPTURE_B_IRQ_PRIORITY,
        .capture_C_irq_priority = RTE_UTIMER_CHANNEL11_CAPTURE_C_IRQ_PRIORITY,
        .capture_D_irq_priority = RTE_UTIMER_CHANNEL11_CAPTURE_D_IRQ_PRIORITY,
        .capture_E_irq_priority = RTE_UTIMER_CHANNEL11_CAPTURE_E_IRQ_PRIORITY,
        .capture_F_irq_priority = RTE_UTIMER_CHANNEL11_CAPTURE_F_IRQ_PRIORITY,
        .over_flow_irq_priority = RTE_UTIMER_CHANNEL11_OVER_FLOW_IRQ_PRIORITY,
        .under_flow_irq_priority= RTE_UTIMER_CHANNEL11_UNDER_FLOW_IRQ_PRIORITY
    }
};

int32_t ARM_UTIMER_Initialize (UTIMER_resources_t *UTIMER, uint8_t channel, ARM_UTIMER_SignalEvent_t cb_unit_event) {

    if ((UTIMER->info == NULL) || (cb_unit_event == NULL))  {   return ARM_DRIVER_ERROR_PARAMETER;  }
    if (channel > ARM_UTIMER_CHANNEL11)                     {   return ARM_DRIVER_ERROR_PARAMETER;  }

    UTIMER_TIMER_INFO *info_ptr = (UTIMER_TIMER_INFO*)(UTIMER->info);

    if(info_ptr[channel].flag == UTIMER_CHANNEL_INITIALIZED )           {   return ARM_DRIVER_OK;   }

    info_ptr[channel].CB_function_ptr = cb_unit_event;

    info_ptr[channel].flag = UTIMER_CHANNEL_INITIALIZED;

    return ARM_DRIVER_OK;
}

int32_t ARM_UTIMER_PowerControl (UTIMER_resources_t *UTIMER, uint8_t channel, ARM_POWER_STATE state) {

    int32_t ret;

    if (UTIMER->info == NULL)                           {   return ARM_DRIVER_ERROR_PARAMETER;  }
    if (channel > ARM_UTIMER_CHANNEL11)                 {   return ARM_DRIVER_ERROR_PARAMETER;  }
    if (state   > ARM_POWER_FULL)                       {   return ARM_DRIVER_ERROR_PARAMETER;  }

    UTIMER_TIMER_INFO *info_ptr = (UTIMER_TIMER_INFO*)(UTIMER->info);

    switch (state)
    {
        case ARM_POWER_OFF:
        {
            ret = UTIMER_ll_PowerOff (UTIMER, channel);

            if (ret == ARM_DRIVER_OK) {
                info_ptr[channel].flag = UTIMER_CHANNEL_POWERED_OFF;
            }
            break;
        }
        case ARM_POWER_FULL:
        {
            if (info_ptr[channel].flag == UTIMER_CHANNEL_POWERED_FULL)  {   return ARM_DRIVER_OK;   }
            if (info_ptr[channel].flag != UTIMER_CHANNEL_INITIALIZED)   {   return ARM_DRIVER_ERROR;    }

            ret = UTIMER_ll_PowerFull (UTIMER, channel);

            if (ret == ARM_DRIVER_OK) {
                info_ptr[channel].flag = UTIMER_CHANNEL_POWERED_FULL;
            }
            break;
        }
        case ARM_POWER_LOW:
        default:
                return ARM_DRIVER_ERROR_UNSUPPORTED;
        break;
    }
    return ret;
}

int32_t ARM_UTIMER_Control (UTIMER_resources_t *UTIMER, uint8_t channel, uint32_t control_code, void *arg) {

    int32_t ret;

    if ((UTIMER->info == NULL) || (arg == NULL))    {   return ARM_DRIVER_ERROR_PARAMETER;  }
    if (channel > ARM_UTIMER_CHANNEL11)             {   return ARM_DRIVER_ERROR_PARAMETER;  }
    if (control_code > ARM_UTIMER_GET_DIRECTION)    {   return ARM_DRIVER_ERROR_PARAMETER;  }

    UTIMER_TIMER_INFO *info_ptr = (UTIMER_TIMER_INFO*)(UTIMER->info);

    switch (control_code)
    {
        case ARM_UTIMER_MODE_CONFIG:
        {
            if(info_ptr[channel].flag != UTIMER_CHANNEL_POWERED_FULL)         {   return ARM_DRIVER_ERROR;    }

            ret = UTIMER_ll_Mode_Config (UTIMER, channel, (UTIMER_MODE_CONFIG*)arg);
            break;
        }
        case ARM_UTIMER_SET_COUNT:
        {
            if(info_ptr[channel].flag < UTIMER_CHANNEL_CONTROL)                {   return ARM_DRIVER_ERROR;    }

            ret = UTIMER_ll_Set_Count (UTIMER, channel, (UTIMER_SET_OPERATION_CONFIG *) arg);
            break;
        }
        case ARM_UTIMER_GET_COUNT:
        {
            if(info_ptr[channel].flag < UTIMER_CHANNEL_CONTROL)                {   return ARM_DRIVER_ERROR;    }

            ret = UTIMER_ll_Get_Count (UTIMER, channel, (UTIMER_GET_OPERATION_CONFIG *) arg);
            break;
        }
        case ARM_UTIMER_GET_DIRECTION:
        {
            if(info_ptr[channel].flag < UTIMER_CHANNEL_CONTROL)                {   return ARM_DRIVER_ERROR;    }

            ret = UTIMER_ll_Get_Direction (UTIMER, channel, (uint32_t *) arg);
            break;
        }
        default:
        {
            return ARM_DRIVER_ERROR;
            break;
        }
    }

    if (ret == ARM_DRIVER_OK) {
        info_ptr[channel].flag = UTIMER_CHANNEL_CONTROL;
    }
    return ret;
}

int32_t ARM_UTIMER_Configure_Trigger (UTIMER_resources_t *UTIMER, uint8_t channel, uint8_t triggerFor, uint32_t triggerType) {

    int32_t ret;

    if (UTIMER->info == NULL)                           {   return ARM_DRIVER_ERROR_PARAMETER;  }
    if (channel > ARM_UTIMER_CHANNEL11)                 {   return ARM_DRIVER_ERROR_PARAMETER;  }
    if (triggerFor > ARM_UTIMER_TRIGGER_FOR_CAPTURE_B)  {   return ARM_DRIVER_ERROR_PARAMETER;  }

    //TODO: validate the "triggerType" argument

    UTIMER_TIMER_INFO *info_ptr = (UTIMER_TIMER_INFO*)(UTIMER->info);

    if (info_ptr[channel].flag < UTIMER_CHANNEL_CONTROL)   {   return ARM_DRIVER_ERROR;    }

    ret = UTIMER_ll_Configure_Trigger (UTIMER, channel, triggerFor, triggerType);

    if (ret == ARM_DRIVER_OK) {
        info_ptr[channel].flag = UTIMER_CHANNEL_TRIGGERED;
    }
    return ret;
}

int32_t ARM_UTIMER_Start (UTIMER_resources_t *UTIMER, uint8_t channel) {

    int32_t ret;

    if (UTIMER->info == NULL)           {   return ARM_DRIVER_ERROR_PARAMETER;  }
    if (channel > ARM_UTIMER_CHANNEL11) {   return ARM_DRIVER_ERROR_PARAMETER;  }

    UTIMER_TIMER_INFO *info_ptr = (UTIMER_TIMER_INFO*)(UTIMER->info);

    if(info_ptr[channel].flag < UTIMER_CHANNEL_CONTROL)    {   return ARM_DRIVER_ERROR;    }

    ret = UTIMER_ll_Start (UTIMER, channel);

    if (ret == ARM_DRIVER_OK) {
        info_ptr[channel].flag = UTIMER_CHANNEL_STARTED;
    }
    return ret;
}

int32_t ARM_UTIMER_Stop (UTIMER_resources_t *UTIMER, uint8_t channel, bool count_clear_option) {

    int32_t ret;

    if (channel > ARM_UTIMER_CHANNEL11)                 {   return ARM_DRIVER_ERROR_PARAMETER;  }

    UTIMER_TIMER_INFO *info_ptr = (UTIMER_TIMER_INFO*)(UTIMER->info);

    ret = UTIMER_ll_Stop (UTIMER, channel, count_clear_option);

    if (ret == ARM_DRIVER_OK) {
        info_ptr[channel].flag = UTIMER_CHANNEL_STOPED;
    }
    return ret;
}

int32_t ARM_UTIMER_Uninitialize (UTIMER_resources_t *UTIMER, uint8_t channel) {

    int32_t ret = 0;

    if (UTIMER->info == NULL)           {   return ARM_DRIVER_ERROR_PARAMETER;  }
    if (channel > ARM_UTIMER_CHANNEL11) {   return ARM_DRIVER_ERROR_PARAMETER;  }

    UTIMER_TIMER_INFO *info_ptr = (UTIMER_TIMER_INFO*)(UTIMER->info);

    info_ptr[channel].CB_function_ptr = NULL;
    info_ptr[channel].channel_direction_backup = 0;
    info_ptr[channel].channel_mode_backup = 0;

    ret = UTIMER_ll_Uninitialize (UTIMER, channel);

    if (ret == ARM_DRIVER_OK) {
        info_ptr[channel].flag = UTIMER_CHANNEL_STOPED;
    }

    return ret;
}

UTIMER_resources_t UTIMER0_Resources =
{
    .reg_base   = (uint32_t)UTIMER_BASE,
    .info       = (UTIMER_TIMER_INFO*)&Utimer_info
};

void UTIMER_IRQHandler_Compare_A (uint8_t channel) {

    UTIMER_ll_IRQHandler_Compare_A (&UTIMER0_Resources, channel);
}

void UTIMER_IRQHandler_Compare_B (uint8_t channel) {

    UTIMER_ll_IRQHandler_Compare_B (&UTIMER0_Resources, channel);
}

void UTIMER_IRQHandler_Compare_C (uint8_t channel) {

    UTIMER_ll_IRQHandler_Compare_C (&UTIMER0_Resources, channel);
}

void UTIMER_IRQHandler_Compare_D (uint8_t channel) {

    UTIMER_ll_IRQHandler_Compare_D (&UTIMER0_Resources, channel);
}

void UTIMER_IRQHandler_Compare_E (uint8_t channel) {

    UTIMER_ll_IRQHandler_Compare_E (&UTIMER0_Resources, channel);
}

void UTIMER_IRQHandler_Compare_F (uint8_t channel) {

    UTIMER_ll_IRQHandler_Compare_F (&UTIMER0_Resources, channel);
}

void UTIMER_IRQHandler_Compare_OverFlow (uint8_t channel) {

    UTIMER_ll_IRQHandler_Compare_OverFlow (&UTIMER0_Resources, channel);
}

void UTIMER_IRQHandler_Compare_UnderFlow (uint8_t channel) {

    UTIMER_ll_IRQHandler_Compare_UnderFlow (&UTIMER0_Resources, channel);
}

void UTIMER_IRQHandler0  (void) {    UTIMER_IRQHandler_Compare_A        (ARM_UTIMER_CHANNEL0);   }
void UTIMER_IRQHandler1  (void) {    UTIMER_IRQHandler_Compare_B        (ARM_UTIMER_CHANNEL0);   }
void UTIMER_IRQHandler2  (void) {    UTIMER_IRQHandler_Compare_C        (ARM_UTIMER_CHANNEL0);   }
void UTIMER_IRQHandler3  (void) {    UTIMER_IRQHandler_Compare_D        (ARM_UTIMER_CHANNEL0);   }
void UTIMER_IRQHandler4  (void) {    UTIMER_IRQHandler_Compare_E        (ARM_UTIMER_CHANNEL0);   }
void UTIMER_IRQHandler5  (void) {    UTIMER_IRQHandler_Compare_F        (ARM_UTIMER_CHANNEL0);   }
void UTIMER_IRQHandler6  (void) {    UTIMER_IRQHandler_Compare_UnderFlow(ARM_UTIMER_CHANNEL0);   }
void UTIMER_IRQHandler7  (void) {    UTIMER_IRQHandler_Compare_OverFlow (ARM_UTIMER_CHANNEL0);   }

void UTIMER_IRQHandler8  (void) {    UTIMER_IRQHandler_Compare_A        (ARM_UTIMER_CHANNEL1);   }
void UTIMER_IRQHandler9  (void) {    UTIMER_IRQHandler_Compare_B        (ARM_UTIMER_CHANNEL1);   }
void UTIMER_IRQHandler10 (void) {    UTIMER_IRQHandler_Compare_C        (ARM_UTIMER_CHANNEL1);   }
void UTIMER_IRQHandler11 (void) {    UTIMER_IRQHandler_Compare_D        (ARM_UTIMER_CHANNEL1);   }
void UTIMER_IRQHandler12 (void) {    UTIMER_IRQHandler_Compare_E        (ARM_UTIMER_CHANNEL1);   }
void UTIMER_IRQHandler13 (void) {    UTIMER_IRQHandler_Compare_F        (ARM_UTIMER_CHANNEL1);   }
void UTIMER_IRQHandler14 (void) {    UTIMER_IRQHandler_Compare_UnderFlow(ARM_UTIMER_CHANNEL1);   }
void UTIMER_IRQHandler15 (void) {    UTIMER_IRQHandler_Compare_OverFlow (ARM_UTIMER_CHANNEL1);   }

void UTIMER_IRQHandler16 (void) {    UTIMER_IRQHandler_Compare_A        (ARM_UTIMER_CHANNEL2);   }
void UTIMER_IRQHandler17 (void) {    UTIMER_IRQHandler_Compare_B        (ARM_UTIMER_CHANNEL2);   }
void UTIMER_IRQHandler18 (void) {    UTIMER_IRQHandler_Compare_C        (ARM_UTIMER_CHANNEL2);   }
void UTIMER_IRQHandler19 (void) {    UTIMER_IRQHandler_Compare_D        (ARM_UTIMER_CHANNEL2);   }
void UTIMER_IRQHandler20 (void) {    UTIMER_IRQHandler_Compare_E        (ARM_UTIMER_CHANNEL2);   }
void UTIMER_IRQHandler21 (void) {    UTIMER_IRQHandler_Compare_F        (ARM_UTIMER_CHANNEL2);   }
void UTIMER_IRQHandler22 (void) {    UTIMER_IRQHandler_Compare_UnderFlow(ARM_UTIMER_CHANNEL2);   }
void UTIMER_IRQHandler23 (void) {    UTIMER_IRQHandler_Compare_OverFlow (ARM_UTIMER_CHANNEL2);   }

void UTIMER_IRQHandler24 (void) {    UTIMER_IRQHandler_Compare_A        (ARM_UTIMER_CHANNEL3);   }
void UTIMER_IRQHandler25 (void) {    UTIMER_IRQHandler_Compare_B        (ARM_UTIMER_CHANNEL3);   }
void UTIMER_IRQHandler26 (void) {    UTIMER_IRQHandler_Compare_C        (ARM_UTIMER_CHANNEL3);   }
void UTIMER_IRQHandler27 (void) {    UTIMER_IRQHandler_Compare_D        (ARM_UTIMER_CHANNEL3);   }
void UTIMER_IRQHandler28 (void) {    UTIMER_IRQHandler_Compare_E        (ARM_UTIMER_CHANNEL3);   }
void UTIMER_IRQHandler29 (void) {    UTIMER_IRQHandler_Compare_F        (ARM_UTIMER_CHANNEL3);   }
void UTIMER_IRQHandler30 (void) {    UTIMER_IRQHandler_Compare_UnderFlow(ARM_UTIMER_CHANNEL3);   }
void UTIMER_IRQHandler31 (void) {    UTIMER_IRQHandler_Compare_OverFlow (ARM_UTIMER_CHANNEL3);   }

void UTIMER_IRQHandler32 (void) {    UTIMER_IRQHandler_Compare_A        (ARM_UTIMER_CHANNEL4);   }
void UTIMER_IRQHandler33 (void) {    UTIMER_IRQHandler_Compare_B        (ARM_UTIMER_CHANNEL4);   }
void UTIMER_IRQHandler34 (void) {    UTIMER_IRQHandler_Compare_C        (ARM_UTIMER_CHANNEL4);   }
void UTIMER_IRQHandler35 (void) {    UTIMER_IRQHandler_Compare_D        (ARM_UTIMER_CHANNEL4);   }
void UTIMER_IRQHandler36 (void) {    UTIMER_IRQHandler_Compare_E        (ARM_UTIMER_CHANNEL4);   }
void UTIMER_IRQHandler37 (void) {    UTIMER_IRQHandler_Compare_F        (ARM_UTIMER_CHANNEL4);   }
void UTIMER_IRQHandler38 (void) {    UTIMER_IRQHandler_Compare_UnderFlow(ARM_UTIMER_CHANNEL4);   }
void UTIMER_IRQHandler39 (void) {    UTIMER_IRQHandler_Compare_OverFlow (ARM_UTIMER_CHANNEL4);   }

void UTIMER_IRQHandler40 (void) {    UTIMER_IRQHandler_Compare_A        (ARM_UTIMER_CHANNEL5);   }
void UTIMER_IRQHandler41 (void) {    UTIMER_IRQHandler_Compare_B        (ARM_UTIMER_CHANNEL5);   }
void UTIMER_IRQHandler42 (void) {    UTIMER_IRQHandler_Compare_C        (ARM_UTIMER_CHANNEL5);   }
void UTIMER_IRQHandler43 (void) {    UTIMER_IRQHandler_Compare_D        (ARM_UTIMER_CHANNEL5);   }
void UTIMER_IRQHandler44 (void) {    UTIMER_IRQHandler_Compare_E        (ARM_UTIMER_CHANNEL5);   }
void UTIMER_IRQHandler45 (void) {    UTIMER_IRQHandler_Compare_F        (ARM_UTIMER_CHANNEL5);   }
void UTIMER_IRQHandler46 (void) {    UTIMER_IRQHandler_Compare_UnderFlow(ARM_UTIMER_CHANNEL5);   }
void UTIMER_IRQHandler47 (void) {    UTIMER_IRQHandler_Compare_OverFlow (ARM_UTIMER_CHANNEL5);   }

void UTIMER_IRQHandler48 (void) {    UTIMER_IRQHandler_Compare_A        (ARM_UTIMER_CHANNEL6);   }
void UTIMER_IRQHandler49 (void) {    UTIMER_IRQHandler_Compare_B        (ARM_UTIMER_CHANNEL6);   }
void UTIMER_IRQHandler50 (void) {    UTIMER_IRQHandler_Compare_C        (ARM_UTIMER_CHANNEL6);   }
void UTIMER_IRQHandler51 (void) {    UTIMER_IRQHandler_Compare_D        (ARM_UTIMER_CHANNEL6);   }
void UTIMER_IRQHandler52 (void) {    UTIMER_IRQHandler_Compare_E        (ARM_UTIMER_CHANNEL6);   }
void UTIMER_IRQHandler53 (void) {    UTIMER_IRQHandler_Compare_F        (ARM_UTIMER_CHANNEL6);   }
void UTIMER_IRQHandler54 (void) {    UTIMER_IRQHandler_Compare_UnderFlow(ARM_UTIMER_CHANNEL6);   }
void UTIMER_IRQHandler55 (void) {    UTIMER_IRQHandler_Compare_OverFlow (ARM_UTIMER_CHANNEL6);   }

void UTIMER_IRQHandler56 (void) {    UTIMER_IRQHandler_Compare_A        (ARM_UTIMER_CHANNEL7);   }
void UTIMER_IRQHandler57 (void) {    UTIMER_IRQHandler_Compare_B        (ARM_UTIMER_CHANNEL7);   }
void UTIMER_IRQHandler58 (void) {    UTIMER_IRQHandler_Compare_C        (ARM_UTIMER_CHANNEL7);   }
void UTIMER_IRQHandler59 (void) {    UTIMER_IRQHandler_Compare_D        (ARM_UTIMER_CHANNEL7);   }
void UTIMER_IRQHandler60 (void) {    UTIMER_IRQHandler_Compare_E        (ARM_UTIMER_CHANNEL7);   }
void UTIMER_IRQHandler61 (void) {    UTIMER_IRQHandler_Compare_F        (ARM_UTIMER_CHANNEL7);   }
void UTIMER_IRQHandler62 (void) {    UTIMER_IRQHandler_Compare_UnderFlow(ARM_UTIMER_CHANNEL7);   }
void UTIMER_IRQHandler63 (void) {    UTIMER_IRQHandler_Compare_OverFlow (ARM_UTIMER_CHANNEL7);   }

void UTIMER_IRQHandler64 (void) {    UTIMER_IRQHandler_Compare_A        (ARM_UTIMER_CHANNEL8);   }
void UTIMER_IRQHandler65 (void) {    UTIMER_IRQHandler_Compare_B        (ARM_UTIMER_CHANNEL8);   }
void UTIMER_IRQHandler66 (void) {    UTIMER_IRQHandler_Compare_C        (ARM_UTIMER_CHANNEL8);   }
void UTIMER_IRQHandler67 (void) {    UTIMER_IRQHandler_Compare_D        (ARM_UTIMER_CHANNEL8);   }
void UTIMER_IRQHandler68 (void) {    UTIMER_IRQHandler_Compare_E        (ARM_UTIMER_CHANNEL8);   }
void UTIMER_IRQHandler69 (void) {    UTIMER_IRQHandler_Compare_F        (ARM_UTIMER_CHANNEL8);   }
void UTIMER_IRQHandler70 (void) {    UTIMER_IRQHandler_Compare_UnderFlow(ARM_UTIMER_CHANNEL8);   }
void UTIMER_IRQHandler71 (void) {    UTIMER_IRQHandler_Compare_OverFlow (ARM_UTIMER_CHANNEL8);   }

void UTIMER_IRQHandler72 (void) {    UTIMER_IRQHandler_Compare_A        (ARM_UTIMER_CHANNEL9);   }
void UTIMER_IRQHandler73 (void) {    UTIMER_IRQHandler_Compare_B        (ARM_UTIMER_CHANNEL9);   }
void UTIMER_IRQHandler74 (void) {    UTIMER_IRQHandler_Compare_C        (ARM_UTIMER_CHANNEL9);   }
void UTIMER_IRQHandler75 (void) {    UTIMER_IRQHandler_Compare_D        (ARM_UTIMER_CHANNEL9);   }
void UTIMER_IRQHandler76 (void) {    UTIMER_IRQHandler_Compare_E        (ARM_UTIMER_CHANNEL9);   }
void UTIMER_IRQHandler77 (void) {    UTIMER_IRQHandler_Compare_F        (ARM_UTIMER_CHANNEL9);   }
void UTIMER_IRQHandler78 (void) {    UTIMER_IRQHandler_Compare_UnderFlow(ARM_UTIMER_CHANNEL9);   }
void UTIMER_IRQHandler79 (void) {    UTIMER_IRQHandler_Compare_OverFlow (ARM_UTIMER_CHANNEL9);   }

void UTIMER_IRQHandler80 (void) {    UTIMER_IRQHandler_Compare_A        (ARM_UTIMER_CHANNEL10);   }
void UTIMER_IRQHandler81 (void) {    UTIMER_IRQHandler_Compare_B        (ARM_UTIMER_CHANNEL10);   }
void UTIMER_IRQHandler82 (void) {    UTIMER_IRQHandler_Compare_C        (ARM_UTIMER_CHANNEL10);   }
void UTIMER_IRQHandler83 (void) {    UTIMER_IRQHandler_Compare_D        (ARM_UTIMER_CHANNEL10);   }
void UTIMER_IRQHandler84 (void) {    UTIMER_IRQHandler_Compare_E        (ARM_UTIMER_CHANNEL10);   }
void UTIMER_IRQHandler85 (void) {    UTIMER_IRQHandler_Compare_F        (ARM_UTIMER_CHANNEL10);   }
void UTIMER_IRQHandler86 (void) {    UTIMER_IRQHandler_Compare_UnderFlow(ARM_UTIMER_CHANNEL10);   }
void UTIMER_IRQHandler87 (void) {    UTIMER_IRQHandler_Compare_OverFlow (ARM_UTIMER_CHANNEL10);   }

void UTIMER_IRQHandler88 (void) {    UTIMER_IRQHandler_Compare_A        (ARM_UTIMER_CHANNEL11);   }
void UTIMER_IRQHandler89 (void) {    UTIMER_IRQHandler_Compare_B        (ARM_UTIMER_CHANNEL11);   }
void UTIMER_IRQHandler90 (void) {    UTIMER_IRQHandler_Compare_C        (ARM_UTIMER_CHANNEL11);   }
void UTIMER_IRQHandler91 (void) {    UTIMER_IRQHandler_Compare_D        (ARM_UTIMER_CHANNEL11);   }
void UTIMER_IRQHandler92 (void) {    UTIMER_IRQHandler_Compare_E        (ARM_UTIMER_CHANNEL11);   }
void UTIMER_IRQHandler93 (void) {    UTIMER_IRQHandler_Compare_F        (ARM_UTIMER_CHANNEL11);   }
void UTIMER_IRQHandler94 (void) {    UTIMER_IRQHandler_Compare_UnderFlow(ARM_UTIMER_CHANNEL11);   }
void UTIMER_IRQHandler95 (void) {    UTIMER_IRQHandler_Compare_OverFlow (ARM_UTIMER_CHANNEL11);   }

int32_t ARM_UTIMER0_Initialize (uint8_t channel, ARM_UTIMER_SignalEvent_t cb_unit_event) {

    return ARM_UTIMER_Initialize (&UTIMER0_Resources, channel, cb_unit_event);
}

int32_t ARM_UTIMER0_PowerControl (uint8_t channel, ARM_POWER_STATE state) {

    return ARM_UTIMER_PowerControl (&UTIMER0_Resources, channel, state);
}

int32_t ARM_UTIMER0_Control (uint8_t channel, uint32_t control_code, void *arg) {

    return ARM_UTIMER_Control (&UTIMER0_Resources, channel, control_code, arg);
}

int32_t ARM_UTIMER0_Configure_Trigger (uint8_t channel, uint8_t triggerFor, uint32_t triggerType) {

    return ARM_UTIMER_Configure_Trigger (&UTIMER0_Resources, channel, triggerFor, triggerType);
}

int32_t ARM_UTIMER0_Start (uint8_t channel) {

    return ARM_UTIMER_Start (&UTIMER0_Resources, channel);
}

int32_t ARM_UTIMER0_Stop (uint8_t channel, bool count_clear_option) {

    return ARM_UTIMER_Stop (&UTIMER0_Resources, channel, count_clear_option);
}

int32_t ARM_UTIMER0_Uninitialize (uint8_t channel) {

    return ARM_UTIMER_Uninitialize (&UTIMER0_Resources, channel);
}

/*UTIMER Control Block */
ARM_DRIVER_UTIMER DRIVER_UTIMER0 = {
    ARM_UTIMER0_Initialize,
    ARM_UTIMER0_PowerControl,
    ARM_UTIMER0_Control,
    ARM_UTIMER0_Configure_Trigger,
    ARM_UTIMER0_Start,
    ARM_UTIMER0_Stop,
    ARM_UTIMER0_Uninitialize
};
