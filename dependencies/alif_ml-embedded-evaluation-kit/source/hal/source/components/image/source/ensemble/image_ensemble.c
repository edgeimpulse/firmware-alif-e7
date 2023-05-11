/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement 
 *
 * You should have received a copy of the Alif Semiconductor Software 
 * License Agreement with this file. If not, please write to: 
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "RTE_Components.h"

#include "log_macros.h"
#include "image_data.h"
#include "image_processing.h"
#include "Driver_CPI.h"
#include "Driver_PINMUX_AND_PINPAD.h"
#include "Driver_GPIO.h"
#include "base_def.h"
#include "delay.h"
#include <tgmath.h>
#include <string.h>

static uint8_t rgb_image[CIMAGE_X*CIMAGE_Y*RGB_BYTES] __attribute__((section(".bss.camera_frame_bayer_to_rgb_buf")));      // 560x560x3 = 940,800
static uint8_t raw_image[CIMAGE_X*CIMAGE_Y*RGB_BYTES + 0x460] __attribute__((aligned(32),section(".bss.camera_frame_buf")));   // 560x560x3 = 940,800

extern ARM_DRIVER_GPIO Driver_GPIO1;

int image_init()
{
    DEBUG_PRINTF("image_init(IN)\n");
    int err = camera_init(raw_image);
    DEBUG_PRINTF("image_init(), camera_init: %d\n", err);
	if (err != 0) {
		while(1) {
			Driver_GPIO1.SetValue(PIN_NUMBER_14, GPIO_PIN_OUTPUT_STATE_LOW);
			sleep_or_wait_msec(300);
			Driver_GPIO1.SetValue(PIN_NUMBER_14, GPIO_PIN_OUTPUT_STATE_HIGH);
			sleep_or_wait_msec(300);
		}
	}
	DEBUG_PRINTF("Camera initialized... \n");
	Driver_GPIO1.SetValue(PIN_NUMBER_14, GPIO_PIN_OUTPUT_STATE_HIGH);

    return err;
}

static float current_log_gain = 0.0;
static int32_t current_api_gain = 0;
static int32_t last_requested_api_gain = 0;
static float minimum_log_gain = -INFINITY;
static float maximum_log_gain = +INFINITY;

float get_image_gain(void)
{
    return current_api_gain * 0x1p-16f;
}

static float api_gain_to_log(int32_t api)
{
    return logf(api * 0x1p-16f);
}

static int32_t log_gain_to_api(float gain)
{
    return expf(gain) * 0x1p16f;
}

static void process_autogain(void)
{
    /* Simple "auto-exposure" algorithm. (Although we're actually just
     * adjusting gain at the moment...)
     *
     * We us a discrete velocity form of a PI controller to adjust the
     * current gain to try to make the difference between high pixels
     * and low pixels hit a target.
     *
     * The definition of "low" and "high" pixels has quite an effect on the
     * end result - this is set over in bayer2rgb.c, which does the analysis.
     */

    const float target_highlow_difference = 0.0f;

    // Control constants - output is to the logarithm of gain (as if
    // we're working in decibels), so 1 here would mean a factor of e
    // adjustment if every pixel was high.
    const float Kp = 1.0f * 0.45f;
    const float Ki = 1.0f * 0.54f / 2.0f;
    const float tiny_error = 0x1p-4f;

    static float previous_error = 0;

    if (current_api_gain <= 0) {
        if (current_api_gain < 0) {
            return;
        }
        current_api_gain = camera_gain(0); // Read initial gain
        if (current_api_gain < 0) {
            printf_err("camera_gain(0) returned error %" PRId32 "; disabling autogain\n", current_api_gain);
            return;
        }
        current_log_gain = api_gain_to_log(current_api_gain);
    }

    /* Rescale high-low difference in pixel counts so that it's
     * in range [-1..+1], regardless of image size */
    float high_proportion = (float) exposure_high_count * (1.0f / (CIMAGE_X * CIMAGE_Y));
    float low_proportion = (float) exposure_low_count * (1.0f / (CIMAGE_X * CIMAGE_Y));
    float highlow_difference = high_proportion - low_proportion;
    float error = highlow_difference - target_highlow_difference;

    /* Ignore small errors, so we don't oscillate */
    if (fabsf(error) < tiny_error * fmaxf(high_proportion, low_proportion)) {
        error = 0;
    }

    float delta_controller_output = (Kp + Ki) * error - Kp * previous_error;

    previous_error = error;
    current_log_gain = current_log_gain - delta_controller_output;

    /* Clamp according to limits we've found from the gain API */
    current_log_gain = fminf(current_log_gain, maximum_log_gain);
    current_log_gain = fmaxf(current_log_gain, minimum_log_gain);

    int32_t desired_api_gain = log_gain_to_api(current_log_gain);

    /* Apply the gain, if it's a new request */
    if (desired_api_gain != last_requested_api_gain) {
        int32_t ret = camera_gain(desired_api_gain);
        if (ret < 0) {
            printf_err("Camera gain error %" PRId32 "\n", ret);
            return;
        }
        last_requested_api_gain = desired_api_gain;
        current_api_gain = ret;
        debug("Camera gain changed to %.3f\n", current_api_gain * 0x1p-16f);

        /* Check for saturation, and record it. Knowing our limits avoids
         * making pointless calls to the gain changing API.
         */
        float deviation = ((float) current_api_gain - desired_api_gain) / desired_api_gain;
        if (fabsf(deviation) > 0.25f) {
            if (deviation < 0) {
                maximum_log_gain = api_gain_to_log(current_api_gain);
                debug("Noted maximum gain %.3f\n", current_api_gain * 0x1p-16f);
            } else {
                minimum_log_gain = api_gain_to_log(current_api_gain);
                debug("Noted minimum gain %.3f\n", current_api_gain * 0x1p-16f);
            }
            current_log_gain = api_gain_to_log(current_api_gain);
        }
    }
}

#define FAKE_CAMERA 0

const uint8_t *get_image_data(int ml_width, int ml_height)
{
    extern uint32_t tprof1, tprof2, tprof3, tprof4, tprof5;

#if !FAKE_CAMERA
    camera_start(CAMERA_MODE_SNAPSHOT);
    // It's a huge buffer (941920 bytes) - actually doing it by address can take 0.5ms, while
    // a global clean+invalidate is 0.023ms. (Although there will be a reload cost
    // on stuff we lost).
    // Notably, just invalidate is faster at 0.015ms, but we'd have to be sure
    // there were no writeback cacheable areas.
    // From that Breakeven point for ranged invalidate time = global clean+invalidate would be 43Kbyte.
    // So maybe go to global if >128K, considering cost of refills?
    //SCB_InvalidateDCache_by_Addr(raw_image, sizeof raw_image);
    SCB_CleanInvalidateDCache();
    camera_wait(100);
#else
    static int roll = 0;
    for (int y = 0; y < CIMAGE_Y; y+=2) {
    	uint8_t *p = raw_image + y * CIMAGE_X;
    	int bar = (7 * ((y+roll) % CIMAGE_Y)) / CIMAGE_Y + 1;
    	float barb = bar & 1 ? 255 : 0;
    	float barr = bar & 2 ? 255 : 0;
    	float barg = bar & 4 ? 255 : 0;
    	for (int x = 0; x < CIMAGE_X; x+=2) {
    		float intensity = x * (1.0f/(CIMAGE_X-2));
    		float r = barr * intensity + 0.5f;
    		float g = barg * intensity + 0.5f;
    		float b = barb * intensity + 0.5f;
    		p[0]        = b; p[1]            = g;
    		p[CIMAGE_X] = g; p[CIMAGE_X + 1] = r;
    		p += 2;
    	}
    }
    roll = (roll + 1) % CIMAGE_Y;
#endif
    tprof1 = ARM_PMU_Get_CCNTR();
    // RGB conversion and frame resize
    bayer_to_RGB(raw_image, rgb_image);
    tprof1 = ARM_PMU_Get_CCNTR() - tprof1;
    // Use pixel analysis from bayer_to_RGB to adjust gain
    process_autogain();
    // Cropping and scaling
    crop_and_interpolate(rgb_image, CIMAGE_X, CIMAGE_Y, raw_image, ml_width, ml_height, RGB_BYTES * 8);
    tprof4 = ARM_PMU_Get_CCNTR();
    // Color correction for white balance
    white_balance(ml_width, ml_height, raw_image, rgb_image);
    tprof4 = ARM_PMU_Get_CCNTR() - tprof4;
    return rgb_image;
}
