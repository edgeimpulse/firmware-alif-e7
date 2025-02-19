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

/* Include ----------------------------------------------------------------- */
#include "firmware-sdk-alif/ei_camera_interface.h"
#include "edge-impulse-sdk/dsp/image/processing.hpp"
#include "image_processing.h"
#include "hal.h"
#include "hal_image.h"
#include "delay.h"
#include <cstring>


class EiCameraAlif : public EiCamera
{

    virtual bool init(uint16_t width, uint16_t height) override
    {
        static bool camera_ready = false;

        if(camera_ready == false) {
            int err = hal_image_init();
            if (0 != err) {
                ei_printf("ERROR: hal_image_init failed with error: %d\n", err);
                return false;
            }
            camera_ready = true;
        }
        ei_device_snapshot_resolutions_t rq_res;

        rq_res.width = width;
        rq_res.height = height;
        this->set_resolution(rq_res);

        return true;
    }

    virtual bool ei_camera_capture_rgb888_packed_big_endian(
        uint8_t **image,
        uint32_t image_size) override
    {
        *image = (uint8_t *) hal_get_image_data(this->current_resolution.width, this->current_resolution.height);
        if (!image) {
            ei_printf("ERROR: hal_get_image_data failed");
            return false;
        }
        return true;
    }

    /**
     * @brief Get the list of supported resolutions, ie. not requiring
     * any software processing like crop or resize
     * 
     * @param res pointer to store the list of resolutions
     * @param res_num pointer to a variable that will contain size of the res list
     */
    virtual void get_resolutions(const ei_device_snapshot_resolutions_t **res, uint8_t *res_num) override
    {

        static ei_device_snapshot_resolutions_t snapshot_resolutions[] =
            { { 96,96 }, { 160,160 }, {320,320}, {480, 480} };

        *res = snapshot_resolutions;
        *res_num = ARRAY_LENGTH(snapshot_resolutions);
    }

    virtual ei_device_snapshot_resolutions_t get_min_resolution(void)
    {
        return { 96,96 };
    }

    virtual bool set_resolution(const ei_device_snapshot_resolutions_t res) override
    {
        this->current_resolution = res;
        return true;
    }

    private:
        ei_device_snapshot_resolutions_t current_resolution;
};

EiCamera* EiCamera::get_camera()
{
    static EiCameraAlif cam;
    return &cam;
}
