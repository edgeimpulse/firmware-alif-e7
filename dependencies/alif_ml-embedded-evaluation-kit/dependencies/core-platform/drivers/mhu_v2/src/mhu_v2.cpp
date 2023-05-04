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

#include "ethosu_log.h"

#include <mhu_v2.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace Mailbox {

MHUv2::MHUv2(const uint32_t txBaseAddress, const uint32_t rxBaseAddress) :
    txBaseAddr(reinterpret_cast<volatile uint32_t *>(txBaseAddress)),
    rxBaseAddr(reinterpret_cast<volatile uint32_t *>(rxBaseAddress)) {

    setCombinedRecvInterrupt(true);
    enableAccessRequest(); // Set high throughout
}

void MHUv2::handleMessage() {
    clearMessage();
    notify();
}

MHUv2::~MHUv2() {
    setCombinedRecvInterrupt(false);
    disableAccessRequest();
}

bool MHUv2::verifyHardware() {
    // Sanity check MHUv2.1 id's (tx/rx)
    struct aidr_t *a;
    uint32_t txAIDR = getTxAIDR();
    uint32_t rxAIDR = getRxAIDR();

    a = reinterpret_cast<struct aidr_t *>(&txAIDR);
    if (a->ARCH_MAJOR_REV != 1 || a->ARCH_MINOR_REV != 1) {
        return false;
    }

    a = reinterpret_cast<struct aidr_t *>(&rxAIDR);
    if (a->ARCH_MAJOR_REV != 1 || a->ARCH_MINOR_REV != 1) {
        return false;
    }

    return true;
}

uint32_t MHUv2::getTxStatusForChan(uint8_t chan) {
    assert(chan >= 0);
    assert(chan < 124);
    return txRead((chan * MHUv2_SND_CHAN_WINDOW_SIZE) + MHUv2_CH_ST);
}

uint32_t MHUv2::getRxStatusForChan(uint8_t chan) {
    assert(chan >= 0);
    assert(chan < 124);
    return rxRead((chan * MHUv2_RCV_CHAN_WINDOW_SIZE) + MHUv2_CH_ST);
}

uint32_t MHUv2::getInterruptStatus() {
    return txRead(MHUv2_SND_INT_ST_OFFS);
}

uint32_t MHUv2::getAccessReady() {
    return txRead(MHUv2_SND_ACCESS_READY_OFFS);
}

uint32_t MHUv2::getAccessRequest() {
    return txRead(MHUv2_SND_ACCESS_REQUEST_OFFS);
}

uint32_t MHUv2::getTxAIDR() {
    return txRead(MHUv2_SND_AIDR_OFFS);
}

uint32_t MHUv2::getRxAIDR() {
    return rxRead(MHUv2_RCV_AIDR_OFFS);
}

void MHUv2::enableAccessRequest() {
    txWrite(MHUv2_SND_ACCESS_REQUEST_OFFS, 1);
}

void MHUv2::disableAccessRequest() {
    txWrite(MHUv2_SND_ACCESS_REQUEST_OFFS, 0);
}

/*
 * MHUv2.1
 * sender:   combined clear interrupt
 */
void MHUv2::setCombinedClearInterrupt(bool enable) {
    uint32_t val = txRead(MHUv2_SND_INT_EN_OFFS);
    if (enable) {
        val |= (1 << 2);
    } else {
        val &= ~(1 << 2);
    }
    txWrite(MHUv2_SND_INT_EN_OFFS, val);
}

/*
 * MHUv2.1
 * receiver: combined recv interrupt
 */
void MHUv2::setCombinedRecvInterrupt(bool enable) {
    uint32_t val = rxRead(MHUv2_RCV_INT_EN_OFFS);
    if (enable) {
        val |= (1 << 2);
    } else {
        val &= ~(1 << 2);
    }
    rxWrite(MHUv2_SND_INT_EN_OFFS, val);
}

// Enable/disable R2NR/NR2R interrupts
void MHUv2::setReadyNotReadyInterrupts(bool enable) {
    uint32_t val = txRead(MHUv2_SND_INT_EN_OFFS);
    if (enable) {
        val |= (1 << 0 | 1 << 1);
    } else {
        val &= ~(1 << 0 | 1 << 1);
    }
    txWrite(MHUv2_SND_INT_EN_OFFS, val);
}

void MHUv2::enableClearChanInterrupt(uint8_t chan) {
    assert(chan >= 0);
    assert(chan < 124);
    txWrite((chan * MHUv2_SND_CHAN_WINDOW_SIZE) + MHUv2_CH_INT_EN, 1);
}

void MHUv2::disableClearChanInterrupt(uint8_t chan) {
    assert(chan >= 0);
    assert(chan < 124);
    txWrite((chan * MHUv2_SND_CHAN_WINDOW_SIZE) + MHUv2_CH_INT_EN, 0);
}

/*
 * Set channel status byte (with only minor error/state check(s))
 * Doorbell only, chan 0
 */
bool MHUv2::sendMessage() {
    // Check that the other end is ready to receive
    if (!getAccessReady()) {
        return false;
    }
    txWrite(MHUv2_CH_SET, 1);

    return true;
}

void MHUv2::clearMessage() {
    rxWrite(MHUv2_CH_CLR, 0xFFFFFFFF); // Doorbell uses only chan 0, but clear all 32bits to be safe
}

void MHUv2::txWrite(uint32_t offset, uint32_t value) {
    write32(txBaseAddr, offset, value);
}

void MHUv2::rxWrite(uint32_t offset, uint32_t value) {
    write32(rxBaseAddr, offset, value);
}

uint32_t MHUv2::txRead(uint32_t offset) {
    return read32(txBaseAddr, offset);
}

uint32_t MHUv2::rxRead(uint32_t offset) {
    return read32(rxBaseAddr, offset);
}

void MHUv2::printAIDR(bool tx, bool rx) {
    struct aidr_t *a;
    uint32_t aidr;

    if (tx) {
        aidr = getTxAIDR();
        a    = reinterpret_cast<struct aidr_t *>(&aidr);
        LOG("TX MHUv2 reports: Major rev: %d, Minor rev: %d", a->ARCH_MAJOR_REV, a->ARCH_MINOR_REV);
    }
    if (rx) {
        aidr = getRxAIDR();
        a    = reinterpret_cast<struct aidr_t *>(&aidr);
        LOG("RX MHUv2 reports: Major rev: %d, Minor rev: %d", a->ARCH_MAJOR_REV, a->ARCH_MINOR_REV);
    }
}

} // namespace Mailbox
