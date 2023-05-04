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

#ifndef EI_DEVICE_ALIF_E7
#define EI_DEVICE_ALIF_E7

/* Include ----------------------------------------------------------------- */
#include "firmware-sdk-alif/ei_device_info_lib.h"
#include "firmware-sdk-alif/ei_device_memory.h"

/* Const defines ----------------------------------------------------------- */
#define EI_DEVICE_N_RESOLUTIONS 3

/* Supported baud rates --------------------------------------------------- */
#define DEFAULT_BAUD 115200
#define MAX_BAUD 921600

class EiDeviceAlif : public EiDeviceInfo
{
private:
    ei_device_snapshot_resolutions_t snapshot_resolutions[EI_DEVICE_N_RESOLUTIONS];
public:
    bool get_sensor_list(const ei_device_sensor_t **p_sensor_list, size_t *sensor_list_size);
    bool read_encode_send_sample_buffer(size_t address, size_t length);
    bool get_snapshot_list(const ei_device_snapshot_resolutions_t **resolution_list, size_t *resolution_list_size,
				   const char **color_depth);
    int get_data_output_baudrate(ei_device_data_output_baudrate_t *baudrate);
    void set_max_data_output_baudrate(void);
    void set_default_data_output_baudrate(void);
};

#endif /* EI_DEVICE_ALIF_E7 */