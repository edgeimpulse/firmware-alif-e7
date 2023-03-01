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

def elfFindSymbol(elf, name):
    from elftools.elf.sections import SymbolTableSection

    for section in elf.iter_sections():
        if isinstance(section, SymbolTableSection):
            symbol = section.get_symbol_by_name(name)
            if symbol:
                return symbol[0]

    return None

def elfGetData(elf, address, size):
    for section in elf.iter_sections():
        if address >= section.header['sh_addr'] and \
           (address + size) < (section.header['sh_addr'] + section.header['sh_size']):
            offset = address - section.header['sh_addr']
            return bytearray(section.data()[offset:offset+size])

    return None

def elfGetSymbolData(elf, name):
    from sys import stderr

    symbol = elfFindSymbol(elf, name)
    if not symbol:
        stderr.write(f'Failed to find symbol {name}\n')
        return None

    return elfGetData(elf, symbol.entry.st_value, symbol.entry.st_size)
