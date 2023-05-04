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

from .elf import *
from .types import *
import json
import mmap
import os
from sys import stderr

class InputInterface:
    def readEventRecord(self) -> EventRecord_t:
        ...

class InputFile(InputInterface):
    def __init__(self, fname):
        self.file = open(fname, 'rb')

    def readEventRecord(self):
        data = self.file.read(EventRecord_t.SIZE)
        if len(data) == 0:
            raise EOFError

        yield data

class InputRingBuffer(InputInterface):
    def __init__(self, elfFile):
        from elftools.elf.elffile import ELFFile

        with open(elfFile, 'rb') as f:
            elf = ELFFile(f)
            symbol = elfFindSymbol(elf, 'EventRecorderInfo')
            elfInfo = EventRecorderInfo_t(elfGetData(elf, symbol.entry.st_value, symbol.entry.st_size))
            memInfo = EventRecorderInfo_t(self.read(symbol.entry.st_value, symbol.entry.st_size))

        # Validate EventRecorder info
        if elfInfo.protocolType != memInfo.protocolType or \
            elfInfo.protocolVersion != memInfo.protocolVersion or \
            elfInfo.eventBuffer != memInfo.eventBuffer or \
            elfInfo.eventStatus != memInfo.eventStatus:
            raise Exception(f'EventRecorder info mismatch. elf={elfInfo}, mem={memInfo}')

        self.info = elfInfo
        status = EventStatus_t(self.read(self.info.eventStatus, EventStatus_t.SIZE))
        self.timestamp = status.tsLast
        self.recordIndex = status.recordIndex
        self.overflow = 0

    def readEventRecord(self):
        # Read status and use timestamp to detect if there are new samples
        status = EventStatus_t(self.read(self.info.eventStatus, EventStatus_t.SIZE))
        if self.timestamp == status.tsLast:
            return None

        self.timestamp = status.tsLast

        # Detect firmware reset
        if self.recordIndex > status.recordIndex:
            self.recordIndex = 0

        # Detect of recordIndex has overflowed the ring buffer
        if status.recordIndex - self.recordIndex > self.info.recordCount:
            stderr.write('Warning: Ring buffer overflow\n')
            self.overflow = self.overflow + 1
            self.recordIndex = status.recordIndex

        # Generate data for each event record
        for i in range(self.recordIndex, status.recordIndex):
            i = i % self.info.recordCount
            yield self.read(self.info.eventBuffer + EventRecord_t.SIZE * i, EventRecord_t.SIZE)

        self.recordIndex = status.recordIndex

    def read(self, address, size) -> bytearray:
        ...

class InputDAPLink(InputRingBuffer):
    def __init__(self, elfFile):
        self._open()
        super().__init__(elfFile)
        self.target.reset()

    def _open(self):
        from pyocd.core.helpers import ConnectHelper

        self.session = ConnectHelper.session_with_chosen_probe()
        self.board = self.session.board
        self.target = self.board.target

        self.session.open()

    def read(self, address, size):
        from pyocd.core.exceptions import Error

        for i in range(1000):
            try:
                return bytearray(self.target.read_memory_block8(address, size))
            except Error:
                pass

class InputMem(InputRingBuffer):
    def __init__(self, elfFile, jsonFile):
        with open(jsonFile, 'r') as f:
            jsonDoc = json.loads(f.read())

        self.memoryMap = []
        for memoryMap in jsonDoc['memoryMap']:
            host = int(memoryMap['host'], 16)
            device = int(memoryMap['device'], 16)
            size = int(memoryMap['size'], 16)
            self.memoryMap.append(DevMemDevice(host, device, size))

        super().__init__(elfFile)

    def read(self, device, size):
        for memoryMap in self.memoryMap:
            data = memoryMap.read(device, size)
            if data:
                return data

        stderr.write(f'Warning: No mapping found for device address {hex(device)} size {size}.\n')
        return None

class DevMem:
    def __init__(self, address, size):
        self.base_address = address & ~(mmap.PAGESIZE - 1)
        self.offset = address - self.base_address
        self.size = size + self.offset

        self.fd = os.open('/dev/mem', os.O_RDWR | os.O_SYNC)
        self.mem = mmap.mmap(self.fd, self.size, mmap.MAP_SHARED, mmap.PROT_READ,
                             offset=self.base_address)

    def __del__(self):
        os.close(self.fd)

    def read(self, offset, size):
        self.mem.seek(self.offset + offset)

        data = bytearray(size)
        for i in range(size):
            data[i] = self.mem.read_byte()

        return data

class DevMemDevice(DevMem):
    def __init__(self, host, device, size):
        super().__init__(host, size)

        self.device = device
        self.size = size

    def read(self, device, size):
        offset = device - self.device
        if offset < 0 or (offset + size) > self.size:
            return None

        return super().read(offset, size)
