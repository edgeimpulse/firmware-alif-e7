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
        int err = hal_image_init();
        if (0 != err) {
            ei_printf("ERROR: hal_image_init failed with error: %d\n", err);
            return false;
        }
        return true;
    }

    virtual bool ei_camera_capture_rgb888_packed_big_endian(
        uint8_t *image,
        uint32_t image_size) override
    {
        const uint8_t *image_data = hal_get_image_data(this->current_resolution.width, this->current_resolution.height);
        if (!image_data) {
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
