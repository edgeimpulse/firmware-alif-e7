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

#ifndef MHU_JUNO_HPP
#define MHU_JUNO_HPP

#include <mailbox.hpp>

#include <cstddef>
#include <cstdint>

namespace Mailbox {

// Doorbell implementation only
class MHUJuno : public Mailbox {
public:
    MHUJuno(const uint32_t baseAddress);
    virtual ~MHUJuno();
    virtual bool sendMessage() final;
    virtual void handleMessage() final;
    virtual bool verifyHardware() final;

private:
    /* Offsets */
    static constexpr uint32_t CPU0_INTR_STAT = 0x00;
    static constexpr uint32_t CPU0_INTR_SET  = 0x04;
    static constexpr uint32_t CPU0_INTR_CLR  = 0x08;
    static constexpr uint32_t CPU1_INTR_STAT = 0x10;
    static constexpr uint32_t CPU1_INTR_SET  = 0x14;
    static constexpr uint32_t CPU1_INTR_CLR  = 0x18;
    static constexpr uint32_t OFFSET         = 0x10;
    static constexpr uint32_t PIDR0          = 0xfe0;
    static constexpr uint32_t PIDR1          = 0xfe4;
    static constexpr uint32_t PIDR2          = 0xfe8;
    static constexpr uint32_t PIDR3          = 0xfec;
    static constexpr uint32_t PIDR4          = 0xfd0;
    static constexpr uint32_t CIDR0          = 0xff0;
    static constexpr uint32_t CIDR1          = 0xff4;
    static constexpr uint32_t CIDR2          = 0xff8;
    static constexpr uint32_t CIDR3          = 0xffc;

    volatile uint32_t *baseAddr;

    void clearMessage();
    void write(uint32_t offset, uint32_t val);
    uint32_t read(uint32_t offset);
};

} // namespace Mailbox

#endif /* #ifndef MHU_JUNO_HPP */
