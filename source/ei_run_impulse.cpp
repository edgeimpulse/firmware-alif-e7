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

    imageNN.run_nn(debug, debug? 0:1000, false);
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

        // print the predictions
        ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                  result.timing.dsp, result.timing.classification, result.timing.anomaly);
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            ei_printf("    %s: \t%f\r\n", result.classification[ix].label, result.classification[ix].value);
        }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
        ei_printf("    anomaly score: %f\r\n", result.anomaly);
#endif

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
            // print the predictions
            ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                result.timing.dsp, result.timing.classification, result.timing.anomaly);
            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                ei_printf("    %s: \t", result.classification[ix].label);
                ei_printf_float(result.classification[ix].value);
                ei_printf("\r\n");
            }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
            ei_printf("    anomaly score: ");
            ei_printf_float(result.anomaly);
            ei_printf("\r\n");
#endif

            print_results = 0;
        }

        if(ei_user_invoke_stop_lib()) {
            ei_printf("Inferencing stopped by user\r\n");
            break;
        }
    }

    ei_microphone_inference_end();
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
