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

#ifndef TIMING_ADAPTER_H_
#define TIMING_ADAPTER_H_

#include <stdint.h>

#if defined __cplusplus
extern "C" {
#endif

/** TIMING ADAPTER
 *
 * The timing adapter is an AXI-to-AXI bridge for providing well-defined memory timing
 * to allow performance evaluation of an AXI manager. The bridge works by delaying the
 * responses from the memory according to run-time configurable parameters that can be
 * set in the timing adapter. Parameters include read and write response latencies,
 * no. of outstanding transactions, and a model of interferring traffic.
 */

struct timing_adapter {
    uintptr_t base_addr;
};

/** LIMITATIONS FOR FVP:
 *
 *  - TA_MODE is hardcoded to 1 (one) at all times.
 *  - Only TA_PERFCTRL_AWTRANS and TA_PERFCTRL_ARTRANS support is
 *    implemented for the performance counter.
 */

struct timing_adapter_settings {
    uint32_t maxr;       // 6-bit field. Max no. of pending reads. 0=infinite
    uint32_t maxw;       // 6-bit field. Max no. of pending writes. 0=infinite
    uint32_t maxrw;      // 6-bit field. Max no. of pending reads+writes. 0=infinite
    uint32_t rlatency;   // 12-bit field. Minimum latency (clock cycles) from AVALID to RVALID.
    uint32_t wlatency;   // 12-bit field. Minimum latency (clock cycles) from WVALID&WLAST to BVALID.
    uint32_t pulse_on;   // No. of cycles addresses let through (0-65535).
    uint32_t pulse_off;  // No. of cycles addresses blocked (0-65535).
    uint32_t bwcap;      // 16-bit field. Max no. of 64-bit words transfered per pulse cycle 0=infinite
    uint32_t perfctrl;   // 6-bit field selecting an event for event counter 0=default
    uint32_t perfcnt;    // 32-bit event counter
    uint32_t mode;       // Bit 0: 1=enable dynamic clocking to avoid underrun
                         // Bit 1: 1=enable random AR reordering (0=default)
                         // Bit 2: 1=enable random R reordering (0=default)
                         // Bit 3: 1=enable random B reordering (0=default)
                         // Bit 11-4: Frequency scale 0=full speed, 255=(1/256) speed
    uint32_t maxpending; // (Read-only) Max supported value in MAXR and MAXW registers
    uint32_t histbin;    // Controlls which histogram bin (0-15) that should be accessed by HISTCNT.
    uint32_t histcnt;    // 32-bit field. Read/write the selected histogram bin.
};

enum timing_adapter_perfctrl_settings {
    TA_PERFCTRL_OFF = 0,     // Disable performance counting
    TA_PERFCTRL_CYCLES,      // Count all cycles (root clock)
    TA_PERFCTRL_UNDERRUN_R,  // Unable to meet RLATENCY deadline
    TA_PERFCTRL_UNDERRUN_B,  // Unable to meet WLATENCY deadline
    TA_PERFCTRL_OVERFLOW_AR, // Internal read address FIFO full
    TA_PERFCTRL_OVERFLOW_AW, // Internal write address FIFO full
    TA_PERFCTRL_OVERFLOW_R,  // Internal read data FIFO full
    TA_PERFCTRL_OVERFLOW_W,  // Internal write data FIFO full
    TA_PERFCTRL_OVERFLOW_B,  // Internal write response FIFO full
    TA_PERFCTRL_RREADY,      // RREADY wait state
    TA_PERFCTRL_BREADY,      // BREADY wait state
    TA_PERFCTRL_RTRANS,      // Handshake on R channel
    TA_PERFCTRL_WTRANS,      // Handshake on W channel
    TA_PERFCTRL_BTRANS,      // Handshake on B channel
    TA_PERFCTRL_ARTRANS,     // Handshake on AR channel
    TA_PERFCTRL_AWTRANS,     // Handshake on AW channel
    TA_PERFCTRL_ARQTIME,     // Histogram of how much time spent with outstanding read transactions
    TA_PERFCTRL_AWQTIME,     // Histogram of how much time spent with outstanding write transactions
    TA_PERFCTRL_MCLK_ON,     // Count cycles when DUT clock is on
    TA_PERFCTRL_MCLK_OFF,    // Count cycles when DUT clock is off
    TA_PERFCTRL_ARLEN0 = 32, // Handshake on AR channel with ARLEN=0
    TA_PERFCTRL_AWLEN0 = 48  // Handshake on AW channel with AWLEN=0
};

int ta_init(struct timing_adapter *ta, uintptr_t base_addr);
void ta_uninit(struct timing_adapter *ta);

void ta_set_all(struct timing_adapter *ta, struct timing_adapter_settings *in);
void ta_set_maxr(struct timing_adapter *ta, uint32_t val);
void ta_set_maxw(struct timing_adapter *ta, uint32_t val);
void ta_set_maxrw(struct timing_adapter *ta, uint32_t val);
void ta_set_rlatency(struct timing_adapter *ta, uint32_t val);
void ta_set_wlatency(struct timing_adapter *ta, uint32_t val);
void ta_set_pulse_on(struct timing_adapter *ta, uint32_t val);
void ta_set_pulse_off(struct timing_adapter *ta, uint32_t val);
void ta_set_bwcap(struct timing_adapter *ta, uint32_t val);
void ta_set_perfctrl(struct timing_adapter *ta, uint32_t val);
void ta_set_perfcnt(struct timing_adapter *ta, uint32_t val);
void ta_set_mode(struct timing_adapter *ta, uint32_t val);
void ta_set_histbin(struct timing_adapter *ta, uint32_t val);
void ta_set_histcnt(struct timing_adapter *ta, uint32_t val);

void ta_get_all(struct timing_adapter *ta, struct timing_adapter_settings *out);
uint32_t ta_get_maxr(struct timing_adapter *ta);
uint32_t ta_get_maxw(struct timing_adapter *ta);
uint32_t ta_get_maxrw(struct timing_adapter *ta);
uint32_t ta_get_rlatency(struct timing_adapter *ta);
uint32_t ta_get_wlatency(struct timing_adapter *ta);
uint32_t ta_get_pulse_on(struct timing_adapter *ta);
uint32_t ta_get_pulse_off(struct timing_adapter *ta);
uint32_t ta_get_bwcap(struct timing_adapter *ta);
uint32_t ta_get_perfctrl(struct timing_adapter *ta);
uint32_t ta_get_perfcnt(struct timing_adapter *ta);
uint32_t ta_get_mode(struct timing_adapter *ta);
uint32_t ta_get_maxpending(struct timing_adapter *ta);
uint32_t ta_get_histbin(struct timing_adapter *ta);
uint32_t ta_get_histcnt(struct timing_adapter *ta);
uint32_t ta_get_version(struct timing_adapter *ta);

#if defined __cplusplus
}
#endif

#endif
