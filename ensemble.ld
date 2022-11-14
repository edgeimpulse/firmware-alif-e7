/*
 * Copyright (c) 2021 Arm Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

__STACK_SIZE = 0x00003000;
__HEAP_SIZE  = 0x00008000;

MEMORY
{
  ITCM  (rwx) : ORIGIN = 0x00000000, LENGTH = 0x00040000
  DTCM  (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00040000
  SRAM0 (rwx) : ORIGIN = 0x02000000, LENGTH = 0x00400000
  SRAM1 (rwx) : ORIGIN = 0x08000000, LENGTH = 0x00280000
}

ENTRY(Reset_Handler)

SECTIONS
{
  .text.at_sram1 :
  {
    KEEP(*(.vectors))
    *(.text*)

    KEEP(*(.init))
    KEEP(*(.fini))

    /* .ctors */
    *crtbegin.o(.ctors)
    *crtbegin?.o(.ctors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
    *(SORT(.ctors.*))
    *(.ctors)

    /* .dtors */
    *crtbegin.o(.dtors)
    *crtbegin?.o(.dtors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
    *(SORT(.dtors.*))
    *(.dtors)

    *(.rodata*)

    KEEP(*(.eh_frame*))
    . = ALIGN(16);
  } > SRAM1

  .ARM.extab :
  {
    . = ALIGN(16);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(16);
  } > SRAM1

  __exidx_start = .;
  .ARM.exidx :
  {
    . = ALIGN(16);
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    . = ALIGN(16);
  } > SRAM1
  __exidx_end = .;

  .copy.table :
  {
    . = ALIGN(16);
    __copy_table_start__ = .;
    LONG (__etext)
    LONG (__data_start__)
    LONG ((__data_end__ - __data_start__)/4)
    __copy_table_end__ = .;
    . = ALIGN(16);
  } > SRAM1

  .zero.table :
  {
    . = ALIGN(16);
    __zero_table_start__ = .;
    LONG (__bss_start__)
    LONG ((__bss_end__ - __bss_start__)/4)
    LONG (__sram0_start__)
    LONG ((__sram0_end__ - __sram0_start__)/4)
    __zero_table_end__ = .;
    . = ALIGN(16);
  } > SRAM1

  . = ALIGN(16);
  __etext = .;

  .data.at_dtcm : AT (__etext)
  {
    __data_start__ = .;
    *(vtable)
    *(.data)
    *(.data*)

    . = ALIGN(4);
    /* preinit data */
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);

    . = ALIGN(4);
    /* init data */
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);

    . = ALIGN(4);
    /* finit data */
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);

    KEEP(*(.jcr*))

    . = ALIGN(16);
    __data_end__ = .;
  } > DTCM

  . = ALIGN(16);

  .data.at_sram0 (NOLOAD) :
  {
    . = ALIGN(4);
    __sram0_start__ = .;
    * (*lcd_image_buf*)
    * (*tensor_arena*)


    . = ALIGN(4);
    __sram0_end__ = .;
  } > SRAM0

  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss)
    *(.bss.*)
    *(COMMON)


    . = ALIGN(4);
    __bss_end__ = .;
  } > SRAM0 

  .heap (NOLOAD) :
  {
    . = ALIGN(8);
    __end__ = .;
    PROVIDE(end = .);
    . = . + __HEAP_SIZE;
    . = ALIGN(8);
    __HeapLimit = .;
  } > DTCM 

  .stack (ORIGIN(DTCM) + LENGTH(DTCM) - __STACK_SIZE) (NOLOAD) :
  {
    . = ALIGN(8);
    __StackLimit = .;
    . = . + __STACK_SIZE;
    . = ALIGN(8);
    __StackTop = .;
  } > DTCM
  PROVIDE(__stack = __StackTop);
}
