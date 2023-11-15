/* Edge Impulse ingestion SDK
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

/* Includes ---------------------------------------------------------------- */
#include "ei_device_alif_e7.h"
#include "ei_microphone.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "edge-impulse-sdk/dsp/ei_utils.h"
#include "firmware-sdk-alif/at_base64_lib.h"

#include "uart_tracelib.h"

#include <cstdlib>
#include <cstdio>
#include <cstdint>

using namespace std;

/* Todo: remove global used buffer */
extern microphone_sample_t* mic_sample_buffer;

/** Data Output Baudrate */
const ei_device_data_output_baudrate_t ei_dev_max_data_output_baudrate = {
    ei_xstr(MAX_BAUD),
    MAX_BAUD,
};

const ei_device_data_output_baudrate_t ei_dev_default_data_output_baudrate = {
    ei_xstr(DEFAULT_BAUD),
    DEFAULT_BAUD,
};

static int get_data_output_baudrate_c(ei_device_data_output_baudrate_t *baudrate)
{
    size_t length = strlen(ei_dev_max_data_output_baudrate.str);

    if (length < 32) {
        memcpy(baudrate, &ei_dev_max_data_output_baudrate, sizeof(ei_device_data_output_baudrate_t));
        return 0;
    }
    else {
        return -1;
    }
}

static ei_device_sensor_t sensor_list[] = {
    { 
        .name = "Microphone",
        .frequencies = { 16000.0 },
        .max_sample_length_s = 2,
        .start_sampling_cb = ei_microphone_sample_record
    }
};

bool EiDeviceAlif::get_sensor_list(const ei_device_sensor_t **p_sensor_list, size_t *sensor_list_size)
{
    *p_sensor_list = sensor_list;
    *sensor_list_size = ARRAY_LENGTH(sensor_list);
    return true;
}

bool EiDeviceAlif::read_encode_send_sample_buffer(size_t address, size_t length)
{
    if(mic_sample_buffer) {
        base64_encode((char *)mic_sample_buffer+address, length, ei_putchar);
        return true;
    } else {
        return false;
    }
}


/**
 * @brief      Create resolution list for snapshot setting
 *             The studio and daemon require this list
 * @param      snapshot_list       Place pointer to resolution list
 * @param      snapshot_list_size  Write number of resolutions here
 *
 * @return     False if all went ok
 */
bool EiDeviceAlif::get_snapshot_list(const ei_device_snapshot_resolutions_t **snapshot_list, size_t *snapshot_list_size,
                                         const char **color_depth)
{
    snapshot_resolutions[0].width = 64;
    snapshot_resolutions[0].height = 64;
    snapshot_resolutions[1].width = 96;
    snapshot_resolutions[1].height = 96;
    snapshot_resolutions[2].width = 160;
    snapshot_resolutions[2].height = 160;
    snapshot_resolutions[3].width = 320;
    snapshot_resolutions[3].height = 320;
    snapshot_resolutions[4].width = 480;
    snapshot_resolutions[4].height = 480;

    *snapshot_list      = snapshot_resolutions;
    *snapshot_list_size = EI_DEVICE_N_RESOLUTIONS;
    *color_depth = "RGB";

    return false;
}

/**
 * @brief      Get the data output baudrate
 *
 * @param      baudrate    Baudrate used to output data
 *
 * @return     0
 */
int EiDeviceAlif::get_data_output_baudrate(ei_device_data_output_baudrate_t *baudrate)
{
    return get_data_output_baudrate_c(baudrate);
}

/**
 * @brief      Set output baudrate to max
 *
 */
void EiDeviceAlif::set_max_data_output_baudrate(void)
{
    tracelib_init(NULL, MAX_BAUD);
}

/**
 * @brief      Set output baudrate to default
 *
 */
void EiDeviceAlif::set_default_data_output_baudrate(void)
{
    tracelib_init(NULL, DEFAULT_BAUD);
}

/**
 * @brief get_device is a static method of EiDeviceInfo class
 * It is used to implement singleton paradigm, so we are returning
 * here pointer always to the same object (dev)
 * 
 * @return EiDeviceInfo* 
 */
EiDeviceInfo *EiDeviceInfo::get_device()
{
    static EiDeviceAlif dev;
    return &dev;
}
