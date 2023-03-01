#
# SPDX-FileCopyrightText: Copyright 2022 Arm Limited and/or its affiliates <open-source-office@arm.com>
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the License); you may
# not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import struct

class EventRecord_t:
    SIZE = 16

    INFO_ID_MASK        = 0x0000FFFF
    INFO_MESSAGE_MASK   = 0x000000FF
    INFO_COMPONENT_MASK = 0x0000FF00
    INFO_COMPONENT_POS  = 8
    INFO_FIRST          = 0x01000000
    INFO_LAST           = 0x02000000
    INFO_LOCKED         = 0x04000000
    INFO_VALID          = 0x08000000
    INFO_MSB_TS         = 0x10000000
    INFO_MSB_VAL1       = 0x20000000
    INFO_MSB_VAL2       = 0x40000000
    INFO_TBIT           = 0x80000000

    # Component identifiers
    EVENT_CID       = 0xFF
    ETHOSU_CID      = 0x00

    # Message identifiers
    EVENT_MID_INIT  = 0x00
    EVENT_MID_START = 0x01
    EVENT_MID_STOP  = 0x02
    EVENT_MID_CLOCK = 0x03

    EVENT_ID_INIT   = (EVENT_CID << 8) | EVENT_MID_INIT
    EVENT_ID_START  = (EVENT_CID << 8) | EVENT_MID_START
    EVENT_ID_STOP   = (EVENT_CID << 8) | EVENT_MID_STOP
    EVENT_ID_CLOCK  = (EVENT_CID << 8) | EVENT_MID_CLOCK

    def __init__(self, data):
        self.data = data

        # Unpack the struct and restore the MSB from info to timestamp, val1 and val2
        t = struct.unpack('IIII', data)
        self.timestamp = t[0] & ~EventRecord_t.INFO_TBIT | (t[3] & EventRecord_t.INFO_MSB_TS) << 3
        self.val1 = t[1] & ~EventRecord_t.INFO_TBIT | (t[3] & EventRecord_t.INFO_MSB_VAL1) << 2
        self.val2 = t[2] & ~EventRecord_t.INFO_TBIT | (t[3] & EventRecord_t.INFO_MSB_VAL2) << 1
        self.info = t[3]

    def first(self):
        return self.info & EventRecord_t.INFO_FIRST != 0

    def last(self):
        return self.info & EventRecord_t.INFO_LAST != 0

    def component(self):
        return (self.info & EventRecord_t.INFO_COMPONENT_MASK) >> EventRecord_t.INFO_COMPONENT_POS

    def message(self):
        return self.info & EventRecord_t.INFO_MESSAGE_MASK

    def id(self):
        return self.info & EventRecord_t.INFO_ID_MASK

    def locked(self):
        return self.info & EventRecord_t.INFO_LOCKED != 0

    def valid(self):
        return self.info & EventRecord_t.INFO_VALID != 0

    def __str__(self):
        return f'{{ "timestamp": {hex(self.timestamp)}, "val1": {hex(self.val1)}, "val2": {hex(self.val2)}, "info": "{hex(self.info)}" }}'

class EventStatus_t:
    SIZE = 36

    def __init__(self, data):
        t = struct.unpack('BBHIIIIIIII', data)

        self.state = t[0]
        self.context = t[1]
        self.infoCrc = t[2]
        self.recordIndex = t[3]
        self.recordsWritten = t[4]
        self.recordsDumped = t[5]
        self.tsOverflow = t[6]
        self.tsFreq = t[7]
        self.tsLast = t[8]
        self.initCount = t[9]
        self.signature = t[10]

    def __str__(self):
        return f'{{ state={self.state}, context={self.context}, info_crc={self.infoCrc}, ' \
               f'record_index={self.recordIndex}, records_written={self.recordsWritten}, records_dumped={self.recordsDumped}, ' \
               f'ts_overflow={self.tsOverflow}, ts_freq={self.tsFreq}, ts_last={self.tsLast}, ' \
               f'init_count={self.initCount}, signature={self.signature} }}'

class EventRecorderInfo_t:
    SIZE = 24

    def __init__(self, data):
        t = struct.unpack('BBHIIIIBBBB', data)

        self.protocolType = t[0]
        # self._reserved = t[1]
        self.protocolVersion = t[2]
        self.recordCount = t[3]
        self.eventBuffer = t[4]
        self.eventFilter = t[5]
        self.eventStatus = t[6]
        self.tsSource = t[7]
        # self._reserved1 = t[8]
        # self._reserved2 = t[9]
        # self._reserved3 = t[10]

    def __str__(self):
        return f'{{ protocolType={hex(self.protocolType)}, protocolVersion={hex(self.protocolVersion)}, recordCount={self.recordCount}, eventBuffer={hex(self.eventBuffer)}, eventFilter={hex(self.eventFilter)}, eventStatus={hex(self.eventStatus)}, tsSource={self.tsSource} }}'
