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

#ifndef EI_DEVICE_ALIF_E7
#define EI_DEVICE_ALIF_E7

/* Include ----------------------------------------------------------------- */
#include "firmware-sdk-alif/ei_device_info_lib.h"
#include "firmware-sdk-alif/ei_device_memory.h"

/* Const defines ----------------------------------------------------------- */
#define EI_DEVICE_N_RESOLUTIONS 5

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