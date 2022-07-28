#ifndef EI_MICROPHONE_H
#define EI_MICROPHONE_H

#include <cstdint>
#include <cstdlib>

typedef int16_t microphone_sample_t;

int ei_microphone_init(void);
bool ei_microphone_sample_record(void);
int ei_microphone_inference_get_data(size_t offset, size_t length, float *out_ptr);
bool ei_microphone_inference_start(uint32_t n_samples, float interval_ms);
bool ei_microphone_inference_record_continuous(void);
bool ei_microphone_inference_record(void);
bool ei_microphone_inference_is_recording(void);
void ei_microphone_inference_reset_buffers(void);
bool ei_microphone_inference_end(void);
int ei_microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr);

#endif /* EI_MICROPHONE_H */