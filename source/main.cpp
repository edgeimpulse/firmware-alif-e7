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

#include "hal.h"
#include "firmware-sdk-alif/ei_device_info_lib.h"
#include "firmware-sdk-alif/ei_device_memory.h"
#include "firmware-sdk-alif/at-server/ei_at_server.h"
#include "firmware-sdk-alif/at-server/ei_at_command_set.h"
#include "firmware-sdk-alif/ei_at_handlers_lib.h"
#include "firmware-sdk-alif/ei_image_lib.h"
#include "ei_microphone.h"
#include "ei_run_impulse.h"
#include "delay.h"

#include <cstdio>

/* Project Includes */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include "log_macros.h"

#if !NDEBUG
extern "C" void __stack_chk_fail(void)
{
    ei_printf("Stack overflow caught\n");
    while (1)
    {
    }
} // trap stack overflow
void *__stack_chk_guard = (void *)0xaeaeaeae;
#endif

extern "C" void ei_sleep_c(int32_t time_ms) { ei_sleep( time_ms ); }
extern "C" int Init_SysTick(void);

extern "C" unsigned char UartGetc(void);
extern "C" int arm_ethosu_npu_init(void);

int main()
{
    init_trigger_rx();
    hal_platform_init();

    ei_printf("Hello from Edge Impulse on Alif Ensemble E7\r\n"
              "Compiled on %s %s\r\n",
              __DATE__,
              __TIME__);

    #if ARM_NPU
    arm_ethosu_npu_init();
    ei_printf("ARM ethos init\r\n");
    #endif

    sleep_or_wait_msec(10);

    auto at = ATServer::get_instance();

    at->register_command(AT_CONFIG, AT_CONFIG_HELP_TEXT, nullptr, at_get_config, nullptr, nullptr);
    at->register_command(AT_SAMPLESTART, AT_SAMPLESTART_HELP_TEXT, nullptr, nullptr, at_sample_start, AT_SAMPLESTART_ARGS);
    at->register_command(AT_READBUFFER, AT_READBUFFER_HELP_TEXT, nullptr, nullptr, at_read_buffer, AT_READBUFFER_ARGS);
    // at->register_command(AT_READFILE, AT_READFILE_HELP_TEXT, nullptr, nullptr, at_read_file, AT_READFILE_ARGS);
    at->register_command(AT_MGMTSETTINGS, AT_MGMTSETTINGS_HELP_TEXT, nullptr, at_get_mgmt_url, at_set_mgmt_url, AT_MGMTSETTINGS_ARGS);
    at->register_command(AT_CLEARCONFIG, AT_CLEARCONFIG_HELP_TEXT, at_clear_config, nullptr, nullptr, nullptr);
    at->register_command(AT_DEVICEID, AT_DEVICEID_HELP_TEXT, nullptr, at_get_device_id, at_set_device_id, AT_DEVICEID_ARGS);
    at->register_command(AT_SAMPLESETTINGS, AT_SAMPLESETTINGS_HELP_TEXT, nullptr, at_get_sample_settings, at_set_sample_settings, AT_SAMPLESETTINGS_ARGS);
    at->register_command(AT_SNAPSHOT, AT_SNAPSHOT_HELP_TEXT, nullptr, at_get_snapshot, at_take_snapshot, AT_SNAPSHOT_ARGS);
    at->register_command(AT_SNAPSHOTSTREAM, AT_SNAPSHOTSTREAM_HELP_TEXT, nullptr, nullptr, at_snapshot_stream, AT_SNAPSHOTSTREAM_ARGS);
    at->register_command(AT_UPLOADSETTINGS, AT_UPLOADSETTINGS_HELP_TEXT, nullptr, at_get_upload_settings, at_set_upload_settings, AT_UPLOADSETTINGS_ARGS);
    at->register_command(AT_UPLOADHOST, AT_UPLOADHOST_HELP_TEXT, nullptr, at_get_upload_host, at_set_upload_host, AT_UPLOADHOST_ARGS);
    // at->register_command(AT_UNLINKFILE, AT_UNLINKFILE_HELP_TEXT, nullptr, nullptr, at_unlink_file, AT_UNLINKFILE_ARGS);
    at->register_command(AT_RUNIMPULSE, AT_RUNIMPULSE_HELP_TEXT, run_nn_normal, nullptr, nullptr, nullptr);
    at->register_command(AT_RUNIMPULSEDEBUG, AT_RUNIMPULSEDEBUG_HELP_TEXT, nullptr, nullptr, run_nn_debug, AT_RUNIMPULSEDEBUG_ARGS);
    // at->register_command(AT_RUNIMPULSECONT, AT_RUNIMPULSECONT_HELP_TEXT, at_run_impulse_cont, nullptr, nullptr, nullptr);

    ei_microphone_init();

    int err = hal_image_init();
    if (0 != err) {
        ei_printf("hal_image_init failed with error: %d\n", err);
    }

    while (1)
    {
        // blocking call
        char data = UartGetc();
        if (data != 0xFF)
        {
            at->handle(data);
        }
        // else
        // {
        //     ei_printf("UART read error\n");
        //     break;
        // }
    }
}

// remove unneeded bloat

namespace __gnu_cxx
{
    void __verbose_terminate_handler()
    {
        for (;;)
            ;
    }
}
