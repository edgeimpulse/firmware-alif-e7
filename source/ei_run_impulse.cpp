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
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"
#include "ei_microphone.h"
#include "ei_device_interface.h"

#if defined(EI_CLASSIFIER_SENSOR) && EI_CLASSIFIER_SENSOR == EI_CLASSIFIER_SENSOR_CAMERA
#include "firmware-sdk-alif/ei_image_nn.h"
#include <memory>

void run_nn(bool debug) {

    // size enough for packed RGB888
    const int IMAGE_SIZE = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT * 3;

    uint8* image_p;
    EI_ALLOCATE_AUTO_POINTER(image_p, IMAGE_SIZE);

    if( !image_p ) {
        ei_printf("ERR: run_nn out of memory\n");
        return;
    }

    EiImageNN imageNN(
        image_p,
        IMAGE_SIZE,
        EI_CLASSIFIER_INPUT_WIDTH,
        EI_CLASSIFIER_INPUT_HEIGHT,
        EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,
        EI_CLASSIFIER_LABEL_COUNT);

    imageNN.run_nn(debug, debug? 0:1000, debug);
}


#elif defined(EI_CLASSIFIER_SENSOR) && EI_CLASSIFIER_SENSOR == EI_CLASSIFIER_SENSOR_MICROPHONE
void run_nn(bool debug) {
    if (EI_CLASSIFIER_FREQUENCY != 16000) {
        ei_printf("ERR: Frequency is %d but can only sample at 16000Hz\n", (int)EI_CLASSIFIER_FREQUENCY);
        return;
    }

    bool stop_inferencing = false;

    // summary of inferencing settings (from model_metadata.h)
    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: %.4f ms.\n", (float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

    if(ei_microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT, (float)EI_CLASSIFIER_INTERVAL_MS) == false) {
        ei_printf("ERR: Failed to setup audio sampling\r\n");
        return;
    }

    ei_printf("Starting inferencing, press 'b' to break\n");

    while (stop_inferencing == false) {

        ei_printf("Recording...\n");

        ei_microphone_inference_reset_buffers();
        bool m = ei_microphone_inference_record();
        if (!m) {
            ei_printf("ERR: Failed to record audio...\n");
            break;
        }

        ei_printf("Recording done\n");

        signal_t signal;
        signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
        signal.get_data = &ei_microphone_audio_signal_get_data;
        ei_impulse_result_t result = { 0 };

        EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug);
        if (r != EI_IMPULSE_OK) {
            ei_printf("ERR: Failed to run classifier (%d)\n", r);
            break;
        }

        display_results(&result);

        ei_printf("Starting inferencing in 2 seconds...\n");

        // instead of wait_ms, we'll wait on the signal, this allows threads to cancel us...
        uint64_t end_ms = ei_read_timer_ms() + 2000;
        while(end_ms > ei_read_timer_ms()){
            if(ei_user_invoke_stop_lib()) {
                ei_printf("Inferencing stopped by user\r\n");
                // EiDevice.set_state(eiStateIdle);
                stop_inferencing = true;
                break;
            }
        };
    }

    ei_microphone_inference_end();
}

void run_nn_continuous(bool debug)
{
    if (EI_CLASSIFIER_FREQUENCY != 16000) {
        ei_printf("ERR: Frequency is %d but can only sample at 16000Hz\n", (int)EI_CLASSIFIER_FREQUENCY);
        return;
    }

    bool stop_inferencing = false;
    int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);
    // summary of inferencing settings (from model_metadata.h)
    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: ");
    ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf("ms.\n");
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) /
                                            sizeof(ei_classifier_inferencing_categories[0]));

    ei_printf("Starting inferencing, press 'b' to break\n");

    run_classifier_init();
    ei_microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE, (float)EI_CLASSIFIER_INTERVAL_MS);

    while (stop_inferencing == false) {

        bool m = ei_microphone_inference_record();
        if (!m) {
            ei_printf("ERR: Failed to record audio...\n");
            break;
        }

        signal_t signal;
        signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
        signal.get_data = &ei_microphone_audio_signal_get_data;
        ei_impulse_result_t result = {0};

        EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug);
        if (r != EI_IMPULSE_OK) {
            ei_printf("ERR: Failed to run classifier (%d)\n", r);
            break;
        }

        if (++print_results >= (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW >> 1)) {
            display_results(&result);
            print_results = 0;
        }

        if(ei_user_invoke_stop_lib()) {
            ei_printf("Inferencing stopped by user\r\n");
            break;
        }
    }

    ei_microphone_inference_end();
    run_classifier_deinit();
}

#else

void run_nn(bool debug) {}
#error "EI_CLASSIFIER_SENSOR not configured, cannot configure `run_nn`"

#endif // EI_CLASSIFIER_SENSOR

bool run_nn_normal(void) {
    run_nn(false);
    return true;
}

bool run_nn_debug(const char**, int) {
    run_nn(true);
    return true;
}

bool run_nn_continuous_normal(void) {
#if defined(EI_CLASSIFIER_SENSOR) && EI_CLASSIFIER_SENSOR == EI_CLASSIFIER_SENSOR_MICROPHONE
    run_nn_continuous(false);
#else
    ei_printf("Error no continuous classification available for current model\r\n");
#endif
    return true;
}
