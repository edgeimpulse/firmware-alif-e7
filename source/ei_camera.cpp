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

/* Image buffers */
static uint8_t raw_image[CIMAGE_X*CIMAGE_Y*RGB_BYTES + 0x460];
static uint8_t rgb_image[CIMAGE_X*CIMAGE_Y*RGB_BYTES];

class EiCameraAlif : public EiCamera
{
    virtual bool ei_camera_capture_rgb888_packed_big_endian(
        uint8_t *image,
        uint32_t image_size) override
    {
        while(camera_start(CAMERA_MODE_SNAPSHOT, raw_image) != 0);
        camera_vsync(100); // RGB conversion and frame resize 
        bayer_to_RGB(raw_image+0x460, rgb_image);
        ei::image::processing::crop_and_interpolate_rgb888(
            rgb_image,// const uint8_t *srcImage,
            560,// int srcWidth,
            560,// int srcHeight,
            rgb_image,// uint8_t *dstImage,
            96,// int dstWidth,
            96// int dstHeight
        );
        white_balance(rgb_image, image);
        // memcpy(image, rgb_image, image_size);
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
            { { 96,96 } };

        *res = snapshot_resolutions;
        *res_num = ARRAY_LENGTH(snapshot_resolutions);
    }

    virtual ei_device_snapshot_resolutions_t get_min_resolution(void)
    {
        return { 96,96 };
    }

    virtual bool set_resolution(const ei_device_snapshot_resolutions_t res) override
    {
        return true;
    }
};

EiCamera* EiCamera::get_camera()
{
    static EiCameraAlif cam;
    return &cam;
}