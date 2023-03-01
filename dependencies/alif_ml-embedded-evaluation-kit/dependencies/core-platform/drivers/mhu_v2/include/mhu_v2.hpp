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

#ifndef MHU_V2_HPP
#define MHU_V2_HPP

#include <mailbox.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace Mailbox {
/*
 * SENDER OVERVIEW
 * ------------------------------------------------------------------------
 * Offset       Access Type Register Name                        Short Name
 * ------------------------------------------------------------------------
 * 0x000-0xF7C  -           Sender Channel Window 0-123          -
 * 0xF80        RO          Message Handling Unit Configuration  MHU_CFG
 * 0xF84        RW          Response Configuration               RESP_CFG
 * 0xF88        RW          Access Request                       ACCESS_REQUEST
 * 0xF8C        RO          Access Ready                         ACCESS_READY
 * 0xF90        RO          Interrupt Status                     INT_ST
 * 0xF94        WO          Interrupt Clear                      INT_CLR
 * 0xF98        RW          Interrupt Enable                     INT_EN
 * 0xF9C-0xFC4  RO          Reserved                             -
 * 0xFC8        RO          Implementer Identification Register  IIDR
 * 0xFCC        RO          Architecture Identification Register AIDR
 * 0xFD0-0xFFC  -           IMPL DEF Identification Regs         -
 */

/*
 * RECEIVER OVERVIEW
 * ------------------------------------------------------------------------
 * Offset       Access Type Register Name                        Short Name
 * ------------------------------------------------------------------------
 * 0x000-0xF7C  -           Receiver Channel Window 0-123        -
 * 0xF80        RO          Message Handling Unit Configuration  MHU_CFG
 * 0xF84-0xFC4  RO          Reserved                             -
 * 0xFC8        RO          Implementer Identification Register  IIDR
 * 0xFCC        RO          Architecture Identification Register AIDR
 * 0xFD0-0xFFC  -           IMPL DEF Identification Regs         -
 */

/*
 * Sender Channel Window
 * ------------------------------------------------------------------------
 * Offset       Access Type Register Name                        Short Name
 * ------------------------------------------------------------------------
 * 0x00         RO          Channel Status                       CH_ST
 * 0x04         RO          Reserved                             -
 * 0x08         RO          Reserved                             -
 * 0x0C         WO          Channel Set                          CH_SET
 * 0x10         RO          Reserved                             -
 * 0x14         RO          Reserved                             -
 * 0x18         RO          Reserved                             -
 * 0x1C         RO          Reserved                             -
 */

/*
 * Receiver Channel Window
 * ------------------------------------------------------------------------
 * Offset       Access Type Register Name                        Short Name
 * ------------------------------------------------------------------------
 * 0x00         RO          Channel Status                       CH_ST
 * 0x04         RO          Channel Status Masked                CH_ST_MSK
 * 0x08         WO          Channel Clear                        CH_CLR
 * 0x0C         RO          Reserved                             -
 * 0x10         RO          Channel Mask Status                  CH_MSK_ST
 * 0x14         WO          Channel Mask Set                     CH_MSK_SET
 * 0x18         WO          Channel Mask Clear                   CH_MSK_CLR
 * 0x1C         RO          Reserved                             -
 */

// Doorbell implementation only
// NOTE: MHUv2 is unidirectional. Two MHU's are needed for bidirectional
//       messaging. txBase/rxBase refers to the base address of _two_
//       separate MHU blocks.
class MHUv2 : public Mailbox {
public:
    MHUv2(const uint32_t txBaseAddress, const uint32_t rxBaseAddress);
    virtual ~MHUv2();
    virtual bool sendMessage() final;
    virtual void handleMessage() final;
    virtual bool verifyHardware() final;

private:
    /* Offsets */
    static constexpr uint32_t MHUv2_CH_ST                   = 0x00;
    static constexpr uint32_t MHUv2_CH_ST_MSK               = 0x04;
    static constexpr uint32_t MHUv2_CH_CLR                  = 0x08;
    static constexpr uint32_t MHUv2_CH_SET                  = 0x0C;
    static constexpr uint32_t MHUv2_CH_MSK_ST               = 0x10;
    static constexpr uint32_t MHUv2_CH_MSK_SET              = 0x14;
    static constexpr uint32_t MHUv2_CH_MSK_CLR              = 0x18;
    static constexpr uint32_t MHUv2_CH_INT_CLR              = 0x14;
    static constexpr uint32_t MHUv2_CH_INT_EN               = 0x18;
    static constexpr uint32_t MHUv2_SND_CHAN_WINDOW_SIZE    = 0x20;
    static constexpr uint32_t MHUv2_SND_MHU_CFG_OFFS        = 0xF80;
    static constexpr uint32_t MHUv2_SND_RESP_CFG_OFFS       = 0xF84;
    static constexpr uint32_t MHUv2_SND_ACCESS_REQUEST_OFFS = 0xF88;
    static constexpr uint32_t MHUv2_SND_ACCESS_READY_OFFS   = 0xF8C;
    static constexpr uint32_t MHUv2_SND_INT_ST_OFFS         = 0xF90;
    static constexpr uint32_t MHUv2_SND_INT_CLR_OFFS        = 0xF94;
    static constexpr uint32_t MHUv2_SND_INT_EN_OFFS         = 0xF98;
    static constexpr uint32_t MHUv2_SND_IIDR_OFFS           = 0xFC8;
    static constexpr uint32_t MHUv2_SND_AIDR_OFFS           = 0xFCC;
    static constexpr uint32_t MHUv2_RCV_CHAN_WINDOW_SIZE    = 0x20;
    static constexpr uint32_t MHUv2_RCV_MHU_CFG_OFFS        = 0xF80;
    static constexpr uint32_t MHUv2_RCV_IIDR_OFFS           = 0xFC8;
    static constexpr uint32_t MHUv2_RCV_AIDR_OFFS           = 0xFCC;
    static constexpr uint32_t MHUv2_RCV_INT_EN_OFFS         = 0xF98;

    struct aidr_t {
        uint32_t ARCH_MINOR_REV : 4;
        uint32_t ARCH_MAJOR_REV : 4;
        uint32_t RESERVED : 24;
    };

    volatile uint32_t *txBaseAddr;
    volatile uint32_t *rxBaseAddr;

    void clearMessage();
    void printAIDR(bool tx = true, bool rx = true);

    void txWrite(uint32_t offset, uint32_t value);
    void rxWrite(uint32_t offset, uint32_t value);
    uint32_t txRead(uint32_t offset);
    uint32_t rxRead(uint32_t offset);

    // Sender/tx
    uint32_t getAccessReady();
    uint32_t getAccessRequest();
    uint32_t getInterruptStatus();
    uint32_t getTxAIDR();
    uint32_t getTxStatusForChan(uint8_t chan);
    void enableAccessRequest();
    void disableAccessRequest();
    void setCombinedClearInterrupt(bool enable);
    void setReadyNotReadyInterrupts(bool enable);

    // Receiver/rx
    uint32_t getRxAIDR();
    uint32_t getRxStatusForChan(uint8_t chan);
    void setCombinedRecvInterrupt(bool enable);
    void enableClearChanInterrupt(uint8_t chan);
    void disableClearChanInterrupt(uint8_t chan);
};

} // namespace Mailbox

#endif /* #ifndef MHU_V2_HPP */
