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
