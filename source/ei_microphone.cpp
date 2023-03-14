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
#include "ei_microphone.h"

#include "edge-impulse-sdk/CMSIS/DSP/Include/dsp/support_functions.h"
#include "edge-impulse-sdk/dsp/memory.hpp"
#include "firmware-sdk-alif/ei_microphone_lib.h"
#include "hal.h"

//TODO: use multiply of memory block size
#define MIC_SAMPLE_SIZE 2048

uint32_t wlen = 32;
uint32_t sampling_rate = 16000;
microphone_sample_t* mic_sample_buffer = nullptr;

/** Status and control struct for inferencing struct */
typedef struct {
    int16_t *buffers[2];
    uint8_t buf_select;
    uint8_t buf_ready;
    uint32_t buf_count;
    uint32_t n_samples;
} inference_t;

static inference_t inference;

class EiMicrophoneAlif : public EiMicrophone {
    uint32_t size;
    microphone_sample_t *dst_buffer;

public:
    int async_start(microphone_sample_t *buffer, uint32_t size) override
    {
        this->sample_size = size;
        this->sample_buffer = buffer;
        hal_get_audio_data(buffer, size);
        return 0;
    }

    uint32_t await_samples() override
    {
        int err = hal_wait_for_audio();
        if (err) {
            ei_printf("ERROR: hal_get_audio_data failed with error: %d\n", err);
            return 0;
        }
        hal_audio_preprocessing(this->sample_buffer, this->sample_size);
        return this->sample_size;
    }
private:
    uint32_t sample_size = 0;
    microphone_sample_t *sample_buffer = 0;
};

static EiMicrophoneAlif micAlif;

int ei_microphone_init(void)
{
    int err = hal_audio_init(sampling_rate, wlen);
    return 0;
}

bool ei_microphone_sample_record(void)
{
    auto size = ei_microphone_get_buffer_size();
    if(mic_sample_buffer) {
        ei_free(mic_sample_buffer);
    }
    mic_sample_buffer = (microphone_sample_t*)ei_malloc(size);
    if (!mic_sample_buffer) {
        ei_printf("Failed to allocate memory for audio sampling");
        return false;
    }
    EiDeviceRAM ram;
    ram.assign_memory((uint32_t*)mic_sample_buffer, size);
    // return ei_microphone_sample_record_lib(&mic, &ram);
    return ei_microphone_sample_record_lib(&micAlif, &ram);
}

static void inference_samples_callback(const int16_t *buffer, uint32_t sample_count)
{
    for (uint32_t i = 0; i < sample_count; i++) {
        if (inference.buf_count >= inference.n_samples) {
            inference.buf_select ^= 1;
            inference.buf_count = 0;
            inference.buf_ready = 1;
        }
    }
}

bool ei_microphone_inference_start(uint32_t n_samples, float interval_ms)
{
    // free up the sampling buffer
    if(mic_sample_buffer) {
        ei_free(mic_sample_buffer);
        mic_sample_buffer = nullptr;
    }

    inference.buffers[0] = (int16_t *)ei_malloc(n_samples * sizeof(microphone_sample_t));
    if (inference.buffers[0] == NULL) {
        return false;
    }

    inference.buffers[1] = (int16_t *)ei_malloc(n_samples * sizeof(microphone_sample_t));
    if (inference.buffers[1] == NULL) {
        delete inference.buffers[0];
        return false;
    }

    inference.buf_select = 0;
    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    // micAlif.async_start(inference.buffers[inference.buf_select], inference.n_samples);

    return true;
}

bool ei_microphone_inference_record_continuous(void)
{
    inference.buf_ready = 0;
    inference.buf_count = 0;

    micAlif.await_samples();

    inference.buf_select ^= 1;
    inference.buf_ready = 0;

    micAlif.async_start(inference.buffers[inference.buf_select], inference.n_samples);

    return true;
}

bool ei_microphone_inference_record(void)
{
    inference.buf_ready = 0;
    inference.buf_count = 0;

    micAlif.async_start(inference.buffers[inference.buf_select], inference.n_samples);
    micAlif.await_samples();

    // record_ready = false;
    inference.buf_select ^= 1;
    inference.buf_ready = 0;

    return true;
}

bool ei_microphone_inference_is_recording(void)
{
    return inference.buf_ready == 0;
}

void ei_microphone_inference_reset_buffers(void)
{
    inference.buf_ready = 0;
    inference.buf_count = 0;
}

bool ei_microphone_inference_end(void)
{
    int32_t status;

    ei_free(inference.buffers[0]);
    ei_free(inference.buffers[1]);

    return true;
}

/**
 * Get raw audio signal data
 */
int ei_microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    arm_q15_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);

    return 0;
}
