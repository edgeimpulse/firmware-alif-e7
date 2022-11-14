/* Edge Impulse inferencing library
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "hal/hal.h"
#include "hal/uart_stdout.h"
#include "firmware-sdk-alif/ei_device_info_lib.h"
#include "firmware-sdk-alif/ei_device_memory.h"
#include "firmware-sdk-alif/at-server/ei_at_server.h"
#include "firmware-sdk-alif/at-server/ei_at_command_set.h"
#include "firmware-sdk-alif/ei_at_handlers_lib.h"
#include "firmware-sdk-alif/ei_image_lib.h"
#include "ei_microphone.h"
#include "ei_run_impulse.h"

#include <cstdio>

/* Project Includes */
#include "Driver_PINMUX_AND_PINPAD.h"
#include "Driver_GPIO.h" 

#include "RTE_Device.h"
#include "RTE_Components.h"

#include "hal/bsp_core_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "image_processing.h"


extern ARM_DRIVER_GPIO Driver_GPIO1;

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

void SetupLEDs()
{
	// Green LED
	Driver_GPIO1.Initialize(PIN_NUMBER_15,NULL);
	Driver_GPIO1.PowerControl(PIN_NUMBER_15,  ARM_POWER_FULL);
	Driver_GPIO1.SetDirection(PIN_NUMBER_15, GPIO_PIN_DIRECTION_OUTPUT);
	PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_15, PINMUX_ALTERNATE_FUNCTION_0);
	PINPAD_Config (PORT_NUMBER_1, PIN_NUMBER_15, (0x09 | PAD_FUNCTION_OUTPUT_DRIVE_STRENGTH_04_MILI_AMPS));
	Driver_GPIO1.SetValue(PIN_NUMBER_15, GPIO_PIN_OUTPUT_STATE_LOW);

	// Red LED
	Driver_GPIO1.Initialize(PIN_NUMBER_14,NULL);
	Driver_GPIO1.PowerControl(PIN_NUMBER_14,  ARM_POWER_FULL);
	Driver_GPIO1.SetDirection(PIN_NUMBER_14, GPIO_PIN_DIRECTION_OUTPUT);
	PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_14, PINMUX_ALTERNATE_FUNCTION_0);
	PINPAD_Config (PORT_NUMBER_1, PIN_NUMBER_14, (0x09 | PAD_FUNCTION_OUTPUT_DRIVE_STRENGTH_04_MILI_AMPS));
	Driver_GPIO1.SetValue(PIN_NUMBER_14, GPIO_PIN_OUTPUT_STATE_LOW);
}

extern "C" void ei_sleep_c(int32_t time_ms) { ei_sleep( time_ms ); }
extern "C" int Init_SysTick(void);

int main()
{
    Init_SysTick();

    // System init takes place in Reset function, see startup_M55_HP.c
#if defined(ARM_NPU)
    /* If Arm Ethos-U NPU is to be used, we initialize it here */
    if (0 != arm_npu_init()) {
        ei_printf("Failed to initialize NPU");
    }
#endif /* ARM_NPU */

    if (UartStdOutInit())
    {
        // non zero return on uart init
        while (1)
            ;
    }
    
    // Setup Pin-Mux and Pad Control registers
    SetupLEDs();

	for (int i = 0; i < 3; i++) {
      if (camera_init() != 0) {
        ei_printf("Failed to initialize Camera, retrying\n");
      } else {
          break;
      }
  }

    //setvbuf(stdout, NULL, _IONBF, 0);
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

    while (1)
    {
        // blocking call
        char data = UartGetc();
        if (data != 0xFF)
        {
            at->handle(data);
        }
        else
        {
            ei_printf("UART read error\n");
            break;
        }
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
