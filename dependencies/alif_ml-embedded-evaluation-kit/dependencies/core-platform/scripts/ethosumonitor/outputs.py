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

from .types import *
from sys import stderr

class OutputInterface:
    def flush(self):
        ...

    def writeEventRecord(self, data: bytearray):
        ...

class OutputBinary(OutputInterface):
    def __init__(self, fname):
        self.file = open(fname, 'wb')

    def flush(self):
        self.file.flush()

    def writeEventRecord(self, data: bytearray):
        self.file.write(data)

class OutputJson(OutputInterface):
    def __init__(self, fname):
        self.file = open(fname, 'w')

        self.count = 0
        self.nextId = 0
        self.timestamp = 0
        self.event = []

    def flush(self):
        self.file.flush()

    def writeEventRecord(self, data: bytearray):
        record = EventRecord_t(data)

        if record.first():
            # Drop messages that don't originate from Ethos-U
            if record.component() != EventRecord_t.ETHOSU_CID:
                return

            self.nextId = 0
            self.timestamp = record.timestamp
            self.eventConfig = []
            self.eventCount = []

        messageIndex = record.message()

        if self.nextId != messageIndex or self.timestamp != record.timestamp:
            stderr.write(f'Expected record id {self.nextId} and timestamp {self.timestamp} but got {messageIndex} and {record.timestamp}. count={self.count}, locked={record.locked()}, valid={record.valid()}\n')
            stderr.write(f'record={record}\n')
            return

        self.nextId = messageIndex + 1

        if messageIndex == 0:
            self.cycleCount = record.val2 << 32 | record.val1
        elif messageIndex == 1:
            self.qread = record.val1
            self.status = record.val2
        else:
            self.eventConfig.append(record.val1)
            self.eventCount.append(record.val2)

        if record.last():
            self.file.write(f'{{ "timestamp": {self.timestamp}, "qread": {self.qread}, "status": {self.status}, "cycleCount": {self.cycleCount}, "eventConfig": [ {", ".join(map(str, self.eventConfig))} ], "eventCount": [ {", ".join(map(str, self.eventCount))} ] }}\n')
