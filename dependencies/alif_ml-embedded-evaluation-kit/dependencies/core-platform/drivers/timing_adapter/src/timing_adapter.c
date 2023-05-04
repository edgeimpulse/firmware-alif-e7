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

#include <stdint.h>
#include <timing_adapter.h>

// Register offsets
#define TA_MAXR       0x00
#define TA_MAXW       0x04
#define TA_MAXRW      0x08
#define TA_RLATENCY   0x0C
#define TA_WLATENCY   0x10
#define TA_PULSE_ON   0x14
#define TA_PULSE_OFF  0x18
#define TA_BWCAP      0x1C
#define TA_PERFCTRL   0x20
#define TA_PERFCNT    0x24
#define TA_MODE       0x28
#define TA_MAXPENDING 0x2C
#define TA_HISTBIN    0x30
#define TA_HISTCNT    0x34
#define TA_VERSION    0x38

// Register masks
#define TA_MAXR_MASK       0x0000003F
#define TA_MAXW_MASK       0x0000003F
#define TA_MAXRW_MASK      0x0000003F
#define TA_RLATENCY_MASK   0x00000FFF
#define TA_WLATENCY_MASK   0x00000FFF
#define TA_PULSE_ON_MASK   0x0000FFFF
#define TA_PULSE_OFF_MASK  0x0000FFFF
#define TA_BWCAP_MASK      0x0000FFFF
#define TA_PERFCTRL_MASK   0x0000003F
#define TA_PERFCNT_MASK    0xFFFFFFFF
#define TA_MODE_MASK       0x00000FFF
#define TA_MAXPENDING_MASK 0xFFFFFFFF
#define TA_HISTBIN_MASK    0x0000000F
#define TA_HISTCNT_MASK    0xFFFFFFFF

#define TA_VERSION_SUPPORTED 0x1117

int ta_init(struct timing_adapter *ta, uintptr_t base_addr) {
    ta->base_addr = base_addr;

    if (ta_get_version(ta) != TA_VERSION_SUPPORTED) {
        return -1;
    }
    return 0;
}

void ta_uninit(struct timing_adapter *ta) {
    ta->base_addr = 0;
}

// -- Set API --------------------------------------
void ta_set_all(struct timing_adapter *ta, struct timing_adapter_settings *in) {
    ta_set_maxr(ta, in->maxr);
    ta_set_maxw(ta, in->maxw);
    ta_set_maxrw(ta, in->maxrw);
    ta_set_rlatency(ta, in->rlatency);
    ta_set_wlatency(ta, in->wlatency);
    ta_set_pulse_on(ta, in->pulse_on);
    ta_set_pulse_off(ta, in->pulse_off);
    ta_set_bwcap(ta, in->bwcap);
    ta_set_perfctrl(ta, in->perfctrl);
    ta_set_perfcnt(ta, in->perfcnt);
    ta_set_mode(ta, in->mode);
    ta_set_histbin(ta, in->histbin);
    ta_set_histcnt(ta, in->histcnt);
}

void ta_set_maxr(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_MAXR) = val & TA_MAXR_MASK;
};

void ta_set_maxw(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_MAXW) = val & TA_MAXW_MASK;
};

void ta_set_maxrw(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_MAXRW) = val & TA_MAXRW_MASK;
};

void ta_set_rlatency(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_RLATENCY) = val & TA_RLATENCY_MASK;
};

void ta_set_wlatency(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_WLATENCY) = val & TA_WLATENCY_MASK;
};

void ta_set_pulse_on(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_PULSE_ON) = val & TA_PULSE_ON_MASK;
};

void ta_set_pulse_off(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_PULSE_OFF) = val & TA_PULSE_OFF_MASK;
};

void ta_set_bwcap(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_BWCAP) = val & TA_BWCAP_MASK;
};

void ta_set_perfctrl(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_PERFCTRL) = val & TA_PERFCTRL_MASK;
};

void ta_set_perfcnt(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_PERFCNT) = val & TA_PERFCNT_MASK;
};

void ta_set_mode(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_MODE) = val & TA_MODE_MASK;
};

void ta_set_histbin(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_HISTBIN) = val & TA_HISTBIN_MASK;
};

void ta_set_histcnt(struct timing_adapter *ta, uint32_t val) {
    *(volatile uint32_t *)(ta->base_addr + TA_HISTCNT) = val & TA_HISTCNT_MASK;
};

// -- Get API --------------------------------------
void ta_get_all(struct timing_adapter *ta, struct timing_adapter_settings *out) {
    out->maxr       = ta_get_maxr(ta);
    out->maxw       = ta_get_maxw(ta);
    out->maxrw      = ta_get_maxrw(ta);
    out->rlatency   = ta_get_rlatency(ta);
    out->wlatency   = ta_get_wlatency(ta);
    out->pulse_on   = ta_get_pulse_on(ta);
    out->pulse_off  = ta_get_pulse_off(ta);
    out->bwcap      = ta_get_bwcap(ta);
    out->perfctrl   = ta_get_perfctrl(ta);
    out->perfcnt    = ta_get_perfcnt(ta);
    out->mode       = ta_get_mode(ta);
    out->maxpending = ta_get_maxpending(ta);
    out->histbin    = ta_get_histbin(ta);
    out->histcnt    = ta_get_histcnt(ta);
}

uint32_t ta_get_maxr(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_MAXR) & TA_MAXR_MASK;
};

uint32_t ta_get_maxw(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_MAXW) & TA_MAXW_MASK;
};

uint32_t ta_get_maxrw(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_MAXRW) & TA_MAXRW_MASK;
};

uint32_t ta_get_rlatency(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_RLATENCY) & TA_RLATENCY_MASK;
};

uint32_t ta_get_wlatency(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_WLATENCY) & TA_WLATENCY_MASK;
};

uint32_t ta_get_pulse_on(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_PULSE_ON) & TA_PULSE_ON_MASK;
};

uint32_t ta_get_pulse_off(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_PULSE_OFF) & TA_PULSE_OFF_MASK;
};

uint32_t ta_get_bwcap(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_BWCAP) & TA_BWCAP_MASK;
};

uint32_t ta_get_perfctrl(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_PERFCTRL) & TA_PERFCTRL_MASK;
};

uint32_t ta_get_perfcnt(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_PERFCNT) & TA_PERFCNT_MASK;
};

uint32_t ta_get_mode(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_MODE) & TA_MODE_MASK;
};

uint32_t ta_get_maxpending(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_MAXPENDING) & TA_MAXPENDING_MASK;
};

uint32_t ta_get_histbin(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_HISTBIN) & TA_HISTBIN_MASK;
};

uint32_t ta_get_histcnt(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_HISTCNT) & TA_HISTCNT_MASK;
};

uint32_t ta_get_version(struct timing_adapter *ta) {
    return *(volatile uint32_t *)(ta->base_addr + TA_VERSION);
};
