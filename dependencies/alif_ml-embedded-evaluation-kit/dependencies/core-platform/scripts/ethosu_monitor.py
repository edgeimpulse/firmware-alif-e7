#!/usr/bin/env python3

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

from ethosumonitor.inputs import *
from ethosumonitor.outputs import *
from sys import stderr, exit

def eventLoop(input: InputInterface, output: OutputInterface):
    count = 0

    try:
        while(True):
            for record in input.readEventRecord():
                output.writeEventRecord(record)
                count = count + 1
    except KeyboardInterrupt:
        stderr.write(f'count={count}, input={input}\n')
        pass
    except EOFError:
        pass

    output.flush()

def getDAPLink(args):
    return InputDAPLink(args.elf)

def getMem(args):
    return InputMem(args.elf, args.memory_map)

def getFile(args):
   return InputFile(args.file)

def getOutput(args):
    if args.output_format == 'binary':
        return OutputBinary(args.output)
    else:
        return OutputJson(args.output)

def addOutputArguments(parser):
    parser.add_argument('--output-format', choices=['binary', 'json'], default='json', help='Output format.')
    parser.add_argument('-o', '--output', default='/dev/stdout', help='Output file.')

def main():
    import argparse

    parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
                                     description='Ethos-U monitor downloading profiling data.',
                                     epilog='''
Event Recorder:
  The Event Recorder library is used to write performance data to a ring buffer
  in memory. The ring buffer has a limited size and must be continuously
  streamed to a host machine before it overflows.

  This script demonstrates how to stream performance data using DAPLink or
  /dev/mem. Support for other technologies can be added implementing the
  InputInterface class in inputs.py.
''')
    subparsers = parser.add_subparsers()

    subparser = subparsers.add_parser('daplink',
                                      formatter_class=argparse.RawDescriptionHelpFormatter,
                                      description='Download performance data using DAPLink.',
                                      epilog='''
DAPLink:
  Arm Mbed DAPLink is an open source project that enables programming and
  debugging application software running on an Arm Cortex CPU. A host machine
  can connect to the target device using for example USB or JTAG.

  This script demonstrates how DAPLink can be used to stream Event Recorder
  data from a target device. The ELF file passed to the script must be the
  same application that is running on the device, and is used to find the
  location of the Event Recorder ring buffer.

  $ ethosu_monitor.py daplink --target mps3_an540 myapplication.elf
''')
    subparser.set_defaults(getInput=getDAPLink)
    subparser.add_argument('--target', default='mps3_an540', help='DAPLink target platform.')
    subparser.add_argument('elf', help='Elf file running on the target.')
    addOutputArguments(subparser)

    subparser = subparsers.add_parser('memory',
                                      formatter_class=argparse.RawDescriptionHelpFormatter,
                                      description='Download performance data using /dev/mem.',
                                      epilog='''
/dev/mem:
  For a Linux based system the Event Recorder buffer should be stored in shared
  memory accessible from Linux. This allows Linux to read device the Event
  Recorder ring buffer using /dev/mem.

  The address of the Event Recorder ring buffer is found parsing the ELF
  file. Because the device and Linux do not share the same address space a
  memory map is required to translate device addresses into host addresses.
  Please see sample.json for reference.

  $ ethosu_monitor.py memory --memory-map config.json myapplication.elf
''')
    subparser.set_defaults(getInput=getMem)
    subparser.add_argument('--memory-map', required=True, help='JSON file describing physical memory map of target.')
    subparser.add_argument('elf', help='Elf file running on the target.')
    addOutputArguments(subparser)

    subparser = subparsers.add_parser('file',
                                      formatter_class=argparse.RawDescriptionHelpFormatter,
                                      description='Replay performance data stored in binary file.',
                                      epilog='''
file:
  Event Recorder data can be written in binary format for later processing.
  This will likely have less latency than the default JSON format, reducing
  the risk over ring buffer overflows.

  $ ethosu_monitor.py daplink --output-format binary --output samples.bin myapplication.elf

  The binary data can later be unpacked to JSON.

  $ ethosu_monitor.py file samples.bin --output-format json
''')
    subparser.set_defaults(getInput=getFile)
    subparser.add_argument('file', help='Binary file containing recorded performance data.')
    addOutputArguments(subparser)

    args = parser.parse_args()

    if 'getInput' not in args:
        parser.print_help()
        exit(2)

    input = args.getInput(args)
    output = getOutput(args)
    eventLoop(input, output)

if __name__ == '__main__':
    main()
