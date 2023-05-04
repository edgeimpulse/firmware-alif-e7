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

#include <mhu_juno.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace Mailbox {

MHUJuno::MHUJuno(const uint32_t baseAddress) {
    baseAddr = reinterpret_cast<volatile uint32_t *>(baseAddress);
}

void MHUJuno::handleMessage() {
    clearMessage();
    notify();
}

MHUJuno::~MHUJuno() {}

// Doorbell only
bool MHUJuno::sendMessage() {
    write(CPU1_INTR_SET, 1);
    return true;
}
// Doorbell only
void MHUJuno::clearMessage() {
    write(CPU0_INTR_CLR, 0xF);
}

bool MHUJuno::verifyHardware() {
    uint32_t pidr0 = read(PIDR0);
    uint32_t pidr1 = read(PIDR1);
    uint32_t pidr2 = read(PIDR2);
    uint32_t pidr3 = read(PIDR3);
    uint32_t pidr4 = read(PIDR4);
    uint32_t cidr0 = read(CIDR0);
    uint32_t cidr1 = read(CIDR1);
    uint32_t cidr2 = read(CIDR2);
    uint32_t cidr3 = read(CIDR3);

    if (pidr0 != 0x56 || pidr1 != 0xb8 || pidr2 != 0x0b || pidr3 != 0x00 || pidr4 != 0x04 || cidr0 != 0x0d ||
        cidr1 != 0xf0 || cidr2 != 0x05 || cidr3 != 0xb1) {
        return false;
    }
    return true;
}

void MHUJuno::write(const uint32_t offset, const uint32_t value) {
    write32(baseAddr, offset, value);
}

uint32_t MHUJuno::read(const uint32_t offset) {
    return read32(baseAddr, offset);
}

} // namespace Mailbox
