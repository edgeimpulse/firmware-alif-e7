/* The Clear BSD License
 *
 * Copyright (c) 2025 EdgeImpulse Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "hal.h"

#ifndef EI_CORE_CLOCK_HZ
#ifdef EI_CONFIG_ETHOS_U55_256
#define EI_CORE_CLOCK_HZ 400000000 //default to M55_0 core
#else
#define EI_CORE_CLOCK_HZ 160000000
#endif
#endif

/* Extern function prototypes ---------------------------------------------- */
extern "C" uint64_t Get_SysTick_Cycle_Count(void);
extern "C" unsigned char UartGetcNoBlock(void);

EI_IMPULSE_ERROR ei_run_impulse_check_canceled()
{
    return EI_IMPULSE_OK;
}

/**
 * Cancelable sleep, can be triggered with signal from other thread
 */
EI_IMPULSE_ERROR ei_sleep(int32_t time_ms)
{
    if(time_ms<0) { return EI_IMPULSE_OK; }

    uint64_t time = ei_read_timer_ms();
    // cast so that we get correct wrap around behavior
    while( ei_read_timer_ms() - time < (uint64_t) time_ms )
        ;
    return EI_IMPULSE_OK;
}

uint64_t ei_read_timer_us()
{
    return Get_SysTick_Cycle_Count() / ( EI_CORE_CLOCK_HZ / 1000000 );
}

uint64_t ei_read_timer_ms()
{
    return ei_read_timer_us() / 1000;
}

void ei_printf(const char *format, ...)
{
    va_list myargs;
    va_start(myargs, format);
    vprintf(format, myargs);
    va_end(myargs);
    fflush(stdout);
}

void ei_printf_float(float f)
{
    ei_printf("%f", f);
}

void ei_putchar(char c) 
{ 
    putchar(c); 
    fflush(stdout);
}

char ei_getchar(void)
{
    auto c = UartGetcNoBlock();
    if (c == 0xFF ) { return 0; } //weird ei convention
    else { return c; ei_printf("ch: %c\r\n", c);}
}

void *ei_malloc(size_t size)
{
    return malloc(size);
}

void *ei_calloc(size_t nitems, size_t size)
{
    return calloc(nitems, size);
}

void ei_free(void *ptr)
{
    free(ptr);
}

#if defined(__cplusplus) && EI_C_LINKAGE == 1
extern "C"
#endif
    void
    DebugLog(const char *s)
{
    ei_printf("%s", s);
}
